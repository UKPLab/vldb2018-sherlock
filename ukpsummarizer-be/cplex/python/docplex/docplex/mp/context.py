# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

import os

from copy import deepcopy
import six
from six import iteritems
import socket
import sys
import warnings
from os.path import isfile, isabs

from docplex.util.environment import get_environment

from docplex.mp.utils import is_string, open_universal_newline
from docplex.mp.environment import Environment
from docplex.mp.params.cplex_params import get_params_from_cplex_version
from docplex.mp.params.parameters import RootParameterGroup
from docplex.mp.utils import DOcplexException
from docplex.mp.error_handler import docplex_fatal

try:
    from docplex.worker.solvehook import get_solve_hook
except ImportError:
    get_solve_hook = None

# some utility methods
def _get_value_as_int(d, option):
    try:
        value = int(d[option])
    except Exception:
        value = None
    return value


def _convert_to_int(value):
    if str(value).lower() == 'none':
        return None
    try:
        value = int(value)
    except Exception:
        value = None
    return value


def _get_value_as_string(d, option):
    return d.get(option, None)


def _get_value_as_boolean(d, option):
    try:
        value = _convert_to_bool(d[option])
    except Exception:
        value = None
    return value

_BOOLEAN_STATES = {'1': True, 'yes': True, 'true': True, 'on': True,
                   '0': False, 'no': False, 'false': False, 'off': False}


def _convert_to_bool(value):
    if value is None:
        return None
    svalue = str(value).lower()
    if svalue == "none":
        return None
    if svalue not in _BOOLEAN_STATES:
        raise ValueError('Not a boolean: %s' % value)
    return _BOOLEAN_STATES[svalue]


class open_filename_universal(object):
    def __init__(self, filename, *args, **kwargs):
        self.closing = kwargs.pop('closing', False)
        if isinstance(filename, six.string_types):
            self.fh = open_universal_newline(filename, "r")
            self.closing = True
        else:
            self.fh = filename

    def __enter__(self):
        return self.fh

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.closing:
            self.fh.close()
        return False


def is_auto_publishing(context):
    try:
        return context.solver.auto_publish == True
    except AttributeError:
        return False


def is_auto_publishing_solve_details(context):
    if get_solve_hook == None:
        return False  # not in a worker
    try:
        auto_publish_details = context.solver.auto_publish.solve_details
    except AttributeError:
        try:
            auto_publish_details = context.solver.auto_publish
        except AttributeError:
            auto_publish_details = False
    return auto_publish_details


def is_auto_publishing_json_solution(context):
    if get_solve_hook == None:
        return False  # not in a worker
    try:
        auto_publish = context.solver.auto_publish.json_solution
    except AttributeError:
        try:
            auto_publish = context.solver.auto_publish
        except AttributeError:
            auto_publish = False
    return auto_publish


def check_credentials(context):
    #     Checks if the context has syntactically valid credentials. The context
    #     has valid credentials when it has an `url` and a `key` fields and that
    #     both fields are string.
    # 
    #     If the credentials are not defined, `message` contains a message describing
    #     the cause.
    # 
    #     Returns:
    #         (has_credentials, message): has_credentials` - True if the context contains syntactical credentials.
    #            and `message`  - contains a message if applicable.
    has_credentials = True
    message = None
    if not context.url or not context.key:
        has_credentials = False
    elif not is_string(context.url):
        message = "DOcplexcloud: URL is not a string: {0!s}".format(context.url)
        has_credentials = False
    elif not is_string(context.key):
        message = "API key is not a string: {0!s}".format(context.key)
        has_credentials = False
    if context.key and has_credentials:
        has_credentials = isinstance(context.key, six.string_types)
    return has_credentials, message


def has_credentials(context):
    # Checks if the context has valid credentials.
    #
    # Returns:
    #    True if the context has valid credentials.
    # ignore message
    has_credentials, _ = check_credentials(context)
    return has_credentials


def print_context(context):
    # prints the context.
    def print_r(node, prefix):
        for n in sorted(node):
            path = ".".join([prefix, n] if prefix else [n])
            if not n.startswith("_") and isinstance(node.get(n), (dict, SolverContext)):
                print(path + " %s" % (type(node.get(n))))
                print_r(node.get(n), path)
            elif not n.startswith("_"):
                print(path + " = %s (%s)" % (node.get(n), type(node.get(n))))
    print_r(context, "context")


class BaseContext(dict):
    # Class for handling the list of parameters.
    def __init__(self, **kwargs):
        """ Create a new context.

        Args:
            List of ``key=value`` to initialize context with.
        """
        super(BaseContext, self).__init__()
        for k, v in kwargs.items():
            self.set_attribute(k, v)

    def __setattr__(self, name, value):
        self.set_attribute(name, value)

    def __getattr__(self, name):
        return self.get_attribute(name)

    def set_attribute(self, name, value):
        self[name] = value

    def get_attribute(self, name, default=None):
        if name.startswith('__'):
            raise AttributeError
        res = self.get(name, default)
        return res

class SolverContext(BaseContext):
    # for internal use
    def __init__(self, **kwargs):
        super(SolverContext, self).__init__(**kwargs)
        self.log_output = False
        self.max_threads = get_environment().get_available_core_count()
        self.auto_publish = True

    def __deepcopy__(self, memo):
        # We override deepcopy here just to make sure that we don't deepcopy
        # file descriptors...
        cls = self.__class__
        result = cls.__new__(cls)
        memo[id(self)] = result
        for k, v in iteritems(self):
            # do not duplicate those (io like objects)
            if k == "log_output" and hasattr(v, "write"):
                value = v
            else:
                value = deepcopy(v, memo)
            setattr(result, k, value)
        return result

    def get_log_output_as_stream(self):
        log_output = None
        try:
            log_output = self.log_output
        except AttributeError:
            return None

        output_stream = None
        # if log_output is an object with a lower attribute, let's use it
        # as string.lower and check for some known string values
        if hasattr(log_output, "lower"):
            k = log_output.lower()
            if k in _BOOLEAN_STATES:
                if _convert_to_bool(k):
                    output_stream = sys.stdout
                else:
                    output_stream = None
            if k in ["stdout", "sys.stdout"]:
                output_stream = sys.stdout
            if k in ["stderr", "sys.stderr"]:
                output_stream = sys.stderr
        # if log_output is == to True, just use stdout
        if log_output == True:
                output_stream = sys.stdout
        # if it has a write() attribute, just return it
        if hasattr(log_output, "write"):
            output_stream = log_output

        return output_stream

    log_output_as_stream = property(get_log_output_as_stream)


class Context(BaseContext):
    """ The context used to control the behavior of solve engine.

    Attributes:
        cplex_parameters: A
           :class:`docplex.mp.params.parameters.RootParameterGroup` to store
           CPLEX parameters.
        solver.auto_publish: If ``True``, a model being solved will automatically
            publish all publishable items (``solve_details``,
            ``json_solution``).
        solver.log_output: This attribute can have the following values:

            * True: When True, logs are printed to sys.out.
            * False: When False, logs are not printed.
            * A file-type object: Logs are printed to that file-type object.

        solver.docloud: The parent node for attributes controlling the solve on Decision Optimization on Cloud.
        solver.docloud.url: The DOcplexcloud service URL.
        solver.docloud.key: The DOcplexcloud service API key.
        solver.docloud.run_deterministic: Specific engine parameters are uploaded to keep the
            run deterministic.
        solver.docloud.verbose: Makes the connector verbose.
        solver.docloud.timeout: The timeout for requests.
        solver.docloud.waittime: The wait time to wait for jobs to finish.
        solver.docloud.verify: If True, verifies SSL certificates.
        solver.docloud.log_requests: If True, the REST requests are logged.
        solver.docloud.log_poll_interval: The interval for log polling.
        solver.docloud.progress_poll_interval: The interval for progress polling.
        solver.docloud.exchange_format: The exchange format to use.
            When setting the format, you can use the following strings: "lp".
            When getting the format, the property type is
            `docplex.mp.format.ExchangeFormat`.
        solver.docloud.job_parameters: dict of job parameters passed to DOCplexcloud.
    """
    def __init__(self, **kwargs):
        # store env used for initialization
        self['_env_at_init'] = kwargs.get('_env')
        # map lazy members to f(model) (actually f(self) ) returning the
        # initial value
        self['_lazy_members'] = \
           {'cplex_parameters': lambda m: m.init_cplex_parameters()}
        # initialize fields of this
        super(Context, self).__init__(solver=SolverContext(docloud=CreateDefaultDOcloudContext()),
                                      # cplex_parameters=cplex_parameters,
                                      docplex_tests=BaseContext())
        # update will also ensure compatibility with older kwargs like
        # 'url' and 'api_key'
        self.update(kwargs, create_missing_nodes=True)

    def init_cplex_parameters(self):
        local_env = self.get('_env_at_init') or Environment.get_default_env()
        cplex_version = local_env.cplex_version
        cplex_parameters = get_params_from_cplex_version(cplex_version)
        return cplex_parameters

    def __getattr__(self, name):
        if name not in self:
            lazy_members = self.get('_lazy_members')
            if lazy_members and name in lazy_members:
                self[name] = lazy_members[name](self)
        return self.get_attribute(name)

    def _get_raw_cplex_parameters(self):
        # NON LAZY: may return None
        return self.get('cplex_parameters')

    @staticmethod
    def make_default_context(file_list=None, logger=None, **kwargs):
        """Creates a default context.

        If `file_list` is a string, then it is considered to be the name
        of a config file to be read.

        If `file_list` is a list, it is considered to be a list of names
        of a config files to be read.

        if `file_list` is None or not specified, the following files are
        read if they exist:

            * the PYTHONPATH is searched for the following files:

                * cplex_config.py
                * cplex_config_<hostname>.py
                * docloud_config.py

        Python files are evaluated with a `context` object in the current
        scope, and you set values from this context::

            context.solver.docloud.url = 'http://testing.blabla.ibm.com'
            context.solver.docloud.key = 'This is an api_key'
            context.cplex_parameters.emphasis.memory = 1
            context.cplex_parameters.emphasis.mip = 2

        Args:
            file_list: The list of config files to read.
            kwargs: context parameters to override. See :func:`docplex.mp.context.Context.update`
        """
        context = Context()
        context.read_settings(file_list=file_list, logger=logger)
        context.update(kwargs)
        return context

    def copy(self):
        # Makes a deep copy of the context.
        #
        # Returns:
        #   A deep copy of the context.
        return deepcopy(self)

    def clone(self):
        # Makes a deep copy of the context.
        #
        # Returns:
        #   A deep copy of the context.
        return deepcopy(self)

    def update_from_list(self, values, logger=None):
        # For each pair of `(name, value)` in values, try to set the
        # attribute.
        for name, value in values:
            try:
                self._set_value(self, name, value)
            except AttributeError:
                if logger is not None:
                    logger.warning("Ignoring undefined attribute : {0}".format(name))

    def _set_value(self, root, property_spec, property_value):
        property_list = property_spec.split('.')
        property_chain = property_list[:-1]
        to_be_set = property_list[-1]
        o = root
        for c in property_chain:
            o = getattr(o, c)
        try:
            target_attribute = getattr(o, to_be_set)
        except AttributeError:
            target_attribute = None
        if target_attribute is None:
            # Simply set the attribute
            try:
                setattr(o, to_be_set, property_value)
            except DOcplexException:
                pass  # ignore this
        else:
            # try a set_converted_value if it's a Parameter
            try:
                target_attribute.set(property_value)
            except AttributeError:
                # no set(), just setattr
                setattr(o, to_be_set, property_value)

    def update(self, kwargs, create_missing_nodes=False):
        """ Updates this context from child parameters specified in ``kwargs``.

        The following keys are recognized:

            - cplex_parameters: A set of CPLEX parameters to use instead of the parameters defined as ``context.cplex_parameters``.
            - agent: Changes the ``context.solver.agent`` parameter.
                Supported agents include:

                - ``docloud``: forces the solve operation to use DOcplexcloud
                - ``local``: forces the solve operation to use native CPLEX

            - url: Overwrites the URL of the DOcplexcloud service defined by ``context.solver.docloud.url``.
            - key: Overwrites the authentication key of the DOcplexcloud service defined by ``context.solver.docloud.key``.
            - log_output: if ``True``, solver logs are output to stdout.
                If this is a stream, solver logs are output to that stream object.
                Overwrites the ``context.solver.log_output`` parameter.

        Args:
            kwargs: A ``dict`` containing keyword args to use to update this context.
            create_missing_nodes: When a keyword arg specify a parameter that is not already member of this context,
                creates the parameter if ``create_missing_nodes`` is True.

        """
        for k in kwargs:
            value = kwargs.get(k)
            if value is not None:
                self.update_key_value(k, value,
                                      create_missing_nodes=create_missing_nodes)

    def update_cplex_parameters(self, arg_params):
        # INTERNAL
        if isinstance(arg_params, RootParameterGroup):
            self.cplex_parameters = arg_params
        else:
            new_params = self.cplex_parameters.copy()
            # try a dictionary of parameter qualified names, parameter values
            # e.g. cplex_parameters={'mip.tolerances.mipgap': 0.01, 'timelimit': 180}
            try:
                for pk, pv in iteritems(arg_params):
                    p = new_params.find_parameter(key=pk)
                    if not p:
                        docplex_fatal('Cannot find matching parameter from: {0!r}'.format(pk))
                    else:
                        p.set(pv)
                self.cplex_parameters = new_params

            except (TypeError, AttributeError):
                docplex_fatal('Expecting CPLEX parameters or dict, got: {0!r}'.format(arg_params))

    def update_key_value(self, k, value, create_missing_nodes=False, warn=True):
        if k is 'docloud_context':
            warnings.warn('docloud_context is deprecated, use context.solver.docloud instead')
            self.solver.docloud = value
        elif k is 'cplex_parameters':
            if isinstance(value, RootParameterGroup):
                self.cplex_parameters = value
            else:
                self.update_cplex_parameters(value)

        elif k is 'url':
            self.solver.docloud.url = value
        elif k is 'api_key' or k is 'key':
            self.solver.docloud.key = value
        elif k is 'log_output':
            self.solver.log_output = value
        elif k is 'override':
            self.update_from_list(iteritems(value))
        elif k is 'proxies':
            self.solver.docloud.proxies = value
        elif k is '_env':
            # do nothing this is just here to avoid creating too many envs
            pass
        elif k is 'agent':
            self.solver.agent = value
        else:
            if create_missing_nodes:
                self[k] = value
            elif warn:
                warnings.warn("Unknown quick-setting in Context: {0:s}, value: {1!s}".format(k, value),
                              stacklevel=2)

    def read_settings(self, file_list=None, logger=None):
        """Reads settings for a list of files.

        If `file_list` is a string, then it is considered to be the name
        of a config file to be read.

        If `file_list` is a list, it is considered to be a list of names
        of config files to be read.

        if `file_list` is None or not specified, the following files are
        read if they exist:

            * the PYTHONPATH is searched for the following files:

                * cplex_config.py
                * cplex_config_<hostname>.py
                * docloud_config.py

        Python files are evaluated with a `context` object in the current
        scope, and you set values from this context::

            context.solver.docloud.url = 'http://testing.blabla.ibm.com'
            context.solver.docloud.key = 'This is an api_key'
            context.cplex_parameters.emphasis.memory = 1
            context.cplex_parameters.emphasis.mip = 2

        Args:
            file_list: The list of config files to read.
        """
        if file_list is None:
            file_list = []
            targets = ['cplex_config.py',
                       'cplex_config_{0}.py'.format(socket.gethostname()),
                       'docloud_config.py'
                       ]
            for target in targets:
                if isabs(target) and isfile(target) and target not in file_list:
                    file_list.append(target)
                else:
                    for d in sys.path:
                        f = os.path.join(d, target)
                        if os.path.isfile(f):
                            abs_name = os.path.abspath(f)
                            if abs_name not in file_list:
                                file_list.append(f)

            if len(file_list) == 0:
                file_list = None  # let read_settings use its default behavior

        if isinstance(file_list, six.string_types):
            file_list = [file_list]

        if file_list is not None:
            for f in file_list:
                if os.path.isfile(f):
                    if logger:
                        logger.info("Reading settings from %s" % f)
                    if f.endswith(".py"):
                        self.read_from_python_file(f)


    def read_from_python_file(self, filename):
        # Evaluates the content of a Python file containing code to set up a
        # context.
        #
        # Args:
        #    filename (str): The name of the file to evaluate.
        if os.path.isfile(filename):
            with open_universal_newline(filename, 'r') as f:
                # This is so that there is a context in the scope of the exec 
                context = self
                exec(f.read())
        return self


def CreateDefaultDOcloudContext():
    # Returns a context to use as the context.solver.docloud member.
    #
    # This is a Context with predefined fields.
    #
    dctx = BaseContext()
    # There'se a bunch of properties defined so that we can set
    # any values
    dctx.url = None
    dctx.key = None
    dctx.run_deterministic = False
    dctx.verbose = False
    dctx.timeout = None
    dctx.waittime = None
    dctx.verify = None  # default is None so that we use defaults
    dctx.log_requests = None
    dctx.exchange_format = None
    dctx.debug_dump = None
    dctx.debug_dump_dir = None
    dctx.log_poll_interval = None
    dctx.progress_poll_interval = None
    dctx.verbose_progress_logger = None
    dctx.delete_job = True
    # if true, download job info after solve() has finished and fire
    # the last details as a progress_info. Mostly for debug.
    dctx.fire_last_progress = False
    # Mostly for debug: This callback is called when the solve is finished.
    # It should be a method taking **kwargs. It will be called with those kwargs:
    # - jobid: the jobid
    # - client: the docloud client used to connect to docloud
    # - connector: the DOcloudConnector
    dctx.on_solve_finished_cb = None
    # The proxies
    dctx.proxies = None
    # additional job parameters
    dctx.job_parameters = None
    # mangle names into x<...?>
    dctx.mangle_names = False
    return dctx

