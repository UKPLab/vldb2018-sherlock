# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------
# Author: Olivier OUDOT, IBM Analytics, France Lab, Sophia-Antipolis

"""
This module contains principally the class :class:`CpoModel` that handles all the
elements that compose a CPO model:

 * the variables of the domain (integer variables, interval variables, sequence variables and state functions),
 * the constraints of the model,
 * optional objective value(s),
 * optional search phases,
 * optional starting point (available for CPO solver release greater or equal to 12.7.0).

The different model expressions and elements are created using services provided by modules:

 * :mod:`docplex.cp.expression` for the simple expression elements,
 * :mod:`docplex.cp.modeler` to build complex expressions and constraints using the specialized CP Optimizer functions.

The solving of the model is handled by an object of class :class:`~docplex.cp.solver.solver.CpoSolver` that takes
this model as parameter.
However, most important solving functions are callable directly from this model to avoid explicit
creation of the *CpoSolver* object:

 * :meth:`~CpoModel.solve` solves the model and returns an object of class :class:`~docplex.cp.solution.CpoSolveResult`.
 * :meth:`~CpoModel.start_search` creates a solver that can iterate over multiple solutions of the model.
 * :meth:`~CpoModel.refine_conflict` identifies a minimal conflict for the infeasibility and return it as an object
   of class :class:`~docplex.cp.solution.CpoRefineConflictResult`.
 * :meth:`~CpoModel.propagate` invokes the propagation on the current model and returns a partial solution in an object
   of class :class:`~docplex.cp.solution.CpoSolveResult`.

All these methods are taking a variable number of optional parameters that allow to modify the solving context.
The list of arguments is not limited. Each named argument is used to replace the leaf attribute that has
the same name in the global *context* structure initialized in the module :mod:`docplex.cp.config` and its
customizations.

The most important of these parameters are:

 * **context** sets a complete customized context to be used instead of the default one defined in the module :mod:`docplex.cp.config`,
 * **params** overwrites the solving parameters (object of class :class:`~docplex.cp.parameters.CpoParameters`)
   that are defined in the *context* object,
 * **url** and **key** modify access to *DOcplexcloud* (if it is the selected solving agent),
 * **agent** forces the selection of a particular solving agent,
 * **trace_cpo** activates the printing of the model in CPO format before its solve,
 * any CP Optimizer solving parameter, as defined in module :mod:`docplex.cp.parameters`, such as:

    * **TimeLimit** indicates a limit in seconds in the time spent in the solve,
      or **ConflictRefinerTimeLimit** that does the same for conflict refiner,
    * **LogVerbosity**, with values in ['Quiet', 'Terse', 'Normal', 'Verbose'],
    * **Workers** specifies the number of threads assigned to solve the model (default value is the number of cores),
    * **SearchType**, with value in ['DepthFirst', 'Restart', 'MultiPoint', 'Auto'], to select a particular solving algorithm,
    * **RandomSeed** changes the seed of the random generator,
    * and so on.

Detailed description
--------------------
"""

# Following imports required to allow modeling just importing this module
from docplex.cp.modeler import *
from docplex.cp.solution import *
from docplex.cp.expression import *
from docplex.cp.function import *
from docplex.cp.solver.solver_listener import CpoSolverListener

# Imports required locally
import docplex.cp.expression as expression
import docplex.cp.modeler as modeler
from docplex.cp.solver.solver import CpoSolver
from docplex.cp.cpo_compiler import CpoCompiler
import docplex.cp.config as config
import docplex.cp.utils as utils
import inspect
import sys
import time
import copy


###############################################################################
##  Constants
###############################################################################

# List of all modeler public functions
_MODELER_PUBLIC_FUNCTIONS = list_module_public_functions(modeler, ('maximize', 'minimize'))
#_MODELER_PUBLIC_FUNCTIONS = list_module_public_functions(modeler)


###############################################################################
##  Public classes
###############################################################################

class CpoModel(object):
    """ This class is the Python container of a CPO model.
    """

    def __init__(self, name=None, sfile=None):
        """ Constructor.

        Args:
            name:  Model name, None for automatic (source file name).
            sfile: Source file, None for automatic.
        """
        ctx = config.get_default()
        super(CpoModel, self).__init__()
        self.format_version   = None     # Version of the CPO format
        self.expr_list        = []       # List of model root expressions as tuples (expression, location)
        self.parameters       = None     # Solving parameters
        self.search_phases    = []       # List of search phases
        self.starting_point   = None     # Starting point
        self.objective        = None     # Objective function
        self.map_expr         = {}       # Dictionary of expressions. Key is expression name.
        self.kpis             = {}       # Dictionary of KPIs. Key is publish name.
        self.listeners        = []       # Solver listeners

        # Indicate to set in the model information of source location
        self.source_loc       = ctx.get_by_path("model.add_source_location", True)
        # Indicate to always name added constraints (for conflict refiner)
        self.name_constraints = ctx.get_by_path("model.name_all_constraints", False)

        # Initialize times to compute modeling time
        self.create_time      = time.time()        # Model creation absolute time
        self.last_add_time    = self.create_time   # Last time something has been added to the model

        # Store filename of the calling Python source
        if sfile is None:
            mod = inspect.getmodule(inspect.stack()[1][0])
            if mod is not None:
                sfile = mod.__file__
        self.source_file = sfile

        # Store model name
        if name is None:
            if sfile:
                name = utils.get_file_name_only(sfile)
        self.name = name

        # Duplicate constructor functions to make them callable from the model
        self.integer_var       = expression.integer_var
        self.integer_var_list  = expression.integer_var_list
        self.integer_var_dict  = expression.integer_var_dict
        self.binary_var        = expression.binary_var
        self.binary_var_list   = expression.binary_var_list
        self.binary_var_dict   = expression.binary_var_dict
        self.interval_var      = expression.interval_var
        self.interval_var_list = expression.interval_var_list
        self.interval_var_dict = expression.interval_var_dict
        self.sequence_var      = expression.sequence_var
        self.transition_matrix = expression.transition_matrix
        self.tuple_set         = expression.tuple_set
        self.state_function    = expression.state_function

        # Copy all modeler functions in the model object
        for f in _MODELER_PUBLIC_FUNCTIONS:
            setattr(self, f.__name__, f)

        # Special case for builtin functions
        self.min = modeler.min_of
        self.max = modeler.max_of
        self.sum = modeler.sum_of
        self.abs = modeler.abs_of
        self.range = modeler.in_range
        self.all = modeler.all_of
        self.any = modeler.any_of


    def __enter__(self):
        # Implemented for compatibility with cplex
        return self


    def __exit__(self, exception_type, exception_value, traceback):
        # Implemented for compatibility with cplex
        return False  # No exception handling


    def add(self, expr):
        """ Adds a CP expression to the model.

        This method adds a CP expression to the model.
        All the variables that are used by this expression are automatically added to the model.

        The order in which expressions are added to the model is preserved when it is submitted for solving.

        Args:
            expr: Expression to add.
        Raises:
            CpoException in case of error.
        """
        # Check simple boolean expressions
        if is_bool(expr):
            assert expr, "Try to add an expression which is already false"
            return

        # Check expression
        assert isinstance(expr, CpoExpr), "Argument 'expr' should be a CpoExpr instead of " + str(type(expr))

        # Determine calling location
        if self.source_loc:
            f = inspect.currentframe().f_back
            loc = (f.f_code.co_filename, f.f_lineno)
        else:
            loc = None

        # Check type of expression
        etyp = expr.type
        etyp = expr.type
        if etyp in (Type_Constraint, Type_BoolExpr):
            # Check if expression is named
            if self.name_constraints and not expr.name:
                expr.set_name(expression._CONSTRAINT_ID_ALLOCATOR.allocate())
            self.expr_list.append((expr, loc))
        elif etyp is Type_SearchPhase:
            self.search_phases.append((expr, loc))
        elif etyp is Type_Objective:
            if self.objective is not None:
                # if self.objective is not expr:
                #     raise CpoException("Only one objective function can be added to the model.")
                self.remove(self.objective)
            self.objective = expr
            self.expr_list.append((expr, loc))
        else:
            self.expr_list.append((expr, loc))

        # Add to the map of named expressions
        self._add_named_expr(expr)

        # Update last add time
        self.last_add_time = time.time()


    def remove(self, expr):
        """ Remove an expression from the model.

        This method removes from the model the first occurrence of the expression given as parameter.
        It does not remove the expression if it used as sub-expression of another expression.

        Args:
            expr: Expression to remove.
        Returns:
            True if expression has been removed, False if not found
        """
        etyp = expr.type

        # Process case of search phase
        if etyp is Type_SearchPhase:
            return self._remove_from_expr_list(expr, self.search_phases)

        # Check if it is current objective expression
        if expr is self.objective:
            self.objective = None

        # Remove from list of expressions
        return self._remove_from_expr_list(expr, self.expr_list)


    def minimize(self, expr):
        """ Add an objective expression to minimize.

        DEPRECATED: use add(minimize()) instead.

        Calling this method is equivalent to add(minimize(expr)) except that, if exist,
        the previously defined objective expression is removed to be replaced by this new one.

        Args:
            expr: Expression to minimize.
        Returns:
            Minimization expression that has been added
        """
        # Add new minimization expression
        res = minimize(expr)
        self.add(res)
        return res


    def maximize(self, expr):
        """ Add an objective expression to maximize.

        DEPRECATED: use add(maximize()) instead.

        Calling this method is equivalent to add(maximize(expr)) except that, if exist,
        the previously defined objective expression is removed to be replaced by this new one.

        Args:
            expr: Expression to maximize.
        Returns:
            Maximization expression that has been added
        """
        # Add new maximization expression
        res = maximize(expr)
        self.add(res)
        return res


    def set_parameters(self, params):
        """ Set the solving parameters associated to this model.

        Args:
            params: Solving parameters, object of class :class:`~docplex.cp.parameters.CpoParameters`, or None.
        """
        assert isinstance(params, CpoParameters), "argument 'params' should be an object of class CpoParameters"
        self.parameters = params


    def get_parameters(self):
        """ Get the solving parameters associated to this model.

        Returns:
            Solving parameters, object of class :class:`~docplex.cp.parameters.CpoParameters`, or None if not defined.
        """
        return self.parameters


    def set_search_phases(self, phases):
        """ Set a list of search phases

        Args:
            phases: Array of search phases, or single phase
        """
        # Check arguments
        if not is_array(phases):
            phases = [phases]

        # Determine calling location
        if self.source_loc:
            f = inspect.currentframe().f_back
            loc = (f.f_code.co_filename, f.f_lineno)
        else:
            loc = None

        # Set list of phases
        self.search_phases = []
        for p in phases:
            if not p.is_type(Type_SearchPhase):
                raise AssertionError("Argument 'phases' should be an array of SearchPhases")
            self.search_phases.append((p, loc))


    def add_search_phase(self, phase):
        """ Add a search phase to the list of search phases

        This method is deprecated since release 2.3. Use :meth:`~CpoModel.set_search_phases` or
        :meth:`~CpoModel.add` instead.

        Args:
            phase: Phase to add to the list
        """
        warnings.warn("Method 'add_search_phase' is deprecated since release 2.4.", DeprecationWarning)

        # Check arguments
        assert isinstance(phase, CpoExpr) and phase.is_type(Type_SearchPhase), "Argument 'phase' should be a SearchPhases"

        # Determine calling location
        if self.source_loc:
            f = inspect.currentframe().f_back
            loc = (f.f_code.co_filename, f.f_lineno)
        else:
            loc = None

        # Append to list of phases
        self.search_phases.append((phase, loc))


    def get_search_phases(self):
        """ Get the list of search phases.

        Returns:
            List of search phases (pairs (expression, location)), [] if none.
        """
        return self.search_phases


    def set_starting_point(self, stpoint):
        """ Set a model starting point.

        A starting point specifies a (possibly partial) solution that could be used by CP Optimizer
        to start the search.

        Starting point is available for CPO solver release greater or equal to 12.7.0.

        Args:
            stpoint: Starting point, object of class :class:`~docplex.cp.solution.CpoModelSolution`
        """
        assert (stpoint is None) or isinstance(stpoint, CpoModelSolution), "Argument 'stpoint' should be None or an object of class CpoModelSolution"
        self.starting_point = stpoint


    def get_starting_point(self):
        """ Get the model starting point

        Returns:
            Model starting point, None if none
        """
        return self.starting_point


    def add_kpi(self, var, name=None):
        # """ Add a a Key Performance Indicator to the model.
        #
        # A KPI is a model variable whose value is considered as representative of the global solution.
        # If the model is solved in a cloud context, these KPIs can be associated to the objective value in the
        # solve details that are sent periodically to the client.
        #
        # Args:
        #     var:              Model variable to be used as KPI(s).
        #     name (optional):  Name used to publish this KPI. If absent, variable name is used.
        # """
        assert isinstance(var, CpoVariable), "Argument 'var' should be a model variable"
        if name is None:
            name = var.get_name()
        self.kpis[name] = var


    def get_kpis(self):
        # """ Returns the dictionary of this model KPIs.
        #
        # Returns:
        #     Dictionary of KPIs. Key is publish name, value is model variable.
        # """
        return self.kpis


    def get_all_expressions(self):
        """ Gets the list of all model expressions

        Returns:
            List of model expressions
            Each expression is a tuple (expr, loc) where loc is a tuple (source_file, line).
        """
        return self.expr_list


    def get_expression(self, name):
        """ Gets an expression from its name (expression or variable).

        Args:
            name: Name of the expression.
        Returns:
            Expression, None if not found.
        """
        return self.map_expr.get(name)


    def get_all_variables(self):
        """ Gets the list of all model variables (for debugging purpose only).

        This method goes across all model expressions to identify all variables that are pointed by them.
        Calling this method on a big model may be slow.

        Returns:
            List of model variables.
        """
        # Initialize stack of expresssions to parse
        estack = [x for x, l in self.expr_list]
        estack.extend([x for x, l in self.search_phases])
        if self.objective:
            estack.append(self.objective)
        # Loop while expression stack is not empty
        varlist = []     # Result list
        doneset = set()  # Set of expressions already processed
        while estack:
            e = estack.pop()
            eid = id(e)
            if not eid in doneset:
                doneset.add(eid)
                if e.type.is_variable:
                    varlist.append(e)
                # Stack children expressions
                estack.extend(e.children)
        return varlist


    def get_optimization_expression(self):
        """ Gets the optimization expression (maximization or minimization).

        Returns:
            Optimization expression, None if satisfaction problem.
        """
        # Search last optimization expression
        for x, l in reversed(self.get_all_expressions()):
            if isinstance(x, CpoFunctionCall) and x.operation.is_optim:
                return x
        return None


    def get_name(self):
        """ Gets the name of the model.

        If the name is not explicitly defined, the name is the source file name without its extension.
        If source file name is also undefined, name is None.

        Returns:
            Name of the model, None if undefined.
        """
        return self.name


    def get_format_version(self):
        """ Gets the version of the CPO format

        Returns:
            String containing the version of the CPO format. None for default.
        """
        return self.format_version


    def get_source_file(self):
        """ Gets the name of the source file from which model has been created.

        Returns:
            Python source file name. None if undefined.
        """
        return self.source_file


    def get_modeling_duration(self):
        """ Get the time spent in modeling.

        This time is computes as difference between the last time an expression has been added
        and the model object creation time.

        Returns:
            Modeling duration in seconds
        """
        return self.last_add_time - self.create_time


    def print_information(self, out=None):
        """ Prints model information.

        Args:
            out: Output stream or file name, default is sys.stdout.
        """
        if out is None:
            out = sys.stdout
        if is_string(out):
            make_directories(os.path.dirname(out))
            with open(out, 'w') as f:
                self._write_information(f)
        else:
            self._write_information(out)


    def _write_information(self, out):
        """ Write model information on a output stream

        Args:
            out: Output stream
        """
        name = self.get_name()
        out.write("Model: {}\n".format(name if name else "Anonymous"))
        sfile = self.get_source_file()
        if sfile:
            out.write(" - source file: {}\n".format(sfile))
        lexpr = self.get_all_expressions()
        out.write(" - expressions: {}\n".format(len(lexpr)))
        out.write(" - modeling time: {0:.2f} sec\n".format(self.get_modeling_duration()))


    def solve(self, **kwargs):
        """ Solves the model.

        This method solves the model using the appropriate :class:`~docplex.cp.solver.solver.CpoSolver`
        created according to default solving context, possibly modified by the parameters of this method.

        The class :class:`~docplex.cp.solver.solver.CpoSolver` contains the actual implementation of this method,
        but also some others functions allowing to invoke more specialized functions.
        An advanced programming may require to explicitly create a CpoSolver instead of calling function at model level.
        Please refer to this class for more details.

        All necessary solving parameters are taken from the solving context that is constructed from the following list
        of sources, each one overwriting the previous:

           - the parameters that are set in the model itself,
           - the default solving context that is defined in the module :mod:`~docplex.cp.config`
           - the user-specific customizations of the context that may be defined (see :mod:`~docplex.cp.config` for details),
           - the optional arguments of this method.

        Args:
            context (Optional): Complete solving context.
                                If not given, solving context is the default one that is defined in the module
                                :mod:`~docplex.cp.config`.
            params (Optional):  Solving parameters (object of class :class:`~docplex.cp.parameters.CpoParameters`)
                                that overwrite those in the solving context.
            url (Optional):     URL of the DOcplexcloud service that overwrites the one defined in the solving context.
            key (Optional):     Authentication key of the DOcplexcloud service that overwrites the one defined in
                                the solving context.
            (param) (Optional): Any individual solving parameter as defined in class :class:`~docplex.cp.parameters.CpoParameters`
                               (for example *TimeLimit*, *Workers*, *SearchType*, etc).
            (others) (Optional): Any leaf attribute with the same name in the solving context
                                (for example *agent*, *trace_log*, *trace_cpo*, etc).
        Returns:
            Model solve result (object of class :class:`~docplex.cp.solution.CpoSolveResult`).
        Raises:
            :class:`~docplex.cp.utils.CpoException`: (or derived) if error.
        """
        solver = self._create_solver(**kwargs)
        msol = solver.solve()
        solver.end()
        return msol


    def start_search(self, **kwargs):
        """ Start a new search sequence to retrieve multiple solutions of the model.

        This method returns a new :class:`~docplex.cp.solver.solver.CpoSolver` object
        that acts as an iterator of the different solutions of the model.
        All solutions can be retrieved using a loop like:
        ::

           lsols = mdl.start_search()
           for sol in lsols:
               sol.print_solution()

        A such solution iteration can be interrupted at any time by calling :meth:`~docplex.cp.solver.solver.CpoSolver.end_search`
        that returns a fail solution including the last solve status.

        Note that, to be sure to retrieve all solutions and only once each,
        recommended parameters are *start_search(SearchType='DepthFirst', Workers=1)*

        Args:
            context (Optional): Complete solving context.
                                If not given, solving context is the default one that is defined in the module
                                :mod:`~docplex.cp.config`.
            params (Optional):  Solving parameters (object of class :class:`~docplex.cp.parameters.CpoParameters`)
                                that overwrite those in the solving context.
            url (Optional):     URL of the DOcplexcloud service that overwrites the one defined in the solving context.
            key (Optional):     Authentication key of the DOcplexcloud service that overwrites the one defined in
                                the solving context.
            (param) (Optional): Any individual solving parameter as defined in class :class:`~docplex.cp.parameters.CpoParameters`
                               (for example *TimeLimit*, *Workers*, *SearchType*, etc).
            (others) (Optional): Any leaf attribute with the same name in the solving context
                                (for example *agent*, *trace_log*, *trace_cpo*, etc).
        Returns:
            Object of class :class:`~docplex.cp.solver.solver.CpoSolver` allowing to iterate over the different solutions.
        Raises:
            :class:`~docplex.cp.utils.CpoException`: (or derived) if error.
        """
        solver = self._create_solver(**kwargs)
        return solver


    def refine_conflict(self, **kwargs):
        """ This method identifies a minimal conflict for the infeasibility of the current model.

        Given an infeasible model, the conflict refiner can identify conflicting constraints and variable domains
        within the model to help you identify the causes of the infeasibility.
        In this context, a conflict is a subset of the constraints and/or variable domains of the model
        which are mutually contradictory.
        Since the conflict is minimal, removal of any one of these constraints will remove that
        particular cause for infeasibility.
        There may be other conflicts in the model; consequently, repair of a given conflict
        does not guarantee feasibility of the remaining model.

        Conflict refiner is controlled by the following parameters, that can be set as parameters of this method:

         * ConflictRefinerBranchLimit
         * ConflictRefinerFailLimit
         * ConflictRefinerIterationLimit
         * ConflictRefinerOnVariables
         * ConflictRefinerTimeLimit

        that are described in module :mod:`docplex.cp.parameters`.

        Note that the general *TimeLimit* parameter is used as a limiter for each conflict refiner iteration, but the
        global limitation in time must be set using *ConflictRefinerTimeLimit* that is infinite by default.

        This method creates a new :class:`~docplex.cp.solver.solver.CpoSolver` with given arguments, and then call
        its method :meth:`~docplex.cp.solver.solver.CpoSolver.refine_conflict`.
        The class :class:`~docplex.cp.solver.solver.CpoSolver` contains the actual implementation of this method,
        but also some others functions allowing to invoke more specialized functions. An advanced programming may
        require to explicitly create a CpoSolver instead of calling function at model level.
        Please refer to this class for more details.

        This function is available on DOcplexcloud and with local CPO solver with release number greater or equal to 12.7.0.

        Args:
            context (Optional): Complete solving context.
                                If not given, solving context is the default one that is defined in the module
                                :mod:`~docplex.cp.config`.
            params (Optional):  Solving parameters (object of class :class:`~docplex.cp.parameters.CpoParameters`)
                                that overwrite those in the solving context.
            url (Optional):     URL of the DOcplexcloud service that overwrites the one defined in the solving context.
            key (Optional):     Authentication key of the DOcplexcloud service that overwrites the one defined in
                                the solving context.
            (param) (Optional): Any individual solving parameter as defined in class :class:`~docplex.cp.parameters.CpoParameters`
                               (for example *TimeLimit*, *Workers*, *SearchType*, etc).
            (others) (Optional): Any leaf attribute with the same name in the solving context
                                (for example *agent*, *trace_log*, *trace_cpo*, etc).
        Returns:
            List of constraints that cause the conflict (object of class :class:`~docplex.cp.solution.CpoRefineConflictResult`)
        Raises:
            :class:`~docplex.cp.utils.CpoNotSupportedException`: if method not available in the solver agent.
            :class:`~docplex.cp.utils.CpoException`: (or derived) if error.
        """
        solver = self._create_solver(**kwargs)
        rsol = solver.refine_conflict()
        solver.end()
        return rsol


    def propagate(self, cnstr=None, **kwargs):
        """ This method invokes the propagation on the current model.

        Constraint propagation is the process of communicating the domain reduction of a decision variable to
        all of the constraints that are stated over this variable.
        This process can result in more domain reductions.
        These domain reductions, in turn, are communicated to the appropriate constraints.
        This process continues until no more variable domains can be reduced or when a domain becomes empty
        and a failure occurs.
        An empty domain during the initial constraint propagation means that the model has no solution.

        The result is a object of class :class:`~docplex.cp.solution.CpoSolveResult`, the same than the one
        returned by the method :meth:`solve`.
        However, variable domains may not be completely defined.

        This method creates a new :class:`~docplex.cp.solver.solver.CpoSolver` with given arguments, and then call
        its method :meth:`~docplex.cp.solver.solver.CpoSolver.propagate`.
        The class :class:`~docplex.cp.solver.solver.CpoSolver` contains the actual implementation of this method,
        but also some others functions allowing to invoke more specialized functions. An advanced programming may
        require to explicitly create a CpoSolver instead of calling function at model level.
        Please refer to this class for more details.

        This function is available on DOcplexcloud and with local CPO solver with release number greater or equal to 12.7.0.

        Args:
            cnstr (Optional):   Optional constraint to be added to the model before invoking propagation.
                                If not given, solving context is the default one that is defined in the module
            context (Optional): Complete solving context.
                                If not given, solving context is the default one that is defined in the module
                                :mod:`~docplex.cp.config`.
            params (Optional):  Solving parameters (object of class :class:`~docplex.cp.parameters.CpoParameters`)
                                that overwrite those in the solving context.
            url (Optional):     URL of the DOcplexcloud service that overwrites the one defined in the solving context.
            key (Optional):     Authentication key of the DOcplexcloud service that overwrites the one defined in
                                the solving context.
            (param) (Optional): Any individual solving parameter as defined in class :class:`~docplex.cp.parameters.CpoParameters`
                               (for example *TimeLimit*, *Workers*, *SearchType*, etc).
            (others) (Optional): Any leaf attribute with the same name in the solving context
                                (for example *agent*, *trace_log*, *trace_cpo*, etc).
        Returns:
            Propagation result (object of class :class:`~docplex.cp.solution.CpoSolveResult`)
        Raises:
            :class:`~docplex.cp.utils.CpoNotSupportedException`: if method not available in the solver agent.
            :class:`~docplex.cp.utils.CpoException`: (or derived) if error.
        """
        # Check if an optional constraint has been given
        if cnstr is None:
            mdl = self
        else:
            # Clone the model and add constraint
            mdl = self.clone()
            mdl.add(cnstr)
        # Call propagation
        solver = mdl._create_solver(**kwargs)
        psol = solver.propagate()
        solver.end()
        return psol


    def run_seeds(self, nbrun, **kwargs):
        """ This method runs *nbrun* times the CP optimizer search with different random seeds
        and computes statistics from the result of these runs.

        Result statistics are displayed on the log output that should be activated.
        If the appropriate configuration variable *context.solver.add_log_to_solution* is set to True (default),
        log is also available in the *CpoRunResult* result object, accessible as a string using the method
        :meth:`~docplex.cp.solution.CpoRunResult.get_solver_log`

        Each run of the solver is stopped according to single solve conditions (TimeLimit for example).
        Total run time is then expected to take *nbruns* times the duration of a single run.

        This function is available only with local CPO solver with release number greater or equal to 12.8.

        Args:
            nbrun:              Number of runs with different seeds.
            context (Optional): Complete solving context.
                                If not given, solving context is the default one that is defined in the module
                                :mod:`~docplex.cp.config`.
            params (Optional):  Solving parameters (object of class :class:`~docplex.cp.parameters.CpoParameters`)
                                that overwrite those in the solving context.
            url (Optional):     URL of the DOcplexcloud service that overwrites the one defined in the solving context.
            key (Optional):     Authentication key of the DOcplexcloud service that overwrites the one defined in
                                the solving context.
            (param) (Optional): Any individual solving parameter as defined in class :class:`~docplex.cp.parameters.CpoParameters`
                               (for example *TimeLimit*, *Workers*, *SearchType*, etc).
            (others) (Optional): Any leaf attribute with the same name in the solving context
                                (for example *agent*, *trace_log*, *trace_cpo*, etc).
        Returns:
            Run result, object of class :class:`~docplex.cp.solution.CpoRunResult`.
        Raises:
            :class:`~docplex.cp.utils.CpoNotSupportedException`: if method not available in the solver agent.
            :class:`~docplex.cp.utils.CpoException`: (or derived) if error.
        """
        solver = self._create_solver(**kwargs)
        rsol = solver.run_seeds(nbrun)
        solver.end()
        return rsol


    def add_solver_listener(self, lstnr):
        """ Add a solver listener.

        A solver listener is an object extending the class :class:`~docplex.cp.solver.solver_listener.CpoSolverListener`
        which provides multiple functions that are called to notify about the different solving steps.

        Args:
            lstnr:  Solver listener
        """
        assert isinstance(lstnr, CpoSolverListener), \
            "Listener should be an object of class docplex.cp.solver.solver_listener.CpoSolverListener"
        self.listeners.append(lstnr)


    def remove_solver_listener(self, lstnr):
        """ Remove a solver listener previously added with :meth:`~docplex.cp.model.CpoModel.add_listener`.

        Args:
            lstnr:  Listener to remove.
        """
        self.listeners.remove(lstnr)


    def export_model(self, out=None, **kwargs):
        """ Exports/prints the model in the standard CPO file format.

        Args:
            out (Optional):     Target output, stream or file name. Default is sys.stdout.
            context (Optional): Complete solving context.
                                If not given, solving context is the default one that is defined in the module
                                :mod:`~docplex.cp.config`.
            params (Optional):  Solving parameters (object of class :class:`~docplex.cp.parameters.CpoParameters`)
                                that overwrite those in the solving context.
            add_source_location (Optional): Add source location into generated text
            length_for_alias (Optional): Minimum name length to use shorter alias instead
            (others) (Optional): Any leaf attribute with the same name in the solving context
        """
        CpoCompiler(self, **kwargs).print_model(out)


    def import_model(self, file):
        """ Import a model from a file containing a model expressed in CPO file format.

        Args:
            file:   Input file
        """
        import docplex.cp.cpo_parser as cpo_parser
        prs = cpo_parser.CpoParser(self)
        prs.parse(file)


    def export_as_cpo(self, out=None, **kwargs):
        """ Deprecated form of method :meth:`export_model`.
        """
        self.export_model(out, **kwargs)


    def get_cpo_string(self, **kwargs):
        """ Compiles the model in CPO file format into a string.

        Args:
            context:             Global solving context. If not given, context is the default context that is set in config.py.
            params:              Solving parameters (CpoParameters) that overwrites those in solving context
            add_source_location: Add source location into generated text
            length_for_alias:    Minimum name length to use shorter alias instead
            (others):            All other context parameters that can be changed
        Returns:
            String containing the model.
        """
        return CpoCompiler(self, **kwargs).get_as_string()


    def check_equivalence(self, other):
        """ Checks that this model is equivalent to another.

        Variables and expressions are compared, but not names that may differ because of automatic naming.

        Args:
            other:  Other model to compare with.
        Raises:
            Exception if models are not equivalent
        """

        # Check object types
        if not isinstance(other, CpoModel):
            raise CpoException("Other model is not an object of class CpoModel")

        # Compare expressions that are not variables
        lx1 = [x for x, l in self.expr_list if not isinstance(x, CpoVariable)]
        #print("Expressions of 1:")
        #for x in lx1: print("   {}".format(x))
        lx2 = [x for x, l in other.expr_list if not isinstance(x, CpoVariable)]
        #print("Expressions of 2:")
        #for x in lx2: print("   {}".format(x))
        if len(lx1) != len(lx2):
            raise CpoException("Different number of expressions, {} vs {}.".format(len(lx1), len(lx2)))
        for i in range(len(lx1)):
            #print("Check expression {}\n   and\n{}".format(lx1[i][0], lx2[i][0]))
            if not lx1[i].equals(lx2[i]):
                raise CpoException("The expression {} differs: {} vs {}".format(i, lx1[i][0], lx2[i][0]))

        # Compare search phases
        lx1 = self.search_phases
        lx2 = other.search_phases
        if len(lx1) != len(lx2):
            raise CpoException("Different number of search phases, {} vs {}.".format(len(lx1), len(lx2)))
        for i in range(len(lx1)):
            if not lx1[i][0].equals(lx2[i][0]):
                raise CpoException("The search phase {} differs: {} vs {}".format(i, lx1[i][0], lx2[i][0]))


    def equals(self, other):
        """ Checks if this model is equal to another.

        Args:
            other:  Other model to compare with.
        Returns:
            True if models are identical, False otherwise.
        """
        # Check object types
        if not isinstance(other, CpoModel):
            return False
        # Do not compare variables as there may me more with Python as all are named (for example SequenceVar)
        # Check list of expressions (will also compare variables)
        if len(self.expr_list) != len(other.expr_list):
            return False
        for x1, x2 in zip(self.expr_list, other.expr_list):
            if not x1[0].equals(x2[0]):
                # print("different expressions: \n1: {}\n2: {}".format(x1[0], x2[0]))
                return False
        return True


    def clone(self):
        """ Create a copy of this model """
        res = copy.copy(self)
        res.expr_list = list(self.expr_list)
        if self.parameters is not None:
            res.parameters = self.parameters.copy()
        res.search_phases = list(self.search_phases)
        res.map_expr = dict(self.map_expr)
        return res


    def __eq__(self, other):
        """ Check if this model is equal to another

        Args:
            other:  Other model to compare with
        Returns:
            True if models are identical, False otherwise
        """
        return self.equals(other)


    def __ne__(self, other):
        """ Check inequality of this object with another """
        return not self.__eq__(other)


    def __str__(self):
        """ Convert the model into string (returns model name) """
        return self.get_name()


    def _create_solver(self, **kwargs):
        """ Create a new solver instance attached to this model

        Args:
            kwargs: Parameters to pass to solver creation
        Returns:
            New solver properly initialized.
        """
        solver = CpoSolver(self, **kwargs)
        for l in self.listeners:
            solver.add_listener(l)
        return solver


    def _add_named_expr(self, expr):
        """ If named, add an expression to the map of named expressions.

        Args:
            expr: Expression
        Raises:
            CpoException if name already used for another expression
        """
        name = expr.name
        if name:
            if (self.map_expr.setdefault(name, expr) is not expr):
                raise CpoException("An expression named '{}' already exists: {}".format(name, self.map_expr[name]))


    def _ensure_all_root_constraints_named(self):
        """ Check if all top-level constraints have a name.

        If not all constraints have a name, this method assign a name to them and returns False.

        If all are already named, this method returns True.

        Return:
            True if all constraints are already named, False otherwise.
        """
        # Check if constraints already named
        if self.name_constraints:
            return True
        # Loop on each top-level constraints
        for (expr, l) in self.expr_list:
            if expr.type in (Type_Constraint, Type_BoolExpr) and (not expr.name):
                name = expression._CONSTRAINT_ID_ALLOCATOR.allocate()
                expr.set_name(name)
                self._add_named_expr(expr)
        self.name_constraints = True
        return False


    def _remove_from_expr_list(self, expr, elist):
        """ Remove an expression from a list of expressions (and map of names)
        Args:
            expr:  Expression to remove.
            elist: List of expressions where search
        Returns:
            True if expression has been removed, False if not found
        """
        for ix, (x, l) in enumerate(elist):
            if x is expr:
                del elist[ix]
                if expr.name:
                    del self.map_expr[expr.name]
                return True
        return False


###############################################################################
##  Private Functions
###############################################################################

