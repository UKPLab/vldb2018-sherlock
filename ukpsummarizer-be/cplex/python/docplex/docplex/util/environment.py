# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2016
# --------------------------------------------------------------------------

'''
Representation of the DOcplex solving environment.

This module handles the various elements that allow an
optimization program to run independently from the solving environment.
This environment may be:

 * on premise, using a local version of CPLEX Optimization Studio to solve MP problems, or
 * on DOcplexcloud, with the Python program running inside the Python Worker.

As much as possible, the adaptation to the solving environment is
automatic. The functions that are presented here are useful for handling
very specific use cases.

The following code is a program that sums its input (``sum.py``)::

    import json
    import docplex.util.environment as environment

    sum = 0
    # open program input named "data.txt" and sum the contents
    with environment.get_input_stream("data.txt") as input:
        for i in input.read().split():
            sum += int(i)
    # write the result as a simple json in program output "solution.json"
    with environment.get_output_stream("solution.json") as output:
        output.write(json.dumps({'result': sum}))

Let's put some data in a ``data.txt`` file::

    4 7 8
    19

When you run ``sum.py`` with a Python interpreter, it opens the ``data.txt`` file and sums all of the integers
in that file. The result is saved as a JSON fragment in file ``solution.json``::

    $ python sum.py
    $ more solution.json
    {"result": 38}

To submit the program to the DOcplexcloud service, we write a ``submit.py`` program that uses
the `DOcplexcloud Python API <https://developer.ibm.com/docloud/documentation/docloud/python-api/>`_
to create and submit a job. That job has two attachments:

- ``sum.py``, the program to execute and
- ``data.txt``, the data expected by ``sum.py``.

After the solve is completed, the result of the program is downloaded and saved as ``solution.json``::

    from docloud.job import JobClient

    url = "ENTER_YOUR_URL_HERE"
    key = "ENTER_YOUR_KEY_HERE"
    client = JobClient(url, key)
    client.execute(input=["sum.py", "data.txt"], output="solution.json")

Then you run ``submit.py``::

    $ python submit.py
    $ more solution.json
    {"result": 38}

Environment representation can be accessed with different ways:

    * direct object method calls, after retrieving an instance using 
      :meth:`docplex.util.environment.get_environment` and using methods of
      :class:`docplex.util.environment.Environment`.
    * using the function in package `docplex.util.environment`. They will call
       the corresponding methods of Environment in the platform
       `default_environment`:

           * :meth:`docplex.util.environment.get_input_stream`
           * :meth:`docplex.util.environment.get_output_stream`
           * :meth:`docplex.util.environment.read_df`
           * :meth:`docplex.util.environment.write_df`
           * :meth:`docplex.util.environment.get_available_core_count`
           * :meth:`docplex.util.environment.get_parameter`
           * :meth:`docplex.util.environment.update_solve_details`
           * :meth:`docplex.util.environment.add_abort_callback`
           * :meth:`docplex.util.environment.remove_abort_callback`

'''
import json
from functools import partial
import os
import tempfile
import threading
import warnings

try:
    import pandas
except ImportError:
    pandas = None

from six import iteritems


class NotAvailableError(Exception):
    ''' The exception raised when a feature is not available
    '''
    pass


def default_solution_storage_handler(env, solution):
    ''' The default solution storage handler.

    The storage handler is a function which first argument is the
    :class:`~Environment` on which a solution should be saved. The `solution`
    is a dict containing all the data for an optimization solution.

    The storage handler is responsible for storing the solution in the
    environment.

    For each (key, value) pairs of the solution, the default solution storage
    handler does the following depending of the type of `value`, in the
    following order:

        * If `value` is a `pandas.DataFrame`, then the data frame is saved
          as an output with the specified `name`. Note that `name` must include
          an extension file for the serialization. See
          :meth:`Environment.write_df` for supported formats.
        * If `value` is a `bytes`, it is saved as binary data with `name`.
        * The `value` is saved as an output with the `name`, after it has been
          converted to JSON.

    Args:
        env: The :class:`~Environment`
        solution: a dict containing the solution.
    '''
    for (name, value) in iteritems(solution):
        if pandas and isinstance(value, pandas.DataFrame):
            _, ext = os.path.splitext(name)
            if ext.lower() == '':
                name = '%s.csv' % name  # defaults to csv if no format specified
            env.write_df(value, name)
        elif isinstance(value, bytes):
            with env.get_output_stream(name) as fp:
                fp.write(value)
        else:
            # try jsonify
            with env.get_output_stream(name) as fp:
                json.dump(value, fp)


class Environment(object):
    ''' Methods for interacting with the execution environment.

    Internally, the ``docplex`` package provides the appropriate implementation
    according to the actual execution environment.
    The correct instance of this class is returned by the method
    :meth:`docplex.util.environment.get_environment` that is provided in this
    module.

    Attributes:
        abort_callbacks: A list of callbacks that are called when the script is
            run on DOcplexcloud and a job abort operation is requested. You
            add your own callback using::

                env.abort_callbacks += [your_cb]

            or::

                env.abort_callbacks.append(your_cb)

            You remove a callback using::

                env.abort_callbacks.remove(your_cb)

        solution_storage_handler: A function called when a solution is to be
            stored. The storage handler is a function which first argument is
            the :class:`~Environment` on which a solution should be saved. The
            `solution` is a dict containing all the data for an optimization
            solution. The default is :meth:`~default_solution_storage_handler`.
        is_dods: True if the environment is Decision Optimization for Data Science.
    '''
    def __init__(self):
        self.output_lock = threading.Lock()
        self.solution_storage_handler = default_solution_storage_handler
        self.abort_callbacks = []
        self.is_dods = False

    def store_solution(self, solution):
        with self.output_lock:
            self.solution_storage_handler(self, solution)

    def get_input_stream(self, name):
        ''' Get an input of the program as a stream (file-like object).

        An input of the program is a file that is available in the working directory.

        When run on DOcplexcloud, all input attachments are copied to the working directory before
        the program is run. ``get_input_stream`` lets you open the input attachments of the job.

        Args:
            name: Name of the input object.
        Returns:
            A file object to read the input from.
        '''
        return None

    def read_df(self, name, reader=None, **kwargs):
        ''' Reads an input of the program as a ``pandas.DataFrame``.

        ``pandas`` must be installed.

        ``name`` is the name of the input object, as a filename. If a reader
        is not user provided, the reader used depends on the filename extension.

        The default reader used depending on extension are:

            * ``.csv``: ``pandas.read_csv()``
            * ``.msg``: ``pandas.read_msgpack()``

        Args:
            name: The name of the input object
            reader: an optional reader function
            **kwargs: additional parameters passed to the reader
        Raises:
            NotAvailableError: raises this error when ``pandas`` is not
                available.
        '''
        if pandas is None:
            raise NotAvailableError('read_df() is only available if pandas is installed')
        _, ext = os.path.splitext(name)
        default_kwargs = None
        if reader is None:
            default_readers = {'.csv': (pandas.read_csv, {'index_col': 0}),
                               '.msg': (pandas.read_msgpack, None)}
            reader, default_kwargs = default_readers.get(ext.lower(), None)
        if reader is None:
            raise ValueError('no default reader defined for files with extension: \'%s\'' % ext)
        with self.get_input_stream(name) as ost:
            # allow
            params = {}
            if default_kwargs:
                params.update(default_kwargs)
            if kwargs:
                params.update(kwargs)
            return reader(ost, **params)

    def write_df(self, df, name, writer=None, **kwargs):
        ''' Write a ``pandas.DataFrame`` as an output of the program.

        ``pandas`` must be installed.

        ``name`` is the name of the input object, as a filename. If a writer
        is not user provided, the writer used depends on the filename extension.

        The default writer used depending on extension are:

            * ``.csv``: ``DataFrame.to_csv()``
            * ``.msg``: ``DataFrame.to_msgpack()``

        Args:
            name: The name of the input object
            writer: an optional writer function
            **kwargs: additional parameters passed to the writer
        Raises:
            NotAvailableError: raises this error when ``pandas`` is not
                available.
        '''
        if pandas is None:
            raise NotAvailableError('write_df() is only available if pandas is installed')
        _, ext = os.path.splitext(name)
        if writer is None:
            try:
                default_writers = {'.csv': df.to_csv,
                                   '.msg': df.to_msgpack}
                writer = default_writers.get(ext.lower(), None)
            except AttributeError:
                raise NotAvailableError('Could not write writer function for extension: %s' % ext)
        if writer is None:
            raise ValueError('no default writer defined for files with extension: \'%s\'' % ext)
        with self.get_output_stream(name) as ost:
            writer(ost, **kwargs)

    def get_output_stream(self, name):
        ''' Get a file-like object to write the output of the program.

        The file is recorded as being part of the program output.
        This method can be called multiple times if the program contains
        multiple output objects.

        When run on premise, the output of the program is written as files in
        the working directory. When run on DOcplexcloud, the files are attached
        as output attachments.

        The stream is opened in binary mode, and will accept 8 bits data.

        Args:
            name: Name of the output object.
        Returns:
            A file object to write the output to.
        '''
        return None

    def get_available_core_count(self):
        ''' Returns the number of cores available for processing if the environment
        sets a limit.

        This number is used in the solving engine as the number of threads.

        Returns:
            The available number of cores or ``None`` if the environment does not
            limit the number of cores.
        '''
        return None

    def get_parameter(self, name):
        ''' Returns a parameter of the program.

        On DOcplexcloud, this method returns the job parameter whose name is specified.

        Args:
            name: The name of the parameter.
        Returns:
            The parameter whose name is specified or None if the parameter does
            not exists.
        '''
        return None

    def notify_start_solve(self, solve_details):
        # ===============================================================================
        #         '''Notify the solving environment that a solve is starting.
        #
        #         If ``context.solver.auto_publish.solve_details`` is set, the underlying solver will automatically
        #         send details. If you want to craft and send your own solve details, you can use the following
        #         keys (non exhaustive list):
        #
        #             - MODEL_DETAIL_TYPE : Model type
        #             - MODEL_DETAIL_CONTINUOUS_VARS : Number of continuous variables
        #             - MODEL_DETAIL_INTEGER_VARS : Number of integer variables
        #             - MODEL_DETAIL_BOOLEAN_VARS : Number of boolean variables
        #             - MODEL_DETAIL_INTERVAL_VARS : Number of interval variables
        #             - MODEL_DETAIL_SEQUENCE_VARS : Number of sequence variables
        #             - MODEL_DETAIL_NON_ZEROS : Number of non zero variables
        #             - MODEL_DETAIL_CONSTRAINTS : Number of constraints
        #             - MODEL_DETAIL_LINEAR_CONSTRAINTS : Number of linear constraints
        #             - MODEL_DETAIL_QUADRATIC_CONSTRAINTS : Number of quadratic constraints
        #
        #         Args:
        #             solve_details: A ``dict`` with solve details as key/value pairs
        #         See:
        #             :attr:`.Context.solver.auto_publish.solve_details`
        #         '''
        # ===============================================================================
        pass

    def update_solve_details(self, details):
        '''Update the solve details.

        You use this method to send solve details to the DOcplexcloud service.
        If ``context.solver.auto_publish`` is set, the underlying
        solver will automatically update solve details once the solve has
        finished.

        Args:
            details: A ``dict`` with solve details as key/value pairs.
        '''
        pass

    def notify_end_solve(self, status):
        # ===============================================================================
        #         '''Notify the solving environment that the solve as ended.
        #
        #         The ``status`` can be a docloud.status.JobSolveStatus enum or an integer.
        #
        #         When ``status`` is an integer, it is converted with the following conversion table:
        #
        #             0 - UNKNOWN: The algorithm has no information about the solution.
        #             1 - FEASIBLE_SOLUTION: The algorithm found a feasible solution.
        #             2 - OPTIMAL_SOLUTION: The algorithm found an optimal solution.
        #             3 - INFEASIBLE_SOLUTION: The algorithm proved that the model is infeasible.
        #             4 - UNBOUNDED_SOLUTION: The algorithm proved the model unbounded.
        #             5 - INFEASIBLE_OR_UNBOUNDED_SOLUTION: The model is infeasible or unbounded.
        #
        #         Args:
        #             status: The solve status
        #         '''
        # ===============================================================================
        pass

    def set_stop_callback(self, cb):
        '''Sets a callback that is called when the script is run on
        DOcplexcloud and a job abort operation is requested.

        You can also use the ``stop_callback`` property to set the callback.

        Deprecated since 2.4 - Use self.abort_callbacks += [cb] instead'

        Args:
            cb: The callback function
        '''
        warnings.warn('set_stop_callback() is deprecated since 2.4 - Use Environment.abort_callbacks.append(cb) instead')

    def get_stop_callback(self):
        '''Returns the stop callback that is called when the script is run on
        DOcplexcloud and a job abort operation is requested.

        You can also use the ``stop_callback`` property to get the callback.

        Deprecated since 2.4 - Use the abort_callbacks property instead')

        '''
        warnings.warn('get_stop_callback() is deprecated since 2.4 - Use the abort_callbacks property instead')
        return None

    stop_callback = property(get_stop_callback, set_stop_callback)


class LocalEnvironment(Environment):
    # The environment solving environment using all local input and outputs.
    def __init__(self):
        super(LocalEnvironment, self).__init__()

    def get_input_stream(self, name):
        return open(name, "rb")

    def get_output_stream(self, name):
        return open(name, "wb")


class OutputFileWrapper(object):
    # Wraps a file object so that on __exit__() and on close(), the wrapped file is closed and
    # the output attachments are actually set in the worker
    def __init__(self, file, solve_hook, attachment_name):
        self.file = file
        self.solve_hook = solve_hook
        self.attachment_name = attachment_name
        self.closed = False

    def __getattr__(self, name):
        if name == 'close':
            return self.my_close
        else:
            return getattr(self.file, name)

    def __enter__(self, *args, **kwargs):
        return self.file.__enter__(*args, **kwargs)

    def __exit__(self, *args, **kwargs):
        self.file.__exit__(*args, **kwargs)
        self.close()

    def close(self):
        # actually close the output then set attachment
        if not self.closed:
            self.file.close()
            self.solve_hook.set_output_attachments({self.attachment_name: self.file.name})
            self.closed = True


def worker_env_stop_callback(env):
    # wait for the output lock to be released to make sure that the latest
    # solution store operation has ended.
    with env.output_lock:
        pass
    # call all abort callbacks
    for cb in env.abort_callbacks:
        cb()


class WorkerEnvironment(Environment):
    # The solving environment when we run in the DOcplexCloud worker.
    def __init__(self, solve_hook):
        super(WorkerEnvironment, self).__init__()
        self.solve_hook = solve_hook
        self.solve_hook.stop_callback = partial(worker_env_stop_callback, self)

    def get_available_core_count(self):
        return self.solve_hook.get_available_core_count()

    def get_input_stream(self, name):
        # inputs are in the current working directory
        return open(name, "rb")

    def get_output_stream(self, name):
        # open the output in a place we know we can write
        f = tempfile.NamedTemporaryFile(mode="w+b", delete=False)
        return OutputFileWrapper(f, self.solve_hook, name)

    def get_parameter(self, name):
        return self.solve_hook.get_parameter_value(name)

    def update_solve_details(self, details):
        self.solve_hook.update_solve_details(details)

    def notify_start_solve(self, solve_details):
        self.solve_hook.notify_start_solve(None,  # model
                                           solve_details)

    def notify_end_solve(self, status):
        try:
            from docloud.status import JobSolveStatus
            engine_status = JobSolveStatus(status)
            self.solve_hook.notify_end_solve(None,  # model, unused
                                             None,  # has_solution, unused
                                             engine_status,
                                             None,  # reported_obj, unused
                                             None,  # var_value_dict, unused
                                             )
        except ImportError:
            raise RuntimeError("This should have been called only when in a worker environment")

    def set_stop_callback(self, cb):
        warnings.warn('set_stop_callback() is deprecated since 2.4 - Use Environment.abort_callbacks.append(cb) instead')
        self.abort_callbacks += [cb]

    def get_stop_callback(self):
        warnings.warn('get_stop_callback() is deprecated since 2.4 - Use the abort_callbacks property instead')
        return self.abort_callbacks[1] if self.abort_callbacks else None


def _get_default_environment():
    # creates a new instance of the default environment
    try:
        import docplex.worker.solvehook as worker_env
        hook = worker_env.get_solve_hook()
        if hook:
            return WorkerEnvironment(hook)
    except ImportError:
        pass
    return LocalEnvironment()

default_environment = _get_default_environment()


def get_environment():
    ''' Returns the Environment object that represents the actual execution
    environment.

    Note: the default environment is the value of the
    ``docplex.util.environment.default_environment`` property.

    Returns:
        An instance of the :class:`.Environment` class that implements methods
        corresponding to actual execution environment.
    '''
    return default_environment


def get_input_stream(name):
    ''' Get an input of the program as a stream (file-like object),
    with the default environment.

    An input of the program is a file that is available in the working directory.

    When run on DOcplexcloud, all input attachments are copied to the working directory before
    the program is run. ``get_input_stream`` lets you open the input attachments of the job.

    Args:
        name: Name of the input object.
    Returns:
        A file object to read the input from.
    '''
    return default_environment.get_input_stream(name)


def get_output_stream(name):
    ''' Get a file-like object to write the output of the program.

    The file is recorded as being part of the program output.
    This method can be called multiple times if the program contains
    multiple output objects.

    When run on premise, the output of the program is written as files in
    the working directory. When run on DOcplexcloud, the files are attached
    as output attachments.

    The stream is opened in binary mode, and will accept 8 bits data.

    Args:
        name: Name of the output object.
    Returns:
        A file object to write the output to.
    '''
    return default_environment.get_output_stream(name)


def read_df(name, reader=None, **kwargs):
    ''' Reads an input of the program as a ``pandas.DataFrame`` with the
    default environment.

    ``pandas`` must be installed.

    ``name`` is the name of the input object, as a filename. If a reader
    is not user provided, the reader used depends on the filename extension.

    The default reader used depending on extension are:

        * ``.csv``: ``pandas.read_csv()``
        * ``.msg``: ``pandas.read_msgpack()``

    Args:
        name: The name of the input object
        reader: an optional reader function
        **kwargs: additional parameters passed to the reader
    Raises:
        NotAvailableError: raises this error when ``pandas`` is not
            available.
    '''
    return default_environment.read_df(name, reader=reader, **kwargs)


def write_df(df, name, writer=None, **kwargs):
    ''' Write a ``pandas.DataFrame`` as an output of the program with the
    default environment.

    ``pandas`` must be installed.

    ``name`` is the name of the input object, as a filename. If a writer
    is not user provided, the writer used depends on the filename extension.

    The default writer used depending on extension are:

        * ``.csv``: ``DataFrame.to_csv()``
        * ``.msg``: ``DataFrame.to_msgpack()``

    Args:
        name: The name of the input object
        writer: an optional writer function
        **kwargs: additional parameters passed to the writer
    Raises:
        NotAvailableError: raises this error when ``pandas`` is not
            available.
    '''
    return default_environment.write_df(df, name, writer=writer, **kwargs)


def get_available_core_count():
    ''' Returns the number of cores available for processing if the environment
    sets a limit, with the default environment.

    This number is used in the solving engine as the number of threads.

    Returns:
        The available number of cores or ``None`` if the environment does not
        limit the number of cores.
    '''
    return default_environment.get_available_core_count()


def get_parameter(name):
    ''' Returns a parameter of the program, with the default environment.

    On DOcplexcloud, this method returns the job parameter whose name is specified.

    Args:
        name: The name of the parameter.
    Returns:
        The parameter whose name is specified.
    '''
    return default_environment.get_parameter(name)


def update_solve_details(details):
    '''Update the solve details, with the default environment

    You use this method to send solve details to the DOcplexcloud service.
    If ``context.solver.auto_publish`` is set, the underlying
    solver will automatically update solve details once the solve has
    finished.

    Args:
        details: A ``dict`` with solve details as key/value pairs.
    '''
    return default_environment.update_solve_details(details)


def add_abort_callback(cb):
    '''Adds the specified callback to the default environment.

    The abort callback is called when the script is run on
    DOcplexcloud and a job abort operation is requested.

    Args:
        cb: The abort callback
    '''
    default_environment.abort_callbacks += [cb]


def remove_abort_callback(cb):
    '''Adds the specified callback to the default environment.

    The abort callback is called when the script is run on
    DOcplexcloud and a job abort operation is requested.

    Args:
        cb: The abort callback
    '''
    default_environment.abort_callbacks.remove(cb)