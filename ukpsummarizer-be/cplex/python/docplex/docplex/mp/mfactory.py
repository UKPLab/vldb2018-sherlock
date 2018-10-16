# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

# gendoc: ignore

from docplex.mp.sosvarset import SOSVariableSet
from docplex.mp.operand import LinearOperand
from docplex.mp.linear import Var, LinearExpr, AbstractLinearExpr, ZeroExpr
from docplex.mp.operand import Operand
from docplex.mp.constants import ComparisonType, UpdateEvent, ObjectiveSense
from docplex.mp.constr import LinearConstraint, RangeConstraint, \
    IndicatorConstraint, PwlConstraint, EquivalenceConstraint, IfThenConstraint
from docplex.mp.functional import MaximumExpr, MinimumExpr, AbsExpr, PwlExpr, LogicalAndExpr, LogicalOrExpr
from docplex.mp.pwl import PwlFunction
from docplex.mp.compat23 import fast_range
from docplex.mp.utils import *
from docplex.mp.kpi import KPI
from docplex.mp.solution import SolveSolution


def fix_format_string(fmt, dimen=1, key_format='_%s'):
    ''' Fixes a format string so that it contains dimen slots with %s inside
        arguments are:
         --- dimen is the number of slots we need
         --- key_format is the format in which the %s is embedded. By default '_%s'
             for example if each item has to be surrounded by {} set key_format to _{%s}
    '''
    assert (dimen >= 1)
    actual_nb_slots = 0
    curpos = 0
    str_size = len(fmt)
    while curpos < str_size and actual_nb_slots < dimen:
        new_pos = fmt.find('%', curpos)
        if new_pos < 0:
            break
        actual_nb_slots += 1
        if actual_nb_slots >= dimen:
            break
        curpos = new_pos + 2
    # how much slots do we need to add to the end of the string??
    nb_missing = max(0, dimen - actual_nb_slots)
    return fmt + nb_missing * (key_format % '%s')


def compile_naming_function(keys, user_name, arity=1, key_format=None, _default_key_format='_%s'):
    # INTERNAL
    # builds a naming rule from an input , a dimension, and an optional meta-format
    # Makes sure the format string does contain the right number of format slots
    assert user_name is not None

    if is_string(user_name):
        if key_format is None:
            used_key_format = _default_key_format
        elif is_string(key_format):
            # -- make sure some %s is inside, otherwise add it
            if '%s' in key_format:
                used_key_format = key_format
            else:
                used_key_format = key_format + '%s'
        else:  # pragma: no cover
            raise DOcplexException("key format expects string format or None, got: {0!r}".format(key_format))

        fixed_format_string = fix_format_string(user_name, arity, used_key_format)
        if 1 == arity:
            return lambda k: fixed_format_string % str(k)
        else:
            # here keys are tuples of size >= 2
            return lambda key_tuple: fixed_format_string % key_tuple

    elif is_function(user_name):
        return user_name

    elif is_iterable(user_name):
        # check that the iterable has same len as keys,
        # otherwise thereis no more default naming and None cannot appear in CPLEX name arrays
        list_names = list(user_name)
        if len(list_names) < len(keys):
            raise DOcplexException("An aray of names should have same len as keys, expecting: {0}, go: {1}"
                                   .format(len(keys), len(list_names)))
        key_to_names_dict = {k: nm for k, nm in izip(keys, list_names)}
        # use a closure
        return lambda k: key_to_names_dict[k]  # if k in key_to_names_dict else default_fn()

    else:
        raise DOcplexException('Cannot use this for naming variables: {0!r} - expecting string, function or iterable'
                               .format(user_name))


class _AbstractModelFactory(object):
    def __init__(self, model):
        self._model = model
        self._checker = model._checker


class ModelFactory(_AbstractModelFactory):
    @staticmethod
    def float_or_default(bound, default_bound):
        return default_bound if bound is None else float(bound)

    def is_free_lb(self, var_lb):
        return var_lb <= - self.infinity

    def is_free_ub(self, var_ub):
        return var_ub >= self.infinity

    def __init__(self, model, engine, ordered, term_dict_type):
        _AbstractModelFactory.__init__(self, model)

        self._var_container_counter = 0
        self.number_validation_fn = model._checker.get_number_validation_fn()
        self._engine = engine
        self.infinity = engine.get_infinity()
        self.one_expr = None
        self.ordered = ordered
        self.term_dict_type = term_dict_type

    def set_ordering(self, ordered, dict_type):
        self.ordered = ordered
        self.term_dict_type = dict_type

    def new_zero_expr(self):
        return ZeroExpr(self._model)

    def new_one_expr(self):
        return LinearExpr(self._model, e=None, constant=1, safe=True)

    def _get_cached_one_expr(self):
        if self.one_expr is None:
            self.one_expr = LinearExpr(self._model, e=None, constant=1, safe=True)
        return self.one_expr

    def fatal(self, msg, *args):
        self._model.fatal(msg, *args)

    def warning(self, msg, *args):
        self._model.warning(msg, args)

    def update_engine(self, engine):
        # the model has already disposed the old engine, if any
        self._engine = engine
        self.infinity = engine.get_infinity()

    def new_var(self, vartype, lb=None, ub=None, varname=None):
        self_model = self._model
        self._checker.check_var_domain(lb, ub, varname)
        logger = self_model.logger
        rlb = vartype.resolve_lb(lb, logger)
        rub = vartype.resolve_ub(ub, logger)
        used_varname = None if self_model.ignore_names else varname
        var = Var(self_model, vartype, used_varname, rlb, rub, _safe_lb=True, _safe_ub=True)

        idx = self._engine.create_one_variable(vartype, rlb, rub, varname)
        self_model._register_one_var(var, idx, varname)
        return var

    def new_constraint_status_var(self, ct):
        # INTERNAL
        model = self._model
        binary_vartype = model.binary_vartype
        if model.ignore_names:
            varname = None
        else:
            # use name if any else use truncated ct string representation
            base_varname = '[{0:s}]'.format(ct.name or str_holo(ct, maxlen=20))
            # if name is already taken, use unique index at end to disambiguate
            varname = model._get_non_ambiguous_varname(base_varname)

        svar = Var(model, binary_vartype, lb=0, ub=1, _safe_lb=True, _safe_ub=1, name=varname)
        svar.notify_origin(ct)  # generated

        idx = self._engine.create_one_variable(binary_vartype, 0, 1, varname)
        model._register_one_var(svar, idx, varname)
        return svar

    # --- sequences
    def make_key_seq(self, keys, name):
        # INTERNAL Takes as input a candidate keys input and returns a valid key sequence
        used_name = name
        check_keys = True
        if is_iterable(keys):
            if is_pandas_dataframe(keys):
                used_keys = keys.index.values
            elif has_len(keys):
                used_keys = keys
            elif is_iterator(keys):
                used_keys = list(keys)
            else:
                # TODO: make a test for this case.
                self.fatal("Cannot handle iterable var keys: {0!s} : no len() and not an iterator",
                           keys)  # pragma: no cover

        elif is_int(keys) and keys >= 0:
            # if name is str and we have a size, trigger automatic names
            used_name = None if name is str else name
            used_keys = range(keys)
            check_keys = False
        else:
            self.fatal("Unexpected var keys: {0!s}, expecting iterable or integer", keys)  # pragma: no cover

        if check_keys and len(used_keys) > 0:
            self._checker.typecheck_key_seq(used_keys)
        return used_name, used_keys

    def _expand_names(self, keys, user_name, arity, key_format):
        if user_name is None or self._model.ignore_names:
            # no automatic names, ever
            return []
        else:
            # default_naming_fn = self._model._create_automatic_varname
            actual_naming_fn = compile_naming_function(keys, user_name, arity, key_format)
            computed_names = [actual_naming_fn(key) for key in keys]
            return computed_names

    def _expand_bounds(self, keys, var_bound, default_bound, size, true_if_lb):
        ''' Converts raw bounds data (either LB or UB) to CPLEX-compatible bounds list.
            If lbs is None, this is the default, return [].
            If lbs is [] take the default again.
            If it is a number, build a list of size <size> with this number.
            If it is a list, use it if size ok (check numbers??),
            else try it as a function over keys.
        '''
        if var_bound is None:
            # default lb is zero, default ub is infinity
            return []

        elif is_number(var_bound):
            self._checker.typecheck_num(var_bound, caller='in variable bound')
            if true_if_lb:
                if var_bound == default_bound:
                    return []
                else:
                    return [float(var_bound)] * size
            else:
                # ub
                if var_bound >= default_bound:
                    return []
                else:
                    return [float(var_bound)] * size

        elif is_ordered_sequence(var_bound):
            nb_bounds = len(var_bound)
            if nb_bounds < size:
                # see how we can use defaults for those missing bounds
                self.fatal("Variable bounds list is too small, expecting: %d, got: %d" % (size, nb_bounds))
            else:
                for b, b_value in enumerate(var_bound):
                    if b_value is not None and not is_number(b_value):
                        self.fatal("Variable bounds list expects numbers, got: {0!r} (pos: #{1})",
                                   b_value, b)
                float_bounds = [self.float_or_default(bv, default_bound) for bv in var_bound]

                if nb_bounds > size:
                    self.warning(
                        "Variable bounds list is too large, required: %d, got: %d." % (size, nb_bounds))
                    return float_bounds[:size]
                else:
                    return float_bounds

        elif is_iterator(var_bound):
            # unfold the iterator, as CPLEX needs a list
            return list(var_bound)

        elif isinstance(var_bound, dict):
            return [var_bound.get(k, default_bound) for k in keys]
        else:
            # try a function?
            try:
                _computed_bounds = [var_bound(k) for k in keys]
                for b, bnd in enumerate(_computed_bounds):
                    if bnd is None:
                        _computed_bounds[b] = default_bound
                    elif not is_number(bnd):
                        self.fatal("computed bound expects a number, got: {0!s}", bnd)
                    else:
                        # conversion to float()
                        _computed_bounds[b] = float(bnd)
                return _computed_bounds

            except TypeError:
                self._bad_bounds_fatal(var_bound)

            except Exception as e:  # pragma: no cover
                self.fatal("error calling function model bounds: {0!s}, error: {1!s}", var_bound, e)

    def _bad_bounds_fatal(self, bad_bound):
        self.fatal("unexpected variable bound: {0!s}, expecting: None|number|function|iterable", bad_bound)

    def new_multitype_var_list(self, size, vartypes, lbs=None, ubs=None, names=None, key_format=None):
        if not size:
            return []
        mdl = self._model
        assert size == len(vartypes)
        assert size == len(lbs)
        assert size == len(ubs)
        assert size == len(names)

        allvars = [Var(mdl, vartypes[k],
                       names[k] if names[k] else None,
                       lbs[k],
                       ubs[k],
                       _safe_lb=True,
                       _safe_ub=True) for k in fast_range(size)]

        indices = self._engine.create_multitype_variables(size, vartypes, lbs, ubs, names)
        mdl._register_block_vars(allvars, indices, names)
        return allvars

    def var_list(self, keys, vartype, lb, ub, name=None, key_format=None):
        actual_name, fixed_keys = self.make_key_seq(keys, name)
        ctn = self._new_var_container(vartype, key_list=[fixed_keys], lb=lb, ub=ub, name=name)
        return self.new_var_list(ctn, fixed_keys, vartype, lb, ub, actual_name, 1, key_format)

    def new_var_list(self, var_container,
                     key_seq, vartype,
                     lb=None, ub=None,
                     name=str,
                     arity=1, key_format=None):
        number_of_vars = len(key_seq)
        if 0 == number_of_vars:
            return []

        # compute defaults once
        default_lb = vartype.default_lb
        default_ub = vartype.default_ub

        xlbs = self._expand_bounds(key_seq, lb, default_lb, number_of_vars, true_if_lb=True)
        xubs = self._expand_bounds(key_seq, ub, default_ub, number_of_vars, true_if_lb=False)
        # at this point both list are either [] or have size numberOfVars

        all_names = self._expand_names(key_seq, name, arity, key_format)

        self._checker.check_vars_domain(xlbs, xubs, all_names)
        safe_lbs = not xlbs
        safe_ubs = not xubs

        mdl = self._model
        allvars = [Var(mdl, vartype,
                       all_names[k] if all_names else None,
                       xlbs[k] if xlbs else default_lb,
                       xubs[k] if xubs else default_ub,
                       container=var_container,
                       _safe_lb=safe_lbs,
                       _safe_ub=safe_ubs) for k in fast_range(number_of_vars)]

        # query the engine for a list of indices.
        indices = self._engine.create_variables(key_seq, vartype, xlbs, xubs, all_names)
        mdl._register_block_vars(allvars, indices, all_names)
        return allvars

    def constant_expr(self, cst, safe_number=False, context=None, force_clone=False):
        if 0 == cst:
            return self.new_zero_expr()
        elif 1 == cst:
            if force_clone:
                return LinearExpr(self._model, e=None, constant=1, safe=True)
            else:
                return self._get_cached_one_expr()
        else:
            if safe_number:
                k = cst
            else:
                self_number_validation_fn = self.number_validation_fn
                k = self_number_validation_fn(cst) if self_number_validation_fn else cst
            return LinearExpr(self._model, e=None, constant=k, safe=True)

    def linear_expr(self, arg=None, constant=0, name=None, safe=False):
        return LinearExpr(self._model, arg, constant, name, safe=safe)

    # def to_valid_number(self, e, checked_num=False, context_msg=None, infinity=1e+20):
    #     return self._checker.to_valid_number(e, checked_num, context_msg, infinity)

    _operand_types = (AbstractLinearExpr, Var, ZeroExpr)

    @staticmethod
    def _is_operand(arg, accept_numbers=False):
        return isinstance(arg, Operand) or (accept_numbers and is_number(arg))

    def _to_linear_operand(self, e, force_clone=False, context=None):
        if isinstance(e, LinearOperand):
            if force_clone:
                return e.clone()
            else:
                return e
        elif is_number(e):
            return self.constant_expr(cst=e, context=context, force_clone=force_clone, safe_number=False)
        else:
            try:
                return e.to_linear_expr()
            except AttributeError:
                # delegate to the factory
                return self.linear_expr(e)

    def _to_linear_expr(self, e, linexpr_class=LinearExpr, force_clone=False, context=None):
        # TODO: replace by to_linear_operand
        if isinstance(e, linexpr_class):
            if force_clone:
                return e.clone()
            else:
                return e
        elif isinstance(e, self._operand_types):
            return e.to_linear_expr()
        elif is_number(e):
            return self.constant_expr(cst=e, context=context, force_clone=force_clone, safe_number=False)
        else:
            try:
                return e.to_linear_expr()
            except AttributeError:
                # delegate to the factory
                return self.linear_expr(e)

    def _to_expr(self, e):
        # INTERNAL
        if hasattr(e, "iter_terms"):
            return e
        elif is_number(e):
            return self.constant_expr(cst=e, safe_number=True)
        else:
            try:
                return e.to_expr()
            except AttributeError:
                self.fatal("cannot convert to expression: {0!r}", e)

    def new_binary_constraint(self, lhs, sense, rhs, name=None):
        ctsense = ComparisonType.parse(sense)
        return self._new_binary_constraint(lhs, ctsense, rhs, name)

    def _new_binary_constraint(self, lhs, sense, rhs, name=None):
        # noinspection PyPep8
        left_expr = self._to_linear_operand(lhs, context="LinearConstraint.left_expr")
        right_expr = self._to_linear_operand(rhs, context="LinearConstraint.right_expr")
        self._checker.typecheck_two_in_model(self._model, left_expr, right_expr, "new_binary_constraint")
        ct = LinearConstraint(self._model, left_expr, sense, right_expr, name)
        return ct

    def new_le_constraint(self, e, rhs, ctname=None):
        return self._new_binary_constraint(e, ComparisonType.LE, rhs, name=ctname)

    def new_eq_constraint(self, e, rhs, ctname=None):
        return self._new_binary_constraint(e, ComparisonType.EQ, rhs, name=ctname)

    def new_ge_constraint(self, e, rhs, ctname=None):
        return self._new_binary_constraint(e, ComparisonType.GE, rhs, name=ctname)

    def _check_range_feasibility(self, lb, ub, expr):
        # INTERNAL
        if not lb <= ub:
            self._model.warning("infeasible range constraint, lb={0}, ub={1}, expr={2}", lb, ub, expr)

    def new_range_constraint(self, lb, expr, ub, name=None, check_feasible=True):
        self._check_range_feasibility(lb, ub, expr)
        linexpr = self._to_linear_operand(expr)
        rng = RangeConstraint(self._model, linexpr, lb, ub, name)
        linexpr.notify_used(rng)
        return rng

    def new_indicator_constraint(self, binary_var, linear_ct, active_value=1, name=None):
        # INTERNAL
        indicator_ct = IndicatorConstraint(self._model, binary_var, linear_ct, active_value, name)
        return indicator_ct

    def new_equivalence_constraint(self, binary_var, linear_ct, active_value=1, name=None):
        # INTERNAL
        equiv_ct = EquivalenceConstraint(self._model, binary_var, linear_ct, active_value, name)
        return equiv_ct

    def new_if_then_constraint(self, if_ct, then_ct, negate=False):
        # INTERNAL
        indicator_ct = IfThenConstraint(self._model, if_ct, then_ct, negate=negate)
        return indicator_ct

    def new_batch_equivalence_constraints(self, bvars, linear_cts, active_values, names):
        return [self.new_equivalence_constraint(bv, lct, active, name)
                for bv, lct, active, name in izip(bvars, linear_cts, active_values, names)]

    def new_batch_indicator_constraints(self, bvars, linear_cts, active_values, names):
        return [self.new_indicator_constraint(bv, lct, active, name)
                for bv, lct, active, name in izip(bvars, linear_cts, active_values, names)]

    def new_constraint_or(self, ct1, ct2):
        status1 = ct1.get_resolved_status_var()
        status2 = ct2.get_resolved_status_var()
        orexpr = self.new_logical_or_expr([status1, status2])
        orct = self._new_binary_constraint(lhs=orexpr, sense=ComparisonType.EQ, rhs=1)
        return orct

    def new_constraint_and(self, ct1, ct2):
        status1 = ct1.get_resolved_status_var()
        status2 = ct2.get_resolved_status_var()
        and_expr = self.new_logical_and_expr([status1, status2])
        and_ct = self._new_binary_constraint(lhs=and_expr, rhs=1, sense=ComparisonType.EQ)
        return and_ct

    # updates

    def update_linear_constraint_exprs(self, ct):
        self._engine.update_linear_constraint(ct, UpdateEvent.LinearConstraintGlobal)

    def update_indicator_constraint_expr(self, ind, expr, event):
        self._engine.update_logical_constraint(ind, UpdateEvent.IndicatorLinearConstraint, expr)

    def _check_logical_ct_edited(self, linct, new_expr):
        log_ct = linct.get_super_logical_ct()
        if log_ct is not None:
            # check that expression is discrete
            if log_ct.is_equivalence() and not new_expr.is_discrete():
                self.fatal(
                    'Linear constraint: {0} is used in equivalence, cannot be modified with non-integer expr: {1}',
                    linct, new_expr)
            self._engine.update_logical_constraint(log_ct, event=UpdateEvent.IndicatorLinearConstraint)

    def set_linear_constraint_expr_from_pos(self, lct, pos, new_expr, update_subscribers=True):
        # INTERNAL
        # pos is 0 for left, 1 for right
        new_operand = self._to_linear_operand(e=new_expr, force_clone=False)
        self._check_logical_ct_edited(lct, new_operand)

        old_expr = lct.get_expr_from_pos(pos)

        exprs = [lct._left_expr, lct._right_expr]
        exprs[pos] = new_operand
        # -- event
        if old_expr.is_constant() and new_operand.is_constant():
            event = UpdateEvent.LinearConstraintRhs
        else:
            event = UpdateEvent.LinearConstraintGlobal
        # ---
        self._engine.update_linear_constraint(lct, event, *exprs)
        lct.set_expr_from_pos(pos, new_operand)
        if update_subscribers:
            # -- update  subscribers
            old_expr.notify_unsubscribed(lct)
            new_operand.notify_used(lct)

    def set_linear_constraint_right_expr(self, ct, new_rexpr):
        self.set_linear_constraint_expr_from_pos(ct, pos=1, new_expr=new_rexpr)

    def set_linear_constraint_left_expr(self, ct, new_lexpr):
        self.set_linear_constraint_expr_from_pos(ct, pos=0, new_expr=new_lexpr)

    def set_linear_constraint_sense(self, ct, arg_newsense):
        new_sense = ComparisonType.parse(arg_newsense)
        if new_sense != ct.sense:
            self._engine.update_linear_constraint(ct, UpdateEvent.LinearConstraintType, new_sense)
            ct._internal_set_sense(new_sense)

    def set_range_constraint_lb(self, rngct, new_lb):
        self.set_range_constraint_bounds(rngct, new_lb, None)

    def set_range_constraint_ub(self, rngct, new_ub):
        self.set_range_constraint_bounds(rngct, None, new_ub)

    def set_range_constraint_bounds(self, rngct, new_lb, new_ub):
        lb_to_use = rngct.lb if new_lb is None else new_lb
        ub_to_use = rngct.ub if new_ub is None else new_ub
        # assuming the new bound has been typechecked already..
        self._engine.update_range_constraint(rngct, UpdateEvent.RangeConstraintBounds, lb_to_use, ub_to_use)
        if new_lb is not None:
            rngct._internal_set_lb(new_lb)
        if new_ub is not None:
            rngct._internal_set_ub(new_ub)
        self._check_range_feasibility(lb_to_use, ub_to_use, rngct.expr)

    def set_range_constraint_expr(self, rngct, new_expr):
        new_op = self._to_linear_operand(new_expr)
        self._engine.update_range_constraint(rngct, UpdateEvent.RangeConstraintExpr, new_op)
        old_expr = rngct.expr
        rngct._expr = new_expr
        old_expr.notify_unsubscribed(rngct)

    # ---------------------
    def new_logical_and_expr(self, bvars):
        # assume bvars is a sequence of binary vars
        nb_args = len(bvars)
        if not nb_args:
            return self.new_one_expr()
        elif 1 == nb_args:
            return bvars[0]
        else:
            return LogicalAndExpr(self._model, bvars)

    def new_logical_or_expr(self, bvars):
        # assume bvars is a sequence of binary vars
        nb_args = len(bvars)
        if not nb_args:
            return self.new_zero_expr()
        elif 1 == nb_args:
            return bvars[0]
        else:
            return LogicalOrExpr(self._model, bvars)

    def new_max_expr(self, *args):
        nb_args = len(args)
        if 0 == nb_args:
            return - self.infinity
        elif 1 == nb_args:
            return args[0]
        else:
            return MaximumExpr(self._model, [self._to_linear_operand(a) for a in args])

    def new_min_expr(self, *args):
        nb_args = len(args)
        if 0 == nb_args:
            return self.infinity
        elif 1 == nb_args:
            return args[0]
        else:
            return MinimumExpr(self._model, [self._to_linear_operand(a) for a in args])

    def new_abs_expr(self, e):
        if is_number(e):
            return abs(e)
        else:
            self_model = self._model
            return AbsExpr(self_model, self._to_linear_operand(e))

    def resync_whole_model(self):
        self_model = self._model
        self_engine = self._engine

        for var in self_model.iter_variables():
            # do not call create_one_var public API
            # or resync would loop
            idx = self_engine.create_one_variable(var.vartype, var.lb, var.ub, var.name)
            if idx != var.get_index():  # pragma: no cover
                print("index discrepancy: {0!s}, new index= {1}, old index={2}"
                      .format(var, idx, var.get_index()))

        for ct in self_model.iter_constraints():
            if isinstance(ct, LinearConstraint):
                self_engine.create_linear_constraint(ct)
            elif isinstance(ct, RangeConstraint):
                self_engine.create_range_constraint(ct)
            elif isinstance(ct, IndicatorConstraint):
                self_engine.create_indicator_constraint(ct)
            else:
                self_model.error("Unexpected constraint type: {0!s} - ignored", type(ct))  # pragma: no cover

        # send objective
        self_engine.set_objective_sense(self_model.objective_sense)
        self_engine.set_objective_expr(self_model.objective_expr, old_objexpr=None)

    def new_sos(self, dvars, sos_type, name):
        # INTERNAL
        new_sos = SOSVariableSet(model=self._model, variable_sequence=dvars, sos_type=sos_type, name=name)
        return new_sos

    def new_piecewise(self, pwl_def, name):
        self_model = self._model
        # Note that this object is specific only to DOcplex. It does not map to a Cplex object.
        pwl = PwlFunction(self_model, pwl_def=pwl_def, name=name)
        return pwl

    def new_pwl_expr(self, pwl_func, e, usage_counter, add_counter_suffix=True, resolve=True):
        self_model = self._model
        if is_number(e):
            return PwlExpr(self_model, pwl_func, e, usage_counter, add_counter_suffix=add_counter_suffix,
                           resolve=resolve)
        else:
            return PwlExpr(self_model, pwl_func, self._to_linear_operand(e), usage_counter,
                           add_counter_suffix=add_counter_suffix, resolve=resolve)

    def new_pwl_constraint(self, pwl_expr, ctname=None):
        self_model = self._model
        return PwlConstraint(self_model, pwl_expr, ctname)

    def default_objective_sense(self):
        return ObjectiveSense.Minimize

    def new_kpi(self, kpi_arg, name_arg):
        # make a name
        if name_arg:
            publish_name = name_arg
        elif hasattr(kpi_arg, 'name') and kpi_arg.name:
            publish_name = kpi_arg.name
        else:
            publish_name = str_holo(kpi_arg, maxlen=32)
        new_kpi = KPI.new_kpi(self._model, kpi_arg, publish_name)
        return new_kpi

    def new_constraint_block(self, cts, names):
        # INTERNAL
        if names is not None and not self._model.ignore_names:
            return self._new_constraint_block2(cts, names)
        else:
            return self._new_constraint_block1(cts)

    def _new_constraint_block2(self, cts, ctnames):
        posted_cts = []
        prepfn = self._model._prepare_constraint
        checker = self._checker
        check_trivials = checker.check_trivial_constraints()

        for ct, ctname in izip(cts, ctnames):
            checker.typecheck_linear_constraint(ct, accept_ranges=False)
            if prepfn(ct, ctname, check_for_trivial_ct=check_trivials):
                posted_cts.append(ct)
        self._post_constraint_block(posted_cts)
        return posted_cts

    def _new_constraint_block1(self, cts):
        posted_cts = []
        checker = self._checker
        prepfn = self._model._prepare_constraint
        check_trivial = self._checker.check_trivial_constraints()
        # look first
        ctseq = list(cts)
        if not ctseq:
            return []

        try:
            ct, ctname = ctseq[0]
            tuple_mode = True
        except (TypeError, ValueError):
            # TypeError is for non-tuple
            # ValueError is for nonlength-2 tuples
            # not a tuple: we have only constraints and no names
            tuple_mode = False

        if tuple_mode:
            for ct, ctname in ctseq:
                checker.typecheck_linear_constraint(ct, accept_ranges=False)
                checker.typecheck_string(ctname, accept_empty=True)
                if prepfn(ct, ctname, check_for_trivial_ct=check_trivial, arg_checker=checker):
                    posted_cts.append(ct)
        else:
            for ct in ctseq:
                checker.typecheck_linear_constraint(ct, accept_ranges=True)
                if prepfn(ct, ctname=None, check_for_trivial_ct=check_trivial, arg_checker=checker):
                    posted_cts.append(ct)
        self._post_constraint_block(posted_cts)
        return posted_cts

    def _post_constraint_block(self, posted_cts):
        if posted_cts:
            ct_indices = self._engine.create_block_linear_constraints(posted_cts)
            self._model._register_block_cts(self._model._linct_scope, posted_cts, ct_indices)


    # --- range block

    def new_range_block(self, lbs, exprs, ubs, names):
        try:
            n_exprs = len(exprs)
            if n_exprs != len(lbs):
                self.fatal('incorrect number of expressions: expecting {0}, got: {1}'.format(len(lbs), n_exprs))
        except TypeError:
            pass  # no len available.
        if names:
            ranges = [self.new_range_constraint(lb, exp, ub, name) for lb, exp, ub, name in izip(lbs, exprs, ubs, names)]
        else:
            ranges = [self.new_range_constraint(lb, exp, ub) for lb, exp, ub in izip(lbs, exprs, ubs)]
        self._post_constraint_block(ranges)
        return ranges

    def new_solution(self, var_value_dict=None, name=None):
        return SolveSolution(model=self._model, var_value_map=var_value_dict, name=name)

    def _new_var_container(self, vartype, key_list, lb, ub, name):
        # INTERNAL
        ctn = _VariableContainer(vartype, key_list, lb, ub, name)
        old_varctn_counter = self._var_container_counter
        ctn._index = old_varctn_counter
        ctn._index_offset = self._model.number_of_variables  # nb of variables before ctn
        self._var_container_counter = old_varctn_counter + 1

        self._model._add_var_container(ctn)
        return ctn


class _VariableContainer(object):
    def __init__(self, vartype, key_seq, lb, ub, name):
        self._index = 0
        self._index_offset = 0
        self._vartype = vartype
        self._keys = key_seq
        self._lb = lb
        self._ub = ub
        self._name = name
        self._name_str = None

    @property
    def index(self):
        return self._index

    def copy(self, target_model):
        copied_ctn = self.__class__(self.vartype, self._keys, self.lb, self.ub, self._name)
        return copied_ctn

    def copy_relaxed(self, target_model):
        copied_ctn = self.__class__(target_model.continuous_vartype, self._keys, self.lb, self.ub, self._name)
        return copied_ctn

    def keys(self):
        return self._keys

    @property
    def vartype(self):
        return self._vartype

    @property
    def nb_dimensions(self):
        return len(self._keys)

    @property
    def namer(self):
        return self._name

    @property
    def lb(self):
        return self._lb

    @property
    def ub(self):
        return self._ub

    @property
    def name(self):
        """
        Try to extract a name string from the initial container name.
        handles strings with or without formats, arrays, function.

        :return: A string.
        """
        return self._lazy_compute_name_string()
        
    def _lazy_compute_name_string(self):
        if self._name_str is not None:
            return self._name_str
        else:
            raw_name = self._name
            if is_string(raw_name):
                # drop opl-style formats
                s_name = raw_name.replace("({%s})", "")
                # purge fields
                pos_pct = raw_name.find('%')
                if pos_pct >= 0:
                    s_name = raw_name[:pos_pct - 1]
                elif raw_name.find('{') > 0:
                    pos = raw_name.find('{')
                    s_name = raw_name[:pos - 1]
            elif is_iterable(raw_name):
                from os.path import commonprefix
                s_name = commonprefix(raw_name)
            else:
                # try a function
                from os.path import commonprefix
                try:
                    all_names = [raw_name(k) for k in self._keys]
                    s_name = commonprefix(all_names)
                except TypeError:
                    s_name = ''
            self._name_str = s_name
            return s_name

    def get_var_key(self, dvar):
        # INTERNAL
        # containers store expanded keys (as tuples).
        dvar_index = dvar.get_index()
        relative_offset = dvar_index - self._index_offset
        try:
            return self._keys[relative_offset]
        except IndexError:
            return None

    def size(self, dim_index):
        return len(self._keys[dim_index]) if dim_index < self.nb_dimensions else 0

    def shape(self):
        return tuple(len(k) for k in self._keys)

    @property
    def dimension_string(self):
        dim_string = "".join(["[%d]" % self.size(d) for d in range(self.nb_dimensions)])
        return dim_string

    def to_string(self):
        # dvar xxx
        dim_string = self.dimension_string
        ctname = self._name or 'x'
        return "dvar {0} {1} {2}".format(self.vartype.short_name, ctname, dim_string)

    def __str__(self):
        return self.to_string()
