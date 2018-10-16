Changelog
---------

Changed in 2.4.61:
``````````````````
* Both ``docplex.mp`` & ``docplex.cp``:
   * Support for CPLEX engines 12.8. Some features of docplex2.4 are available only with engines >= 12.8.
   * Adding new ports (AIX, plinux).
   * Examples are now available as Zeppelin notebooks.

* In ``docplex.mp``:
   * Express a linear problem as a scikit-learn tranformer by providing a numpy, a pandas or scipy matrix.
   * Logical constraints: constratins equivalence, if-then & rshift operator.
   * Solve hook to add a method to be called at each intermediate solution.
   * KPIS automatically published at each intermediate solution if running on docplexcloud python worker.
   * Support for scipy coo & csr matrixes.

* In ``docplex.cp``:
   * add new method run_seeds() to execute a model multiple times, available with local solver 12.8.
   * add support of new solver infos 'SearchStatus' eand 'SearchStopCause'.
   * In method ``docplex.cp.model.CpoModel.propagate()``, add possibility to add an optional constraint to the model.
   * add domain iterator in integer variables and integer variables solutions, allowing to get domain
     as a list of individual integers.
   * add possibility to identify some model variables as KPIs of the model.
   * add abort_search() method on solver (not supported everywhere)
   * Rework code generation to enhance performances and remove unused variables that was pointed by removed expressions.
   * add possibility to add one or more CpoSolverListener to put some callback functions
     when solve is started, ended, or when a solution is found.
     Implementation is provided in new python module ``docplex.cp.solver.solver_listener`` that also contains sample
     listeners SolverProgressPanelListener and AutoStopListener.
   * Using parameter *context.solver.solve_with_start_next*, enable solve() method to execute a start/next loop instead
     of standard solve. This enables, for optimization problems, usage of SolveListeners with a greater progress accuracy.
   * Completely remove deprecated 'angel' to identify local solver.
   * Deprecate usage of methods ``minimize()`` and ``maximize()`` on ``docplex.cp.CpoModel``. 
   * Add methods ``get_objective_bounds()`` and ``get_objective_gaps()`` in solution objects.

  
  
Changed in 2.3.44 (2017.09):
````````````````````````````

* Module ``docplex.cp.model.solver_angel.py`` has been renamed ``solver_local.py``. 
  A shadow copy with previous name still exist to preserve ascending compatibility.
  Module ``docplex.cp.model.config.py`` is modified to refer this new module.
* Class ``docplex.cp.model.solver_local.SolverAngel`` has been renamed ``SolverLocal``. 
  A shadow copy with previous name still exist to preserve ascending compatibility.
* Class ``docplex.cp.model.solver_local.AngelException`` has been renamed ``LocalSolverException``. 
  A shadow copy with previous name still exist to preserve ascending compatibility.
* Functions logical_and() and logical_or() are able to accept a list of model boolean expressions.
* Fix defect on allowed_assignments() and forbiden_assignments() that was wrongly converting 
  list of tupes into tuple_set.
* Update all examples to add comments and split them in sections data / prepare / model / solve
* Add new sched_RCPSPMM_json.py example that reads data from JSON file instead of raw data file.
* Rename all visu examples with more explicit names.
* Remove the object class CpoTupleSet. Tuple sets can be constructed only by calling tuple_set() method, or more
  simply by passing directly a Python iterable of iterables when a tupleset is required 
  (in expressions allowed_assignments() and forbidden_assignments)
* Allow logical_and() and logical_or() to accept a list of boolean expressions.
* Add overloading of builtin functions all() and any() as other form of logical_and() and logical_or().
* In no_overlap() and state_function(), transition matrix can be passed directly as a Python iterable of iterables of integers, 
* Editable transition matrix, created with a size only, is deprecated. However it is still available for ascending compatibility.
* Add conditional() modeling function
* Parameter 'AutomaticReplay' is deprecated.
* Add get_search_status() and get_stop_cause() on object CpoSolveResult, available for solver COS12.8
* Improved performance of ``Var.reduced_cost()`` in ``docplex.mp``.

Changed in 2.2.34 (2017.07):
````````````````````````````

* Methods ``docplex.cp.model.export_model()`` and ``docplex.cp.model.import_model()``
  have been added to respectively generate or parse a model in CPO format.
* Methods ``docplex.cp.model.minimize()`` and ``docplex.cp.model.maximize()``
  have been added to directly indicate an objective at model level.
* Notebook example ``scheduling_tuto.ipynb`` contains an extensive tutorial
  to solve scheduling problems with CP.
* Modeling methof sum() now supports sum of cumul expressions.
* Methods ``docplex.cp.model.start_search()`` allows to start a new 
  search sequence directly from the model object.
* When setting ``context.solver.auto_publish`` is set, and using the CPLEX
  engine, KPIs and current objective are automatically published when the
  script is run on DOcplexcloud Python worker.
* When setting ``context.solver.auto_publish`` is set, and using the CP
  engine, current objective is automatically published when the
  script is run on DOcplexcloud Python worker.
* ``docplex.util.environment.Environment.set_stop_callback`` and
  ``docplex.util.environment.Environment.get_stop_callback`` are added so that
  you can add a callback when the DOcplexcloud job is aborted.


Changed in 2.1.28:
``````````````````

* New methods ``Model.logical_or()`` and ``Model.logical_and()`` handle
  logical operations on binary variables.
* DOcplex now supports CPLEX 12.7.1 and Benders decomposition. Set annotations
  on constraints and variables using the ``benders_annotation`` property and use
  the proper CPLEX parameters governing Benders decomposition.
* CPLEX tutorials: in the documentation and as notebooks in the examples.
* Fixed a bug in ``docplex.mp.solution.SolveSolution.display()`` and in 
  ``docplex.mp.solution.Model.report_kpi()`` when using unicode variable names.
* There's now a simple command line interface for DOcplexcloud. It can be run
  in a terminal. ``python -m docplex.cli help`` for more info. That command
  line reads your DOcplexcloud credentials in your cplex_config.py file. It
  allows you to submit, list, delete jobs on DOcplexcloud. The cli is available
  in notebooks too, using the ``%docplex_cli`` magics. ``%docplex_cli help`` for
  some help. In a notebook, credentials can be passed using `%docplex_url` and
  `%docplex_key` magics.
* Removing constraints in 1 call
* Bug fixes when editing an existing model.
* Bug fix in the relaxation mechanism when using docplexcloud.


Changed in 2.0.15:
``````````````````

* Piecewise linear (PWL) functions are now supported. An API is now available
  on ``docplex.mp.model`` to create PWL functions and to create constraints using these PWL functions.
  PWL functions may be defined with breakpoints (default API) or by using slopes. Some simple arithmetic is
  also available to build new PWL functions by adding, subtracting, or scaling existing PWL functions.
* DOcplex has undergone a significant overhaul effort that has resulted in an average of 30-50% improvement
  of modeling run-time performance. All parts of the API benefit from the performance improvements: creation of variables and constraints, removal of constraints, computation of sums of variables, and so on.
* Constraints are now fully editable: 
  the expressions of a constraint can be modified.
  Similarly, the objective expression can also be modified. This allows for complex workflows in which the model is modified after a solve and then solved again. 
* docplex is now available on Anaconda cloud and can be installed via the conda installation packager.
  See the `IBM Anaconda home <https://anaconda.org/IBMDecisionOptimization>`_
  CPLEX Community Edition for Python is also provided on Anaconda Cloud to get free local solving capabilities with limitations.
* Support of ``~/.docplexrc`` configuration files for ``docplex.mp.context.Context`` is now dropped.
  This feature has been deprecated since 1.0.0.
* Known incompatibility: class ``docplex.mp.model.AbstractModel`` moved to ``docplex.mp.absmodel.AbstractModel``. 
  Samples using this class have been updated.


Changed in 1.0.630:
```````````````````

* Added support for CPLEX 12.7 and Python 3.5.
* Upgraded the DOcplexcloud client to version 1.0.202.
* Module ``docplex.mp.advmodel`` is now officially supported. This module
  provides support for efficient, specialized aggregator methods for large
  models.
* When solving on DOcplexcloud, proxies can now be specified with the
  ``context.solver.docloud.proxies`` property.
* When two constraints are defined with the same name, issue a warning instead of
  a fatal exception. The last constraint defined will take over the first one in the name directory.
* Fix ValueError when passing a pandas DataFrame as variable keys (using
  DataFrame indexes).
* Solution.get_values() returns a collection of variable values in one call.
* ``docplex.mp.model`` no longer imports ``docloud.status``. Any status
  previously initialized as ``JobSolveStatus.UNKNOWN`` is now initialized as
  ``None``.
* Minor improvements to notebooks and examples.
