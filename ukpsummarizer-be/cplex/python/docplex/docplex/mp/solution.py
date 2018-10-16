# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

from __future__ import print_function

import json
import math
import sys

import six

try:  # pragma: no cover
    from itertools import zip_longest as izip_longest
except ImportError:  # pragma: no cover
    from itertools import izip_longest

from six import iteritems, iterkeys

from docplex.mp.compat23 import StringIO
from docplex.mp.constants import SolveAttribute
from docplex.mp.utils import is_iterable, is_number, is_string, str_holo, OutputStreamAdapter
from docplex.mp.utils import make_output_path2, DOcplexException
from docplex.mp.linear import Var
from docplex.mp.error_handler import docplex_fatal

from collections import defaultdict


# noinspection PyAttributeOutsideInit
class SolveSolution(object):
    """
    The :class:`SolveSolution` class holds the result of a solve.
    """

    # a symbolic value for no objective ?
    NO_OBJECTIVE_VALUE = -1e+75

    INFEAS_KEY = 'infeasibities'

    @staticmethod
    def _is_discrete_value(v):
        return v == int(v)

    def __init__(self, model, var_value_map=None, obj=None, name=None, solved_by=None, keep_zeros=True,
                 rounding=False):
        """ SolveSolution(model, var_value_map, obj, name)

        Creates a new solution object, associated to a a model.

        Args:
            model: The model to which the solution is associated. This model cannot be changed.

            obj: The value of the objective in the solution. A value of None means the objective is not defined at the
                time the solution is created, and will be set later.

            var_value_map: a Python dictionary containing associations of variables to values.

            name: a name for the solution. The default is None, in which case the solution is named after the
                model name.

        :return: A solution object.
        """
        assert model is not None
        assert solved_by is None or is_string(solved_by)
        assert obj is None or is_number(obj)

        self.__model = model
        self._checker = model._checker
        self._name = name
        self._problem_name = model.name
        self._problem_objective_expr = model.objective_expr if model.has_objective() else None
        self._objective = self.NO_OBJECTIVE_VALUE if obj is None else obj
        self._solved_by = solved_by
        self.__var_value_map = {}
        self._attribute_map = defaultdict(dict)
        self.__round_discrete = rounding
        self._solve_status = None
        self._keep_zeros = keep_zeros

        if var_value_map is not None:
            self._store_var_value_map(var_value_map, keep_zeros=keep_zeros, rounding=rounding)

    @staticmethod
    def make_engine_solution(model, var_value_map=None, obj=None, location=None, solve_status=None):
        # INTERNAL
        sol = SolveSolution(model,
                            var_value_map=None,
                            obj=obj,
                            solved_by=location,
                            rounding=True,
                            keep_zeros=False)
        # trust engines
        for var, value in iteritems(var_value_map):
            if 0 != value:
                sol._set_var_value_internal(var=var, value=value, rounding=True, do_warn_on_non_discrete=False, )
        if solve_status is not None:
            sol._set_solve_status(solve_status)
        return sol

    def _get_var_by_name(self, varname):
        return self.__model.get_var_by_name(varname)

    def clear(self):
        """ Clears all solve result data.

        All data related to the model are left unchanged.
        """
        self.__var_value_map = {}
        self._objective = self.NO_OBJECTIVE_VALUE
        self._attribute_map = {}
        self._solve_status = None

    def is_empty(self):
        """
        Checks whether the solution is empty.

        Returns:
            Boolean: True if the solution is empty; in other words, the solution has no defined objective and no variable value.
        """
        return not self.has_objective() and not self.__var_value_map

    @property
    def problem_name(self):
        return self._problem_name

    @property
    def solved_by(self):
        '''
        Returns a string indicating how the solution was produced.

        - If the solution was created by a program, this field returns None.
        - If the solution originated from a local CPLEX solve, this method returns the string 'cplex_local'.
        - If the solution originated from a DOcplexcloud solve, this method returns 'cplex_cloud'.

        Returns:
            A string, or None.

        '''
        return self._solved_by

    def get_name(self):
        """ This property allows to get/set a name on the solution.

        In some cases , it might be interesting to build different solutions for the same model,
        in this case, use the name property to disinguish them.

        """
        return self._name

    def set_name(self, solution_name):
        self._checker.typecheck_string(solution_name, accept_empty=False, accept_none=True,
                                       header='SolveSolution.set_name(): ')
        self._name = solution_name

    name = property(get_name, set_name)

    def _resolve_var(self, var_key, do_raise):
        # INTERNAL: accepts either strings or variable objects
        # returns a variable or None
        if isinstance(var_key, Var):
            var = var_key
        elif is_string(var_key):
            var = self._get_var_by_name(var_key)
            # var might be None here if the name is unknown
        else:
            var = None
        # --
        if var is None:
            if do_raise:
                self.model.fatal("Expecting variable or name, got: {0!r}", var_key)
            else:
                self.model.warning("Expecting variable or name, got: {0!r} - ignored", var_key)
        return var

    def _typecheck_var_key_value(self, var_key, value, caller):
        # INTERNAL
        self._checker.typecheck_num(value, caller=caller)
        if not is_string(var_key) and not isinstance(var_key, Var):
            self.model.fatal("{0} expects either Var or string, got: {1!r}", caller, var_key)

    def add_var_value(self, var_key, value):
        """ Adds a new (variable, value) pair to this solution.

        Args:
            var_key: A decision variable (:class:`docplex.mp.linear.Var`) or a variable name (string).
            value (number): The value of the variable in the solution.
        """
        self._typecheck_var_key_value(var_key, value, caller="Solution.add_var_value")
        self._set_var_key_value(var_key, value, keep_zero=True, rounding=False, do_warn_on_non_discrete=True)

    def __setitem__(self, var_key, value):
        # aleays keep zero, no warnings, no checks
        self._set_var_key_value(var_key, value, keep_zero=self._keep_zeros, rounding=False,
                                do_warn_on_non_discrete=False)

    def set_var_key_value(self, var_key, value, keep_zero, rounding, do_warn_on_rounding):
        # INTERNAL
        self._typecheck_var_key_value(var_key, value, caller="Solution.add_var_value")
        self._set_var_key_value(var_key, value, keep_zero, rounding, do_warn_on_rounding)

    def _set_var_key_value(self, var_key, value, keep_zero, rounding, do_warn_on_non_discrete):
        # INTERNAL: no checks done.
        if value or keep_zero:
            var = self._resolve_var(var_key, do_raise=False)
            if var is not None:
                self._set_var_value_internal(var, value, rounding, do_warn_on_non_discrete=do_warn_on_non_discrete)

    def _set_var_value_internal(self, var, value, rounding, do_warn_on_non_discrete):
        # INTERNAL, no check
        stored_value = value
        if var.is_discrete():
            if not self._is_discrete_value(value):
                if rounding:
                    stored_value = self.model.round_nearest(value)
                if do_warn_on_non_discrete:
                    if rounding:
                        self.error_handler.warning(
                            "Trying to assign non-discrete value: {1} to discrete variable {0} - rounded to {2}",
                            (var, value, stored_value))
                    else:
                        self.error_handler.warning(
                            "Discrete variable {0!r} has been assigned non-discrete value: {1}",
                            (var, value))
        # ---
        self.__var_value_map[var] = stored_value

    def is_attributes_fetched(self, attr_name):
        return attr_name and attr_name in self._attribute_map

    @property
    def model(self):
        """
        This property returns the model associated with the solution.
        """
        return self.__model

    @property
    def solve_details(self):
        """
        This property returns the solve_details associated with the solution.
        """
        return self.__model.solve_details

    @property
    def error_handler(self):
        return self.__model.error_handler

    def get_objective_value(self):
        """
        Gets the objective value as defined in the solution.
        When the objective value has not been defined, a special value `NO_SOLUTION` is returned.
        To check whether the objective has been set, use :func:`has_objective`.

        Returns:
            float: The value of the objective as defined by the solution.
        """
        return self._objective

    def set_objective_value(self, obj):
        """
        Sets the objective value of the solution.
        
        Args:
            obj (float): The value of the objective in the solution.
        """
        self._objective = obj

    def has_objective(self):
        """
        Checks whether or not the objective has been set.

        Returns:
            Boolean: True if the solution defines an objective value.
        """
        return self._objective != self.NO_OBJECTIVE_VALUE

    def _has_problem_objective(self):
        return self.model.has_objective()

    @property
    def objective_value(self):
        """ This property is used to get or set the objective valueof the solution.

        When the objective value has not been defined, a special value `NO_SOLUTION` is returned.
        To check whether the objective has been set, use :func:`has_objective`.

        """
        return self._objective

    @objective_value.setter
    def objective_value(self, new_objvalue):
        self.set_objective_value(new_objvalue)

    @property
    def solve_status(self):
        return self._solve_status

    def _set_solve_status(self, new_status):
        # INTERNAL
        self._solve_status = new_status

    def _store_var_value_map(self, key_value_map, keep_zeros=False, rounding=False):
        # INTERNAL
        for e, val in iteritems(key_value_map):
            # need to check var_keys and values
            self.set_var_key_value(var_key=e, value=val, keep_zero=keep_zeros, rounding=rounding,
                                   do_warn_on_rounding=False)

    def store_infeasibilities(self, infeasibilities, infeas_key=INFEAS_KEY):
        assert isinstance(infeasibilities, dict)
        self._attribute_map[infeas_key] = infeasibilities

    def _store_attribute_result(self, attr_name, attr_idx_map, obj_mapper):
        attr_obj_map = {obj_mapper(idx): attr_val
                        for idx, attr_val in iteritems(attr_idx_map)
                        if attr_val and obj_mapper(idx) is not None}
        self._attribute_map[attr_name] = attr_obj_map

    def store_reduced_costs(self, rcs, mapper):
        self._store_attribute_result(SolveAttribute.reduced_costs.name, rcs, obj_mapper=mapper)

    def store_dual_values(self, duals, mapper):
        self._store_attribute_result(SolveAttribute.duals.name, duals, obj_mapper=mapper)

    def store_slack_values(self, slacks, mapper):
        self._store_attribute_result(SolveAttribute.slacks.name, slacks, obj_mapper=mapper)

    def iter_var_values(self):
        """Iterates over the (variable, value) pairs in the solution.

        Returns:
            iterator: A dict-style iterator which returns a two-component tuple (variable, value)
            for all variables mentioned in the solution.
        """
        return iteritems(self.__var_value_map)

    def iter_variables(self):
        """Iterates over all variables mentioned in the solution.

        Returns:
           iterator: An iterator object over all variables mentioned in the solution.
        """
        return iterkeys(self.__var_value_map)

    def contains(self, dvar):
        """
        Checks whether or not a decision variable is mentioned in the solution.

        This predicate can also be used in the form `var in solution`, because the
        :func:`__contains_` method has been redefined for this purpose.

        Args:
            dvar (:class:`docplex.mp.linear.Var`): The variable to check.

        Returns:
            Boolean: True if the variable is mentioned in the solution.
        """
        return dvar in self.__var_value_map

    def __contains__(self, dvar):
        return self.contains(dvar)

    def get_value(self, dvar_arg):
        """
        Gets the value of a solution variable in a solution.
        If the variable is not mentioned in the solution,
        the method returns 0 and does not raise an exception.
        Note that this method can also be used as :func:`solution[dvar]`
        because the :func:`__getitem__` method has been overloaded.

        Args:
            dvar_arg: A decision variable (:class:`docplex.mp.linear.Var`) or a variable name (string).

        Returns:
            float: The value of the variable in the solution.
        """
        dvar = self._resolve_var(dvar_arg, do_raise=True)
        return self.__var_value_map.get(dvar, 0)

    def get_var_value(self, dvar):
        self._checker.typecheck_var(dvar)
        return self._get_var_value(dvar)

    def _get_var_value(self, dvar):
        # INTERNAL
        return self.__var_value_map.get(dvar, 0)

    def get_values(self, dvars):
        """
        Gets the value of a sequence of variables in a solution.
        If a variable is not mentioned in the solution,
        the method assumes 0 and does not raise an exception.

        Args:
            dvars: an ordered sequence of decision variables.

        Returns:
            list: A list of float values, in the same order as the variable sequence.

        """
        checker = self._checker
        checker.check_ordered_sequence(arg=dvars,
                                       header='SolveSolution.get_values() expects ordered sequence of variables')
        dvar_seq = checker.typecheck_var_seq(dvars)
        return self._get_values(dvar_seq)

    def _get_values(self, dvars):
        # internal: no checks are done.
        self_value_map = self.__var_value_map
        return [self_value_map.get(dv, 0) for dv in dvars]

    def get_value_dict(self, var_dict, keep_zeros=True, precision=1e-6):
        # assume var_dict is a key-> variable dictionary
        if keep_zeros:
            return {k: self._get_var_value(v) for k, v in iteritems(var_dict)}
        else:
            value_dict = {}
            for key, dvar in iteritems(var_dict):
                dvar_value = self._get_var_value(dvar)
                if abs(dvar_value) >= precision:
                    value_dict[key] = dvar_value
            return value_dict

    @property
    def number_of_var_values(self):
        """ This property returns the number of variable values stored in this solution.

        """
        return len(self.__var_value_map)

    def __getitem__(self, dvar):
        return self.get_value(dvar)

    def get_status(self, ct):
        """ Returns the status of a linear constraint in the solution.

        Returns 1 if the constraint is satisfied, else returns 0. This is particularly useful when using
        the status variable of constraints.

        :param ct: A linear constraint
        :return: a number (1 or 0)
        """
        self._checker.typecheck_linear_constraint(ct)
        return self._get_status(ct)

    def _get_status(self, ct):
        # INTERNAL
        ct_status_var = ct._get_status_var()
        if ct_status_var:
            return self.__var_value_map.get(ct_status_var, 0)
        elif ct.has_valid_index():
            # a posted constraint is true if there is a solution
            return 1
        else:
            return 1 if ct.is_satisfied(self) else 0

    def find_unsatisfied_constraints(self, tolerance=1e-6):
        unsats = []
        for ct in self.model.iter_constraints():
            if not ct.is_satisfied(self, tolerance):
                unsats.append(ct)
        return unsats

    def check(self, tolerance=1e-6):
        return not (self.find_unsatisfied_constraints(tolerance))

    def equals_solution(self, other, check_models=False, check_explicit=False, obj_precision=1e-3, var_precision=1e-6):
        if check_models and (self.model != other.model):
            return False

        if math.fabs(self.objective_value - other.objective_value) >= obj_precision:
            return False

        for dvar, val in self.iter_var_values():
            if check_explicit and not other.contains(dvar):
                return False
            this_val = self.get_value(dvar)
            other_val = other.get_value(dvar)
            if math.fabs(this_val - other_val) >= var_precision:
                return False

        for other_dvar, other_val in other.iter_var_values():
            if check_explicit and not self.contains(other_dvar):
                return False
            this_val = self.get_value(other_dvar)
            other_val = other.get_value(other_dvar)
            if math.fabs(this_val - other_val) >= var_precision:
                return False

        return True

    def get_attribute(self, mobjs, attr, default_attr_value=0):
        assert is_iterable(mobjs)

        if attr not in self._attribute_map:
            # warn
            return [0] * len(mobjs)
        else:
            attr_map = self._attribute_map[attr]
            return [attr_map.get(mobj, default_attr_value) for mobj in mobjs]

    def get_slack(self, ct):
        return self._attribute_map[SolveAttribute.slacks.name].get(ct, 0)

    def get_infeasibility(self, ct, infeas_key=INFEAS_KEY):
        return self._attribute_map[infeas_key].get(ct, 0)

    def display_attributes(self):
        for attr_key in self._attribute_map:
            attr_value_map = self._attribute_map[attr_key]

            print("#{0}={1:d}".format(attr_key, len(attr_value_map)))
            for obj, attr_val in iteritems(attr_value_map):
                obj_qualifier = obj.name if obj.has_username() else str(obj)
                print(" {0}.{1} = {2}".format(obj_qualifier, attr_key, attr_val))

    def display(self,
                print_zeros=True,
                header_fmt="solution for: {0:s}",
                objective_fmt="{0}: {1:.{prec}f}",
                value_fmt="{varname:s} = {value:.{prec}f}",
                iter_vars=None,
                **kwargs):
        print_generated = kwargs.get("print_generated", False)
        problem_name = self._problem_name
        if header_fmt and problem_name:
            print(header_fmt.format(problem_name))
        if self._problem_objective_expr is not None and objective_fmt and self.has_objective():
            obj_prec = self.model.objective_expr.float_precision
            obj_name = self._problem_objective_name()
            print(objective_fmt.format(obj_name, self._objective, prec=obj_prec))
        if iter_vars is None:
            iter_vars = self.iter_variables()
        print_counter = 0
        for dvar in iter_vars:
            if print_generated or not dvar.is_generated():
                var_value = self.get_value(dvar)
                if print_zeros or var_value != 0:
                    print_counter += 1
                    varname = dvar.to_string()
                    if type(value_fmt) != type(varname):
                        # infamous mix of str and unicode. Should happen only
                        # in py2. Let's convert things
                        if isinstance(value_fmt, str):
                            value_fmt = value_fmt.decode('utf-8')
                        else:
                            value_fmt = value_fmt.encode('utf-8')
                    output = value_fmt.format(varname=varname,
                                              value=var_value,
                                              prec=dvar.float_precision,
                                              counter=print_counter)
                    try:
                        print(output)
                    except UnicodeEncodeError:
                        encoding = sys.stdout.encoding if sys.stdout.encoding else 'ascii'
                        print(output.encode(encoding,
                                            errors='backslashreplace'))

    def to_string(self, print_zeros=True):
        oss = StringIO()
        self.to_stringio(oss, print_zeros=print_zeros)
        return oss.getvalue()

    def _problem_objective_name(self, default_obj_name="objective"):
        # INTERNAL
        # returns the string used for displaying the objective
        # if the problem has an objective with a name, use it
        # else return the default (typically "objective"
        self_objective_expr = self._problem_objective_expr
        if self_objective_expr is not None and self_objective_expr.has_name():
            return self_objective_expr.name
        else:
            return default_obj_name

    def to_stringio(self, oss, print_zeros=True):
        problem_name = self._problem_name
        if problem_name:
            oss.write("solution for: %s\n" % problem_name)
        if self._problem_objective_expr is not None and self.has_objective():
            obj_name = self._problem_objective_name()
            oss.write("%s: %g\n" % (obj_name, self._objective))

        value_fmt = "{var:s}={value:.{prec}f}"
        for dvar, val in self.iter_var_values():
            if not dvar.is_generated():
                var_value = self.get_value(dvar)
                if print_zeros or var_value != 0:
                    oss.write(value_fmt.format(var=str(dvar), value=var_value, prec=dvar.float_precision))
                    oss.write("\n")

    def __str__(self):
        return self.to_string()

    def __repr__(self):
        if self.has_objective():
            s_obj = "obj={0:g}".format(self.objective_value)
        else:
            s_obj = "obj=N/A"
        s_values = ",".join(["{0!s}:{1:g}".format(var, val) for var, val in iteritems(self.__var_value_map)])
        r = "docplex.mp.solution.SolveSolution({0},values={{{1}}})".format(s_obj, s_values)
        return str_holo(r, maxlen=72)

    def __iter__(self):
        # INTERNAL: this is necessary to prevent solution from being an iterable.
        # as it follows getitem protocol, it can mistakenly be interpreted as an iterable
        raise TypeError

    def print_mst(self):
        """
        Writes the solution in an output stream "out" (assumed to satisfy the file interface)
        in CPLEX MST format.
        """
        SolutionMSTPrinter.print_one_solution(sol=self, out=sys.stdout)

    def print_mst_to_stream(self, out):
        SolutionMSTPrinter.print_to_stream(self, out)

    def export_as_mst_string(self):
        return SolutionMSTPrinter.print_to_string(self)

    def export_as_mst(self, path=None, basename=None):
        """ Exports a solution to a file in CPLEX mst format.

        Args:
            basename: Controls the basename with which the solution is printed.
                Accepts None, a plain string, or a string format.
                If None, the model's name is used.
                If passed a plain string, the string is used in place of the model's name.
                If passed a string format (either with %s or {0}), this format is used to format the
                model name to produce the basename of the written file.

            path: A path to write the file, expects a string path or None.
                Can be a directory, in which case the basename
                that was computed with the basename argument is appended to the directory to produce
                the file.
                If given a full path, the path is directly used to write the file, and
                the basename argument is not used.
                If passed None, the output directory will be ``tempfile.gettempdir()``.

        Example:
            Assuming the solution has the name "prob":

            ``sol.export_as_mst()`` will write file prob.mst in a temporary directory.

            ``sol.export_as_mst(path="c:/temp/myprob1.mst")`` will write file "c:/temp/myprob1.mst".

            ``sol.export_as_mst(basename="my_%s_mipstart", path ="z:/home/")`` will write "z:/home/my_prob_mipstart.mst".

        """
        mst_path = make_output_path2(actual_name=self._problem_name,
                                     extension=SolutionMSTPrinter.mst_extension,
                                     path=path,
                                     basename_arg=basename)
        if mst_path:
            self.print_mst_to_stream(mst_path)

    def get_printer(self, key):
        # INTERNAL
        printers = {'json': SolutionJSONPrinter,
                    'xml': SolutionMSTPrinter
                    }

        printer = printers.get(key.lower())
        if not printer:
            raise ValueError("format key must be one of {}".format(printers.keys()))
        return printer

    def export(self, file_or_filename, format="json", **kwargs):
        """ Export this solution.
        
        Args:
            file_or_filename: If ``file_or_filename`` is a string, this argument contains the filename to
                write to. If this is a file object, this argument contains the file object to write to.
            format: The format of the solution. The format can be:
                - json
                - xml
            kwargs: The kwargs passed to the actual exporter
        """

        printer = self.get_printer(format)

        if isinstance(file_or_filename, six.string_types):
            fp = open(file_or_filename, "w")
            close_fp = True
        else:
            fp = file_or_filename
            close_fp = False
        try:
            printer.print_to_stream(self, fp, **kwargs)
        finally:
            if close_fp:
                fp.close()

    def export_as_string(self, format="json", **kwargs):
        oss = StringIO()
        self.export(oss, format=format, **kwargs)
        return oss.getvalue()

    def check_as_mip_start(self):
        """Checks that this solution is a valid MIP start.

        To be valid, it must have:

            * at least one discrete variable (integer or binary), and
            * the values for decision variables should be consistent with the type.

        Returns:
            Boolean: True if this solution is a valid MIP start.
        """
        is_explicit = self._keep_zeros
        if is_explicit and not self.__var_value_map:
            docplex_fatal("MIP start solution is empty, provide at least one discrete variable value")

        discrete_vars = (dv for dv in self.iter_variables() if dv.is_discrete())
        count_values = 0
        count_errors = 0
        for dv in discrete_vars:
            sol_value = self.get_value(dv)
            if not dv.accept_initial_value(sol_value):
                count_errors += 1
                docplex_fatal("Wrong initial value for variable {0!r}: {1}, type: {2!s}",  # pragma: no cover
                              dv.name, sol_value, dv.vartype.short_name)  # pragma: no cover
            else:
                count_values += 1
        if is_explicit and count_values == 0:
            docplex_fatal("MIP start contains no discrete variable")  # pragma: no cover
        return True

    def _to_tuple_list(self, model):
        if self._keep_zeros:
            l = [(dv.get_index(), val) for dv, val in self.iter_var_values()]
        else:
            l = [(dv.get_index(), self[dv]) for dv in model.iter_variables()]
        return l

    def as_dict(self, keep_zeros=False):
        var_value_dict = {}
        # INTERNAL: return a dictionary of variable_name: variable_value
        for dvar, dval in self.iter_var_values():
            dvar_name = dvar.get_name()
            if dvar_name and (keep_zeros or dval):
                var_value_dict[dvar_name] = dval
        return var_value_dict

    def kpi_value_by_name(self, name, match_case=False):
        ''' Returns the solution value of a KPI from its name.

        Args:
            name (string): The string to be matched.

            match_case (boolean): If True, looks for a case-exact match, else
               ignores case. Default is False.

        Returns:
            The value of the KPI, evaluated in the solution.

        Note:
            This method raises an error when the string does not match any KPI in the model.

        See:
            :func: `docplex.mp.model.kpi_by_name`
        '''
        kpi = self.model.kpi_by_name(name, try_match=True, match_case=match_case)
        return kpi._get_solution_value(self)


class SolutionMSTPrinter(object):
    # header contains the final newline
    mst_header = """<?xml version = "1.0" standalone="yes"?>
<?xml-stylesheet href="https://www.ilog.com/products/cplex/xmlv1.0/solution.xsl" type="text/xsl"?>

"""
    mst_extension = ".mst"

    one_solution_start_tag = "<CPLEXSolution version=\"1.0\">"
    one_solution_end_tag = "</CPLEXSolution>"

    # used when several solutions are present
    many_solution_start_tag = "<CPLEXSolutions version=\"1.0\">"
    many_solution_end_tag = "</CPLEXSolutions>"

    @staticmethod
    def print_signature(out):
        from docplex.version import docplex_version_string
        osa = OutputStreamAdapter(out)
        osa.write("<!-- This file has been generated by DOcplex version {}  -->\n".format(docplex_version_string))

    @classmethod
    def print(cls, out, solutions):
        # solutions can be either a plain solution or a sequence or an iterator
        if not is_iterable(solutions):
            cls.print_one_solution(solutions, out)
        else:
            sol_seq = list(solutions)
            nb_solutions = len(sol_seq)
            assert nb_solutions > 0
            if 1 == nb_solutions:
                cls.print_one_solution(sol_seq[0], out)
            else:
                cls.print_many_solutions(sol_seq, out)

    @classmethod
    def print_one_solution(cls, sol, out, print_header=True):
        osa = OutputStreamAdapter(out)
        if print_header:
            osa.write(cls.mst_header)
            cls.print_signature(out)
        # <CPLEXSolution version="1.0">
        osa.write(cls.one_solution_start_tag)
        osa.write("\n")

        # <header
        # problemName="foo"
        # objectiveValue="42"
        # />
        osa.write(" <header\n   problemName=\"{0}\"\n".format(sol.problem_name))
        if sol.has_objective():
            osa.write("   objectiveValue=\"{0:g}\"\n".format(sol.objective_value))
        osa.write("  />\n")

        # prepare reduced costs 
        """ For mst, we don't want this !
        model = sol.model
        if not model._solves_as_mip():
            reduced_costs = model.reduced_costs(model.iter_variables())
        else:
            reduced_costs = []
        """
        #  <variables>
        #    <variable name="x1" index ="1" value="3.14"/>
        #  </variables>
        osa.write(" <variables>\n")
        """ For mst, we don't want this !
        for (dvar, rc) in zip_longest(model.iter_variables(), reduced_costs,
                                      fillvalue=None):
            var_name = dvar.name
            var_value = sol[dvar]
            var_index = dvar.index
            rc_string = ""
            if rc is not None:
                rc_string = "reducedCost=\"{}\"".format(rc)
            osa.write("  <variable name=\"{0}\" index=\"{1}\" value=\"{2:g}\" {3}/>\n"
              .format(var_name, var_index, var_value, rc_string))
        """
        for dvar, val in sol.iter_var_values():
            var_name = dvar.print_name()
            var_value = sol[dvar]
            var_index = dvar.index
            osa.write("  <variable name=\"{0}\" index=\"{1}\" value=\"{2:g}\"/>\n"
                      .format(var_name, var_index, var_value))
        osa.write(" </variables>\n")

        #  </CPLEXSolution version="1.0">
        osa.write(cls.one_solution_end_tag)
        osa.write("\n")

    @classmethod
    def print_many_solutions(cls, sol_seq, out):
        osa = OutputStreamAdapter(out)
        osa.write(cls.mst_header)
        cls.print_signature(out)
        # <CPLEXSolutions version="1.0">
        osa.write(cls.many_solution_start_tag)
        osa.write("\n")

        for sol in sol_seq:
            cls.print_one_solution(sol, out, print_header=False)

        # <CPLEXSolutions version="1.0">
        osa.write(cls.many_solution_end_tag)
        osa.write("\n")

    @classmethod
    def print_to_stream(cls, solutions, out, extension=mst_extension, **kwargs):
        if out is None:
            # prints on standard output
            cls.print(sys.stdout, solutions)
        elif isinstance(out, str):
            # a string is interpreted as a path name
            path = out if out.endswith(extension) else out + extension
            with open(path, "w") as of:
                cls.print_to_stream(solutions, of)
                # print("* file: %s overwritten" % path)
        else:
            try:
                cls.print(out, solutions)

            except AttributeError:  # pragma: no cover
                pass  # pragma: no cover
                # stringio will raise an attribute error here, due to with
                # print("Cannot use this an output: %s" % str(out))

    @classmethod
    def print_to_string(cls, solutions):
        oss = StringIO()
        cls.print_to_stream(solutions, out=oss)
        return oss.getvalue()


from json import JSONEncoder


class SolutionJSONEncoder(JSONEncoder):
    def __init__(self, **kwargs):
        # extract kwargs I know
        self.keep_zeros = None
        if "keep_zeros" in kwargs:
            self.keep_zeros = kwargs["keep_zeros"]
            del kwargs["keep_zeros"]
        super(SolutionJSONEncoder, self).__init__(**kwargs)

    def default(self, solution):
        n = {'CPLEXSolution': self.encode_solution(solution)}
        return n

    def encode_solution(self, solution):
        n = {}
        n["version"] = "1.0"
        n["header"] = self.encode_header(solution)
        n["variables"] = self.encode_variables(solution)
        lc = self.encode_linear_constraints(solution)
        if len(lc) > 0:
            n["linearConstraints"] = lc
        qc = self.encode_quadratic_constraints(solution)
        if len(qc) > 0:
            n["quadraticConstraints"] = qc
        return n

    def encode_header(self, solution):
        n = {}
        n["problemName"] = solution.problem_name
        if solution.has_objective():
            n["objectiveValue"] = "{}".format(solution.objective_value)
        n["solved_by"] = solution.solved_by
        return n

    def encode_linear_constraints(self, solution):
        n = []
        model = solution.model
        was_solved = True
        try:
            model.check_has_solution()
        except DOcplexException:
            was_solved = False
        duals = []
        if not model._solves_as_mip():
            duals = model.dual_values(model.iter_linear_constraints())
        slacks = []
        if was_solved:
            slacks = model.slack_values(model.iter_linear_constraints())
        for (ct, d, s) in izip_longest(model.iter_linear_constraints(),
                                       duals, slacks,
                                       fillvalue=None):
            # basis status is not yet supported
            c = {"name": ct.name,
                 "index": ct.index}
            if s:
                c["slack"] = s
            if d:
                c["dual"] = d
            n.append(c)
        return n

    def encode_quadratic_constraints(self, solution):
        n = []
        model = solution.model
        duals = []
        if not model._solves_as_mip():
            duals = model.dual_values(model.iter_quadratic_constraints())
        slacks = []
        was_solved = True
        try:
            model.check_has_solution()
        except DOcplexException:
            was_solved = False
        if was_solved:
            slacks = model.slack_values(model.iter_quadratic_constraints())
        for (ct, d, s) in izip_longest(model.iter_quadratic_constraints(),
                                       duals, slacks,
                                       fillvalue=None):
            # basis status is not yet supported
            c = {"name": ct.name,
                 "index": ct.index}
            if s:
                c["slack"] = s
            if d:
                c["dual"] = d
            n.append(c)
        return n

    def encode_variables(self, sol):
        model = sol.model
        n = []
        if not model._solves_as_mip():
            reduced_costs = model.reduced_costs(model.iter_variables())
        else:
            reduced_costs = []

        keep_zeros = sol._keep_zeros
        if self.keep_zeros is not None:
            keep_zeros = keep_zeros or self.keep_zeros

        for (dvar, rc) in izip_longest(model.iter_variables(), reduced_costs,
                                       fillvalue=None):
            value = sol[dvar]
            if not keep_zeros and value == 0:
                continue
            v = {"index": "{}".format(dvar.index),
                 "name": dvar.print_name(),
                 "value": "{}".format(value)}
            if rc is not None:
                v["reducedCost"] = rc
            n.append(v)
        return n


class SolutionJSONPrinter(object):
    json_extension = ".json"

    @classmethod
    def print(cls, out, solutions, indent=None, **kwargs):
        # solutions can be either a plain solution or a sequence or an iterator
        sol_to_print = list(solutions) if is_iterable(solutions) else [solutions]
        # encode all solutions in dict ready for json output
        encoder = SolutionJSONEncoder(**kwargs)
        solutions_as_dict = [encoder.default(sol) for sol in sol_to_print]
        # use an output stream adapter for py2/py3 and str/unicode compatibility
        osa = OutputStreamAdapter(out)
        if len(sol_to_print) == 1:  # if only one solution, use at root node
            osa.write(json.dumps(solutions_as_dict[0], indent=indent))
        else:  # for multiple solutions, we want a "CPLEXSolutions" root
            osa.write(json.dumps({"CPLEXSolutions": solutions_as_dict}, indent=indent))

    @classmethod
    def print_to_stream(cls, solutions, out, extension=json_extension, indent=None, **kwargs):
        if out is None:
            # prints on standard output
            cls.print(sys.stdout, solutions, indent=indent, **kwargs)
        elif isinstance(out, str):
            # a string is interpreted as a path name
            path = out if out.endswith(extension) else out + extension
            with open(path, "w") as of:
                cls.print_to_stream(solutions, of, indent=indent, **kwargs)
                # print("* file: %s overwritten" % path)
        else:
            cls.print(out, solutions, indent=indent, **kwargs)

    @classmethod
    def print_to_string(cls, solutions, indent=None):
        oss = StringIO()
        cls.print_to_stream(solutions, out=oss, indent=indent)
        return oss.getvalue()
