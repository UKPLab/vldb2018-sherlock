# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
Configuration of the CP Optimizer Python API

This module is the top-level handler of the configuration parameters for
the CP Optimizer Python API. It contains the default values of the different
configuration parameters.

It should NOT be changed directly.
The preferable way is to add at least one of the following files that contain the changes
to be performed:

 * *cpo_config.py*, a local set of changes on these parameters,
 * *cpo_config_<hostname>.py*, a hostname dependent set of changes.
 * *docloud_config.py* (for DOcloud url and key, file shared with docplex.mp package).

Final set of parameters is obtained by reading first this module, and then those
listed above.
These modules should be visible from the *PYTHONPATH* and are loaded in
this order to overwrite default values.

This module also defines two global variables:

 * *DOCLOUD_CONTEXT*, that contains the configuration necessary to solve a model on DOcloud.
   This context is the context by default, referenced by the global variable 'context'.
 * *LOCAL_CONTEXT*, that contains the configuration appropriate to solve a model with a local
   installation of the CPO solver.
   This configuration is available for solver with version number greater or equal to 12.7.0.

The method :meth:`set_default` allows to set the default configuration to one that is predefined,
or another that has been totally customized.

If called as main, this module prints the actual configuration on standard output, including
all customizations made using the mechanism described above.

Following sections describe the most important parameters that can be easily modified to customize
the behavior of the Python API.
All available parameters are available by consulting the source code of this module.

General parameters
------------------

*context.log_output = sys.stdout*

    This parameter contains the default log stream.
    By default it is set to the standard output.
    A value of *None* can be used to disable all logs.

*context.verbose = 0*

    This parameter controls the verbosity level of the log, between 0 and 9, if *log_output* is not None.
    The default value of 0 means no log.

*context.model.add_source_location = True*

    This parameter indicates that when the model is transformed into CPO format, additional information is added
    to correlate expressions with the Python file and line where it has been generated.
    If any error is raised by the solver during the solve, this information is provided in the
    error description, which allows for easier debugging.

*context.model.length_for_alias = 15*

    This parameter allows to associate a shorter alias to variables whose name is longer than the given length.
    In the CPO representation of the model, variable is declared with its original name and an alias is created
    to use it with a shorter name in model expressions, allowing to reduce the size of the generated CPO format.

    In the returned solution, variable can be still retrieved with their original names.

    By default, the value is 15. A value of None would indicate to always keep original variable names.

*context.model.length_for_rename = None*

    This parameter allows to replace the names of the variables when it is longer than the given length.
    A shorter name is generated and is used everywhere in the generated model CPO format in place of the original name.
    This allows to drastically reduce the size of the model generated in the CPO format.

    In the returned solution, the value of such variables can be retrieved thanks to a mapping between previous and
    new names, that is maintained in the client Python program.

    By default, the value is None, indicating to keep original variable names.

*context.model.name_all_constraints = False*

    This parameter enables the naming of all constraints when the model is generated in CPO format.
    It is mandatory only if the *refine conflict* function is called.
    Anyway, if the *refine conflict* function is called, and if the CPO format of the model has already been generated,
    it is generated again with this option set in order to allow proper completion of the request.
    Setting it to *True* is preferable only if *refine conflict* function is called on a big model.

*context.model.dump_directory = None*

    This parameter gives the name of a directory where the CPO files that are generated for solving models are stored
    for logging purpose.

    If not None, the directory is created and generated models are stored in files named `<model_name>.cpo`.

*context.model.cache.size = 10000*

    This parameter gives the maximum capacity of the internal cache used to speed-up conversion of Python expressions
    into CPO expressions.

*context.model.cache.active = True*

    This parameter allows to enable or disable the expression cache mechanism.
    Value os a boolean (True or False). Default value is True.

*context.params.\**

    The parameter `context.params` is an instance of the class
    :class:`~docplex.cp.parameters.CpoParameters` (in :doc:`parameters.py</docplex.cp.parameters.py>`)
    which describes all of the public solver parameters as properties.

    The default configuration limits the solving time to 100 seconds by using following settings:
    ::

        context.params.TimeMode = "ElapsedTime"
        context.params.TimeLimit = 100

    These parameters may have a different default setting if the solver is not *DOcplexcloud*.

Configuration of the model solving
----------------------------------

*context.solver.trace_cpo = False*

    This parameter indicates to trace the CPO model that is generated before submitting it for solving.
    The model is printed on the `context.log_output stream`, if given.

*context.solver.trace_log = False*

    This parameter indicates to trace the log generated by the solver when solving the CPO model.
    The log is printed on the `context.log_output stream`, if given.

    The default value of this parameter is False for a solve on the cloud, and True for a local solve.

*context.solver.enable_undocumented_params = False*

    This parameter allows to enable the possibility to set solving parameters that are not in the public parameters
    detailed in the class
    :class:`~docplex.cp.parameters.CpoParameters` (in :doc:`parameters.py</docplex.cp.parameters.py>`).

*context.solver.add_log_to_solution = True*

    This parameter indicates to add the solver log content to the solution object.
    By default, this parameter is True but it can be set to False if the log is very big or of no interest.

*context.solver.agent = 'docloud'*

    This parameter specifies the name of the solver agent that is used to solve the model.
    The value of this parameter is the name of a child context of `context.solver`, which contains necessary attributes
    that allow to create and run the required agent.

    There are two different agents described in the default configuration file:

       * `docloud`, the default agent, for solving a CPO model using the DOcplexcloud service.
       * `local`, the agent allowing to solve models locally using the CP Optimizer Interactive coming with
         versions of COS greater or equal to 12.7.0.

    If the CP Optimizer Interactive program *cpoptimizer(.exe)* is detected in the system path, the default solver
    agent is automatically set to *local* instead of *docloud*.

*context.solver.log_prefix = "[Solver] "*

    Prefix that is added to every message that is logged by the solver component.

Configuration of the `docloud` solving agent
--------------------------------------------

*context.solver.docloud.url = "https://api-oaas.docloud.ibmcloud.com/job_manager/rest/v1/"*

    This parameter is used to specify the URL of the *DOcplexcloud* service.

*context.solver.docloud.key = "'Set your key in docloud_config.py'"*

    This parameter contains the personal key for authorizing access to the *DOcplexcloud* service.
    Access credentials (base URL and access key) can be retrieved after registration from `<http://developer.ibm.com/docloud/docs/api-key/>`_.

*context.solver.docloud.verify_ssl = True*

    This parameter allows to enable/disable the verification of SSL certificates.

*context.solver.docloud.proxies = None*

    This parameter allows to optionally define proxies to be used in the connection with *DOcplexcloud*.
    It is a Python dictionary protocol_name / endpoint, as described in http://docs.python-requests.org/en/master/user/advanced/#proxies.

*context.solver.docloud.request_timeout = 30*

    This parameter contains the maximum time, in seconds, that a response is waited for after a unitary request to *DOcplexcloud* server.

*context.solver.docloud.result_wait_extra_time = 60*

    This parameter is a time in seconds added to the expected solve time to compute the total result waiting timeout.

*context.solver.docloud.clean_job_after_solve = True*

    This parameter indicates whether the job is automatically cleaned after the model is solved.
    If not set to True, the model stays on the *DOcplexcloud* server and is visible from its *DropSolve* interface.
    Note that the server may block future solving requests if there are too many jobs waiting.

*context.solver.docloud.polling = Context(min=1, max=3, incr=0.2)*

    This parameter describes how the Python client polls the result of the solve on *DOcplexcloud*.
    Polling delay is inside an interval [min, max], starting by min, growing to max with the given increment.


Configuration of the `local` solving agent
------------------------------------------

*context.solver.local.execfile*

    Name or full path of the CP Optimizer Interactive executable file.
    By default, it is set to *cpoptimizer(.exe)*, which supposes that the program is visible from the system path.


Configuration for best performances
-----------------------------------

To configure the CP Python API for best performances, the following configuration settings may be used.
Obviously, this performance is won at the cost of the loss of some features that may be useful in other cases.
::

    context.verbose = 0
    context.model.add_source_location = False
    context.model.length_for_rename = 10
    context.model.name_all_constraints = False
    context.model.dump_directory = None
    context.solver.trace_cpo = False
    context.solver.trace_log = False
    context.solver.add_log_to_solution = False


Detailed description
--------------------
"""

from docplex.cp.utils import Context, CpoException, search_file_in_path, IS_IN_NOTEBOOK, is_string
from docplex.cp.parameters import CpoParameters, ALL_PARAMETER_NAMES

import sys, socket, os, platform, traceback

try:
    import docplex.util.environment as runenv
    ENVIRONMENT_PRESENT = True
except:
    ENVIRONMENT_PRESENT = False

EXE_EXTENSION = ".exe" if platform.system() == 'Windows' else ""


##############################################################################
## Define default context for DOcloud solving
##############################################################################

#-----------------------------------------------------------------------------
# Global context

# Create default context infrastructure
DOCLOUD_CONTEXT = Context(model=Context(),
                          params=CpoParameters(),
                          solver=Context())
context = DOCLOUD_CONTEXT

# Default log output
context.log_output = sys.stdout

# Default log verbosity
context.verbose = 0

# Visu enable indicator (internal, can be disabled for testing purpose)
context.visu_enabled = True


#-----------------------------------------------------------------------------
# Modeling context

# Indicate to add source location in model
context.model.add_source_location = True

# Minimal variable name length that trigger use of shorter alias. None for no alias.
context.model.length_for_alias = 15

# Minimal variable name length that trigger renaming variable with a shorter name. None for no rename.
context.model.length_for_rename = None

# Automatically add a name to every top-level constraint
context.model.name_all_constraints = False

# Name of the directory where store copy of the generated CPO files. None for no dump.
context.model.dump_directory = None

# Expression cache
context.model.cache = Context()
context.model.cache.size = 10000
context.model.cache.active = True


#-----------------------------------------------------------------------------
# Solving parameters

# Default time limit
context.params.TimeLimit = 100

# Workers count
context.params.Workers = 4


#-----------------------------------------------------------------------------
# Solving context

# Indicate to trace CPO model before solving
context.solver.trace_cpo = False

# Indicate to trace solver log on log_output.
context.solver.trace_log = False

# Enable undocumented parameters
context.solver.enable_undocumented_params = False

# Max number of threads allowed for model solving
context.solver.max_threads = None
if ENVIRONMENT_PRESENT:
    context.solver.max_threads = runenv.get_environment().get_available_core_count()

# Indicate to add solver log to the solution
context.solver.add_log_to_solution = True

# Indicate to auto-publish solve details and results in environment
context.solver.auto_publish = True

# Indicate to replace simple solve by a start/next loop
context.solver.solve_with_start_next = False

# Log prefix
context.solver.log_prefix = "[Solver] "

# Name of the agent to be used for solving. Value is name of one of this context child context (i.e. 'docloud').
context.solver.agent = 'docloud'


#-----------------------------------------------------------------------------
# DoCloud solving agent context

context.solver.docloud = Context()

# Agent class name
context.solver.docloud.class_name = "docplex.cp.solver.solver_docloud.CpoSolverDocloud"

# Url of the DOCloud service
context.solver.docloud.url = "https://api-oaas.docloud.ibmcloud.com/job_manager/rest/v1/"

# Authentication key.
context.solver.docloud.key = "'Set your key in docloud_config.py''"

# Secret key.
context.solver.docloud.secret = None

# Indicate to verify SSL certificates
context.solver.docloud.verify_ssl = True

# Proxies (map protocol_name/endpoint, as described in http://docs.python-requests.org/en/master/user/advanced/#proxies)
context.solver.docloud.proxies = None

# Default unitary request timeout in seconds
context.solver.docloud.request_timeout = 30

# Time added to expected solve time to compute the total result waiting timeout
context.solver.docloud.result_wait_extra_time = 60

# Clean job after solve indicator
context.solver.docloud.clean_job_after_solve = True

# Add 'Connection close' in all headers
context.solver.docloud.always_close_connection = False

# Log prefix
context.solver.docloud.log_prefix = "[DOcloud] "

# Polling delay (min, max and increment)
context.solver.docloud.polling = Context(min=1, max=3, incr=0.2)


#-----------------------------------------------------------------------------
# Local solving agent context

context.solver.local = Context(class_name = "docplex.cp.solver.solver_local.CpoSolverLocal",
                               execfile   = "cpoptimizer" + EXE_EXTENSION,
                               parameters = ['-angel'],
                               log_prefix = "[Local] ")

LOCAL_CONTEXT = context.clone()

LOCAL_CONTEXT.params.pop('TimeLimit')
LOCAL_CONTEXT.params.pop('Workers')

LOCAL_CONTEXT.solver.trace_log = not IS_IN_NOTEBOOK
LOCAL_CONTEXT.solver.agent = 'local'
LOCAL_CONTEXT.solver.max_threads = None

# Select local context if exec file is visible in the path
cpfile = search_file_in_path(LOCAL_CONTEXT.solver.local.execfile)
if cpfile:
    LOCAL_CONTEXT.solver.local.execpath = cpfile
    context = LOCAL_CONTEXT


##############################################################################
## Public functions
##############################################################################

def get_default():
    """ Get the default context

    Default context is also accessible with the global variable 'context' in this module.

    Returns:
        Current default context
    """
    return context

def set_default(ctx):
    """ Set the default context.

    Default context becomes accessible in the global variable 'context' in this module.

    Args:
        ctx: New default context
    """
    if ctx is None:
        ctx = Context()
    else:
        assert isinstance(ctx, Context), "Context object must be of class Context"
    sys.modules[__name__].context = ctx


# Attribute values denoting a default value
DEFAULT_VALUES = ("ENTER YOUR KEY HERE", "ENTER YOUR URL HERE", "default")

def _is_defined(arg, kwargs):
    return (arg in kwargs) and kwargs[arg] and (kwargs[arg] not in DEFAULT_VALUES)

def _get_effective_context(**kwargs):
    """ Build a effective context from a variable list of arguments that may specify changes to default.

    Args:
        context (optional):   Source context, if not default.
        params (optional):    Solving parameters (CpoParameters) that overwrite those in the solving context
        (others) (optional):  All other context parameters that can be changed.
    Returns:
        Updated (cloned) context
    """
    # If 'url' and 'key' are defined, force agent to be docloud
    if ('agent' not in kwargs) and not ENVIRONMENT_PRESENT:
        url = kwargs.get('url')
        key = kwargs.get('key')
        if url and key and is_string(url) and is_string(key) and url.startswith('http'):
            kwargs['agent'] = 'docloud'

    # Determine source context
    ctx = kwargs.get('context')
    if (ctx is None) or (ctx in DEFAULT_VALUES):
        ctx = context
    ctx = ctx.clone()
    # print("\n*** Source context");
    # ctx.print_context()

    # First set parameters if given
    prms = kwargs.get('params')
    if prms is not None:
        ctx.params.add(prms)

    # Process other changes
    rplist = []  # List of replacements to be done in solving parameters
    for k, v in kwargs.items():
        if (k != 'context') and (k != 'params') and (v not in DEFAULT_VALUES):
            rp = ctx.search_and_replace_attribute(k, v)
            # If not found, set in solving parameters
            if (rp is None):
                rplist.append((k, v))

     # Replace or set remaining fields in parameters
    if rplist:
        params = ctx.params
        chkparams = not ctx.solver.enable_undocumented_params
        if isinstance(params, CpoParameters):
            for k, v in rplist:
                if chkparams and not k in ALL_PARAMETER_NAMES:
                    raise CpoException("CPO solver does not accept a parameter named '{}'".format(k))
                setattr(params, k, v)

    # Return
    # print("\n*** Result context");
    # ctx.print_context()
    return ctx


##############################################################################
## Overload this configuration with other customized configuraton python files
##############################################################################

def _eval_file(file):
    """ If exists, evaluate the content of a python module in this module.
    Args:
        file: Python file to evaluate
    """
    for f in filter(os.path.isfile, [dir + "/" + file for dir in sys.path]):
        try:
            exec(open(f).read())
        except Exception as e:
            traceback.print_exc()
            raise Exception("Error while loading config file {}: {}".format(f, str(e)))


# Initialize default list of files to load
FILE_LIST = ("cpo_config.py",
             "cpo_config_" + socket.gethostname() + ".py",
             "docloud_config.py")

# Load all config changes
for f in FILE_LIST:
    _eval_file(f)


##############################################################################
## Print configuration when called as main
##############################################################################

if __name__ == "__main__":
    context.print_context()
