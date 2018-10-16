# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

# gendoc: ignore


import math

from docplex.mp.compat23 import izip
from docplex.mp.constr import AbstractConstraint, LinearConstraint
from docplex.mp.error_handler import docplex_fatal
from docplex.mp.linear import Var, Expr
from docplex.mp.progress import ProgressListener
from docplex.mp.utils import is_int, is_number, is_iterable, is_string, generate_constant, \
    is_ordered_sequence, is_iterator
from docplex.mp.vartype import VarType


class IDocplexTypeChecker(object):
    def typecheck_iterable(self, arg):
        raise NotImplementedError  # pragma: no cover

    def typecheck_valid_index(self, arg):
        raise NotImplementedError  # pragma: no cover

    def typecheck_vartype(self, arg):
        raise NotImplementedError  # pragma: no cover

    def typecheck_var(self, obj):
        raise NotImplementedError  # pragma: no cover

    def typecheck_binary_var(self, obj):
        raise NotImplementedError  # pragma: no cover

    def typecheck_var_seq(self, seq, vtype=None):
        return seq  # pragma: no cover

    def typecheck_var_seq_all_different(self, seq):
        raise NotImplementedError  # pragma: no cover

    def typecheck_num_seq(self, seq):
        raise NotImplementedError  # pragma: no cover

    def typecheck_operand(self, obj, accept_numbers=True, caller=None):
        raise NotImplementedError  # pragma: no cover

    def typecheck_constraint(self, obj):
        raise NotImplementedError  # pragma: no cover

    def typecheck_ct_to_add(self, ct, mdl, header):
        raise NotImplementedError  # pragma: no cover

    def typecheck_linear_constraint(self, obj, accept_ranges=True):
        raise NotImplementedError  # pragma: no cover

    def typecheck_constraint_seq(self, cts, ctype=None):
        # must return sequence unchanged
        return cts  # pragma: no cover

    def typecheck_zero_or_one(self, arg):
        raise NotImplementedError  # pragma: no cover

    def typecheck_num(self, arg, caller=None):
        raise NotImplementedError  # pragma: no cover

    def typecheck_int(self, arg, accept_negative=False, caller=None):
        raise NotImplementedError  # pragma: no cover

    def check_vars_domain(self, lbs, ubs, names):
        raise NotImplementedError  # pragma: no cover

    def check_var_domain(self, lbs, ubs, names):
        raise NotImplementedError  # pragma: no cover

    def typecheck_string(self, arg, accept_empty=False, accept_none=False):
        raise NotImplementedError  # pragma: no cover

    def typecheck_in_model(self, model, mobj, header=''):
        raise NotImplementedError  # pragma: no cover

    def typecheck_key_seq(self, keys, accept_empty_seq=False):
        raise NotImplementedError  # pragma: no cover

    def get_number_validation_fn(self):
        raise NotImplementedError  # pragma: no cover

    def typecheck_progress_listener(self, arg):
        raise NotImplementedError  # pragma: no cover

    def typecheck_two_in_model(self, model, obj1, obj2, ctx_msg):
        raise NotImplementedError  # pragma: no cover

    def check_ordered_sequence(self, arg, header):
        raise NotImplementedError  # pragma: no cover

    def check_trivial_constraints(self):
        raise NotImplementedError  # pragma: no cover

    def check_solution_hook(self, mdl, sol_hook_fn):
        raise NotImplementedError


# noinspection PyAbstractClass
class DOcplexLoggerTypeChecker(IDocplexTypeChecker):
    def __init__(self, logger):
        self._logger = logger

    def fatal(self, msg, *args):
        self._logger.fatal(msg, args)

    def error(self, msg, *args):  # pragma: no cover
        self._logger.error(msg, args)

    def warning(self, msg, *args):  # pragma: no cover
        self._logger.warning(msg, args)


class StandardTypeChecker(DOcplexLoggerTypeChecker):
    def __init__(self, logger):
        DOcplexLoggerTypeChecker.__init__(self, logger)

    def typecheck_iterable(self, arg):
        # INTERNAL: checks for an iterable
        if not is_iterable(arg):
            self.fatal("Expecting iterable, got: {0!s}", arg)

    # safe checks.
    def typecheck_valid_index(self, arg):
        if arg < 0:
            self.fatal("Invalid index: {0!s}", arg)

    def typecheck_vartype(self, arg):
        # INTERNAL: check for a valid vartype
        if not isinstance(arg, VarType):
            self.fatal("Not a variable type: {0!s}, type: {1!s}", arg, type(arg))
        return True

    def typecheck_var(self, obj):
        # INTERNAL: check for Var instance
        if not isinstance(obj, Var):
            self.fatal("Expecting decision variable, got: {0!s} type: {1!s}", obj, type(obj))

    def typecheck_binary_var(self, obj):
        self.typecheck_var(obj)
        if obj.vartype.get_cplex_typecode() != 'B':
            self.fatal('Expecting binary variable, but variable {0!s} has type: {1}'.format(obj,
                                                                                            obj.vartype.short_name))

    def typecheck_var_seq(self, seq, vtype=None):
        # build a list to avoid consuming an iterator
        checked_var_list = list(seq)
        for i, x in enumerate(checked_var_list):
            if not isinstance(x, Var):
                self.fatal("Expecting sequence of variables, got: {0!r} at position {1}", x, i)
            if vtype and x.vartype != vtype:
                self.fatal("Expecting sequence of type {0} variables, got: {1!r} at position {2}",
                           vtype.short_name, x, i)

        return checked_var_list

    def typecheck_num_seq(self, seq):
        # build a list to avoid consuming an iterator
        checked_num_list = list(seq)
        for x in checked_num_list:
            self.typecheck_num(x)
        return checked_num_list

    def typecheck_var_seq_all_different(self, seq):
        # return the checked sequence, so take the list
        seq_as_list = list(seq)
        for v in seq_as_list:
            self.typecheck_var(v)
        # check for all differemt and output a justifier variable apperaing twice.
        inc_set = set([])
        for v in seq_as_list:
            if v in inc_set:
                self.fatal('Variable: {0} appears twice in sequence', v)

            else:
                inc_set.add(v)
        return seq_as_list

    def typecheck_constraint(self, obj):
        if not isinstance(obj, AbstractConstraint):
            self.fatal("Expecting constraint, got: {0!s} with type: {1!s}", obj, type(obj))

    def typecheck_ct_to_add(self, ct, mdl, header):
        if not isinstance(ct, AbstractConstraint):
            self.fatal("Expecting constraint, got: {0!r} with type: {1!s}", ct, type(ct))
        self.typecheck_in_model(mdl, ct, header)

    def typecheck_linear_constraint(self, obj, accept_ranges=True):
        if accept_ranges:
            if not isinstance(obj, AbstractConstraint):
                self.fatal("Expecting linear constraint, got: {0!r}", obj)
            if not obj.is_linear():
                self.fatal("Expecting linear constraint, got: {0!s} with type: {1!s}", obj, type(obj))
        else:
            if not isinstance(obj, LinearConstraint):
                self.fatal("Expecting linear constraint, got: {0!s} with type: {1!s}", obj, type(obj))

    def typecheck_constraint_seq(self, cts, ctype=None):
        checked_cts_list = list(cts)
        expected_type = ctype or AbstractConstraint
        for i, ct in enumerate(checked_cts_list):
            if not isinstance(ct, expected_type):
                self.fatal("Expecting sequence of constraints, got: {0!r} at position {1}", ct, i)
        return checked_cts_list

    def typecheck_zero_or_one(self, arg):
        if arg != 0 and arg != 1:
            self.fatal("expecting 0 or 1, got: {0!s}", arg)

    def typecheck_num(self, arg, caller=None):
        caller_string = "{0}: ".format(caller) if caller is not None else ""
        if not is_number(arg):
            self.fatal("{0}Expecting number, got: {1!r}", caller_string, arg)
        elif math.isnan(arg):
            self.fatal("{0}NaN value detected", caller_string)

    def typecheck_int(self, arg, accept_negative=True, caller=None):
        caller_string = "{0}: ".format(caller) if caller is not None else ""
        if not is_number(arg):
            self.fatal("{0}Expecting number, got: {1!r}", caller_string, arg)
        elif math.isnan(arg):
            self.fatal("{0}NaN value detected", caller_string)
        elif not is_int(arg):
            self.fatal("{0}Expecting integer, got: {1!r}", caller_string, arg)
        elif not accept_negative and arg < 0:
            self.fatal("{0}Expecting positive integer, got: {1!r}", caller_string, arg)

    def check_vars_domain(self, lbs, ubs, names):
        l_ubs = len(ubs)
        l_lbs = len(lbs)
        if l_lbs and l_ubs:
            names = names or generate_constant(None, max(l_lbs, l_ubs))
            for lb, ub, varname in izip(lbs, ubs, names):
                self.check_var_domain(lb, ub, varname)

    def check_var_domain(self, lb, ub, varname):
        if lb is not None and ub is not None and lb > ub:
            self.fatal('Empty variable domain, name={0}, lb={1}, ub={2}'.format(varname, lb, ub))

    def typecheck_string(self, arg, accept_empty=False, accept_none=False, header=''):
        if is_string(arg):
            if not accept_empty and 0 == len(arg):
                self.fatal("A nonempty string is not allowed here")
        elif not (arg is None and accept_none):
            self.fatal("{0}Expecting string, got: {1!r}", header, arg)

    def typecheck_in_model(self, model, mobj, header=''):
        # produces message of the type: "constraint ... does not belong to model
        if mobj.model != model:
            self.fatal("{0} ({2!s}) is not in model '{1:s}'".format(header, model.name, mobj))

    def typecheck_key_seq(self, keys, accept_empty_seq=False):
        if any(k is None for k in keys):
            self.fatal("Variable keys cannot be None, got: {0!r}", keys)

    @staticmethod
    def static_validate_num(e, checked_num=False, infinity=1e+20):
        if not checked_num and not is_number(e):
            docplex_fatal("Expecting number, got: {0!r}".format(e))
        elif -infinity <= e <= infinity:
            return e
        elif e >= infinity:
            return infinity
        else:
            return -infinity

    def get_number_validation_fn(self):
        return self.static_validate_num

    @staticmethod
    def _is_operand(arg, accept_numbers=True):
        return isinstance(arg, (Expr, Var)) or (accept_numbers and is_number(arg))

    def typecheck_operand(self, arg, accept_numbers=True, caller=None):
        if not self._is_operand(arg, accept_numbers=accept_numbers):
            caller_str = "{0}: ".format(caller) if caller else ""
            accept_str = "Expecting expr/var"
            if accept_numbers:
                accept_str += "/number"
            self.fatal("{0}{1}, got: {2!r}", caller_str, accept_str, arg)

    def typecheck_progress_listener(self, arg):
        if not isinstance(arg, ProgressListener):
            self.fatal('Expecting ProgressListener instance, got: {0!r}', arg)

    def typecheck_two_in_model(self, model, mobj1, mobj2, ctx_msg):
        mobj1_model = mobj1._get_model()
        mobj2_model = mobj2._get_model()
        if mobj1_model != mobj2_model:
            self.fatal("Cannot mix objects from different models in {0}. obj1={1!s}, obj2={2!s}"
                       .format(ctx_msg, mobj1, mobj2))
        elif mobj1_model != model:
            self.fatal("Objects do not belong to model {0}. obj1={1!s}, obj2={2!s}"
                       .format(self, mobj1, mobj2))

    def check_trivial_constraints(self):
        return True

    def check_ordered_sequence(self, arg, header):
        # in some cases, we need an ordered sequence, if not the code won't crash
        # but may do unexpected things
        if not is_ordered_sequence(arg) and not is_iterator(arg):
            self.fatal("{0}, got: {1!s}", header, type(arg).__name__)

    def check_solution_hook(self, mdl, sol_hook_fn):
        try:
            dummy_s = mdl.new_solution()
            sol_hook_fn(dummy_s)
        except TypeError:
            self.fatal('Solution hook requires a function taking a solution as argument')


class NumericTypeChecker(StandardTypeChecker):
    def __init__(self, logger):
        StandardTypeChecker.__init__(self, logger)

    @staticmethod
    def static_validate_num(e, infinity=1e+20, context_msg=None):
        if not is_number(e):
            docplex_fatal("Not a number: {}".format(e))
        elif math.isnan(e):
            msg = "NaN value found in expression"
            if context_msg is not None:
                try:
                    msg = "{0}: {1}".format(context_msg(), msg)
                except TypeError:
                    msg = "{0}: {1}".format(context_msg, msg)
            docplex_fatal(msg)
        elif math.isinf(e):
            msg = "Infinite value forbidden in expression"
            if context_msg is not None:
                try:
                    msg = "{0}: {1}".format(context_msg(), msg)
                except TypeError:
                    msg = "{0}: {1}".format(context_msg, msg)
            docplex_fatal(msg)
        elif -infinity <= e <= infinity:
            return e
        elif e >= infinity:
            return infinity
        else:
            return -infinity

    def get_number_validation_fn(self):
        return self.static_validate_num


class DummyTypeChecker(IDocplexTypeChecker):
    # noinspection PyUnusedLocal
    def __init__(self, logger):
        pass

    def typecheck_iterable(self, arg):
        pass  # pragma: no cover

    def typecheck_valid_index(self, arg):
        pass  # pragma: no cover

    def typecheck_vartype(self, arg):
        pass  # pragma: no cover

    def typecheck_var(self, obj):
        pass  # pragma: no cover

    def typecheck_binary_var(self, obj):
        pass  # pragma: no cover

    def typecheck_var_seq(self, seq, vtype=None):
        return seq  # pragma: no cover

    def typecheck_num_seq(self, seq):
        return seq  # pragma: no cover

    def typecheck_var_seq_all_different(self, seq):
        return seq

    def typecheck_operand(self, obj, accept_numbers=True, caller=None):
        pass  # pragma: no cover

    def typecheck_constraint(self, obj):
        pass  # pragma: no cover

    def typecheck_ct_to_add(self, ct, mdl, header):
        pass  # pragma: no cover

    def typecheck_linear_constraint(self, obj, accept_ranges=True):
        pass  # pragma: no cover

    def typecheck_constraint_seq(self, cts, ctype=None):
        # must return sequence unchanged
        return cts  # pragma: no cover

    def typecheck_zero_or_one(self, arg):
        pass  # pragma: no cover

    def typecheck_num(self, arg, caller=None):
        pass  # pragma: no cover

    def typecheck_int(self, arg, accept_negative=True, caller=None):
        pass  # pragma: no cover

    def check_vars_domain(self, lbs, ubs, names):
        # do nothing on variable bounds
        pass

    def check_var_domain(self, lb, ub, varname):
        pass

    def typecheck_string(self, arg, accept_empty=False, accept_none=False):
        pass  # pragma: no cover

    def typecheck_in_model(self, model, mobj, header=''):
        pass  # pragma: no cover

    def typecheck_key_seq(self, keys, accept_empty_seq=False):
        pass  # pragma: no cover

    def typecheck_progress_listener(self, arg):
        pass  # pragma: no cover

    def typecheck_two_in_model(self, model, obj1, obj2, ctx_msg):
        pass  # pragma: no cover

    def check_ordered_sequence(self, arg, header):
        pass  # pragma: no cover

    def check_trivial_constraints(self):
        return False

    def get_number_validation_fn(self):
        return None

    def check_solution_hook(self, mdl, sol_hook_fn):
        pass


#  ------------------------------
# noinspection PyPep8
_tck_map = {'default' : StandardTypeChecker,
            'standard': StandardTypeChecker,
            'std'     : StandardTypeChecker,
            'on'      : StandardTypeChecker,
            # --
            'numeric' : NumericTypeChecker,
            # --
            'off'      : DummyTypeChecker,
            'deploy'   : DummyTypeChecker,
            'no_checks': DummyTypeChecker}


def get_typechecker(arg, logger):
    if arg:
        key = arg.lower()
        if key in _tck_map:
            checker_type = _tck_map[key]
        else:
            msg = 'Unexpected typechecker key: {0} - expecting on|off|std|default|numeric. Using default'.format(key)
            if logger:
                logger.warning(msg)
            else:
                print('*Warning: {0}'.format(msg))
            checker_type = StandardTypeChecker

    else:
        checker_type = StandardTypeChecker
    return checker_type(logger)
