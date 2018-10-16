# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

# pylint: disable=too-many-lines
from __future__ import print_function

import os
import sys
import warnings
from collections import OrderedDict
from itertools import product

import six

from docplex.mp.aggregator import ModelAggregator
from docplex.mp.compat23 import StringIO, izip
from docplex.mp.constants import SOSType, CplexScope, SolveAttribute, ObjectiveSense
from docplex.mp.constr import AbstractConstraint, LinearConstraint, RangeConstraint, \
    IndicatorConstraint, QuadraticConstraint, PwlConstraint, EquivalenceConstraint
from docplex.mp.context import Context, \
    is_auto_publishing_solve_details, is_auto_publishing_json_solution, \
    is_auto_publishing, has_credentials
from docplex.mp.cloudutils import is_url_valid
#from docplex.mp.docloud_engine import DOcloudEngine
from docplex.mp.engine_factory import EngineFactory
from docplex.mp.environment import Environment
from docplex.mp.error_handler import DefaultErrorHandler, docplex_add_trivial_infeasible_ct
from docplex.mp.format import LP_format, SAV_format
from docplex.mp.mfactory import ModelFactory
from docplex.mp.model_stats import ModelStatistics
from docplex.mp.numutils import round_nearest_towards_infinity
from docplex.mp.printer_factory import ModelPrinterFactory
from docplex.mp.pwl import PwlFunction
from docplex.mp.tck import get_typechecker
from docplex.mp.sttck import StaticTypeChecker
from docplex.mp.utils import DOcplexException
from docplex.mp.utils import is_indexable, is_iterable, is_int, is_string, \
    make_output_path2, generate_constant, _AutomaticSymbolGenerator, _IndexScope, _to_list, _ToleranceScheme, \
    compute_is_index, is_number
from docplex.mp.utils import apply_thread_limitations
from docplex.mp.vartype import BinaryVarType, IntegerVarType, ContinuousVarType, SemiContinuousVarType, SemiIntegerVarType
from docplex.util.environment import get_environment
from docplex.mp.xcounter import FastOrderedDict

from docplex.mp.cloudutils import context_must_use_docloud, context_has_docloud_credentials,\
    is_in_docplex_worker

from docplex.mp.progress import KpiRecorder, SolutionHookListener

try:
    from docplex.worker.solvehook import get_solve_hook
except ImportError:  # pragma: no cover
    get_solve_hook = None  # pragma: no cover


# noinspection PyProtectedMember
class Model(object):
    """ This is the main class to embed modeling objects.

    The :class:`Model` class acts as a factory to create optimization objects,
    decision variables, and constraints.
    It provides various accessors and iterators to the modeling objects.
    It also manages solving operations and solution management.

    The Model class is the context manager and can be used with the Python `with` statement:

    .. code-block:: python

       with Model() as mdl:
         # start modeling...

    When the `with` block is finished, the :func:`end` method is called automatically, and all resources
    allocated by the model are destroyed.

    When a model is created without a specified ``context``, a default
    ``Context`` is created and initialized as described in :func:`docplex.mp.context.Context.read_settings`.

    Example::

        # Creates a model named 'my model' with default context
        model = Model('my model')

    In the following example, credentials for the solve service are looked up as
    described in :func:`docplex.mp.context.Context.read_settings`::

        # Creates a model with default ``context``, and override the ``agent``
        # to solve on Decision Optimization on Cloud.
        model = Model(agent='docloud')

    In this example, we create a model to solve with just 2 threads::

        context = Context.make_default_context()
        context.cplex_parameters.threads = 2
        model = Model(context=context)

    Alternatively, this can be coded as::

        model = Model()
        model.context.cplex_parameters.threads = 2

    Args:
        name (optional): The name of the model.
        context (optional): The solve context to be used. If no ``context`` is
            passed, a default context is created.
        agent (optional): The ``context.solver.agent`` is initialized with
            this string.
        log_output (optional): If ``True``, solver logs are output to
            stdout. If this is a stream, solver logs are output to that
            stream object.
        checker (optional): If ``None``, then checking is disabled everywhere. Turning off checking
            may improve performance but should be done only with extreme caution.
    """

    _name_generator = _AutomaticSymbolGenerator(pattern="docplex_model", offset=1)

    @property
    def binary_vartype(self):
        """ This property returns an instance of :class:`docplex.mp.vartype.BinaryVarType`.

        This type instance is used to build all binary decision variable collections of the model.
        """
        return self._binary_vartype

    @property
    def integer_vartype(self):
        """ This property returns an instance of :class:`docplex.mp.vartype.IntegerVarType`.

        This type instance is used to build all integer variable collections of the model.
        """
        return self._integer_vartype

    @property
    def continuous_vartype(self):
        """ This property returns an instance of :class:`docplex.mp.vartype.ContinuousVarType`.

        This type instance is used to build all continuous variable collections of the model.
        """
        return self._continuous_vartype

    @property
    def semicontinuous_vartype(self):
        """ This property returns an instance of :class:`docplex.mp.vartype.SemiContinuousVarType`.

        This type instance is used to build all semi-continuous variable collections of the model.
        """
        return self._semicontinuous_vartype

    @property
    def semiinteger_vartype(self):
        """ This property returns an instance of :class:`docplex.mp.vartype.SemiIntegerType`.

        This type instance is used to build all semi-integer variable collections of the model.
        """
        return self._semiinteger_vartype


    def _make_environment(self):
        env = Environment.get_default_env()
        # rtc-28869
        env.numpy_hook = Model.init_numpy
        return env

    def _lazy_get_environment(self):
        if self._environment is None:
            self._environment = self._make_environment()  # pragma: no cover
        return self._environment

    _saved_numpy_options = None

    _unknown_status = None

    @staticmethod
    def init_numpy():
        """ Static method to customize `numpy` for DOcplex.

        This method makes `numpy` aware of DOcplex.
        All `numpy` arrays with DOcplex objects will be printed by their string representations
        as returned by `str(`) instead of `repr()` as with standard numpy behavior.

        All customizations can be removed by calling the :func:`restore_numpy` method.

        Note:
            This method does nothing if `numpy` is not present.

        See Also:
            :func:`restore_numpy`
        """
        try:
            # noinspection PyUnresolvedReferences
            import numpy as np

            Model._saved_numpy_options = np.get_printoptions()
            np.set_printoptions(formatter={'numpystr': Model._numpy_print_str, 'object': Model._numpy_print_str})
        except ImportError:  # pragma: no cover
            pass  # pragma: no cover

    @staticmethod
    def _numpy_print_str(arg):
        return str(arg) if ModelFactory._is_operand(arg) else repr(arg)

    @staticmethod
    def restore_numpy():  # pragma: no cover
        """ Static method to restore `numpy` to its default state.

        This method is a companion method to :func:`init_numpy`. It restores `numpy` to its original state,
        undoing all customizations that were done for DOcplex.

        Note:
            This method does nothing if numpy is not present.

        See Also:
            :func:`init_numpy`
        """
        try:
            # noinspection PyUnresolvedReferences
            import numpy as np

            if Model._saved_numpy_options is not None:
                np.set_printoptions(Model._saved_numpy_options)
        except ImportError:  # pragma: no cover
            pass  # pragma: no cover

    @property
    def environment(self):
        # for a closed model with no CPLEX, numpy, etc return ClosedEnvironment
        # return get_no_graphics_env()
        # from docplex.environment import ClosedEnvironment
        # return ClosedEnvironment
        return self._lazy_get_environment()

    # ---- type checking

    def typecheck_var(self, obj):
        self._checker.typecheck_var(obj)

    def typecheck_num(self, arg, caller=None):
        self._checker.typecheck_num(arg, caller)

    def unsupported_relational_operator_error(self, left_arg, op, right_arg):
        # INTERNAL
        self.fatal("Unsupported relational operator:  {0!s} {1!s} {2!s}, only <=, ==, >= are allowed", left_arg, op,
                   right_arg)

    def unsupported_neq_error(self, left_arg, right_arg):
        self.fatal("The \"not_equal\" logical constraint is not supported: {0!s} != {1!s}", left_arg, right_arg)

    def cannot_be_used_as_denominator_error(self, denominator, numerator):
        StaticTypeChecker.cannot_be_used_as_denominator_error(self, denominator, numerator)

    def typecheck_as_denominator(self, denominator, numerator):
        StaticTypeChecker.typecheck_as_denominator(self, denominator, numerator)

    def typecheck_as_power(self, e, power):
        # INTERNAL: checks <power> is 0,1,2
        StaticTypeChecker.typecheck_as_power(self, e, power)

    def round_nearest(self, x):
        # INTERNAL
        return round_nearest_towards_infinity(x, self.infinity)

    def _parse_kwargs(self, kwargs):
        # parse some arguments from kwargs
        for arg_name, arg_val in six.iteritems(kwargs):
            if arg_name == "float_precision":
                self.float_precision = arg_val
            elif arg_name in frozenset({'keep_ordering', 'ordering'}):
                self._keep_ordering = bool(arg_val)
            elif arg_name in frozenset({"info_level", "output_level"}):
                self.output_level = arg_val
            elif arg_name in {"agent", "solver_agent"}:
                self.context.solver.agent = arg_val
            elif arg_name == "log_output":
                self.context.solver.log_output = arg_val
            elif arg_name == "warn_trivial":
                self._trivial_cts_message_level = arg_val
            elif arg_name == "max_repr_len":
                self._max_repr_len = int(arg_val)
            elif arg_name == "keep_all_exprs":
                self._keep_all_exprs = bool(arg_val)
            elif arg_name == 'checker':
                self._checker_key = arg_val.lower() if is_string(arg_val) else 'default'
            elif arg_name == 'full_obj':
                self._print_full_obj = bool(arg_val)
            elif arg_name in frozenset({'deploy', 'ignore_names'}):
                self._ignore_names = bool(arg_val)
            elif arg_name == 'clean_before_solve':
                self.clean_before_solve = arg_val
            elif arg_name in frozenset({'url', 'key'}):
                # these two are known, no need to rant
                pass
            elif arg_name in frozenset({"parameters", 'cplex_parameters'}):
                # update parameters either from a params object or a dict
                self.context.update_cplex_parameters(arg_val)
            else:
                self.warning("keyword argument: {0:s}={1!s} - is not recognized (ignored)", arg_name, arg_val)

    def _get_kwargs(self):
        kwargs_map = {'float_precision': self.float_precision,
                      'keep_ordering': self.keep_ordering,
                      'output_level': self.output_level,
                      'solver_agent': self.solver_agent,
                      'log_output': self.log_output,
                      'warn_trivial': self._trivial_cts_message_level,
                      'max_repr_len': self._max_repr_len,
                      'keep_all_exprs': self._keep_all_exprs,
                      'checker': self._checker_key,
                      'full_obj': self._print_full_obj,
                      'deploy': self._ignore_names,
                      'clean_before_solve': self._clean_before_solve}
        return kwargs_map

    _default_varname_pattern = "_x"
    _default_ctname_pattern = "_c"
    _default_indicator_pattern = "_ic"
    _default_quadct_pattern = "_qc"
    _default_pwl_pattern = "_pwl"

    warn_trivial_feasible = 0
    warn_trivial_infeasible = 1
    warn_trivial_none = 2

    def __init__(self, name=None, context=None, **kwargs):
        """Init a new Model.

        Args:
            name (optional): The name of the model
            context (optional): The solve context to be used. If no ``context`` is
                passed, a default context is created.
            agent (optional): The ``context.solver.agent`` is initialized with
                this string.
            log_output (optional): if ``True``, solver logs are output to
                stdout. If this is a stream, solver logs are output to that
                stream object.
        """
        if name is None:
            name = Model._name_generator.new_symbol()
        self._name = name

        self._error_handler = DefaultErrorHandler(output_level='warning')

        # type instances
        self._binary_vartype = BinaryVarType()
        self._integer_vartype = IntegerVarType()
        self._continuous_vartype = ContinuousVarType()
        self._semicontinuous_vartype = SemiContinuousVarType()
        self._semiinteger_vartype = SemiIntegerVarType()

        #
        self.__allvarctns = []
        self.__allvars = []
        self.__vars_by_name = {}
        self.__allcts = []
        self.__cts_by_name = None
        self.__allpwlfuncs = []
        self._benders_annotations = None
        self.__all_implicit_equiv_cts = []

        self._allsos = []

        self._allpwl = []
        self._pwl_counter = {}

        # -- kpis --
        self._allkpis = []

        self._progress_listeners = []
        self._solve_hooks = []  # debugSolveHook()
        self._solution_hook = None
        self._mipstarts = []

        # by default, deploy model is off
        self._ignore_names = False

        # clean engine before solve (mip starts)
        self._clean_before_solve = False  # default is False: faster

        # expression ordering
        self._keep_ordering = False

        # -- float formats
        self._float_precision = 3
        self._float_meta_format = '{%d:.3f}'

        self._environment = self._make_environment()
        self_env = self._environment
        # init context
        if context is None:
            # This is roughly equivalent to Context.make_default_context()
            # but save us a modification to Context.make_default_context()
            # to accept _env
            self.context = Context(_env=self_env)
            self.context.read_settings()  # read default settings
        else:
            self.context = context
            # a flag to indicate whether ot not parameters have been version-checked.
        self._synced_params = False

        self._engine_factory = EngineFactory(env=self_env)

        if 'docloud_context' in kwargs:
            warnings.warn(
                "Model construction with DOcloudContext is deprecated, use initializer with docplex.mp.context.Context instead.",
                DeprecationWarning, stacklevel=2)

        # offset between generate dnames and zero based indices.
        self._name_offset = 1

        # maximum length for expression in repr strings...
        self._max_repr_len = -1

        # control whether to warn about trivial constraints
        self._trivial_cts_message_level = self.warn_trivial_infeasible

        # internal
        self._keep_all_exprs = True  # use False to get fast clone...with the risk of side effects...

        # full objective lp
        self._print_full_obj = False

        self._checker_key = 'default'

        # update from kwargs, before the actual inits.
        self._parse_kwargs(kwargs)

        self._set_term_dict_type(self._keep_ordering)

        self._checker = get_typechecker(arg=self._checker_key, logger=self.error_handler)

        # -- scopes
        name_offset = self._name_offset
        self._var_scope = _IndexScope(self.iter_variables, self._default_varname_pattern, offset=name_offset)
        self._linct_scope = _IndexScope(self.iter_linear_constraints, self._default_ctname_pattern, offset=name_offset)
        self._indicator_scope = _IndexScope(self.iter_indicator_constraints, self._default_indicator_pattern,
                                            offset=name_offset)
        self._quadct_scope = _IndexScope(self.iter_quadratic_constraints, self._default_quadct_pattern,
                                         offset=name_offset)
        self._pwl_scope = _IndexScope(self.iter_pwl_constraints, self._default_pwl_pattern, offset=name_offset)
        self._scopes = [self._var_scope, self._linct_scope, self._indicator_scope, self._quadct_scope, self._pwl_scope]
        self._ctscopes = [self._linct_scope, self._indicator_scope, self._quadct_scope, self._pwl_scope]
        # a counter to generate unique numbers, incremented at each new request
        self._unique_counter = -1

        # init engine
        engine = self._make_new_engine_from_agent(self.solver_agent, self.context)
        self.__engine = engine

        self_keep_ordering = self.keep_ordering
        self_term_dict_type = self._term_dict_type
        self._lfactory = ModelFactory(self, engine, ordered=self_keep_ordering, term_dict_type=self_term_dict_type)
        from docplex.mp.quadfact import QuadFactory
        self._qfactory = QuadFactory(self, engine, ordered=self_keep_ordering, term_dict_type=self_term_dict_type)
        # after parse kwargs
        self._aggregator = ModelAggregator(self._lfactory, self._qfactory, ordered=self_keep_ordering, counter_type=self_term_dict_type)

        self._solution = None
        self._solve_details = None

        # stats
        self._linexpr_instance_counter = 0
        self._linexpr_clone_counter = 0
        self._quadexpr_instance_counter = 0
        self._quadexpr_clone_counter = 0

        # all the following must be placed after an engine has been set.
        self._objective_expr = None

        self.set_objective(sense=self._lfactory.default_objective_sense(),
                           expr=self._new_default_objective_expr())

    def _set_term_dict_type(self, ordered):
        if not ordered or Environment.env_is_python36:
            self._term_dict_type = dict
        else:
            self._term_dict_type = FastOrderedDict

    def _sync_params(self, params):
        # INTERNAL: execute only once
        self_env = self._environment
        # parameters
        self_cplex_parameters_version = self.context.cplex_parameters.cplex_version
        self_engine = self.__engine
        if self_engine.has_cplex():
            installed_cplex_version = self_env.cplex_version
            # installed version is different from parameters: reset all defaults
            if installed_cplex_version != self_cplex_parameters_version:  # pragma: no cover
                # cplex is more recent than parameters. must update defaults.
                self.trace(
                    "reset parameter defaults, from parameter version: {0} to installed version: {1}"  # pragma: no cover
                        .format(self_cplex_parameters_version, installed_cplex_version))  # pragma: no cover
                resets = self_engine._sync_parameter_defaults_from_cplex(params)  # pragma: no cover
                if resets:
                    for p, old, new in resets:
                        if p.short_name != 'randomseed':  # usual practice to change randomseed at each version
                            self.info('parameter changed, name: {0}, old default: {1}, new default: {2}',
                                      p.short_name, old, new)

    def _add_solve_hook(self, hook):
        if hook is not None:
            self._solve_hooks.append(hook)

    @property
    def infinity(self):
        """ This property returns the numerical value used as the upper bound for continuous decision variables.

        Note:
            CPLEX usually sets this limit to 1e+20.
        """
        return self.__engine.get_infinity()

    @property
    def progress_listeners(self):
        """ This property returns the list of progress listeners.
        """
        return self._progress_listeners

    def get_name(self):
        return self._name

    def _set_name(self, name):
        self._check_name(name)
        self._name = name

    def _check_name(self, new_name):
        self._checker.typecheck_string(arg=new_name, accept_empty=False, accept_none=False)
        if ' ' in new_name:
            self.warning("Model name contains whitespaces: |{0:s}|", new_name)

    name = property(get_name, _set_name)

    # adjust the maximum length of repr.. strings
    def _get_max_repr_len(self):
        return self._max_repr_len

    def _set_max_repr_len(self, max_repr):
        self._max_repr_len = max_repr

    max_repr_len = property(_get_max_repr_len, _set_max_repr_len)

    def _get_keep_ordering(self):
        return self._keep_ordering

    def _set_keep_ordering(self, ordered):  # pragma: no cover
        # INTERNAL
        self._keep_ordering = bool(ordered)
        self._set_term_dict_type(self._keep_ordering)
        new_term_dict_type = self._term_dict_type
        self._lfactory.set_ordering(ordered, new_term_dict_type)
        self._aggregator.set_ordering(ordered, new_term_dict_type)

    keep_ordering = property(_get_keep_ordering)


    @property
    def ignore_names(self):
        """ This property is used to ignore all names in the model.

         This flag indicates whether names are used or not.
         When set to True, all names are ignored. This could lead to performance 
         improvements when building large models.

         Note that changing this property only affects artefacts created -after- the flag
         has been set.
         
         By default, this flag is set to False.

         """
        return self._ignore_names

    @ignore_names.setter
    def ignore_names(self, deployed):
        self._ignore_names = bool(deployed)

    # compat
    @property
    def deploy(self):
        return self._ignore_names

    @deploy.setter
    def deploy(self, flag):
        self.ignore_names = flag

    @property
    def float_precision(self):
        """ This property is used to get or set the float precision of the model.

        The float precision is an integer number of digits, used
        in printing the solution and objective.
        This number of digits is used for variables and expressions which are not discrete.
        Discrete variables and objectives are printed with no decimal digits.

        """
        return self._float_precision

    @float_precision.setter
    def float_precision(self, nb_digits):
        used_digits = nb_digits
        if nb_digits < 0:
            self.warning("Negative float precision given: {0}, using 0 instead", nb_digits)
            used_digits = 0
        else:
            max_digits = self.environment.max_nb_digits
            bitness = self.environment.bitness
            if nb_digits > max_digits:
                self.warning("Given precision of {0:d} goes beyond {1:d}-bit capability, using maximum: {2:d}".
                             format(nb_digits, bitness, max_digits))
                used_digits = max_digits
        self._float_precision = used_digits
        # recompute float format
        self._float_meta_format = '{%%d:.%df}' % nb_digits


    @property
    def clean_before_solve(self):
        return self._clean_before_solve

    @clean_before_solve.setter
    def clean_before_solve(self, must_clean):
        self._clean_before_solve = bool(must_clean)


    # generate a new , unique counter, in the scope of this model
    def _new_unique_counter(self):
        # INTERNAL
        self._unique_counter += 1
        return self._unique_counter

    @property
    def time_limit_parameter(self):
        # INTERNAL
        return self.parameters.timelimit

    def get_time_limit(self):
        """
        Returns:
            The time limit for the model.

        """
        return self.time_limit_parameter.get()

    def set_time_limit(self, time_limit):
        """ Set a time limit for solve operations.

        Args:
            time_limit: The new time limit; must be a positive number.

        """
        self._checker.typecheck_num(time_limit)
        if time_limit < 0:
            self.fatal("Negative time limit: {0}", time_limit)
        elif time_limit < 1:
            self.warning("Time limit too small: {0} - using 1 instead", time_limit)
            time_limit = 1
        else:
            pass

        self.time_limit_parameter.set(time_limit)

    @property
    def solver_agent(self):
        return self.context.solver.agent

    @property
    def error_handler(self):
        return self._error_handler

    @property
    def logger(self):
        return self._error_handler

    @property
    def solution(self):
        """ This property returns the current solution of the model or None if the model has not yet been solved
        or if the last solve has failed.
        """
        return self._solution

    def _get_solution(self):
        # INTERNAL
        return self._solution

    def new_solution(self, var_value_dict=None, name=None):
        return self._lfactory.new_solution(var_value_dict, name)

    @property
    def solution_hook(self):
        """
        This property is used to get or set a solution hook.

        A solution hook is a function that takes a solution argument. This function is called
        wich each intermediate solution and also with the final solution. For LP models,
        this function is called only once and has a limited interest; whoever, for MIP prblems, the function
        is called for each and every intermediate solution.

        Note:
            The default type checker will try to call this function when setting it to the model, so if you areusing counters
            to keep track of solutions, either discount 1 for this dummy call, or disable type-checking entirely by
            adding the keyword argument checker='off' when creating the model.


        """
        return self._solution_hook

    @solution_hook.setter
    def solution_hook(self, sol_hook):
        self._checker.check_solution_hook(self, sol_hook)
        if not self.get_engine().has_cplex():
            self._solution_hook_warn_no_cplex()
        self._solution_hook = sol_hook

    def _solution_hook_warn_no_cplex(self):
        # INTERNAL
        self.warning('Solution hook requires a local CPLEX, is ignored on cloud.')

    @property
    def mip_starts(self):
        """ This property returns the list of MIP start solutions (a list of instances of :class:`docplex.mp.solution.SolveSolution`)
        attached to the model if MIP starts have been defined, possibly an empty list.
        """
        return self._mipstarts[:]

    def clear_mip_starts(self):
        """  Clears all MIP start solutions associated with the model.
        """
        self._mipstarts = []

    def remove_mip_start(self, mipstart):
        """ Removes one MIP start solution from the list of MIP starts associated with the model.

        Args:
            mipstart: A MIP start solution, an instance of :class:`docplex.mp.solution.SolveSolution`.
        """
        self._mipstarts.remove(mipstart)

    def fatal(self, msg, *args):
        self._error_handler.fatal(msg, args)

    def error(self, msg, *args):
        self._error_handler.error(msg, args)

    def warning(self, msg, *args):
        self._error_handler.warning(msg, args)

    def info(self, msg, *args):
        self._error_handler.info(msg, args)

    def trace(self, msg, *args):
        self.error_handler.trace(msg, args)

    def get_output_level(self):
        return self._error_handler.get_output_level()

    def set_output_level(self, new_output_level):
        self._error_handler.set_output_level(new_output_level)

    def set_quiet(self):
        self.error_handler.set_quiet()

    output_level = property(get_output_level, set_output_level)

    def set_log_output(self, out=None):
        self.context.solver.log_output = out
        outs = self.context.solver.log_output_as_stream
        self.__engine.notify_trace_output(outs)

    def get_log_output(self):
        return self.context.solver.log_output_as_stream

    log_output = property(get_log_output, set_log_output)

    def set_log_output_as_stream(self, outs):
        self.__engine.notify_trace_output(outs)

    def is_logged(self):
        return self.context.solver.log_output_as_stream is not None

    def clear(self):
        """ Clears the model of all modeling objects.
        """
        self._clear_internal()

    def _clear_internal(self, terminate=False):
        self.__allvars = []
        self.__allvarctns = []
        self.__vars_by_name = {}
        self.__allcts = []
        self.__cts_by_name = None
        self.__allpwlfuncs = []
        self._benders_annotations = None
        self._allkpis = []
        self.clear_kpis()
        self._last_solve_status = self._unknown_status
        self._solution = None
        self._mipstarts = []
        self._clear_scopes()
        self._allsos = []
        self._allpwl = []
        self._pwl_counter = {}
        if not terminate:
            self.set_objective(sense=self._lfactory.default_objective_sense(),
                               expr=self._new_default_objective_expr())

    def _clear_scopes(self):
        for a_scope in self._scopes:
            a_scope.reset()

    def set_checker(self, checker_key):
        # internal
        new_checker = get_typechecker(arg=checker_key, logger=self.error_handler)
        self._checker_key = checker_key
        self._checker = new_checker
        self._aggregator._checker = new_checker
        self._lfactory._checker = new_checker
        self._qfactory._checker = new_checker

    def _make_new_engine_from_agent(self, solver_agent, context):
        new_engine = self._engine_factory.new_engine(solver_agent, self.environment, model=self, context=context)
        new_engine.notify_trace_output(self.context.solver.log_output_as_stream)
        return new_engine

    def _set_engine(self, e2):
        self.__engine = e2
        self._lfactory.update_engine(e2)

    def _clear_engine(self):
        # INTERNAL
        old_engine = self.__engine
        if old_engine:
            # dispose of old engine.
            old_engine.end()
            # from Ryan
            del old_engine
            self.__engine = None


    def set_new_engine_from_agent(self, new_agent):
        self_context = self.context
        # set new agent
        if new_agent is None:
            new_agent = self_context.solver.agent
        elif is_string(new_agent):
            self_context.solver.agent = new_agent
        else:
            self.fatal('unexpected value for agent: {0!r}, expecting string or None', new_agent)
        new_engine = self._make_new_engine_from_agent(new_agent, self_context)
        self._set_engine(new_engine)

    def get_engine(self):
        # INTERNAL for testing
        return self.__engine

    def ensure_setup(self):
        # defined for compatibility with AbstractModel
        pass

    def print_information(self):
        """ Prints general informational statistics on the model.

        Prints the number of variables and their breakdown by type.
        Prints the number of constraints and their breakdown by type.

        """
        print("Model: %s" % self.name)
        self.get_statistics().print_information()

        # --- annotations
        self_anno = self._benders_annotations
        if self_anno:
            anno_stats = self.get_annotation_stats()
            print(" - annotations: {0}".format(len(self_anno)))
            print("   - {0}".format(', '.join('{0}: {1}'.format(cpx.descr, anno_stats[cpx]) for cpx in CplexScope if cpx in anno_stats)))


        # --- parameters
        self_params = self.context._get_raw_cplex_parameters()
        if self_params and self_params.has_nondefaults():
            print(" - parameters:")
            self_params.print_information(indent_level=5)  # 3 for " - " + 2 = 5
        else:
            print(" - parameters: defaults")

    def _is_empty(self):
        # INTERNAL
        return 0 == self.number_of_constraints and 0 == self.number_of_variables

    @property
    def number_of_linear_expr_instances(self):  # pragma: no cover
        # INTERNAL
        return self._linexpr_instance_counter

    # @profile
    def __notify_new_model_object(self, descr,
                                  mobj, mindex, mobj_name,
                                  name_dir, idx_scope,
                                  is_name_safe=False):
        """
        Notifies the return af an object being create on the engine.
        :param descr: A string describing the type of the object being created (e.g. Constraint, Variable).
        :param mobj:  The newly created modeling object.
        :param mindex: The index as returned by the engine.
        :param name_dir: The directory of objects by name (e.g. name -> constraitn directory).
        :param idx_scope:  The index scope
        """
        mobj.set_index(mindex)

        if name_dir is not None:
            mobj_name = mobj_name or mobj.get_name()
            if mobj_name:
                # in some cases, names are checked before register
                if not is_name_safe:
                    if mobj_name in name_dir:
                        old_name_value = name_dir[mobj_name]
                        # Duplicate constraint name: foo
                        self.warning("Duplicate {0} name: {1} already used for {2!r}", descr, mobj_name, old_name_value)

                name_dir[mobj_name] = mobj

        # store in idx dir if any
        if idx_scope:
            idx_scope.notify_obj_index(mobj, mindex)

    def _register_one_var(self, var, var_index, var_name):
        self.__notify_new_var(var, var_index, var_name)
        self.__allvars.append(var)

    # @profile
    def _register_block_vars(self, allvars, indices, allnames):
        varname_dict = self.__vars_by_name
        if allnames:
            for var, var_index, var_name in izip(allvars, indices, allnames):
                var._index = var_index
                if var_name:
                    if var_name in varname_dict:
                        old_name_value = varname_dict[var_name]
                        # Duplicate constraint name: foo
                        self.warning("Duplicate variable name: {0} already used for {1!s}", var_name, old_name_value)
                    varname_dict[var_name] = var
        else:
            for var, var_index in izip(allvars, indices):
                var._index = var_index
        self.__allvars.extend(allvars)
        # update variable scope once
        self._var_scope.notify_obj_indices(objs=allvars, indices=indices)

    def __notify_new_var(self, var, var_index, var_name):
        self.__notify_new_model_object("variable", var, var_index, var_name, self.__vars_by_name, self._var_scope)

    def _register_one_constraint(self, ct, ct_index, is_ctname_safe=False):
        """
        INTERNAL
        :param ct: The new constraint to register.
        :param ct_index: The index as returned by the engine.
        :param is_ctname_safe: True if ct name has been checked for duplicates already.
        :return:
        """
        scope = ct._get_index_scope()

        self.__notify_new_model_object(
            "constraint", ct, ct_index, None,
            self.__cts_by_name, scope,
            is_name_safe=is_ctname_safe)

        self.__allcts.append(ct)

    def _ensure_cts_name_dir(self):
        # INTERNAL: make sure the constraint name dir is present.
        if self.__cts_by_name is None:
            self.__cts_by_name = {ct.get_name(): ct for ct in self.iter_constraints() if ct.has_user_name()}
        return self.__cts_by_name

    def _register_block_cts(self, scope, cts, indices):
        # INTERNAL: assert len(cts) == len(indices)
        ct_name_map = self.__cts_by_name
        # --
        if ct_name_map:
            for ct, ct_index in izip(cts, indices):
                ct.set_index(ct_index)
                ct_name = ct.get_name()
                if ct_name:
                    ct_name_map[ct_name] = ct
        else:
            for ct, ct_index in izip(cts, indices):
                ct._index = ct_index

        scope.notify_obj_indices(cts, indices)
        # extend container faster than append()
        self.__allcts.extend(cts)

    def _register_implicit_equivalence_ct(self, eqct):
        self.__all_implicit_equiv_cts.append(eqct)

    def iter_implicit_equivalence_cts(self):
        return iter(self.__all_implicit_equiv_cts)

    # iterators
    def iter_var_containers(self):
        # INTERNAL
        return iter(self.__allvarctns)

    def _add_var_container(self, ctn):
        # INTERNAL
        self.__allvarctns.append(ctn)

    def _is_binary_var(self, dvar):
        return dvar._vartype.get_cplex_typecode() == 'B'

    def _is_integer_var(self, dvar):
        return dvar._vartype.get_cplex_typecode() == 'I'

    def _is_continuous_var(self, dvar):
        return dvar._vartype.get_cplex_typecode() == 'C'

    def _is_semicontinuous_var(self, dvar):
        return dvar._vartype.get_cplex_typecode() == 'S'

    def _is_semiinteger_var(self, dvar):
        return dvar._vartype.get_cplex_typecode() == 'N'


    def _count_variables_w_code(self, cpxcode):
        cnt = 0
        for v in self.iter_variables():
            if v._vartype.get_cplex_typecode() == cpxcode:
                cnt += 1
        return cnt

    def _iter_variables_w_code(self, cpxcode):
        for v in self.iter_variables():
            if v._vartype.get_cplex_typecode() == cpxcode:
                yield v

    @property
    def number_of_variables(self):
        """ This property returns the total number of decision variables, all types combined.

        """
        return len(self.__allvars)

    @property
    def number_of_binary_variables(self):
        """ This property returns the total number of binary decision variables added to the model.
        """
        return self._count_variables_w_code('B')

    @property
    def number_of_integer_variables(self):
        """ This property returns the total number of integer decision variables added to the model.
        """
        return self._count_variables_w_code('I')

    @property
    def number_of_continuous_variables(self):
        """ This property returns the total number of continuous decision variables added to the model.
        """
        return self._count_variables_w_code('C')

    @property
    def number_of_semicontinuous_variables(self):
        """ This property returns the total number of semi-continuous decision variables added to the model.
        """
        return self._count_variables_w_code('S')

    @property
    def number_of_semiinteger_variables(self):
        """ This property returns the total number of semi-integer decision variables added to the model.
        """
        return self._count_variables_w_code('N')

    def _has_discrete_var(self):
        # INTERNAL
        for v in self.iter_variables():
            if v.is_discrete():
                return True
        else:
            return False

    def _solves_as_mip(self):
        # INTERNAL: will the model solve as a MIP?
        # returns TRue if the model contains a discrete variable or if it has SOS or
        #  if it has Piecewise Linear constraints
        return self._has_discrete_var() or self._allsos or self._pwl_counter or self.number_of_semicontinuous_variables

    def get_statistics(self):
        """ Returns statistics on the model.

        :returns: A new instance of :class:`docplex.mp.model_stats.ModelStatistics`.
        """
        return ModelStatistics._make_new_stats(self)

    statistics = property(get_statistics)

    def iter_pwl_functions(self):
        """ Iterates over all the piecewise linear functions in the model.

        Returns the PWL functions in the order they were added to the model.

        Returns:
            An iterator object.
        """
        return iter(self.__allpwlfuncs)

    def iter_variables(self):
        """ Iterates over all the variables in the model.

        Returns the variables in the order they were added to the model,
        regardless of their type.

        Returns:
            An iterator object.
        """
        return iter(self.__allvars)

    def iter_binary_vars(self):
        """ Iterates over all binary decision variables in the model.

        Returns the variables in the order they were added to the model.

        Returns:
            An iterator object.
        """
        return self._iter_variables_w_code('B')

    def iter_integer_vars(self):
        """ Iterates over all integer decision variables in the model.

        Returns the variables in the order they were added to the model.

        Returns:
            An iterator object.
        """
        return self._iter_variables_w_code('I')

    def iter_continuous_vars(self):
        """ Iterates over all continuous decision variables in the model.

        Returns the variables in the order they were added to the model.

        Returns:
            An iterator object.
        """
        return self._iter_variables_w_code('C')

    def iter_semicontinuous_vars(self):
        """ Iterates over all semi-continuous decision variables in the model.

        Returns the variables in the order they were added to the model.

        Returns:
            An iterator object.
        """
        return self._iter_variables_w_code('S')

    def iter_semiinteger_vars(self):
        """ Iterates over all semi-integer decision variables in the model.

        Returns the variables in the order they were added to the model.

        Returns:
            An iterator object.
        """
        return self._iter_variables_w_code('N')


    def get_var_by_name(self, name):
        """ Searches for a variable from a name.

        Returns a variable if it finds one with exactly this name, or None.

        Args:
            name (str): The name of the variable being searched for.

        :returns: A variable (instance of :class:`docplex.mp.linear.Var`) or None.
        """
        return self.__vars_by_name.get(name, None)

    def _get_non_ambiguous_varname(self, base_name):
        # INTERNAL
        varname_map = self.__vars_by_name
        if varname_map is not None and base_name in varname_map:
            return '{0}#{1}'.format(base_name, len(self.__allvars))
        else:
            return base_name

    def find_matching_vars(self, pattern, match_case=False):
        key_pattern = pattern if match_case else pattern.lower()
        matches = []
        for dv in self.iter_variables():
            dvname = dv.name
            if dvname:
                if match_case:
                    if key_pattern in dvname:
                        matches.append(dv)
                else:
                    if key_pattern in dvname.lower():
                        matches.append(dv)
        return matches

    # index management
    def _build_index_dict(self, mobj_it, raise_on_invalid_index=False):
        #  INTERNAL
        idx_dict = {}
        for mobj in mobj_it:
            if mobj.has_valid_index():
                idx_dict[mobj.index] = mobj
            elif raise_on_invalid_index:  # pragma: no cover
                self.fatal("Object has invalid index: {0!s}", mobj)  # pragma: no cover
        return idx_dict

    def get_var_by_index(self, idx):
        # INTERNAL
        self._checker.typecheck_valid_index(idx)
        return self._var_scope.get_object_by_index(idx)

    def _var_by_index(self, idx):
        # INTERNAL
        return self._var_scope.get_object_by_index(idx)

    def set_var_type(self, dvar, new_vartype):
        # INTERNAL
        self._checker.typecheck_vartype(new_vartype)
        if new_vartype != dvar.vartype:
            self.__engine.set_var_type(dvar, new_vartype)
            # change type in the Var object.
            dvar._set_vartype_internal(new_vartype)
        return self

    def set_var_name(self, dvar, new_name):
        # INTERNAL: use var.name to set variable names
        if new_name != dvar.name:
            self.__engine.rename_var(dvar, new_name)
            dvar._set_name(new_name)

    def set_var_lb(self, var, candidate_lb):
        # INTERNAL: use var.lb to set lb
        new_lb = var.vartype.resolve_lb(candidate_lb, self)
        self.__engine.set_var_lb(var, new_lb)
        var._internal_set_lb(new_lb)

    def set_var_ub(self, var, candidate_ub):
        # INTERNAL: use var.ub to set ub
        new_ub = var.vartype.resolve_ub(candidate_ub, self)
        self.__engine.set_var_ub(var, new_ub)
        var._internal_set_ub(new_ub)

    def get_constraint_by_name(self, name):
        """ Searches for a constraint from a name.

        Returns the constraint if it finds a constraint with exactly this name, or None
        if no constraint has this name.

        This function will not raise an exception if the named constraint is not found.

        Args:
            name (str): The name of the constraint being searched for.

        Returns:
            A constraint or None.
        """
        return self._ensure_cts_name_dir().get(name)

    def get_constraint_by_index(self, idx):
        # INTERNAL
        self._checker.typecheck_valid_index(idx)
        return self._linct_scope.get_object_by_index(idx)

    def get_indicator_by_index(self, idx):
        self._checker.typecheck_valid_index(idx)
        return self._indicator_scope.get_object_by_index(idx)

    def get_quadratic_by_index(self, idx):
        # INTERNAL
        self._checker.typecheck_valid_index(idx)
        return self._quadct_scope.get_object_by_index(idx)

    @property
    def number_of_constraints(self):
        """ This property returns the total number of constraints that were added to the model.

        The number includes linear constraints, range constraints, and indicator constraints.
        """
        return len(self.__allcts)

    def iter_constraints(self):
        """ Iterates over all constraints (linear, ranges, indicators).

        Returns:
          An iterator object over all constraints in the model.
        """
        return iter(self.__allcts)

    def _count_constraints_with_type(self, cttype):
        return self._count_constraints_filtered(lambda ct: isinstance(ct, cttype))

    def _count_constraints_filtered(self, ct_filter):
        return sum(1 for _ in filter(ct_filter, self.__allcts))

    def gen_constraints_with_type(self, cttype):
        for ct in self.__allcts:
            if isinstance(ct, cttype):
                yield ct

    @property
    def number_of_range_constraints(self):
        """ This property returns the total number of range constraints added to the model.

        """
        return self._count_constraints_with_type(RangeConstraint)

    @property
    def number_of_linear_constraints(self):
        """ This property returns the total number of linear constraints added to the model.

        This counts binary linear constraints (<=, >=, or ==) and
        range constraints.

        See Also:
            :func:`number_of_range_constraints`

        """
        return self._count_constraints_filtered(lambda c: c.is_linear())

    def iter_range_constraints(self):
        return self.gen_constraints_with_type(RangeConstraint)

    def iter_binary_constraints(self):
        return self.gen_constraints_with_type(LinearConstraint)

    def iter_linear_constraints(self):
        """
        Returns an iterator on the linear constraints of the model.
        This includes binary linear constraints and ranges but not indicators.

        Returns:
            An iterator object.
        """
        for c in self.iter_constraints():
            if c.is_linear():
                yield c

    def iter_indicator_constraints(self):
        """ Returns an iterator on indicator constraints in the model.

        Returns:
            An iterator object.
        """
        return self.gen_constraints_with_type(IndicatorConstraint)

    @property
    def number_of_indicator_constraints(self):
        """ This property returns the number of indicator constraints in the model.
        """
        return self._count_constraints_with_type(IndicatorConstraint)

    @property
    def number_of_equivalence_constraints(self):
        """ This property returns the number of equivalence constraints in the model.
        """
        explicits = self._count_constraints_with_type(EquivalenceConstraint)
        implicits = len(self.__all_implicit_equiv_cts)
        return explicits + implicits


    def iter_quadratic_constraints(self):
        """
        Returns an iterator on the quadratic constraints of the model.

        Returns:
            An iterator object.
        """
        return self.gen_constraints_with_type(QuadraticConstraint)

    @property
    def number_of_quadratic_constraints(self):
        """ This property returns the number of quadratic constraints in the model.
        """
        return self._count_constraints_with_type(QuadraticConstraint)

    def has_quadratic_constraint(self):
        return any(c.is_quadratic() for c in self.__allcts)

    def iter_logical_constraints(self, include_implicits=False):
        for ct in self.iter_constraints():
            if ct.is_logical():
                yield ct
        if include_implicits:
            for imp_eqct in self.iter_implicit_equivalence_cts():
                yield imp_eqct


    def var(self, vartype, lb=None, ub=None, name=None):
        """ Creates a decision variable and stores it in the model.

        Args:
            vartype: The type of the decision variable;
                This field expects a concrete instance of the abstract class
                :class:`docplex.mp.vartype.VarType`.

            lb: The lower bound of the variable; either a number or None, to use the default.
                 The default lower bound for all three variable types is 0.

            ub: The upper bound of the variable domain; expects either a number or None to use the type's default.
                The default upper bound for Binary is 1, otherwise positive infinity.

            name: An optional string to name the variable.

        :returns: The newly created decision variable.
        :rtype: :class:`docplex.mp.linear.Var`

        Note:
            The model holds local instances of BinaryVarType, IntegerVarType, ContinuousVarType which
            are accessible by properties (resp. binary_vartype, integer_vartype, continuous_vartype).

        See Also:
            :func:`infinity`,
            :func:`binary_vartype`,
            :func:`integer_vartype`,
            :func:`continuous_vartype`

        """
        self._checker.typecheck_vartype(vartype)
        return self._var(vartype, lb, ub, name)

    def _var(self, vartype, lb=None, ub=None, name=None):
        # INTERNAL
        if lb is not None:
            self._checker.typecheck_num(lb, caller='Var.lb')
        if ub is not None:
            self._checker.typecheck_num(ub, caller='Var.ub')
        return self._lfactory.new_var(vartype, lb, ub, name)

    def continuous_var(self, lb=None, ub=None, name=None):
        """ Creates a new continuous decision variable and stores it in the model.

        Args:
            lb: The lower bound of the variable, or None. The default is 0.
            ub: The upper bound of the variable, or None, to use the default. The default is model infinity.
            name (string): An optional name for the variable.

        :returns: A decision variable with type :class:`docplex.mp.vartype.ContinuousVarType`.
        :rtype: :class:`docplex.mp.linear.Var`
        """
        return self._var(self.continuous_vartype, lb, ub, name)

    def integer_var(self, lb=None, ub=None, name=None):
        """ Creates a new integer variable and stores it in the model.

        Args:
            lb: The lower bound of the variable, or None. The default is 0.
            ub: The upper bound of the variable, or None, to use the default. The default is model infinity.
            name: An optional name for the variable.

        :returns: An instance of the :class:`docplex.mp.linear.Var` class with type `IntegerVarType`.
        :rtype: :class:`docplex.mp.linear.Var`
        """
        return self._var(self.integer_vartype, lb, ub, name)

    def binary_var(self, name=None):
        """ Creates a new binary decision variable and stores it in the model.

        Args:
            name (string): An optional name for the variable.

        :returns: A decision variable with type :class:`docplex.mp.vartype.BinaryVarType`.
        :rtype: :class:`docplex.mp.linear.Var`
        """
        return self._var(self.binary_vartype, name=name)

    def semicontinuous_var(self, lb, ub=None, name=None):
        """ Creates a new semi-continuous decision variable and stores it in the model.

        Args:
            lb: The lower bound of the variable  (which must be strictly positive).
            ub: The upper bound of the variable, or None, to use the default. The default is model infinity.
            name (string): An optional name for the variable.

        :returns: A decision variable with type :class:`docplex.mp.vartype.SemiContinuousVarType`.
        :rtype: :class:`docplex.mp.linear.Var`
        """
        self._checker.typecheck_num(lb)  # lb cannot be None
        return self._var(self.semicontinuous_vartype, lb, ub, name)

    def semiinteger_var(self, lb, ub=None, name=None):
        """ Creates a new semi-integer decision variable and stores it in the model.

        Args:
            lb: The lower bound of the variable (which must be strictly positive).
            ub: The upper bound of the variable, or None, to use the default. The default is model infinity.
            name (string): An optional name for the variable.

        :returns: A decision variable with type :class:`docplex.mp.vartype.SemiIntegerVarType`.
        :rtype: :class:`docplex.mp.linear.Var`
        """
        self._checker.typecheck_num(lb)  # lb cannot be None
        return self._var(self.semiinteger_vartype, lb, ub, name)


    def var_list(self, keys, vartype, lb=None, ub=None, name=str, key_format=None):
        self._checker.typecheck_vartype(vartype)
        return self._lfactory.var_list(keys, vartype, lb, ub, name, key_format)

    def var_dict(self, keys, vartype, lb=None, ub=None, name=str, key_format=None):
        self._checker.typecheck_vartype(vartype)
        return self._var_dict(keys, vartype, lb, ub, name, key_format)

    def _var_dict(self, keys, vartype, lb=None, ub=None, name=str, key_format=None):
        # INTERNAL
        actual_name, key_seq = self._lfactory.make_key_seq(keys, name)
        ctn = self._lfactory._new_var_container(vartype, key_list=[key_seq], lb=lb, ub=ub, name=name)
        var_list = self._lfactory.new_var_list(ctn, key_seq, vartype, lb, ub, actual_name, 1, key_format)
        _dict_type = OrderedDict if self._keep_ordering else dict
        return _dict_type(izip(key_seq, var_list))

    def binary_var_list(self, keys, lb=None, ub=None, name=str, key_format=None):
        """ Creates a list of binary decision variables and stores them in the model.

        Args:
            keys: Either a sequence of objects or an integer.
            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if keys is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...
                        
        Example:
            If you want each key string to be surrounded by {}, use a special key_format: "_{%s}",
            the %s denotes where the key string will be formatted and appended to `name`.

        :returns: A list of :class:`docplex.mp.linear.Var` objects with type :class:`doc.mp.vartype.BinaryVarType`.

        Example:
            `mdl.binary_var_list(3, "z")` returns a list of size 3
            containing three binary decision variables with names `z_0`, `z_1`, `z_2`.

        """
        return self.var_list(keys, self.binary_vartype, name=name, lb=lb, ub=ub, key_format=key_format)

    def integer_var_list(self, keys, lb=None, ub=None, name=str, key_format=None):
        """ Creates a list of integer decision variables with type `IntegerVarType`, stores them in the model,
        and returns the list.

        Args:
            keys: Either a sequence of objects or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.
            lb: Lower bounds of the variables. Accepts either a floating-point number,
                a list of numbers with the same size as keys,
                a function (which will be called on each key argument), or None.
            ub: Upper bounds of the variables.  Accepts either a floating-point number,
                a list of numbers with the same size as keys,
                a function (which will  be called on each key argument), or None.
            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
            key_format: A format string or None. This format string describes how keys contribute to variable names.
                The default is "_%s". For example if name is "x" and each key object is represented by a string
                like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...

        Note:
            Using None as the lower bound means the default lower bound (0) is used.
            Using None as the upper bound means the default upper bound (the model's positive infinity)
            is used.

        :returns: A list of :class:`doc.mp.linear.Var` objects with type `IntegerVarType`.

        """
        return self.var_list(keys, self.integer_vartype, lb, ub, name, key_format)

    def continuous_var_list(self, keys, lb=None, ub=None, name=str, key_format=None):
        """
        Creates a list of continuous decision variables with type :class:`docplex.mp.vartype.ContinuousVarType`,
        stores them in the model, and returns the list.

        Args:
            keys: Either a sequence of objects or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.

            lb: Lower bounds of the variables. Accepts either a floating-point number,
                a list of numbers, a function, or None.
                Use a number if all variables share the same lower bound.
                Otherwise either use an explicit list of numbers
                or use a function if lower bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means using the default lower bound (0) is used.

            ub: Upper bounds of the variables. Accepts either a floating-point number,
                a list of numbers, a function, or None.
                Use a number if all variables share the same upper bound.
                Otherwise either use an explicit list of numbers
                or use a function if upper bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default upper bound (model infinity) is used.

            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
                If passed a function, this function is called on each key object to generate a name.
                The default behavior is to call :func:`str()` on each key object.

            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...
        Note:
            When `keys` is either an empty list or the integer 0, an empty list is returned.


        :returns: A list of :class:`docplex.mp.linear.Var` objects with type :class:`docplex.mp.vartype.ContinuousVarType`.

        See Also:
            :func:`infinity`

        """
        return self.var_list(keys, self.continuous_vartype, lb, ub, name, key_format)

    def semicontinuous_var_list(self, keys, lb, ub=None, name=str, key_format=None):
        """
        Creates a list of semi-continuous decision variables with type :class:`docplex.mp.vartype.SemiContinuousVarType`,
        stores them in the model, and returns the list.

        Args:
            keys: Either a sequence of objects or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.

            lb: Lower bounds of the variables. Accepts either a floating-point number,
                a list of numbers, or a function.
                Use a number if all variables share the same lower bound.
                Otherwise either use an explicit list of numbers or
                use a function if lower bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                Note that the lower bound of a semi-continuous variable must be strictly positive.

            ub: Upper bounds of the variables. Accepts either a floating-point number,
                a list of numbers, a function, or None.
                Use a number if all variables share the same upper bound.
                Otherwise either use an explicit list of numbers or
                use a function if upper bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default upper bound (model infinity) is used.

            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
                If passed a function, this function is called on each key object to generate a name.
                The default behavior is to call :func:`str()` on each key object.

            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...
        Note:
            When `keys` is either an empty list or the integer 0, an empty list is returned.


        :returns: A list of :class:`docplex.mp.linear.Var` objects with type :class:`docplex.mp.vartype.SemiContinuousVarType`.

        See Also:
            :func:`infinity`

        """
        return self.var_list(keys, self.semicontinuous_vartype, lb, ub, name, key_format)

    def semiinteger_var_list(self, keys, lb, ub=None, name=str, key_format=None):
        """
        Creates a list of semi-integer decision variables with type :class:`docplex.mp.vartype.SemiIntegerVarType`,
        stores them in the model, and returns the list.

        Args:
            keys: Either a sequence of objects or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.

            lb: Lower bounds of the variables. Accepts either a floating-point number,
                a list of numbers, or a function.
                Use a number if all variables share the same lower bound.
                Otherwise either use an explicit list of numbers or
                use a function if lower bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                Note that the lower bound of a semi-integer variable must be strictly positive.

            ub: Upper bounds of the variables. Accepts either a floating-point number,
                a list of numbers, a function, or None.
                Use a number if all variables share the same upper bound.
                Otherwise either use an explicit list of numbers or
                use a function if upper bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default upper bound (model infinity) is used.

            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
                If passed a function, this function is called on each key object to generate a name.
                The default behavior is to call :func:`str()` on each key object.

            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...
        Note:
            When `keys` is either an empty list or the integer 0, an empty list is returned.


        :returns: A list of :class:`docplex.mp.linear.Var` objects with type :class:`docplex.mp.vartype.SemiIntegerVarType`.

        See Also:
            :func:`infinity`

        """
        return self.var_list(keys, self.semiinteger_vartype, lb, ub, name, key_format)

    def continuous_var_dict(self, keys, lb=None, ub=None, name=None, key_format=None):
        """ Creates a dictionary of continuous decision variables, indexed by key objects.

        Creates a dictionary that allows retrieval of variables from business
        model objects. Keys can be either a Python collection, an iterator, or a generator.

        A key can be any Python object, with the exception of None.
        Keys are used in the naming of variables.

        Note:
            If `keys` is empty, this method returns an empty dictionary.

        Args:
            keys: Either a sequence of objects, an iterator, or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.

            lb: Lower bounds of the variables. Accepts either a floating-point number,
                a list of numbers, or a function.
                Use a number if all variables share the same lower bound.
                Otherwise either use an explicit list of numbers or
                use a function if lower bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default lower bound (0) is used.

            ub: Upper bounds of the variables. Accepts either a floating-point number,
                a list of numbers, a function, or None.
                Use a number if all variables share the same upper bound.
                Otherwise either use an explicit list of numbers or
                use a function if upper bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default upper bound (model infinity) is used.

            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
                If passed a function, this function is called on each key object to generate a name.
                The default behavior is to call :func:`str` on each key object.

            key_format: A format string or None. This format string describes how `keys` contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...

        :returns: A dictionary of :class:`docplex.mp.linear.Var` objects (with type `ContinuousVarType`) indexed by
                  the objects in `keys`.

        See Also:
            :class:`docplex.mp.linear.Var`,
            :func:`infinity`
        """
        return self._var_dict(keys, self.continuous_vartype, lb=lb, ub=ub, name=name, key_format=key_format)

    def integer_var_dict(self, keys, lb=None, ub=None, name=None, key_format=None):
        """  Creates a dictionary of integer decision variables, indexed by key objects.

        Creates a dictionary that allows retrieval of variables from business
        model objects. Keys can be either a Python collection, an iterator, or a generator.

        A key can be any Python object, with the exception of None.
        Keys are used in the naming of variables.

        Note:
            If `keys` is empty, this method returns an empty dictionary.

        Args:
            keys: Either a sequence of objects, an iterator, or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.

            lb: Lower bounds of the variables. Accepts either a floating-point number,
                a list of numbers, or a function.
                Use a number if all variables share the same lower bound.
                Otherwise either use an explicit list of numbers or
                use a function if lower bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default lower bound (0) is used.

            ub: Upper bounds of the variables. Accepts either a floating-point number,
                a list of numbers, a function, or None.
                Use a number if all variables share the same upper bound.
                Otherwise either use an explicit list of numbers or
                use a function if upper bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default upper bound (model infinity) is used.


            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
                If passed a function, this function is called on each key object to generate a name.
                The default behavior is to call :func:`str` on each key object.

            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...

        :returns:  A dictionary of :class:`docplex.mp.linear.Var` objects (with type `IntegerVarType`) indexed by the
                   objects in `keys`.

        See Also:
            :func:`infinity`
        """
        return self._var_dict(keys, self.integer_vartype, lb=lb, ub=ub, name=name, key_format=key_format)

    def binary_var_dict(self, keys, lb=None, ub=None, name=None, key_format=None):
        """ Creates a dictionary of binary decision variables, indexed by key objects.

        Creates a dictionary that allows retrieval of variables from business
        model objects. Keys can be either a Python collection, an iterator, or a generator.

        A key can be any Python object, with the exception of None.
        Keys are used in the naming of variables.

        Note:
            If `keys` is empty, this method returns an empty dictionary.

        Args:
            keys: Either a sequence of objects, an iterator, or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.

            name (string): Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.

            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...

        :returns: A dictionary of :class:`docplex.mp.linear.Var` objects with type
                  :class:`docplex.mp.vartype.BinaryVarType` indexed by the objects in `keys`.
        """
        return self._var_dict(keys, self.binary_vartype, lb=lb, ub=ub, name=name, key_format=key_format)

    def semiinteger_var_dict(self, keys, lb, ub=None, name=str, key_format=None):
        """  Creates a dictionary of semi-integer decision variables, indexed by key objects.

        Creates a dictionary that allows retrieval of variables from business
        model objects. Keys can be either a Python collection, an iterator, or a generator.

        A key can be any Python object, with the exception of None.
        Keys are used in the naming of variables.

        Note:
            If `keys` is empty, this method returns an empty dictionary.

        Args:
            keys: Either a sequence of objects, an iterator, or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.

            lb: Lower bounds of the variables. Accepts either a floating-point number,
                a list of numbers, or a function.
                Use a number if all variables share the same lower bound.
                Otherwise either use an explicit list of numbers or
                use a function if lower bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.

            ub: Upper bounds of the variables. Accepts either a floating-point number,
                a list of numbers, a function, or None.
                Use a number if all variables share the same upper bound.
                Otherwise either use an explicit list of numbers or
                use a function if upper bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default upper bound (model infinity) is used.


            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
                If passed a function, this function is called on each key object to generate a name.
                The default behavior is to call :func:`str` on each key object.

            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...

        :returns:  A dictionary of :class:`docplex.mp.linear.Var` objects (with type `SemiIntegerVarType`) indexed by the
                   objects in `keys`.

        See Also:
            :func:`infinity`
        """
        return self._var_dict(keys, self.semiinteger_vartype, lb, ub, name, key_format)

    def semicontinuous_var_dict(self, keys, lb, ub=None, name=str, key_format=None):
        """  Creates a dictionary of semi-continuous decision variables, indexed by key objects.

        Creates a dictionary that allows retrieval of variables from business
        model objects. Keys can be either a Python collection, an iterator, or a generator.

        A key can be any Python object, with the exception of None.
        Keys are used in the naming of variables.

        Note:
            If `keys` is empty, this method returns an empty dictionary.

        Args:
            keys: Either a sequence of objects, an iterator, or a positive integer. If passed an integer,
                it is interpreted as the number of variables to create.

            lb: Lower bounds of the variables. Accepts either a floating-point number,
                a list of numbers, or a function.
                Use a number if all variables share the same lower bound.
                Otherwise either use an explicit list of numbers or
                use a function if lower bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.

            ub: Upper bounds of the variables. Accepts either a floating-point number,
                a list of numbers, a function, or None.
                Use a number if all variables share the same upper bound.
                Otherwise either use an explicit list of numbers or
                use a function if upper bounds vary depending on the key, in which case,
                the function will be called on each `key` in `keys`.
                None means the default upper bound (model infinity) is used.


            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if `keys` is a sequence) or the
                index of the variable within the range, if an integer argument is passed.
                If passed a function, this function is called on each key object to generate a name.
                The default behavior is to call :func:`str` on each key object.

            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...

        :returns:  A dictionary of :class:`docplex.mp.linear.Var` objects (with type `SemiIntegerVarType`) indexed by the
                   objects in `keys`.

        See Also:
            :func:`infinity`
        """
        return self._var_dict(keys, self.semicontinuous_vartype, lb, ub, name, key_format)

    def var_multidict(self, vartype, seq_of_key_seqs, lb=None, ub=None, name=None, key_format=None):
        # INTERNAL
        self._checker.typecheck_vartype(vartype)
        self._checker.typecheck_iterable(seq_of_key_seqs)
        # ---
        fixed_keys = [self._lfactory.make_key_seq(ks, name)[1] for ks in seq_of_key_seqs]
        # the sequence of keysets should answer to len(no generators here)
        dimension = len(fixed_keys)
        if dimension < 1:
            self.fatal("len of key sequence must be >= 1, got: {0}", dimension)  # pragma: no cover

        # create cartesian product of keys...
        all_key_tuples = list(product(*fixed_keys))
        # check empty list
        if not all_key_tuples:
            self.fatal('multidict has no keys to index the variables')

        ctn = self._lfactory._new_var_container(vartype, key_list=all_key_tuples, lb=lb, ub=ub, name=name)

        cube_vars = self._lfactory.new_var_list(ctn, all_key_tuples, vartype, lb, ub, name, dimension, key_format)

        _dict_type = OrderedDict if self._keep_ordering else dict
        var_dict = _dict_type(izip(all_key_tuples, cube_vars))
        return var_dict

    def var_matrix(self, vartype, keys1, keys2, lb=None, ub=None, name=None, key_format=None):
        return self.var_multidict(vartype, seq_of_key_seqs=[keys1, keys2],
                                  lb=lb, ub=ub, name=name, key_format=key_format)

    def binary_var_matrix(self, keys1, keys2, name=None, key_format=None):
        """ Creates a dictionary of binary decision variables, indexed by pairs of key objects.

        Creates a dictionary that allows the retrieval of variables from  a tuple
        of two keys, the first one from `keys1`, the second one from `keys2`.
        In short, variables are indexed by the Cartesian product of the two key sets.

        A key can be any Python object, with the exception of None.
        Keys are used in the naming of variables.

        Note:
            If either of `keys1` or `keys2` is empty, this method returns an empty dictionary.

        Args:
            keys1: Either a sequence of objects, an iterator, or a positive integer. If passed an integer N,
                    it is interpreted as a range from 0 to N-1.

            keys2: Same as `keys1`.

            name: Used to name variables. Accepts either a string or
                a function. If given a string, the variable name is formed by appending the string
                to the string representation of the key object (if keys is a sequence) or the
                index of the variable within the range, if an integer argument is passed.

            key_format: A format string or None. This format string describes how keys contribute to variable names.
                        The default is "_%s". For example if name is "x" and each key object is represented by a string
                        like "k1", "k2", ... then variables will be named "x_k1", "x_k2",...

        :returns: A dictionary of :class:`docplex.mp.linear.Var` objects with type
                  :class:`docplex.mp.vartype.BinaryVarType` indexed by
                  all couples `(k1, k2)` with `k1` in `keys1` and `k2` in `keys2`.
        """
        return self.var_multidict(self.binary_vartype, [keys1, keys2], 0, 1, name=name, key_format=key_format)

    def integer_var_matrix(self, keys1, keys2, lb=None, ub=None, name=None, key_format=None):
        """ Creates a dictionary of integer decision variables, indexed by pairs of key objects.

        Creates a dictionary that allows the retrieval of variables from  a tuple
        of two keys, the first one from `keys1`, the second one from `keys2`.
        In short, variables are indexed by the Cartesian product of the two key sets.

        A key can be any Python object, with the exception of None.

        Arguments `lb`, `ub`, `name`, and `key_format` are interpreted as in :func:`integer_var_dict`.
        """

        return self.var_multidict(self.integer_vartype, [keys1, keys2], lb, ub, name, key_format)

    def continuous_var_matrix(self, keys1, keys2, lb=None, ub=None, name=None, key_format=None):
        """ Creates a dictionary of continuous decision variables, indexed by pairs of key objects.

        Creates a dictionary that allows retrieval of variables from a tuple
        of two keys, the first one from `keys1`, the second one from `keys2`.
        In short, variables are indexed by the Cartesian product of the two key sets.

        A key can be any Python object, with the exception of None.

        Arguments `lb`, `ub`, `name`, and `key_format` are interpreted the same as in :func:`integer_var_dict`.

        """
        return self.var_multidict(self.continuous_vartype, [keys1, keys2], lb, ub, name, key_format)

    def continuous_var_cube(self, keys1, keys2, keys3, lb=None, ub=None, name=None, key_format=None):
        """ Creates a dictionary of continuous decision variables, indexed by triplets of key objects.

        Same as :func:`continuous_var_matrix`, except that variables are indexed by triplets of
        the form `(k1, k2, k3)` with `k1` in `keys1`, `k2` in `keys2`, `k3` in `keys3`.
        """
        return self.var_multidict(self.continuous_vartype, [keys1, keys2, keys3], lb, ub, name, key_format)

    def integer_var_cube(self, keys1, keys2, keys3, lb=None, ub=None, name=str):
        """ Creates a dictionary of integer decision variables, indexed by triplets.

        Same as :func:`integer_var_matrix`, except that variables are indexed by triplets of
        the form `(k1, k2, k3)` with `k1` in `keys1`, `k2` in `keys2`, `k3` in `keys3`.

        See Also:
            :func:`integer_var_matrix`
        """
        return self.var_multidict(self.integer_vartype, [keys1, keys2, keys3], lb, ub, name)

    def binary_var_cube(self, keys1, keys2, keys3, name=None, key_format=None):
        """Creates a dictionary of binary decision variables, indexed by triplets.

        Same as :func:`binary_var_matrix`, except that variables are indexed by triplets of
        the form `(k1, k2, k3)` with `k1` in `keys1`, `k2` in `keys2`, `k3` in `keys3`.

        :returns: A dictionary of :class:`docplex.mp.linear.Var` objects (with type :class:`docplex.mp.vartype.BinaryVarType`) indexed by
            triplets.

        """
        return self.var_multidict(self.binary_vartype, [keys1, keys2, keys3], name=name, key_format=key_format)

    def linear_expr(self, arg=None, name=None):
        ''' Returns a new empty linear expression.

        Args:
            arg: an optional argument to convert to a linear expression. Detailt is None,
                in which case, an empty expression is returned.
            name: An optional string to name the expression.

        :returns: An instance of :class:`docplex.mp.linear.LinearExpr`.
        '''
        self._checker.typecheck_string(arg=name, accept_none=True)
        return self._lfactory.linear_expr(arg=arg, name=name)

    def quad_expr(self, name=None):
        ''' Returns a new empty quadratic expression.

        Args:
            name: An optional string to name the expression.

        :returns: An instance of :class:`docplex.mp.quad.QuadExpr`.
        '''
        return self._qfactory.new_quad(name=name)

    def _linear_expr(self, e=None, constant=0, name=None):
        # INTERNAL
        return self._lfactory.linear_expr(e, constant, name)

    def abs(self, e):
        """ Builds an expression equal to the absolute value of its argument.

        Args:
            e: Accepts any object that can be transformed into an expression:
                decision variables, expressions, or numbers.

        Returns:
            An expression that can be used in arithmetic operators and constraints.

        Note:
            Building the expression generates auxiliary decision variables, including binary decision variables,
            and this may change the nature of the problem from a LP to a MIP.

        """
        self._checker.typecheck_operand(e, caller="Model.abs", accept_numbers=True)
        return self._lfactory.new_abs_expr(e)

    def min(self, *args):
        """ Builds an expression equal to the minimum value of its arguments.

        This method accepts a variable number of arguments.

        If no arguments are provided, returns positive infinity (see :func:`infinity`).

        Args:
            args: A variable list of arguments, each of which is either an expression, a variable,
                or a container.

        If passed a container or an iterator, this container or iterator must be the unique argument.

        If passed one dictionary, returns the minimum of the dictionary values.


        Returns:
            An expression that can be used in arithmetic operators and constraints.

        Example:
            `model.min()` -> returns `model.infinity`.

            `model.min(e)` -> returns `e`.

            `model.min(e1, e2, e3)` -> returns a new expression equal to the minimum of the values of `e1`, `e2`, `e3`.

            `model.min([x1,x2,x3])` where `x1`, `x2` .. are variables or expressions -> returns the minimum of these expressions.

            `model.min([])` -> returns `model.infinity`.

        Note:
            Building the expression generates auxiliary variables, including binary decision variables,
            and this may change the nature of the problem from a LP to a MIP.
        """
        min_args = args
        nb_args = len(args)
        if 0 == nb_args:
            pass
        elif 1 == nb_args:
            unique_arg = args[0]
            if is_iterable(unique_arg):
                if isinstance(unique_arg, dict):
                    min_args = unique_arg.values()
                else:
                    min_args = _to_list(unique_arg)
                for a in min_args:
                    self._checker.typecheck_operand(a, caller="Model.min()")
            else:
                self._checker.typecheck_operand(unique_arg, caller="Model.min()")

        else:
            for arg in args:
                self._checker.typecheck_operand(arg, caller="Model.min")

        return self._lfactory.new_min_expr(*min_args)

    def max(self, *args):
        """ Builds an expression equal to the maximum value of its arguments.

        This method accepts a variable number of arguments.

        Args:
            args: A variable list of arguments, each of which is either an expression, a variable,
                or a container.

        If passed a container or an iterator, this container or iterator must be the unique argument.

        If passed one dictionary, returns the maximum of the dictionary values.

        If no arguments are provided, returns negative infinity (see :func:`infinity`).


        Example:
            `model.max()` -> returns `-model.infinity`.

            `model.max(e)` -> returns `e`.

            `model.max(e1, e2, e3)` -> returns a new expression equal to the maximum of the values of `e1`, `e2`, `e3`.

            `model.max([x1,x2,x3])` where `x1`, `x2` .. are variables or expressions -> returns the maximum of these expressions.

            `model.max([])` -> returns `-model.infinity`.


        Note:
            Building the expression generates auxiliary variables, including binary decision variables,
            and this may change the nature of the problem from a LP to a MIP.
        """
        max_args = args
        nb_args = len(args)
        if 0 == nb_args:
            pass
        elif 1 == nb_args:
            unique_arg = args[0]
            if is_iterable(unique_arg):
                if isinstance(unique_arg, dict):
                    max_args = unique_arg.values()
                else:
                    max_args = _to_list(unique_arg)
                for a in max_args:
                    self._checker.typecheck_operand(a, caller="Model.max")
            else:
                self._checker.typecheck_operand(unique_arg, caller="Model.max")
        else:
            for arg in args:
                self._checker.typecheck_operand(arg, caller="Model.max")

        return self._lfactory.new_max_expr(*max_args)


    def logical_and(self, *args):
        """ Builds an expression equal to the logical AND value of its arguments.

        This method accepts a non-empty variable number of binary variables.

        Args:
            args: A variable list of binary variables, that is, decision variables with type BinaryVarType.
            
        Note:
            If passed an empty number of arguments, this method returns an expression equal to 1.
            
        Returns:
            An expression, equal to 1 if and only if all argument variables are equal to 1,
            else equal to 0.

        """
        bvars = self._checker.typecheck_var_seq(args, vtype=self.binary_vartype)
        return self._lfactory.new_logical_and_expr(bvars)

    def logical_or(self, *args):
        """ Builds an expression equal to the logical OR value of its arguments.

        This method accepts a non-empty variable number of binary variables.

        Args:
            args: A variable list of binary variables, that is, decision variables with type BinaryVarType.

        Note:
            If passed an empty number of arguments, this method a zero expression.

        Returns:
            An expression, equal to 1 if and only if at least one of its
             argument variables is equal to 1, else equal to 0.

        """
        bvars = self._checker.typecheck_var_seq(args, vtype=self.binary_vartype)
        return self._lfactory.new_logical_or_expr(bvars)


    def _to_linear_expr(self, e, force_clone=False, context=None):
        # INTERNAL
        return self._lfactory._to_linear_expr(e, force_clone=force_clone, context=context)

    def scal_prod(self, terms, coefs):
        """
        Creates a linear expression equal to the scalar product of a list of decision variables and a sequence of coefficients.

        This method accepts different types of input for both arguments. The variable sequence can be either a list
        or an iterator of objects that can be converted to linear expressions, that is, variables, expressions, or numbers.
        The most usual case is variables.
        The coefficients can be either a list of numbers, an iterator over numbers, or a number.
        In this last case the scalar product reduces to a sum times this coefficient.


        :param terms: A list or an iterator on variables or expressions.
        :param coefs: A list or an iterator on numbers, or a number.

        Note: 
           If either list or iterator is empty, this method returns zero.

        :return: A linear expression or 0.
        """
        self._checker.check_ordered_sequence(arg=terms, header='Model.scal_prod() requires a list of expressions/variables')
        return self._aggregator.scal_prod(terms, coefs)

    def dot(self, terms, coefs):
        """ Synonym for :func:`scal_prod`.

        """
        return self.scal_prod(terms, coefs)

    def sum(self, args):
        """ Creates a linear expression summing over a sequence.


        Note:
           This method returns 0 if the argument is an empty list or iterator.
        
        :param args: A list of objects that can be converted to linear expressions, that is, linear expressions, decision variables, or numbers.

        :return: A linear expression or 0.
        """
        return self._aggregator.sum(args)

    def sumsq(self, args):
        """ Creates a quadratic expression summing squares over a sequence.

        Each element of the list is squared and added to the result. Quadratic expressions
        are not accepted, as they cannot be squared.

        Note:
           This method returns 0 if the argument is an empty list or iterator.

        :param args: A list of objects that can be converted to linear expressions, that is,
            linear expressions, decision variables, or numbers. Each item is squared and
            added to the result.

        :return: A quadratic expression (possibly constant).
        """
        return self._aggregator.sumsq(args)


    def le_constraint(self, lhs, rhs, name=None):
        """ Creates a "less than or equal to" linear constraint.

        Note:
            This method returns a constraint object, that is not added to the model.
            To add it to the model, use the :func:`add_constraint` method.

        Args:
            lhs: An object that can be converted to a linear expression, typically a variable,
                    a member of an expression.
            rhs: An object that can be converted to a linear expression, typically a variable,
                    a member of an expression.
            name (string): An optional string to name the constraint.

        :returns: An instance of :class:`docplex.mp.linear.LinearConstraint`.
        """
        return self._lfactory.new_le_constraint(lhs, rhs, name)

    def ge_constraint(self, lhs, rhs, name=None):
        """ Creates a "greater than or equal to" linear constraint.

        Note:
            This method returns a constraint object that is not added to the model.
            To add it to the model, use the :func:`add_constraint` method.

        Args:
            lhs: An object that can be converted to a linear expression, typically a variable,
                    a member of an expression.
            rhs: An object that can be converted to a linear expression, typically a variable,
                    a number of an expression.
            name (string): An optional string to name the constraint.

        :returns: An instance of :class:`docplex.mp.linear.LinearConstraint`.
        """
        return self._lfactory.new_ge_constraint(lhs, rhs, name)

    def eq_constraint(self, lhs, rhs, name=None):
        """ Creates an equality constraint.

        Note:
            This method returns a constraint object that is not added to the model.
            To add it to the model, use the :func:`add_constraint` method.

        :param lhs: An object that can be converted to a linear expression, typically a variable,
                    a member of an expression.
        :param rhs: An object that can be converted to a linear expression, typically a variable,
                    a member of an expression.
        :param name: An optional string to name the constraint.

        :returns: An instance of :class:`docplex.mp.linear.LinearConstraint`.
        """
        return self._lfactory.new_eq_constraint(lhs, rhs, name)

    def linear_constraint(self, lhs, rhs, ctsense, name=None):
        """ Creates a linear constraint.

        Note:
            This method returns a constraint object that is not added to the model.
            To add it to the model, use the :func:`add_constraint` method.

        Args:
            lhs: An object that can be converted to a linear expression, typically a variable,
                    a member of an expression.
            rhs: An object that can be converted to a linear expression, typically a variable,
                    a number of an expression.
            ctsense: A constraint sense; accepts either a
                    value of type `ComparisonType` or a string (e.g 'le', 'eq', 'ge').

            name (string): An optional string to name the constraint.

        :returns: An instance of :class:`docplex.mp.linear.LinearConstraint`.
        """
        return self._lfactory.new_binary_constraint(lhs, ctsense, rhs, name)

    def _create_engine_constraint(self, ct):
        # INTERNAL
        eng = self.__engine
        if isinstance(ct, LinearConstraint):
            return eng.create_linear_constraint(ct)
        elif isinstance(ct, RangeConstraint):
            return eng.create_range_constraint(ct)
        elif isinstance(ct, IndicatorConstraint):
            # here check whether linear ct is trivial. if yes do not send to CPLEX
            indicator = ct
            linct = indicator.linear_constraint
            if linct.is_trivial():
                is_feasible = linct._is_trivially_feasible()
                if is_feasible:
                    self.warning("Indicator constraint {0!s} has a trivially feasible constraint (no effect)",
                                 indicator)
                    return -2
                else:
                    self.warning(
                        "indicator constraint {0!s} has a trivially infeasible constraint; variable invalidated",
                        indicator)
                    indicator.invalidate()
                    return -4
            return eng.create_indicator_constraint(ct)
        elif isinstance(ct, EquivalenceConstraint):
            return eng.create_equivalence_constraint(ct)

        elif isinstance(ct, QuadraticConstraint):
            return eng.create_quadratic_constraint(ct)
        elif isinstance(ct, PwlConstraint):
            return eng.create_pwl_constraint(ct)
        else:
            self.fatal("Expecting binary constraint, indicator or range, got: {0!s}", ct)  # pragma: no cover

    def _notify_trivial_constraint(self, ct, ctname, is_feasible):
        self_trivial_warn_level = self._trivial_cts_message_level
        if is_feasible and self_trivial_warn_level > self.warn_trivial_feasible:
            return
        elif self_trivial_warn_level > self.warn_trivial_infeasible:
            return
        # ---
        # herefater we are sure to warn
        if ct is None:
            arg = None
        elif ctname:
            arg = ctname
        elif ct.has_name():
            arg = ct.name
        else:
            arg = str(ct)
        # ---
        ct_typename = ct.short_typename() if ct is not None else "constraint"
        ct_rank = self.number_of_constraints + 1
        # BEWARE: do not use if arg here
        # because if arg is a constraint, boolean conversion won't work.
        if arg is not None:
            if is_feasible:
                self.info("Adding trivial feasible {2}: {0!s}, rank: {1}", arg, ct_rank, ct_typename)
            else:
                self.error("Adding trivial infeasible {2}: {0!s}, rank: {1}", arg, ct_rank, ct_typename)
                docplex_add_trivial_infeasible_ct(ct=arg)
        else:
            if is_feasible:
                self.info("Adding trivial feasible {1}, rank: {0}", ct_rank, ct_typename)
            else:
                self.error("Adding trivial infeasible {1}, rank: {0}", ct_rank, ct_typename)
                docplex_add_trivial_infeasible_ct(ct=None)

    def _prepare_constraint(self, ct, ctname, check_for_trivial_ct, arg_checker=None):
        # INTERNAL
        checker = arg_checker or self._checker
        if ct is True:
            # sum([]) == 0
            self._notify_trivial_constraint(ct=None, ctname=ctname, is_feasible=True)
            return False

        elif ct is False:
            # happens with sum([]) and constant e.g. sum([]) == 2
            self._notify_trivial_constraint(ct=None, ctname=ctname, is_feasible=False)
            msg = "Adding a trivially infeasible constraint"
            if ctname:
                msg += ' with name: {0}'.format(ctname)
            # analogous to 0 == 1, model is sure to fail
            self.fatal(msg)

        else:
            checker.typecheck_ct_to_add(ct, self, 'add_constraint')
            # -- watch for trivial cts e.g. linexpr(0) <= linexpr(1)
            if check_for_trivial_ct and ct.is_trivial():
                if ct._is_trivially_feasible():
                    self._notify_trivial_constraint(ct, ctname, is_feasible=True)
                elif ct._is_trivially_infeasible():
                    self._notify_trivial_constraint(ct, ctname, is_feasible=False)

        # --- name management ---
        if ctname:
            ct_name_map = self.__cts_by_name
            if ct_name_map is not None:
                if ctname in ct_name_map:
                    self.warning("Duplicate constraint name: {0!s}, used for: {1}", ctname, ct_name_map[ctname])
                ct_name_map[ctname] = ct
            ct.name = ctname
        # ---

        # check for already posted cts.
        if ct._index >= 0:
            self.warning("constraint has already been posted: {0!s}, index is: {1}", ct, ct.index)  # pragma: no cover
            return False  # pragma: no cover
        return True

    def _add_constraint_internal(self, ct, ctname=None):
        used_ct_name = None if self._ignore_names else ctname

        check_trivial = self._checker.check_trivial_constraints()
        if self._prepare_constraint(ct, used_ct_name, check_for_trivial_ct=check_trivial):
            self._post_constraint(ct)
            return ct
        elif ct is True or ct is False:
            return None
        else:
            return ct

    def _post_constraint(self, ct):
        ct_engine_index = self._create_engine_constraint(ct)
        self._register_one_constraint(ct, ct_engine_index, is_ctname_safe=True)
        return ct

    def _remove_constraint_internal(self, ct):
        """
        No typechecking for this internal version.
        :param ct:
        :return:
        """
        ct_name = ct.name
        if ct_name and self.__cts_by_name:
            try:
                del self.__cts_by_name[ct_name]
            except KeyError:
                # already removed
                pass

        ct_index = ct.unchecked_index
        if ct.is_valid_index(ct_index):
            # remove from model.
            try:
                allcts = self.__allcts
                ctx = compute_is_index(allcts, ct)
                if ctx is not None:
                    del allcts[ctx]
            except ValueError:  # pragma: no cover
                # valid index but not added, this is weird...
                pass

            # remove from engine.
            self.__engine.remove_constraint(ct)
            cscope = ct._get_index_scope()
            cscope.reindex_all(self.__engine)
            self._sync_constraint_indices(cscope.iter)
            cscope.update_indices()
            # unsubscribe exprs.
            ct.notify_deleted()

    def _resolve_ct(self, ct_arg, silent=False, context=None):
        verbose = not silent
        if context:
            printed_context = context + ": "
        else:
            printed_context = ""
        if isinstance(ct_arg, AbstractConstraint):
            return ct_arg
        elif is_string(ct_arg):
            ct = self.get_constraint_by_name(ct_arg)
            if ct is None and verbose:
                self.error("{0}no constraint with name: \"{1}\" - ignored", printed_context, ct_arg)
            return ct
        elif is_int(ct_arg):
            if ct_arg >= 0:
                ct_index = ct_arg
                ct = self.get_constraint_by_index(ct_index)
                if ct is None and verbose:
                    self.error("{0}no constraint with index: \"{1}\" - ignored", printed_context, ct_arg)
                return ct
            else:
                self.error("{0}not a valid index: \"{1}\" - ignored", printed_context, ct_arg)
                return None

        else:
            if verbose:
                self.error("{0}unexpected argument {1!s}, expecting string or constraint", printed_context, ct_arg)
            return None

    def remove_constraint(self, ct_arg):
        """ Removes a constraint from the model.

        Args:
            ct_arg: The constraint to remove. Accepts either a constraint object or a string.
                If passed a string, looks for a constraint with that name.

        """
        ct = self._resolve_ct(ct_arg, silent=False, context="remove_constraint")
        if ct is not None:
            self._checker.typecheck_in_model(self, ct, header="constraint")
            self._remove_constraint_internal(ct)

    def clear_constraints(self):
        """
        This method removes all constraints from the model.
        """
        self.__engine.remove_constraints(cts=None)  # special case to denote all
        # clear containers
        self.__allcts = []
        self.__cts_by_name = None
        # clear constraint index scopes.
        for ctscope in self._ctscopes:
            ctscope.reset()

    def remove_constraints(self, cts=None):
        """
        This method removes a collection of constraints from the model.

        :param cts: a sequence of constraints (linear, range, quadratic, indicators)
        """
        if cts is not None:
            doomed = self._checker.typecheck_constraint_seq(cts)
            self.__engine.remove_constraints(doomed)
            self._remove_constraints_internal(doomed)

    def _remove_constraints_internal(self, doomed):
        # INTERNAL
        self_cts_by_name = self.__cts_by_name
        doomed_indices = set()
        for d in doomed:
            if self_cts_by_name:
                dname = d.get_name()
                if dname:
                    del self_cts_by_name[dname]
            if d._index >= 0:
                doomed_indices.add(d._index)
        # update container
        self.__allcts = [c for c in self.__allcts if c._index not in doomed_indices]
        # TODO: handle reindexing
        doomed_scopes = set(c._get_index_scope() for c in doomed)
        for ds in doomed_scopes:
            ds.reindex_all(self.__engine)
            ds.update_indices()
        for d in doomed:
            d.notify_deleted()

    def remove(self, removed):
        """
        This method removes a constraint or a collection of constraints from the model.

        :param removed: a constraint or a sequence of constraints (linear, range, quadratic, indicators)
        """
        if is_iterable(removed):
            self.remove_constraints(removed)
        else:
            self.remove_constraint(removed)

    def add_range(self, lb, expr, ub, rng_name=None):
        """ Adds a new range constraint to the model.

        A range constraint states that a linear
        expression has to stay within an interval `[lb..ub]`.
        Both `lb` and `ub` have to be float numbers with `lb` smaller than `ub`.

        The method creates a new range constraint and adds it to the model.

        Args:
            lb (float): A floating-point number.
            expr: A linear expression, e.g. X+Y+Z.
            ub (float): A floating-point number, which should be greater than `lb`.
            rng_name (string): An optional name for the range constraint.

        :returns: The newly created range constraint.
        :rtype: :class:`docplex.mp.constr.RangeConstraint`

        Raises:
            An exception if `lb` is greater than `ub`.

        """
        rng = self.range_constraint(lb, expr, ub)
        ctname = None if self._ignore_names else rng_name
        ct = self._add_constraint_internal(rng, ctname)
        return ct

    def indicator_constraint(self, binary_var, linear_ct, active_value=1, name=None):
        """ Creates and returns a new indicator constraint.

        The indicator constraint is not added to the model.

        Args:
            binary_var: The binary variable used to control the satisfaction of the linear constraint.
            linear_ct: A linear constraint (EQ, LE, GE).
            active_value: 0 or 1. The value used to trigger the satisfaction of the constraint. The default is 1.
            name (string): An optional name for the indicator constraint.

        :return:
            The newly created indicator constraint.
        """
        self._checker.typecheck_binary_var(binary_var)
        self._checker.typecheck_linear_constraint(linear_ct)
        self._checker.typecheck_zero_or_one(active_value)
        self._checker.typecheck_in_model(self, binary_var, header="binary variable")
        self._checker.typecheck_in_model(self, linear_ct, header="linear_constraint")
        return self._lfactory.new_indicator_constraint(binary_var, linear_ct, active_value, name)


    def add_indicator(self, binary_var, linear_ct, active_value=1, name=None):
        """ Adds a new indicator constraint to the model.

        An indicator constraint links (one-way) the value of a binary variable to
        the satisfaction of a linear constraint.
        If the binary variable equals the active value, then the constraint is satisfied, but
        otherwise the constraint may or may not be satisfied.

        Args:
             binary_var: The binary variable used to control the satisfaction of the linear constraint.
             linear_ct: A linear constraint (EQ, LE, GE).
             active_value: 0 or 1. The value used to trigger the satisfaction of the constraint. The default is 1.
             name (string): An optional name for the indicator constraint.

        Returns:
            The newly created indicator constraint.
        """
        self._checker.typecheck_string(name, accept_none=True)
        iname = None if self._ignore_names else name
        indicator = self.indicator_constraint(binary_var, linear_ct, active_value, iname)
        return self._add_indicator(indicator)

    _indicator_trivial_feasible_idx = -2
    _indicator_trivial_infeasible_idx = -4

    def _add_indicator(self, indicator, check_trivials=False):
        # INTERNAL
        linear_ct = indicator.linear_constraint
        if check_trivials and self._checker.check_trivial_constraints() and linear_ct.is_trivial():
            is_feasible = linear_ct._is_trivially_feasible()
            if is_feasible:
                self.warning("Indicator constraint {0!s} has a trivial feasible linear constraint (has no effect)",
                             indicator)
                return self._indicator_trivial_feasible_idx
            else:
                self.warning("indicator constraint {0!s} has a trivial infeasible linear constraint - invalidated",
                             indicator)
                indicator.invalidate()
                return self._indicator_trivial_infeasible_idx
        else:
            return self._add_constraint_internal(indicator)

    def add_equivalence(self, binary_var, linear_ct, true_value=1, name=None):
        """ Adds a new equivalence constraint to the model.

        An equivalence constraints links two-way the value of a binary variable to
        the satisfaction of a discrete linear constraint.
        If the binary variable equals the true value, then the constraint is satisfied,
        conversely if the constraint is satisfied, the binary variable is equal to the true value.

        Args:
             binary_var: The binary variable used to control the satisfaction of the linear constraint.
             linear_ct: A linear constraint (EQ, LE, GE).
             true_value: 0 or 1. The value used to trigger the satisfaction of the constraint. The default is 1.
             name (string): An optional name for the equivalence constraint.

        Returns:
            The newly created equivalence constraint.
        """
        checker = self._checker
        checker.typecheck_var(binary_var)
        checker.typecheck_linear_constraint(linear_ct)
        checker.typecheck_zero_or_one(true_value)
        checker.typecheck_in_model(self, binary_var, header="binary variable")
        checker.typecheck_in_model(self, linear_ct, header="linear_constraint")
        StaticTypeChecker.typecheck_discrete_constraint(self, linear_ct, msg='Model.add_equivalence() requires a discrete constraint')
        return self._add_equivalence(binary_var, linear_ct, true_value, name)

    def _add_equivalence(self, binary_var, linear_ct, active_value, name=None):
        # INTERNAL
        used_name = None if self._ignore_names else name
        equiv = self._lfactory.new_equivalence_constraint(binary_var, linear_ct, active_value, name=used_name)
        eqct = self._add_constraint_internal(equiv, used_name)
        return eqct

    def add_equivalences(self, binary_vars, cts, true_values=1, names=None):
        """ Adds a batch of equivalence constraints to the model.

        This method adds a batch of equivalence constraints to the model.

        :param binary_vars: a sequence of binary variables.
        :param cts: a sequence of discrete linear constraints
        :param true_values: the true values to use. Accepts either 1, 0 or a sequence of {0, 1} values.
        :param names: an optional sequence of names

        All sequences must have the same length.

        :return: a sequence of equivalence constraints.
        """
        return self._add_batch_logical_cts(binary_vars, cts, names, true_values, is_equivalence=True)

    def add_indicators(self, binary_vars, cts, true_values=1, names=None):
        """ Adds a batch of indicator constraints to the model.

        This method adds a batch of indicator constraints to the model.

        :param binary_vars: a sequence of binary variables.
        :param cts: a sequence of discrete linear constraints
        :param true_values: the true values to use. Accepts either 1, 0 or a sequence of {0, 1} values.
        :param names: an optional sequence of names

        All sequences must have the same length.

        :return: a sequence of indicator constraints.
        """
        return self._add_batch_logical_cts(binary_vars, cts, names, true_values, is_equivalence=False)


    def _add_batch_logical_cts(self, binary_vars, cts, names, true_values, is_equivalence):
        checker = self._checker
        bvars = checker.typecheck_var_seq(binary_vars, vtype=self.binary_vartype)
        ctseq = checker.typecheck_constraint_seq(cts, ctype=LinearConstraint)
        try:
            n_vars = len(bvars)
            n_cts = len(cts)
        except TypeError:
            # if passed iterators, no len()
            bvars = list(bvars)
            ctseq = list(ctseq)
            n_vars = len(bvars)
            n_cts = len(ctseq)

        if n_vars != n_cts:
            self.fatal('Model.add_equivalences(): binary_vars and linear cts must have same size.')

        if true_values == 0 or true_values == 1:
            actual_true_values = generate_constant(true_values, n_vars)

        elif is_iterable(true_values):
            actual_true_values = list(true_values)
            if len(actual_true_values) != n_vars:
                self.fatal('Model.add_equivalences(): true_values has wrong size. expecting: {0}, got: {1}'
                           .format(n_vars, len(actual_true_values)))
            for a in true_values:
                checker.typecheck_zero_or_one(a)
        else:
            self.fatal('Model.add_equivalence(): true_values expects 0|1 or sequence of {{0, 1}}, got: {0!r}'.format(true_values))

        if names is not None and not self._ignore_names:
            used_names = []
            for n in names:
                checker.typecheck_string(n, accept_empty=False, accept_none=True)
                used_names.append(n or '')
        else:
            used_names = generate_constant('', n_vars)

        if is_equivalence:
            eqcts = self._lfactory.new_batch_equivalence_constraints(bvars, ctseq, actual_true_values, used_names)
            self.add_equivalence_constraints_(eqcts)
            return eqcts
        else:
            indcts = self._lfactory.new_batch_indicator_constraints(bvars, ctseq, actual_true_values, used_names)
            self.add_indicator_constraints_(indcts)
            return indcts

    def add_indicator_constraints(self, indcts):
        """ Adds a sequence of indicator constraints to the model

        :param indcts: a sequence of indicator constraints.

        See Also:
            :func:`indicator_constraint`
        """
        ind_indices = self.__engine.create_batch_indicator_constraints(indcts)
        self._register_block_cts(self._indicator_scope, indcts, ind_indices)
    add_indicator_constraints_ = add_indicator_constraints

    def add_equivalence_constraints(self, eqcts):
        """ Adds a sequence of equivalence constraints to the model

        :param eqcts: a sequence of equivalence constraints.

        See Also:
            :func:`equivalence_constraint`
        """
        eq_indices = self.__engine.create_batch_equivalence_constraints(eqcts)
        self._register_block_cts(self._indicator_scope, eqcts, eq_indices)

    add_equivalence_constraints_ = add_equivalence_constraints

    def if_then(self, if_ct, then_ct, negate=False):
        """ Creates and returns an if-then constraint.

        An if-then constraint links two constraints ct1, ct2 such that when ct1 is satisfied then ct2 is also satisfied.

        :param if_ct: a linear constraint, the satisfaction of which governs the satisfaction of the `then_ct`
        :param then_ct: a linear constraint, which becomes satisfied as soon as `if_ct` is satisfied
            (or when it is not, depending on the `negate` flag).
        :param negate: an optional boolean flag (default is False). If True, `then_ct` is satisfied when `if_ct` is *not* satisfied.

        :return:
            an instance of IfThenConstraint, that is not added to the model.
            Use Model.add_constraint() or Model.add() to add it to the model.

        Note:
            This constraint relies on the status of the `if_ct` constraaint, so this constraint must be discrete,
            otherwise an exception will be raised.
        """
        checker = self._checker
        checker.typecheck_linear_constraint(if_ct)
        checker.typecheck_linear_constraint(then_ct)
        StaticTypeChecker.typecheck_discrete_constraint(logger=self, ct=if_ct, msg='Model.if_then()')
        return self._lfactory.new_if_then_constraint(if_ct, then_ct, negate)

    def add_if_then(self, if_ct, then_ct, negate=False):
        """ Creates a new if-then constraint and adds it to the model

        :param if_ct: a linear constraint, the satisfaction of which governs the satisfaction of the `then_ct`
        :param then_ct: a linear constraint, which becomes satisfied as soon as `if_ct` is satisfied
            (or when it is not, depending on the `negate` flag).
        :param negate: an optional boolean flag (default is False). If True, `then_ct` is satisfied when `if_ct` is *not* satisfied.

       :return:
            an instance of IfThenConstraint.

        Note:
            This constraint relies on the status of the `if_ct` constraaint, so this constraint must be discrete,
            otherwise an exception will be raised.
        """
        ifthen_ct = self.if_then(if_ct, then_ct, negate=negate)
        return self._post_constraint(ifthen_ct)

    def range_constraint(self, lb, expr, ub, rng_name=None):
        """ Creates a new range constraint but does not add it to the model.

        A range constraint states that a linear
        expression has to stay within an interval `[lb..ub]`.
        Both `lb` and `ub` have to be floating-point numbers with `lb` smaller than `ub`.

        The method creates a new range constraint but does not add it to the model.

        Args:
            lb: A floating-point number.
            expr: A linear expression, e.g. X+Y+Z.
            ub: A floating-point number, which should be greater than `lb`.
            rng_name: An optional name for the range constraint.

        Returns:
            The newly created range constraint.
        Raises:
             An exception if `lb` is greater than `ub`.

        """
        self._checker.typecheck_num(lb, 'Model.range_constraint')
        self._checker.typecheck_num(ub, 'Model.range_constraint')
        self._checker.typecheck_string(rng_name, accept_empty=False, accept_none=True)
        rng = self._lfactory.new_range_constraint(lb, expr, ub, rng_name)
        return rng

    def add_constraint(self, ct, ctname=None):
        """ Adds a new linear constraint to the model.

        Args:
            ct: A linear constraint of the form <expr1> <op> <expr2>, where both expr1 and expr2 are
                 linear expressions built from variables in the model, and <op> is a relational operator
                 among <= (less than or equal), == (equal), and >= (greater than or equal).
            ctname (string): An optional string used to name the constraint.

        Returns:
            The newly added constraint.

        See Also:
            :func:`add_constraint_`
        """
        ct = self._add_constraint_internal(ct, ctname)
        return ct

    def add_constraint_(self, ct, ctname=None):
        """ Adds a new linear constraint to the model.

        Args:
            ct: A linear constraint of the form <expr1> <op> <expr2>, where both expr1 and expr2 are
                 linear expressions built from variables in the model, and <op> is a relational operator
                 among <= (less than or equal), == (equal), and >= (greater than or equal).
            ctname (string): An optional string used to name the constraint.

        Note:
            This method does the same as `docplex.mp.model.Model.add_constraint()` except that it has no return value.

        See Also:
            :func:`add_constraint`
        """
        self._add_constraint_internal(ct, ctname)

    def add(self, ct, name=None):
        if is_iterable(ct):
            return self.add_constraints(ct, name)
        else:
            return self.add_constraint(ct, name)

    def add_(self, ct, name=None):
        if is_iterable(ct):
            self.add_constraints_(ct, name)
        else:
            self.add_constraint_(ct, name)

    def add_constraints(self, cts, names=None):
        """ Adds a collection of linear constraints to the model in one operation.

        Each constraint in the collection is added to the model, if it was not already added.
        If present, the `names` collection is used to set names to the constraints.

        Note: The `cts` argument can be a collection of (constraint, name) tuples such
        as `mdl.add_constraints([(x >=3, 'ct0'), (x + y == 1, 'ct1')])`.

        Args:
            cts: A collection of linear constraints or an iterator over a collection of linear constraints.
            names: An optional collection or iterator on strings.

        Returns:
            A list of those constraints added to the model.

        See Also:
            :func:`add_constraints_`
        """
        # build a sequence as we need to traverse it more than once.
        self._checker.typecheck_iterable(cts)
        return self._lfactory.new_constraint_block(cts, names)


    def add_constraints_(self, cts, names=None):
        """ Adds a collection of linear constraints to the model in one operation.

        Same as `docplex.model.Model.add_constraints()` except that is does not return anything.
        """
        self._checker.typecheck_iterable(cts)
        if names is not None and not self.ignore_names:
            self._lfactory._new_constraint_block2(cts, names)
        else:
            self._lfactory._new_constraint_block1(cts)


    def add_ranges(self, lbs, exprs, ubs, names=None):
        checker = self._checker
        lbsl = checker.typecheck_num_seq(lbs)
        ubsl = checker.typecheck_num_seq(ubs)
        checker.typecheck_iterable(exprs)
        range_names = names if names and not self.ignore_names else None
        return self._lfactory.new_range_block(lbsl, exprs, ubsl, range_names)

    # ----------------------------------------------------
    # objective
    # ----------------------------------------------------

    def round_objective_if_discrete(self, raw_obj):
        # INTERNAL
        if self._objective_expr.is_discrete():
            return self.round_nearest(raw_obj)
        else:
            return raw_obj



    def minimize(self, expr):
        """ Sets an expression as the expression to be minimized.

        The argument is converted to a linear expression. Accepted types are variables (instances of
        :class:`docplex.mp.linear.Var` class), linear expressions (instances of
        :class:`docplex.mp.linear.LinearExpr`), or numbers.

        :param expr: A linear expression or a variable.
        """
        self.set_objective(ObjectiveSense.Minimize, expr)

    def maximize(self, expr):
        """
        Sets an expression as the expression to be maximized.

        The argument is converted to a linear expression. Accepted types are variables (instances of
        :class:`docplex.mp.linear.Var` class), linear expressions (instances of
        :class:`docplex.mp.linear.LinearExpr`), or numbers.

        :param expr: A linear expression or a variable.
        """
        self.set_objective(ObjectiveSense.Maximize, expr)

    def is_minimize(self):
        """ Checks whether the model is a minimization model.

        Note:
            This returns True even if the expression to minimize is a constant.
            To check whether the model has a non-constant objective, use :func:`is_optimized`.

        Returns:
           Boolean: True if the model is a minimization model.
        """
        return self._objective_sense is ObjectiveSense.Minimize

    def is_maximize(self):
        """ Checks whether the model is a maximization model.

        Note:
           This returns True even if the expression to maximize is a constant.
           To check whether the model has a non-constant objective, use :func:`is_optimized`.

        Returns:
            Boolean: True if the model is a maximization model.
        """
        return self._objective_sense is ObjectiveSense.Maximize

    def objective_coef(self, dvar):
        """ Returns the objective coefficient of a variable.

        The objective coefficient is the coefficient of the given variable in
        the model's objective expression. If the variable is not explicitly
        mentioned in the objective, it returns 0.

        :param dvar: The decision variable for which to compute the objective coefficient.

        Returns:
            float: The objective coefficient of the variable.
        """
        self._checker.typecheck_var(dvar)
        return self._objective_coef(dvar)

    def _objective_coef(self, dvar):
        return self._objective_expr.unchecked_get_coef(dvar)

    def remove_objective(self):
        """ Clears the current objective.

        This is equivalent to setting "minimize 0".
        Any subsequent solve will look only for a feasible solution.
        You can detect this state by calling :func:`has_objective` on the model.

        """
        self.set_objective(self._lfactory.default_objective_sense(), self._new_default_objective_expr())

    def is_optimized(self):
        """ Checks whether the model has a non-constant objective expression.

        A model with a constant objective will only search for a feasible solution when solved.
        This happens either if no objective has been assigned to the model,
        or if the objective has been removed with :func:`remove_objective`.

        Returns:
            Boolean: True, if the model has a non-constant objective expression.

        """
        return not self._objective_expr.is_constant()


    def has_objective(self):
        # INTERNAL
        return not self._objective_expr.is_zero()

    def set_objective(self, sense, expr):
        """ Sets a new objective.

        Args:
            sense: Either an instance of :class:`docplex.mp.basic.ObjectiveSense` (Minimize or Maximize),
                or a string: "min" or "max".
            expr: Is converted to an expression. Accepted types are variables,
                linear expressions, quadratic expressions or numbers.

        Note:
            When using a number, the search will not optimize but only look for a feasible solution.

        """
        self.set_objective_sense(sense)
        self.set_objective_expr(expr)

    def set_objective_sense(self, sense):
        actual_sense = self._resolve_sense(sense)
        self._objective_sense = actual_sense
        eng = self.__engine
        if eng:
            # when ending the model, the engine is None here
            eng.set_objective_sense(actual_sense)

    def get_objective_sense(self):
        return self._objective_sense

    @property
    def objective_sense(self):
        """ This property is used to get or set the direction of the optimization as an instance of
        :class:`docplex.mp.basic.ObjectiveSense`, either Minimize or Maximize.

        This property also accepts strings as arguments: 'min' for minimize and 'max' for maximize.

        """
        return self._objective_sense

    @objective_sense.setter
    def objective_sense(self, new_sense):
        self.set_objective_sense(new_sense)


    def set_objective_expr(self, new_objexpr):
        if new_objexpr is None:
            expr = self._new_default_objective_expr()
        else:
            expr = self._lfactory._to_expr(new_objexpr)
            #expr.keep()
            expr.notify_used(self)

        eng = self.__engine
        current_objective_expr = self._objective_expr
        if eng:
            # when ending the model, the engine is None here
            eng.set_objective_expr(new_objexpr=expr, old_objexpr=current_objective_expr)
            if current_objective_expr is not None:
                current_objective_expr.notify_unsubscribed(subscriber=self)
        self._objective_expr = expr

    def get_objective_expr(self):
        """ This property is used to get or set the current expression used as the model objective.
        """
        return self._objective_expr

    @property
    def objective_expr(self):
        """ This property is used to get or set the current expression used as the model objective.
        """
        return self._objective_expr

    @objective_expr.setter
    def objective_expr(self, new_expr):
        self.set_objective_expr(new_expr)


    def notify_expr_modified(self, expr, event):
        # INTERNAL
        objexpr = self._objective_expr
        if event and expr is objexpr or expr is objexpr.linear_part:
            # old and new are the same
            self.__engine.update_objective(expr=expr, event=event)

    def notify_expr_replaced(self, old_expr, new_expr):
        if old_expr is self._objective_expr:
            self.__engine.set_objective_expr(new_objexpr=new_expr, old_objexpr=old_expr)
            new_expr.grab_subscribers(old_expr)

    def _new_default_objective_expr(self):
        # INTERNAL
        return self._lfactory.linear_expr(arg=None, constant=0, safe=True)


    def _can_solve(self):
        return self.__engine.can_solve()

    def _make_end_infodict(self):
        return self.solution.as_dict(keep_zeros=False) if self.solution is not None else dict()

    def prepare_actual_context(self, **kwargs):
        # prepares the actual context that will be used for a solve

        # use the provided context if any, or the self.context otherwise
        if not kwargs:
            return self.context

        arg_context = kwargs.get('context') or self.context
        if not isinstance(arg_context, Context):
            self.fatal('Expecting instance of docplex.mp.Context, got: {0!r}', arg_context)
        cloned = False
        context = arg_context

        # update the context with provided kwargs
        for argname, argval in six.iteritems(kwargs):
            # skip context argname if any
            if argname == "url" and (not is_url_valid(argval)) and context.solver.docloud.url:
                pass
            elif argname == 'clean_before_solve':
                pass
            elif argname != "context" and argval is not None:
                if not cloned:
                    context = context.clone()
                    cloned = True
                context.update_key_value(argname, argval)

        return context

    def solve(self, **kwargs):
        """ Starts a solve operation on the model.

        If CPLEX is available, the solve operation will be performed using the native CPLEX.
        If CPLEX is not available, the solve operation will be started on DOcplexcloud.
        The DOcplexcloud connection parameters are looked up in the following order:

            - if ``kwargs`` contains valid ``url`` and ``key`` values, they are used.
            - if ``kwargs`` contains a ``context`` and that context contains
                valid ``solver.docloud.url`` and ``solver.docloud.key`` values,
                those values are used. Other attributes of ``solver.docloud``
                can also be used. See :class:`docplex.mp.context.Context`
            - finally, the model's attribute ``context`` is used. This ``context``
                is set at model creation time.

        If CPLEX is not available and the model has no valid credentials, an error is raised, as there is
        no way to perform the solve.

        Note that if ``url`` and ``key`` parameters are present and the
        values of the parameters are not in the ignored url or key list,
        the solve operation will be started on DOcplexcloud even if CPLEX is
        available.

        Example::

            # forces the solve on DOcplexcloud with the specified url and keys
            model.solve(url='https://foo.com', key='bar')

        Example::

            # set some DOcplexcloud credentials, but depend on another
            # method to decide if solve is local or not
            ctx.solver.docloud.url = 'https://foo.com'
            ctx.solver.docloud.key = 'bar'
            agent = 'local' if method_that_decides_if_solve_is_local() or 'docloud'
            model.solve(context=ctx, agent=agent)

        Args:
            context (optional): An instance of context to be used in instead of
                the context this model was built with.
            cplex_parameters (optional): A set of CPLEX parameters to use
                instead of the parameters defined as
                ``context.cplex_parameters``.
                Accepts either a RootParameterGroup object (obtained by cloning the model's
                parameters), or a dict of path-like names and values.

            agent (optional): Changes the ``context.solver.agent`` parameter.
                Supported agents include:

                - ``docloud``: forces the solve operation to use DOcplexcloud
                - ``local``: forces the solve operation to use native CPLEX

            url (optional): Overwrites the URL of the
                DOcplexcloud service defined by ``context.solver.docloud.url``.
            key (optional): Overwrites the
                authentication key of the DOcplexcloud service defined by
                ``context.solver.docloud.key``.
            log_output (optional): if ``True``, solver logs are output to
                stdout. If this is a stream, solver logs are output to that
                stream object. Overwrites the ``context.solver.log_output``
                parameter.
            proxies (optional): a dict with the proxies mapping.

        Returns:
            A :class:`docplex.mp.solution.SolveSolution` object if the solve operation succeeded.
            None if the solve operation failed.
        """
        if not self.is_optimized():
            self.info("No objective to optimize - searching for a feasible solution")

        context = self.prepare_actual_context(**kwargs)

        # log stuff
        saved_context_log_output = self.context.solver.log_output
        saved_log_output_stream = self.get_log_output()

        try:
            self.set_log_output(context.solver.log_output)

            forced_docloud = context_must_use_docloud(context, **kwargs)
            have_credentials = context_has_docloud_credentials(context, do_warn=True)

            if forced_docloud:
                if have_credentials:
                    return self._solve_cloud(context)
                else:
                    self.fatal("DOcplexcloud context has no valid credentials: {0!s}",
                               context.solver.docloud)

            # from now on docloud_context is None
            elif self.environment.has_cplex:
                # if CPLEX is installed go for it
                force_clean_before_solve = kwargs.get('clean_before_solve')
                return self._solve_local(context, force_clean_before_solve)
            elif have_credentials:
                # no context passed as argument, no Cplex installed, try model's own context
                return self._solve_cloud(context)
            else:
                # no way to solve.. really
                return self.fatal("CPLEX DLL not found: please provide DOcplexcloud credentials")
        finally:
            if saved_log_output_stream != self.get_log_output():
                self.set_log_output_as_stream(saved_log_output_stream)
            if saved_context_log_output != self.context.solver.log_output:
                self.context.solver.log_output = saved_context_log_output

    def _connect_progress_listeners(self):
        # INTERNAL... connect progress listeners (if any) if problem is mip
        if self._progress_listeners:
            if self._solves_as_mip():
                for l in self._progress_listeners:
                    l.reset()
                self.__engine.connect_progress_listeners(self._progress_listeners)
            else:
                self.info("Model: \"{}\" is not a MIP problem, progress listeners are disabled", self.name)

    def _disconnect_progress_listeners(self):
        for pl in self._progress_listeners:
            pl.disconnect()

    def _notify_solve_hit_limit(self, solve_details):
        # INTERNAL
        if solve_details and solve_details.has_hit_limit():
            self.info("solve: {0}".format(solve_details.status))

    def _solve_local(self, context, force_clean_before_solve=None):
        """ Starts a solve operation on the local machine.

        Note:
        This method will never try to solve on DOcplexcloud, regardless of whether the model
        has an attached DOcplexcloud context.
        If CPLEX is not available, an error is raised.

        Args:
            context: a (possibly new) context whose parameters override those of the modle
                during this solve.

        Returns:
            A Solution object if the solve operation succeeded, None otherwise.

        """
        parameters = apply_thread_limitations(context, context.solver)

        self.notify_start_solve()

        self_solve_hooks = self._solve_hooks
        auto_publish_details = is_auto_publishing_solve_details(context)
        auto_publish_solution = is_auto_publishing_json_solution(context)
        if auto_publish_details:
            # don't bother adding the solve hook if not in auto publish mode
            wk_hook = get_solve_hook() if get_solve_hook else None
            if wk_hook is not None:
                self._add_solve_hook(wk_hook)
        else:
            wk_hook = None

        if is_in_docplex_worker and is_auto_publishing(context):
            # publish kpi automatically
            the_env = get_environment()
            env_kpi_hook = lambda kpd: the_env.update_solve_details(kpd)
            self.kpi_recorder = KpiRecorder(self, publish_hook=env_kpi_hook)
            self.add_progress_listener(self.kpi_recorder)

        sol_hook = self._solution_hook
        if sol_hook is not None and self._solves_as_mip():
            automatic_solution_listener = SolutionHookListener(self, sol_hook)
            self._add_progress_listener(automatic_solution_listener)
        else:
            automatic_solution_listener = None

        # connect progress listeners (if any) if problem is mip
        self._connect_progress_listeners()

        # call notifyStart on progress listeners
        self._fire_start_solve_listeners()
        # notify hooks only on solve_local

        if auto_publish_details and self_solve_hooks:
            self_stats = self.get_statistics()
            for h in self_solve_hooks:
                h.notify_start_solve(self, self_stats)  # pragma: no cover

        # --- solve is protected in try/except block
        has_solution = False
        reported_obj = 0
        engine_status = self._unknown_status
        self_engine = self.__engine
        self_params = self.context._get_raw_cplex_parameters()
        if self_params and parameters is not self_params:
            saved_params = {p: p.get() for p in self_params}
        else:
            saved_params = {}

        saved_clean_before_solve = self.clean_before_solve if force_clean_before_solve is not None else None

        new_solution = None
        try:
            if force_clean_before_solve is not None:
                self.clean_before_solve = force_clean_before_solve
            used_parameters = parameters or self.context._get_raw_cplex_parameters()
            #assert used_parameters is not None
            self._apply_parameters_to_engine(used_parameters)

            new_solution = self_engine.solve(self, parameters=parameters)
            has_solution = new_solution is not None
            self._set_solution(new_solution)
            reported_obj = self._reported_objective_value()

            # store solve status as returned by the engine.
            engine_status = self_engine.get_solve_status()
            self._last_solve_status = engine_status
            if sol_hook:
                sol_hook(new_solution)

        except DOcplexException as docpx_e:  # pragma: no cover
            self._set_solution(None)
            raise docpx_e

        except Exception as e:
            self._set_solution(None)
            raise e

        finally:
            solve_details = self_engine.get_solve_details()
            self._notify_solve_hit_limit(solve_details)
            self._solve_details = solve_details
            self._fire_end_solve_listeners(has_solution, reported_obj)
            self._disconnect_progress_listeners()
            if automatic_solution_listener is not None:
                self.remove_progress_listener(automatic_solution_listener)

            # call hooks
            if auto_publish_details:
                for h in self_solve_hooks:
                    h.notify_end_solve(self, has_solution, engine_status, reported_obj,
                                       self._make_end_infodict())  # pragma: no cover
                    h.update_solve_details(solve_details.as_worker_dict())  # pragma: no cover

            # save solution
            if auto_publish_solution and new_solution is not None:
                with get_environment().get_output_stream("solution.json") as output:
                    output.write(new_solution.export_as_string(format="json").encode('utf-8'))

            # unplug worker hook if any
            if wk_hook:
                self_solve_hooks.remove(wk_hook)  # pragma: no cover

            # restore parameters in sync with model, if necessary
            if saved_params:
                for p, v in six.iteritems(saved_params):
                    self_engine.set_parameter(p, v)

            if saved_clean_before_solve is not None:
                self.clean_before_solve = saved_clean_before_solve

        return new_solution

    def get_solve_status(self):
        """ Returns the solve status of the last successful solve.

        If the model has been solved successfully, returns the status stored in the
        model solution. Otherwise returns None`.

        :returns: The solve status of the last successful solve, a string, or None.
        """
        return self._last_solve_status

    def _new_docloud_engine(self, ctx):
        return self._engine_factory.new_docloud_engine(model=self,
                                                       docloud_context=ctx.solver.docloud,
                                                       log_output=ctx.solver.log_output_as_stream)

    def _solve_cloud(self, context):
        if self._solution_hook is not None:
            self._solution_hook_warn_no_cplex()
        parameters = context.cplex_parameters
        # see if we can reuse the local docloud engine if any?
        docloud_engine = self._new_docloud_engine(context)
        self.notify_start_solve()
        self._fire_start_solve_listeners()
        new_solution = docloud_engine.solve(self, parameters=parameters)
        self._set_solution(new_solution)
        self._solve_details = docloud_engine.get_solve_details()

        # store solve status as returned by the engine.
        self._last_solve_status = docloud_engine.get_solve_status()
        reported_obj = self._reported_objective_value()

        solve_details = docloud_engine.get_solve_details()
        self._notify_solve_hit_limit(solve_details)
        self._solve_details = solve_details
        self._fire_end_solve_listeners(new_solution is not None, reported_obj)
        # return solution from cloud engine: either None or a solution instance
        return new_solution

    def solve_cloud(self, context=None):
        # Starts execution of the model on the cloud.
        #
        # This method accepts a context (an instance of Context) to be used when
        # solving on the cloud. If the context argument is None or invalid, then it will
        # use the model's own instance of Context, set at model creation time.
        #
        # Note:
        #    This method will always solve the model on the cloud, whether or not CPLEX
        #    is available on the local machine.
        #
        # Args:
        #    context: An optional context to use on the cloud. If None, uses the model's Context instance, if any.
        #
        # :returns: A :class:`docplex.mp.solution.SolveSolution` object if the solve operation succeeded, else None.
        if not context:
            if self.context.solver.docloud:
                if self.__engine.name == 'docloud':
                    return self.solve()
                else:
                    return self._solve_cloud(self.context)
            else:
                self.fatal("context is None: cannot solve on the cloud")
        elif has_credentials(context.solver.docloud):
            return self._solve_cloud(context)
        else:
            self.fatal("DOcplexcloud context has no valid credentials: {0!s}", context.solver.docloud)

    def notify_start_solve(self):
        # INTERNAL
        pass

    def notify_solve_failed(self):
        pass

    def get_solve_details(self):
        """
        This property returns detailed information about the last solve,  an instance of :class:`docplex.mp.solution.SolveDetails`.

        Note:
            Detailed information is returned whether or not the solve succeeded.
        """
        from copy import copy as shallow_copy

        return shallow_copy(self._solve_details)

    solve_details = property(get_solve_details)

    def notify_solve_relaxed(self, relaxed_solution, solve_details):
        # INTERNAL: used by relaxer
        self._solve_details = solve_details
        self._set_solution(relaxed_solution)
        if relaxed_solution is not None:
            self.notify_start_solve()
        else:
            self.notify_solve_failed()

    def _resolve_sense(self, sense_arg):
        """
        INTERNAL
        :param sense_arg:
        :return:
        """
        return ObjectiveSense.parse(sense_arg, self.error_handler, default_sense=None)  # raise if invalid

    def solve_lexicographic(self, goals,
                            senses=ObjectiveSense.Minimize,
                            tolerances=None,
                            **kwargs):
        """ Performs a lexicographic solve from an ordered collection of goals.

        :param goals: An ordered collection of linear expressions.

        :param senses: Either an ordered collection of senses, one sense, or
           None. The default is None, in which case the solve uses a Minimize
           sense. Each sense can be either a sense object, that is either
           `ObjectiveSense.Minimize` or `Maximize`, or a string "min" or "max".

        :param tolerances: A tuple of two numbers or a sequence of such tuples.
           The first number is used as absolute tolerance, the second number is
           a relative tolerance. Accepts None, in which case, default
           tolerances are used (absolute=1e-6, relative=1e-4).

        Note:
            tolerances are used at each step to constraint the previous
            objective value to be be 'no worse' than the value found in the
            last pass. For example, if relative tolerance is 2% and pass #1 has
            found an objective of 100, then pass #2 will comstraint the first
            goal to be no greater than 102 if minimizing, or
            no less than 98, if maximizing.

        Return:
            If successful, returns a tuple with all pass solutions, reversed else None.
            The current solution of the model is the first solution in the tuple.
        """
        if tolerances is None:
            schemes = generate_constant(_ToleranceScheme(), count_max=None)
        elif is_number(tolerances):
            if 0 <= tolerances <= 1:
                schemes = generate_constant(_ToleranceScheme(relative=tolerances), count_max=None)
            else:
                self.fatal('lexicograhphic tolerances expects number in [0,1], got: {0}', tolerances)

        elif is_indexable(tolerances) and not isinstance(tolerances, tuple):
            schemes = []
            for t in tolerances:
                try:
                    sch = _ToleranceScheme(*t)
                    schemes.append(sch)
                except TypeError:
                    self.fatal('tolerances expects None, a 2-tuple of numbers, or a sequence of 2-tuples')
        else:
            try:
                sch = _ToleranceScheme(*tolerances)
                schemes = generate_constant(sch, count_max=None)
            except TypeError:
                try:
                    tolerances.compute_tolerance(0)
                    # recognized a callable object
                    schemes = generate_constant(tolerances, count_max=None)
                except AttributeError:
                    self.fatal('tolerances expects None, a tuple of (absolute, relative) tolerances, or a sequence of 2-tuples, got: {0!r}', tolerances)

        dump_pass_files = kwargs.get('dump_lps', False)
        old_objective_expr = self._objective_expr
        old_objective_sense = self._objective_sense
        if not goals:
            self.error("solve_lexicographic requires a non-empty list of goals, got: {0!r}".format(goals))
            return None

        if not is_indexable(goals):
            self.fatal("solve_lexicographic requires an indexable collection of goals, got: {0!s}", goals)

        pass_count = 0
        m = self
        results = []
        actual_goals = []
        # keep extra constraints, in order to remove them at the end.
        extra_cts = []
        for gi, g in enumerate(goals):
            goal_expr = self._lfactory._to_expr(g)
            goal_name = g.name

            actual_goals.append((goal_name or "pass%d" % (gi + 1), goal_expr))

        nb_goals = len(actual_goals)
        # --- senses ---
        if not is_iterable(senses, accept_string=False):
            senses = generate_constant(ObjectiveSense.parse(senses), count_max=nb_goals)

        # --- senses ---
        prev_step = (None, None, None)
        all_solutions = []
        try:
            for (goal_name, goal_expr), next_sense, scheme in izip(actual_goals, senses, schemes):
                if goal_expr.is_constant() and pass_count > 1:
                    self.warning("Constant expression in lexicographic solve: {0!s}, skipped", goal_expr)
                    continue
                pass_count += 1

                if pass_count > 1:
                    prev_goal, prev_obj, prev_sense = prev_step
                    tolerance = scheme.compute_tolerance(prev_obj)
                    if prev_sense.is_minimize():
                        pass_ct = m.add_constraint(prev_goal <= prev_obj + tolerance)
                    else:
                        pass_ct = m.add_constraint(prev_goal >= prev_obj - tolerance)

                    extra_cts.append(pass_ct)

                sense = self._resolve_sense(next_sense)

                self.trace("lexicographic: starting pass %d, %s: %s", pass_count, sense.action().lower(), goal_name)
                m.set_objective(sense, goal_expr)
                # print("-- current objective is: {0!s}".format(goal_expr))
                if dump_pass_files:  # pragma: no cover
                    pass_basename = 'lexico_%s_pass%d' % (self.name, pass_count)
                    self.dump_as_lp(basename=pass_basename)
                current_sol = m.solve(**kwargs)
                if current_sol is not None:
                    current_obj = m.objective_value
                    results.append(current_obj)
                    prev_step = (goal_expr, current_obj, sense)
                    all_solutions.append(current_sol)


                else:  # pragma: no cover
                    self.error("lexicographic fails. pass #%d, stop!", pass_count)
                    break
        finally:
            # print("-> start restoring model at end of lexicographic")
            while extra_cts:
                # using LIFO logic to avoid holes in indices.
                ct_to_remove = extra_cts.pop()
                # print("* removing constraint: name: {0}, idx: {1}".format(ct_to_remove.name, ct_to_remove.index))
                self._remove_constraint_internal(ct_to_remove)
            # restore objective whatsove
            self.set_objective(old_objective_sense, old_objective_expr)
            # print("<- end restoring model at end of lexicographic")

        # return a solution or None
        if len(all_solutions) == nb_goals:
            return tuple(reversed(all_solutions))
        else:
            # error inside loop
            return None

    def _has_solution(self):
        # INTERNAL
        return self._solution is not None

    def _set_solution(self, new_solution):
        """
        INTERNAL: Sets this solution as the model's current solution.
        Copies values to variables (for now, let's think more about this)
        :param new_solution:
        :return:
        """
        self._solution = new_solution

    def check_has_solution(self):
        # see if we can refine messages here...
        if self._solution is None:
            self.fatal("Model<{0}> did not solve successfully", self.name)

    def add_mip_start(self, mip_start_sol):
        """  Adds a (possibly partial) solution to use as a starting point for a MIP.

        This is valid only for models with binary or integer decision variables.
        The given solution must contain the value for at least one binary or integer variable.

        This feature is also known as warm start.

        Args:
            mip_start_sol (:class:`docplex.mp.solution.SolveSolution`): The solution object to use as a starting point.

        """
        if not self._solves_as_mip():
            self.error("Problem is not a MIP, MIP start ignored")
            return
        else:
            try:
                mip_start_sol.check_as_mip_start()
                self._mipstarts.append(mip_start_sol)
            except AttributeError:
                self.fatal("add_mip_starts expects solution, got: {0!r}", mip_start_sol)

    @property
    def objective_value(self):
        """ This property returns the value of the objective expression in the solution of the last solve.


        Raises an exception if the model has not been solved successfully.

        """
        self.check_has_solution()
        return self._objective_value()

    def _objective_value(self):
        return self.solution.objective_value

    def _reported_objective_value(self, failure_obj=0):
        return self.solution.objective_value if self.solution else failure_obj

    def _resolve_path(self, path_arg, basename_arg, extension):
        # INTERNAL
        if is_string(path_arg):
            if os.path.isdir(path_arg):
                if path_arg == ".":
                    path_arg = os.getcwd()
                return self._make_output_path(extension, basename_arg, path_arg)
            else:
                # add extension if not present (but not twice!)
                return path_arg if path_arg.endswith(extension) else path_arg + extension
        else:
            assert path_arg is None
            return self._make_output_path(extension, basename_arg, path_arg)

    def _make_output_path(self, extension, basename, path=None):
        return make_output_path2(self.name, extension, basename, path)

    lp_format = LP_format

    def _get_printer(self, exchange_format, do_raise=False):
        # INTERNAL
        printer_kwargs = {'full_obj': self._print_full_obj}
        printer = ModelPrinterFactory.new_printer(exchange_format, do_raise=False, **printer_kwargs)
        if not printer:  # pragma: no cover
            if do_raise:
                self.fatal("Unsupported output format: {0!s}", exchange_format)
            else:
                self.error("Unsupported output format: {0!s}", exchange_format)
        return printer

    def dump_as_lp(self, path=None, basename=None):
        return self.dump(path, basename, exchange_format=LP_format)

    def export_as_lp(self, path=None, basename=None, hide_user_names=False):
        """ Exports a model in LP format.

        Args:
            basename: Controls the basename with which the model is printed.
                Accepts None, a plain string, or a string format.
                if None, uses the model's name;
                if passed a plain string, the string is used in place of the model's name;
                if passed a string format (either with %s or {0}, it is used to format the
                model name to produce the basename of the written file.

            path: A path to write file, expects a string path or None.
                can be either a directory, in which case the basename
                that was computed with the basename argument, is appended to the directory to produce
                the file.
                If given a full path, the path is directly used to write the file, and
                the basename argument is not used.
                If passed None, the output directory will be ``tempfile.gettempdir()``.

            hide_user_names: A Boolean indicating whether or not to keep user names for
                variables and constraints. If True, all names are replaced by `x1`, `x2`, ... for variables,
                and `c1`, `c2`, ... for constraints.

        Examples:
            Assuming the model's name is `mymodel`:
            
            >>> m.export_as_lp()
            
            will write ``mymodel.lp`` in ``gettempdir()``.
            
            >>> m.export_as_lp(basename="foo")
            
            will write ``foo.lp`` in ``gettempdir()``.
            
            >>> m.export_as_lp(basename="foo", path="e:/home/docplex")

            will write file ``e:/home/docplex/foo.lp``.
            
            >>> m.export_as_lp("e/home/docplex/bar.lp")
            
            will write file ``e:/home/docplex/bar.lp``.
            
            >>> m.export_as_lp(basename="docplex_%s", path="e/home/") 
            
            will write file ``e:/home/docplex/docplex_mymodel.lp``.
        """
        return self.export(path, basename, hide_user_names, exchange_format=LP_format)

    def export_as_sav(self, path=None, basename=None):
        """ Exports a model in CPLEX SAV format.

        Exporting to SAV format requires that CPLEX is installed and
        available in PYTHONPATH. If the CPLEX DLL cannot be found, an exception is raised.

        Args:
            basename: Controls the basename with which the model is printed.
                Accepts None, a plain string, or a string format.
                If None, the model's name is used.
                If passed a plain string, the string is used in place of the model's name.
                If passed a string format (either with %s or {0}), it is used to format the
                model name to produce the basename of the written file.

            path: A path to write the file, expects a string path or None.
                Can be a directory, in which case the basename
                that was computed with the basename argument, is appended to the directory to produce
                the file.
                If given a full path, the path is directly used to write the file, and
                the basename argument is not used.
                If passed None, the output directory will be ``tempfile.gettempdir()``.

        Examples:
            See the documentation of  :func:`export_as_lp` for examples of pathname generation.
            The logic is identical for both methods.

        """
        return self._export(path, basename, use_engine=True,
                            hide_user_names=False,
                            exchange_format=SAV_format)

    def dump(self, path=None, basename=None, hide_user_names=False, exchange_format=LP_format):
        return self._export(path, basename,
                            use_engine=True,
                            hide_user_names=hide_user_names,
                            exchange_format=exchange_format)

    def export(self, path=None, basename=None,
               hide_user_names=False, exchange_format=LP_format):
        # INTERNAL
        return self._export(path, basename,
                            hide_user_names=hide_user_names,
                            exchange_format=exchange_format)

    def _export(self, path=None, basename=None,
                use_engine=False, hide_user_names=False,
                exchange_format=LP_format):
        # INTERNAL
        # path is either a nonempty path string or None
        self._checker.typecheck_string(path, accept_none=True, accept_empty=False)
        self._checker.typecheck_string(basename, accept_none=True, accept_empty=False)
        # INTERNAL
        extension = ""
        try:
            extension = exchange_format.extension
        except AttributeError:
            self.fatal("Not a supported exchange format: {0!s}", exchange_format)

        # combination of path/directory and basename resolution are done in resolve_path
        path = self._resolve_path(path, basename, extension)
        ret = self._export_to_path(path, hide_user_names, use_engine, exchange_format)
        if ret:
            self.trace("model file: {0} overwritten", path)
        return ret

    def _export_to_path(self, path, hide_user_names=False, use_engine=False, exchange_format=LP_format):
        # INTERNAL
        self.ensure_setup()
        try:
            if use_engine:
                # rely on engine for the dump
                self_engine = self.__engine
                if self_engine.has_cplex():
                    self_engine.dump(path)
                else:  # pragma: no cover
                    self.fatal(
                        "Format: {0} requires CPLEX, but a local CPLEX installation could not be found, file: {1} could not be written",
                        exchange_format.name, path)
                    return None
            else:
                # a path is not a stream but anyway it will work
                self._export_to_stream(stream=path, hide_user_names=hide_user_names, exchange_format=exchange_format)
            return path

        except IOError:
            self.error("Cannot open file: \"{0}\", model: {1} not exported".format(path, self.name))
            raise

    def _export_to_stream(self, stream, hide_user_names=False, exchange_format=LP_format):
        printer = self._get_printer(exchange_format, do_raise=True)
        if printer:
            printer.set_mangle_names(hide_user_names)
            printer.printModel(self, stream)

    def export_to_stream(self, stream, hide_user_names=False, exchange_format=LP_format):
        """ Export the model to an output stream in LP format.

        A stream can be one of:
            - a string, interpreted as a system path,
            - None, interpreted as `stdout`, or
            - a Python file-type object (e.g. a StringIO() instance).
                
        Args:
            stream: An object defining where the output will be sent.
            
            hide_user_names: An optional Boolean indicating whether or not to keep user names for
                variables and constraints. If True, all names are replaced by `x1`, `x2`, ... for variables,
                and `c1`, `c2`, ,... for constraints. Default is to keep user names.

        """
        self._export_to_stream(stream, hide_user_names, exchange_format)

    def export_as_lp_string(self, hide_user_names=False):
        """ Exports the model to a string in LP format.

        The output string contains the model in LP format.

        Args:
            hide_user_names: An optional Boolean indicating whether or not to keep user names for
                variables and constraints. If True, all names are replaced by `x1`, `x2`, ... for variables,
                and `c1`, `c2`, ... for constraints. Default is to keep user names.

        Returns:
            A string, containing the model exported in LP format.
        """
        return self.export_to_string(hide_user_names, LP_format)

    def export_to_string(self, hide_user_names=False, exchange_format=LP_format):
        # INTERNAL
        oss = StringIO()
        self._export_to_stream(oss, hide_user_names, exchange_format)
        return oss.getvalue()

    def export_parameters_as_prm(self, path=None, basename=None):
        # path is either a nonempty path string or None
        self._checker.typecheck_string(path, accept_none=True, accept_empty=False)
        self._checker.typecheck_string(basename, accept_none=True, accept_empty=False)

        # combination of path/directory and basename resolution are done in resolve_path
        prm_path = self._resolve_path(path, basename, extension='.prm')
        self.parameters.export_prm_to_path(path=prm_path)
        return prm_path

    def export_annotations(self, path=None, basename=None):
        from docplex.mp.anno import ModelAnnotationPrinter

        self._checker.typecheck_string(path, accept_none=True, accept_empty=False)
        self._checker.typecheck_string(basename, accept_none=True, accept_empty=False)

        # combination of path/directory and basename resolution are done in resolve_path
        anno_path = self._resolve_path(path, basename, extension='.ann')
        ap = ModelAnnotationPrinter()
        ap.print_to_stream(self, anno_path)

        return anno_path

    # advanced values
    def _get_engine_attribute(self, arg, attribute, do_check=True):
        if is_iterable(arg):
            if not arg:
                return []
            else:
                return self._get_engine_attributes_internal(arg, attribute)
        else:
            # defer checking to the checker
            if do_check:
                if attribute.requires_vars:
                    self._checker.typecheck_var(arg)
                else:
                    self._checker.typecheck_constraint(arg)
            attrs = self._get_engine_attributes_internal([arg], attribute)
            return attrs[0]

    def _get_engine_attributes_internal(self, mobjs, attribute):
        attr_name = attribute.name
        is_for_vars = attribute.requires_vars
        if not self.solution.is_attributes_fetched(attribute.name):
            if is_for_vars:
                indices = [v._index for v in self.iter_variables()]
                mapper = lambda idx: self._var_by_index(idx)
            else:
                indices = [ct._index for ct in self.iter_linear_constraints()]
                mapper = lambda idx: self.get_constraint_by_index(idx)
            # get index to value from engine
            if indices:
                attr_idx_map = self.__engine.get_solve_attribute(attr_name, indices)
            else:
                # cplex wil crash if called with an empty list (!)
                attr_idx_map = {}
            self.solution._store_attribute_result(attr_name, attr_idx_map, obj_mapper=mapper)
        return self.solution.get_attribute(mobjs, attr_name)

    def check_solved_as_lp(self, arg):
        # first check a solution exists
        self.check_has_solution()
        return self.__engine.solved_as_lp() or self.solves_as_lp(arg)

    def solves_as_lp(self, arg):
        # INTERNAL
        if self._solves_as_mip():
            self.fatal('{0} are only available for LP problems'.format(arg))

    def _check_ct_or_ct_seq(self, arg):
        if is_iterable(arg):
            return self._checker.typecheck_constraint_seq(arg)
        else:
            self._checker.typecheck_constraint(arg)
            return arg

    def dual_values(self, cts):
        self.check_solved_as_lp(arg='dual values')
        dual_arg = self._check_ct_or_ct_seq(cts)
        duals = self._get_engine_attribute(dual_arg, SolveAttribute.duals)
        return duals

    def _dual_value1(self, ct):
        self.check_solved_as_lp(arg='dual values')
        duals = self._get_engine_attribute(ct, SolveAttribute.duals)
        return duals

    def slack_values(self, cts):
        self.check_has_solution()
        slack_arg = self._check_ct_or_ct_seq(cts)
        return self._get_engine_attribute(slack_arg, SolveAttribute.slacks)

    def _slack_value1(self, ct):
        self.check_has_solution()
        return self._get_engine_attribute(ct, SolveAttribute.slacks)

    def reduced_costs(self, dvars):
        self.check_solved_as_lp(arg='reduced costs')
        if is_iterable(dvars):
            rc_arg = self._checker.typecheck_var_seq(dvars)
        else:
            self._checker.typecheck_var(dvars)
            rc_arg = dvars
        return self._get_engine_attribute(rc_arg, SolveAttribute.reduced_costs, do_check=False)

    def _reduced_cost1(self, dvar):
        self.check_solved_as_lp(arg='reduced costs')
        return self._get_engine_attribute(dvar, SolveAttribute.reduced_costs, do_check=False)

    DEFAULT_VAR_VALUE_QUOTED_SOLUTION_FMT = '  \"{varname}\"={value:.{prec}f}'
    DEFAULT_VAR_VALUE_UNQUOTED_SOLUTION_FMT = '  {varname}={value:.{prec}f}'
    DEFAULT_OBJECTIVE_FMT = "{0}: {1:.{prec}f}"

    @classmethod
    def supports_logical_constraints(cls):
        return cls()._supports_logical_constraints()

    def _supports_logical_constraints(self):
        # INTERNAL
        ok, _ = self.get_engine().supports_logical_constraints()
        return ok

    def check_logical_constraint_support(self):
        # INTERNAL
        ok, why = self.get_engine().supports_logical_constraints()
        if not ok:
            assert why
            self.fatal(msg=why)


    def _has_username_with_spaces(self):
        for v in self.iter_variables():
            if v.has_user_name() and ' ' in v.name:
                return True
        else:
            return False

    def print_solution(self, print_zeros=False,
                       objective_fmt=DEFAULT_OBJECTIVE_FMT,
                       var_value_fmt=None,
                       **kwargs):
        """  Prints the values of the model variables after a solve.

        Only valid after a successful solve. If the model has not been solved successfully, an
        exception is raised.

        Args:
            print_zeros (Boolean): If False, only non-zero values are printed. Default is False.

            objective_fmt : A format string in format syntax. The default printout is objective: xx where xx is formatted as a float with prec digits. The value of prec is computed automatically by DOcplex, either 0 if the objective expression is discrete or the model's float precision.

            var_value_fmt : A format string to format the variable name and value. Again, the default uses the automatically computed precision.

        """
        self.check_has_solution()
        if var_value_fmt is None:
            if self._has_username_with_spaces():
                var_value_fmt = self.DEFAULT_VAR_VALUE_QUOTED_SOLUTION_FMT
            else:
                var_value_fmt = self.DEFAULT_VAR_VALUE_UNQUOTED_SOLUTION_FMT
        if not self.has_objective():
            var_value_fmt = var_value_fmt[2:]
        # scope of variables.
        iter_vars = self.iter_variables() if print_zeros else None
        # if some username has a whitespace, use quoted format
        self.solution.display(print_zeros=print_zeros,
                              header_fmt=None,
                              value_fmt=var_value_fmt,
                              iter_vars=iter_vars, **kwargs)

    def report(self):
        """  Prints the value of the objective and the KPIs.
        Only valid after a successful solve, otherwise states that the model is not solved.
        """
        if self._has_solution():
            used_prec = 0 if self.objective_expr.is_discrete() else self._float_precision
            print("* model {0} solved with objective = {1:.{prec}f}".format(self.name,
                                                                            self._objective_value(), prec=used_prec))
            self.report_kpis()
        else:
            self.info("Model {0} has not been solved successfully, no reporting done.".format(self.name))

    def report_kpis(self, selected_kpis=None, kpi_header_format='*  KPI: {1:<{0}} = '):
        """  Prints the values of the KPIs.
        Only valid after a successful solve.
        """
        kpi_format = kpi_header_format + self._float_meta_format % (2,)
        printed_kpis = list(selected_kpis if is_iterable(selected_kpis) else self.iter_kpis())
        try:
            max_kpi_name_len = max(len(k.name) for k in printed_kpis) # max() raises ValueError on empty
        except ValueError:
            max_kpi_name_len = 0
        for kpi in printed_kpis:
            kpi_value = kpi.compute()
            if type(kpi_format) != type(kpi.name):
                # infamous mix of str and unicode. Should happen only
                # in py2. Let's convert things
                if isinstance(kpi_format, str):
                    kpi_format = kpi_format.decode('utf-8')
                else:
                    kpi_format = kpi_format.encode('utf-8')
            output = kpi_format.format(max_kpi_name_len, kpi.name, kpi_value)
            try:
                print(output)
            except UnicodeEncodeError:
                encoding = sys.stdout.encoding if sys.stdout.encoding else 'ascii'
                print(output.encode(encoding,
                                    errors='backslashreplace'))

    def kpis_as_dict(self, s=None, kpi_filter=None, objective_key=None):
        """ Returns KPI values in a solutuion as a dictionary.

        Each KPI has a value in the solution. This method returns a dictionary of KPI values,
        indexed by KPI objects.

        :param s: an instance of solution, as returned by solve()
        :param kpi_filter: an optional predicate to filter some kpis.
            If provided, accepts a function taking one KPI as argument and
            returning a boolean. By default, all KPIs are returned.
        :param objective_key: an optional string key for th eobjective value. If present, the
            value of the objective is added to the dictionary, with this key. By default, this parameter
            is None and the objective is *not* appended to the dictionary.

        :return:
            A dictionary mapping KPIs to values.

        See Also:
            :class:`docplex.mp.solution.SolveSolution`
        """
        if kpi_filter is None:
            kpi_filter = lambda _: True

        kpi_dict = {kpi: kpi.compute(s) for kpi in self.iter_kpis() if kpi_filter(kpi)}
        if objective_key:
            kpi_dict[objective_key] = s.objective_value
        return kpi_dict


    def _report_lexicographic_goals(self, goal_name_values, kpi_header_format):  # pragma: no cover
        # INTERNAL
        kpi_format = kpi_header_format + self._float_meta_format % (1,) # be safe even integer KPIs might yield floats
        printed_kpis = goal_name_values if is_iterable(goal_name_values) else self.iter_kpis()
        for goal_name, goal_expr in printed_kpis:
            goal_value = goal_expr.solution_value
            print(kpi_format.format(goal_name, goal_value))

    def iter_kpis(self):
        """ Returns an iterator over all KPIs in the model.

        Returns:
           An iterator object.
        """
        return iter(self._allkpis)

    def kpi_by_name(self, name, try_match=True, match_case=False, do_raise=True):
        """ Fetches a KPI from a string.

        This method fetches a KPI from a string, using either exact naming or trying
        to match a substring of the KPI name.

        Args:
            name (string): The string to be matched.
            try_match (Boolean): If True, returns KPI whose name is not equal to the
                argument, but contains it. Default is True.
            match_case: If True, looks for a case-exact match, else ignores case. Default is False.
            do_raise: If True, raise an exception when no KPI is found.

        Example:
            If the KPI name is "Total CO2 Cost" then fetching with argument `co2` and `match_case` to False
            will succeed. If `match_case` is True, then no KPI will be returned.

        Returns:
            The KPI expression if found. If the search fails, either raises an exception or returns a dummy
            constant expression with 0.
        """
        for kpi in iter(reversed(self._allkpis)):
            kpi_name = kpi.name
            ok = False
            if kpi_name == name:
                ok = True
            elif try_match:
                if match_case:
                    ok = kpi_name.find(name) >= 0
                else:
                    ok = kpi_name.lower().find(name.lower()) >= 0
            if ok:
                return kpi
        else:
            if do_raise:
                self.fatal('Cannot find any KPI matching: "{0:s}"', name)
            else:
                return self._lfactory.new_zero_expr()

    def kpi_value_by_name(self, name, try_match=True, match_case=False, do_raise=True):
        """ Returns a KPI value from a KPI name.

        This method fetches a KPI value from a string, using either exact naming or trying
        to match a substring of the KPI name.

        Args:
            name (str): The string to be matched.
            try_match (Bool): If True, returns KPI whose name is not equal to the
                argument, but contains it. Default is True.
            match_case: If True, looks for a case-exact match, else ignores case. Default is False.
            do_raise: If True, raise an exception when no KPI is found.

        Example:
            If the KPI name is "Total CO2 Cost" then fetching with argument `co2` and `match_case` to False
            will succeed. If `match_case` is True, then no KPI will be returned.

        Note:
            Expression KPIs require a valid solution to be computed. Make sure that the model contains
            a valid solution before computing an expression-based KPI, otherwise an exception will be raised.
            Functional KPIs do not require a valid solution.

        Returns:
            float: The KPI value.
        """
        kpi = self.kpi_by_name(name, try_match, match_case=match_case, do_raise=do_raise)
        return kpi.compute()

    def add_kpi(self, kpi_arg, publish_name=None):
        """ Adds a Key Performance Indicator to the model.

        Key Performance Indicators (KPIs) are objects that can be evaluated after a solve().
        Typical use is with decision expressions, the evaluation of which return the expression's solution value.

        KPI values are displayed with the method :func:`report_kpis`.

        Args:
            kpi_arg:  Accepted arguments are either an expression, a lambda function with one argument or
                an instance of a subclass of abstract class KPI.

            publish_name (string, optional): The published name of the KPI.

        Note:
            If no publish_name is provided, DOcplex will use the name of the argument. If none,
            it will use the string representation of the argument.

        Example:
            `model.add_kpi(x+y+z, "Total Profit")` adds the expression `(x+y+z)` as a KPI with the name "Total Profit".

            `model.add_kpi(x+y+z)` adds the expression `(x+y+z)` as a KPI with
            the name "x+y+z", assumng variables x,y,z have names 'x', 'y', 'z' (resp.)

        Returns:
            The newly added KPI instance.

        See Also:
            :class:`docplex.mp.kpi.KPI`,
            :class:`docplex.mp.kpi.DecisionKPI`
        """
        new_kpi = self._lfactory.new_kpi(kpi_arg, publish_name)
        new_kpi_name = new_kpi.get_name()
        for kp in self.iter_kpis():
            if kp.name == new_kpi_name:
                self.warning("Duplicate KPI name \"{0!s}\" ", new_kpi_name)
        self._allkpis.append(new_kpi)
        return new_kpi

    def remove_kpi(self, kpi_arg):
        """ Removes a Key Performance Indicator from the model.

        Args:
            kpi_arg:  A KPI instance that was previously added to the model. Accepts either a KPI object or a string.
                If passed a string, looks for a KPI with that name.

        See Also:
            :func:`add_kpi`
            :class:`docplex.mp.kpi.KPI`,
            :class:`docplex.mp.kpi.DecisionKPI`
        """
        if is_string(kpi_arg):
            kpi = self.kpi_by_name(kpi_arg)
            if kpi:
                self._allkpis.remove(kpi)
                kpi.notify_removed()
        else:
            for k, kp in enumerate(self._allkpis):
                if kp is kpi_arg:
                    kx = k
                    break
            else:
                kx = -1
            if kx >= 0:
                removed_kpi = self._allkpis.pop(kx)
                removed_kpi.notify_removed()

            else:
                self.warning('Model.remove_kpi() cannot interpret this either as a string or as a KPI: {0!r}', kpi_arg)

    def clear_kpis(self):
        ''' Clears all KPIs defined in the model.

        
        '''
        self._allkpis = []

    @property
    def number_of_kpis(self):
        return len(self._allkpis)

    def add_progress_listener(self, listener):
        self._checker.typecheck_progress_listener(listener)
        self._add_progress_listener(listener)

    def _add_progress_listener(self, listener):
        self._progress_listeners.append(listener)

    def remove_progress_listener(self, listener):
        self._progress_listeners.remove(listener)

    def iter_progress_listeners(self):  # pragma: no cover
        return iter(self._progress_listeners)

    def _fire_start_solve_listeners(self):
        for l in self._progress_listeners:
            l.notify_start()

    def _fire_end_solve_listeners(self, has_solution, objective_value):
        for l in self._progress_listeners:
            l.notify_end(has_solution, objective_value)

    def fire_jobid(self, jobid):  # pragma: no cover
        # INTERNAL
        for l in self._progress_listeners:
            l.notify_jobid(jobid)

    def fire_progress(self, progress_data):  # pragma: no cover
        for l in self._progress_listeners:
            l.notify_progress(progress_data)

    def clear_progress_listeners(self):
        self._progress_listeners = []

    def prettyprint(self, out=None):
        ModelPrinterFactory.new_pretty_printer().printModel(self, out=out)

    pprint = prettyprint

    def pprint_as_string(self):
        oss = StringIO()
        ModelPrinterFactory.new_pretty_printer().printModel(self, out=oss)
        return oss.getvalue()


    def clone(self, new_name=None):
        """ Makes a deep copy of the model, possibly with a new name.
        Decision variables, constraints, and objective are copied.

        Args:
            new_name (string): The new name to use. If None is provided, returns a "Copy of xxx" where xxx is the original model name.

        :returns: A new model.

        :rtype: :class:`docplex.mp.model.Model`
        """
        return self.copy(new_name)

    def copy(self, copy_name=None, removed_cts=None, relax=False):
        # INTERNAL
        actual_copy_name = copy_name or "Copy of %s" % self.name
        # copy kwargs
        copy_kwargs = self._get_kwargs()
        copy_model = Model(name=actual_copy_name, **copy_kwargs)

        # clone variable containers
        ctn_map = {}
        for ctn in self.iter_var_containers():
            copied_ctn = ctn.copy(copy_model)
            copy_model._add_var_container(copied_ctn)
            ctn_map[ctn] = copied_ctn

        # clone variables
        var_mapping = {}
        continuous = self.continuous_vartype
        for v in self.iter_variables():
            if not v.is_generated():
                copied_type = continuous if relax else v.vartype
                copied_var = copy_model._var(copied_type, v.lb, v.ub, v.name)
                var_ctn = v._container
                if var_ctn:
                    copied_var._container = ctn_map.get(var_ctn)
                var_mapping[v] = copied_var

        # clone PWL functions and add them to var_mapping
        for pwl_func in self.iter_pwl_functions():
            copied_pwl_func = pwl_func.copy(copy_model, var_mapping)
            var_mapping[pwl_func] = copied_pwl_func

        # copy constraints
        setof_removed_cts = set(removed_cts) if removed_cts else {}
        linear_cts = []
        for ct in self.iter_constraints():
            if not ct.is_generated() and ct not in setof_removed_cts:
                if isinstance(ct, PwlConstraint):
                    continue  # PwlConstraint copy is handled when resolving _f_var on copy of PwlExpr
                copied_ct = ct.copy(copy_model, var_mapping)
                if isinstance(ct, LinearConstraint):
                    linear_cts.append(copied_ct)
                else:
                    if linear_cts:
                        # add stored linear cts
                        copy_model.add_constraints(linear_cts)
                        linear_cts = []
                    # always add the non linear ct single (TODO: avoid checks here)
                    copy_model.add_constraint(copied_ct)

        if linear_cts:
            copy_model.add_constraints(linear_cts)

        # clone objective
        if self.is_optimized():
            copy_model.set_objective(self.objective_sense, self.objective_expr.copy(copy_model, var_mapping))

        # clone kpis
        for kpi in self.iter_kpis():
            copy_model.add_kpi(kpi.copy(copy_model, var_mapping))

        if self.context:
            copy_model.context = self.context.copy()

        # clone sos
        for sos in self.iter_sos():
            if not sos.is_generated():
                copy_model._register_sos(sos.copy(copy_model, var_mapping))

        # clone params (some day)
        return copy_model

    def refresh_model(self, do_setup=True):
        # compatibility with AbtractModel
        pass  # pragma: no cover

    def setup(self):
        # compatibility with AbtractModel
        pass  # pragma: no cover

    def _sync_constraint_indices(self, ct_iter=None):
        # INTERNAL: check only when CPLEX is present.
        self_engine = self.__engine
        if self_engine.has_cplex():
            self_engine._sync_constraint_indices(ct_iter or self.iter_constraints())

    def _sync_var_indices(self):
        self_engine = self.__engine
        if self_engine.has_cplex():
            self_engine._sync_var_indices(self.iter_variables())

    def end(self):
        """ Terminates a model instance.

        Since this method destroys the objects associated with the model, you must not use the model
        after you call this member function.

        """
        self._clear_internal(terminate=True)
        self._clear_engine()

    @property
    def parameters(self):
        """ This property returns the root parameter group of the model.

        The root parameter group models the parameter hirerachy.
        It is the way to access any CPLEX parameter and get or set its value.

        Examples:

            .. code-block:: python

               model.parameters.mip.tolerances.mipgap

            Returns the parameter itself, an instance of the `Parameter` class.

            To get the value of the parameter, use the `get()` method, as in:

            .. code-block:: python

               model.parameters.mip.tolerances.mipgap.get()
               >>> 0.0001

            To change the value of the parameter, use a standard Python assignment:

            .. code-block:: python

               model.parameters.mip.tolerances.mipgap = 0.05
               model.parameters.mip.tolerances.mipgap.get()
              >>> 0.05

            Assignment is equivalent to the `set()` method:

            .. code-block:: python

               model.parameters.mip.tolerances.mipgap.set(0.02)
               model.parameters.mip.tolerances.mipgap.get()
               >>> 0.02

        Returns:
            The root parameter group, an instance of the `ParameterGroup` class.

        """
        context_params = self.context.cplex_parameters
        if not self._synced_params:
            self._sync_params(context_params)
            self._synced_params = True
        return context_params

    def get_parameter_from_id(self, parameter_cpx_id):
        """ Finds a parameter from a CPLEX id code.

        Args:
            parameter_cpx_id: A CPLEX parameter id (positive integer, for example, 2009 is mipgap).

        :returns: An instance of :class:`docplex.mp.params.parameters.Parameter` if found, else None.
        """
        assert parameter_cpx_id >= 0
        for p in self.parameters.generate_params():
            if p.cpx_id == parameter_cpx_id:
                return p
        else:
            return None

    def apply_parameters(self):
        self._apply_parameters_to_engine(self.parameters)

    def _sync_parameters_to_engine(self, parameters):
        self._apply_parameters_to_engine(parameters)

    def _apply_parameters_to_engine(self, parameters_to_use):
        if parameters_to_use is not None:
            self_engine = self.__engine
            for param in parameters_to_use:
                self_engine.set_parameter(param, param.current_value)

    # with protocol
    def __enter__(self):
        return self

    def __exit__(self, atype, avalue, atraceback):
        # terminate the model upon exiting a 'with' block.
        self.end()

    def __iadd__(self, e):
        # implements the "+=" dialect a la PulP
        self.add(e)
        return self

    def _resync(self):
        # INTERNAL
        self._lfactory.resync_whole_model()

    def refresh_engine(self, new_agent=None):
        # INTERNAL
        self._clear_engine()
        # attach a new engine
        self.set_new_engine_from_agent(new_agent)
        # resync model to engine.
        self._resync()

    def run_feasopt(self, relaxables, relax_mode):
        relaxable_list = _to_list(relaxables)
        groups = []
        try:
            for pref, ctseq in relaxable_list:
                self._checker.typecheck_num(pref)
                cts = _to_list(ctseq)
                for ct in cts:
                    self._checker.typecheck_constraint(ct)
                groups.append((pref, cts))
        except ValueError:
            self.fatal("expecting container with (preference, constraints), got: {0!s}", relaxable_list)

        feasible = self.__engine.solve_relaxed(mdl=self, relaxable_groups=groups,
                                               prio_name='',
                                               relax_mode=relax_mode)
        return feasible

    def add_sos1(self, dvars, name=None):
        ''' Adds  an SOS of type 1 to the model.

        Args:
            dvars: The variables in the special ordered set.
                This method only accepts ordered sequences of variables or iterators.
                Unordered collections (dictionaries, sets) are not accepted.

            name: An optional name.

        Returns:
            The newly added SOS.
        '''
        return self.add_sos(dvars, sos_arg=SOSType.SOS1, name=name)

    def add_sos2(self, dvars, name=None):
        ''' Adds  an SOS of type 2 to the model.

        Args:
           dvars: The variables in the specially ordered set.
                This method only accepts ordered sequences of variables or iterators.
                Unordered collections (dictionaries, sets) are not accepted.
           name: An optional name.

        Returns:
            The newly added SOS.
        '''
        return self.add_sos(dvars, sos_arg=SOSType.SOS2, name=name)

    def add_sos(self, dvars, sos_arg, name=None):
        ''' Adds  an SOS to the model.

        Args:
           sos_arg: The SOS type. Valid values are numerical (1 and 2) or enumerated (`SOSType.SOS1` and
              `SOSType.SOS2`).
           dvars: The variables in the special ordered set.
                This method only accepts ordered sequences of variables or iterators.
                Unordered collections (dictionaries, sets) are not accepted.
           name: An optional name.

        Returns:
            The newly added SOS.
        '''
        sos_type = SOSType.parse(sos_arg)
        msg = 'Model.add_%s() expects an ordered sequence (or iterator) of variables' % sos_type.lower()
        self._checker.check_ordered_sequence(arg=dvars, header=msg)
        var_seq = self._checker.typecheck_var_seq(dvars)
        var_list = list(var_seq)  # we need len here.
        if len(var_list) < sos_type.min_size():
            self.fatal("A {0:s} variable set must contain at least {1:d} variables, got: {2:d}",
                       sos_type.name, sos_type.min_size(), len(var_list))
        # creates a new sos object
        return self._add_sos(dvars, sos_type, name)

    def _add_sos(self, dvars, sos_type, name=None):
        # INTERNAL
        new_sos = self._lfactory.new_sos(dvars, sos_type=sos_type, name=name)
        self._register_sos(new_sos)
        return new_sos

    def _register_sos(self, new_sos):
        self._allsos.append(new_sos)

    def iter_sos(self):
        ''' Iterates over all SOS sets in the model.

        Returns:
            An iterator object.
        '''
        return iter(self._allsos)

    @property
    def number_of_sos(self):
        ''' This property returns the total number of SOS sets in the model.

        '''
        return len(self._allsos)

    def clear_sos(self):
        ''' Clears all SOS sets in the model.
        '''
        self._allsos = []

    def _generate_sos(self, sos_type):
        # INTERNAL
        for sos_set in self.iter_sos():
            if sos_set.sos_type == sos_type:
                yield sos_set

    def iter_sos1(self):
        ''' Iterates over all SOS1 sets in the model.

        Returns:
            An iterator object.
        '''
        return self._generate_sos(SOSType.SOS1)

    def iter_sos2(self):
        ''' Iterates over all SOS2 sets in the model.

        Returns:
            An iterator object.
        '''
        return self._generate_sos(SOSType.SOS2)

    @property
    def number_of_sos1(self):
        ''' This property returns the total number of SOS1 sets in the model.

        '''
        return sum(1 for _ in self.iter_sos1())

    @property
    def number_of_sos2(self):
        ''' This property returns the total number of SOS2 sets in the model.

        '''
        return sum(1 for _ in self.iter_sos2())

    def piecewise(self, preslope, breaksxy, postslope, name=None):
        """  Adds a piecewise linear function (PWL) to the model, using breakpoints to specify the function.

        Args:
            preslope: Before the first segment of the PWL function there is a half-line; its slope is specified by
                        this argument.
            breaksxy: A list `(x[i], y[i])` of coordinate pairs defining segments of the PWL function.
            postslope: After the last segment of the the PWL function there is a half-line; its slope is specified by
                        this argument.
            name: An optional name.

        Example::

            # Creates a piecewise linear function whose value if '0' if the `x_value` is `0`, with a slope
            # of -1 for negative values and +1 for positive value
            model = Model('my model')
            model.piecewise(-1, [(0, 0)], 1)

            # Note that a PWL function may be discontinuous. Here is an example of a step function:
            model.piecewise(0, [(0, 0), (0, 1)], 0)

        Returns:
            The newly added piecewise linear function.
        """
        if preslope is None:
            self._checker.fatal("argument 'preslope' must be defined")
        if breaksxy is None:
            self._checker.fatal("argument 'breaksxy' must be defined")
        if postslope is None:
            self._checker.fatal("argument 'postslope' must be defined")
        PwlFunction.check_number(self._checker, preslope)
        PwlFunction.check_number(self._checker, postslope)
        PwlFunction.check_list_pair_breaksxy(self._checker, breaksxy)
        return self._piecewise(PwlFunction._PwlAsBreaks(preslope, breaksxy, postslope), name)

    def piecewise_as_slopes(self, slopebreaksx, lastslope, anchor=(0, 0), name=None):
        """  Adds a piecewise linear function (PWL) to the model, using a list of slopes and x-coordinates.

        Args:
            slopebreaksx: A list of tuple pairs `(slope[i], breakx[i])` of slopes and x-coordinates defining the slope of
                        the piecewise function between the previous breakpoint (or minus infinity if there is none)
                        and the breakpoint with x-coordinate `breakx[i]`.
                        For representing a discontinuity, two consecutive pairs with the same value for `breakx[i]`
                        are used. The value of `slope[i]` in the second pair is the discontinuity gap.
            lastslope: The slope after the last specified breakpoint.
            anchor: The coordinates of the 'anchor point'. The purpose of the anchor point is to ground the piecewise
                        linear function specified by the list of slopes and breakpoints.
            name: An optional name.
        Example::

            # Creates a piecewise linear function whose value if '0' if the `x_value` is `0`, with a slope
            # of -1 for negative values and +1 for positive value
            model = Model('my model')
            model.piecewise_as_slopes([(-1, 0)], 1, (0, 0))

            # Here is the definition of a step function to illustrate the case of a discontinuous PWL function:
            model.piecewise_as_slopes([(0, 0), (0, 1)], 0, (0, 0))

        Returns:
            The newly added piecewise linear function.
        """
        PwlFunction.check_number(self._checker, lastslope)
        PwlFunction.check_number_pair(self._checker, anchor)
        PwlFunction.check_list_pair_slope_breakx(self._checker, slopebreaksx, anchor)
        return self._piecewise(PwlFunction._PwlAsSlopes(slopebreaksx, lastslope, anchor), name)

    def _piecewise(self, pwl_def, name=None):
        pwl_func = self._lfactory.new_piecewise(pwl_def, name)
        self._register_one_pwl_func(pwl_func)
        return pwl_func

    def _register_one_pwl_func(self, pwl_func):
        self.__allpwlfuncs.append(pwl_func)

    def _add_pwl_expr(self, pwl_func, arg, name=None):
        pwl_func_usage_counter = self._pwl_counter.get(pwl_func, 0) + 1
        pwl_expr = self._lfactory.new_pwl_expr(pwl_func, arg, pwl_func_usage_counter)
        return pwl_expr

    def _add_pwl_constraint_internal(self, ct):
        """
        INTERNAL
        :param ct: The new PWL constraint to add
        :return:
        """
        ct_engine_index = self._create_engine_constraint(ct)
        self._register_one_pwl_constraint(ct, ct_engine_index)
        return ct

    def _register_one_pwl_constraint(self, ct, ct_index):
        self.__notify_new_model_object(
            "pwl", ct, ct_index, mobj_name=None, name_dir=None, idx_scope=self._pwl_scope, is_name_safe=True)

        self._register_pwl(ct)
        self.__allcts.append(ct)

    def _register_pwl(self, new_pwl_ct):
        # INTERNAL
        # Maintain the number of constraints associated to each piecewise function definition.
        # This counter is used when naming PWL constraints.
        self._pwl_counter[new_pwl_ct.pwl_func] = new_pwl_ct.usage_counter
        self._allpwl.append(new_pwl_ct)

    def iter_pwl_constraints(self):
        """ Iterates over all PWL constraints in the model.

        Returns:
            An iterator object.
        """
        return self.gen_constraints_with_type(PwlConstraint)

    @property
    def number_of_pwl_constraints(self):
        """ This property returns the total number of PWL constraints in the model.
        """
        return len(self._allpwl)

    def _ensure_benders_annotations(self):
        if self._benders_annotations is None:
            self._benders_annotations = {}
        return self._benders_annotations

    def set_benders_annotation(self, obj, group):
        if group is None:
            self_benders = self._benders_annotations
            if self_benders is not None and obj in self_benders:
                del self_benders[obj]
        else:
            self._checker.typecheck_int(group, accept_negative=False, caller='Model.set_benders_annotation')
            self._ensure_benders_annotations()[obj] = group

    def remove_benders_annotation(self, obj):
        self_benders = self._benders_annotations
        if self_benders:
            del self_benders[obj]

    def get_benders_annotation(self, obj):
        self_benders = self._benders_annotations
        return self_benders.get(obj) if self_benders is not None else None

    def iter_benders_annotations(self):
        self_benders = self._benders_annotations
        return six.iteritems(self_benders) if self_benders is not None else iter([])

    def clear_benders_annotations(self):
        self._benders_annotations = None

    def get_annotations_by_scope(self):
        # INTERNAL
        from collections import defaultdict
        annotated_by_scope = defaultdict(list)
        for obj, group in self.iter_benders_annotations():
            annotated_by_scope[obj.cplex_scope()].append((obj, group))
        return annotated_by_scope

    def has_benders_annotations(self):
        self_benders = self._benders_annotations
        return bool(self_benders)

    def get_annotation_stats(self):
        from collections import Counter
        annotated_by_scope = Counter()
        for obj, group in self.iter_benders_annotations():
            annotated_by_scope[obj.cplex_scope()] += 1
        return annotated_by_scope


    def register_callback(self, cb):
        return self.__engine.register_callback(cb)

    def resolve(self):
        # INTERNAL
        self._objective_expr.resolve()
        for c in self.iter_constraints():
            c.resolve()
