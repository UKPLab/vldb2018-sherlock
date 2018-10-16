# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------


from __future__ import print_function
# from six import iteritems, itervalues

import re

from docplex.mp.linear import *
from docplex.mp.constants import ComparisonType
from docplex.mp.constr import LinearConstraint, RangeConstraint, IndicatorConstraint, QuadraticConstraint, PwlConstraint
from docplex.mp.environment import env_is_64_bit
from docplex.mp.mprinter import TextModelPrinter, _ExportWrapper, _NumPrinter

from docplex.mp.format import LP_format
from itertools import chain


# gendoc: ignore


class LPModelPrinter(TextModelPrinter):
    _lp_re = re.compile(r"[a-df-zA-DF-Z!#$%&()/,;?@_`'{}|\"][a-zA-Z0-9!#$%&()/.,;?@_`'{}|\"]*")

    _lp_symbol_map = {ComparisonType.EQ: " = ",  # BEWARE NOT ==
                      ComparisonType.LE: " <= ",
                      ComparisonType.GE: " >= "}

    __new_line_sep = '\n'
    __expr_indent = ' ' * 6

    float_precision_32 = 9
    float_precision_64 = 12  #
    _nb_noncompliant_ids = 0
    _noncompliant_justifier = None

    def __init__(self, hide_user_names=False, **kwargs):
        nb_digits = self.float_precision_64 if env_is_64_bit() else self.float_precision_32
        TextModelPrinter.__init__(self,
                                  indent=1,
                                  comment_start='\\',
                                  hide_user_names=hide_user_names,
                                  nb_digits_for_floats=nb_digits)

        self._noncompliant_varname = None
        # specific printer for lp: do not print +inf/-inf inside constraints!
        self._lp_num_printer = _NumPrinter(nb_digits_for_floats=nb_digits,
                                           num_infinity=1e+20, pinf="1e+20", ninf="-1e+20")
        self._print_full_obj = kwargs.get('full_obj', False)

    def get_format(self):
        return LP_format

    def mangle_names(self):
        return TextModelPrinter.mangle_names(self) or self._noncompliant_varname

    def _print_ct_name(self, ct, name_map):
        lp_ctname = name_map.get(ct._index)
        indented = self._indent_level

        if lp_ctname is not None:
            ct_label = self._indent_space + lp_ctname + ':'
            indented += len(ct_label)
        else:
            ct_label = ''
        ct_indent_space = self._get_indent_from_level(indented)
        return ct_indent_space, ct_label

    def _print_binary_ct(self, wrapper, num_printer, var_name_map, binary_ct, _symbol_map=_lp_symbol_map,
                         allow_empty=False, force_first_sign=False):
        # ensure consistent ordering: left terms then right terms
        iter_diff_coeffs = binary_ct.iter_net_linear_coefs()
        self._print_expr_iter(wrapper, num_printer, var_name_map, iter_diff_coeffs,
                              allow_empty=True,  # when expr is empty print nothing
                              force_first_plus=force_first_sign)
        wrapper.write(_symbol_map.get(binary_ct.sense, " ?? "), separator=False)
        wrapper.write(num_printer.to_string(binary_ct.cplex_num_rhs()), separator=False)

    def _print_ranged_ct(self, wrapper, num_printer, var_name_map, ranged_ct):
        exp = ranged_ct.expr
        (varname, rhs, _) = self._rangeData[ranged_ct]
        self._print_lexpr(wrapper, num_printer, var_name_map, exp)
        wrapper.write('-', separator=False)
        wrapper.write(varname)
        wrapper.write('=')
        wrapper.write(self._num_to_string(rhs))


    def _print_logical_ct(self, wrapper, num_printer, var_name_map, logical_ct,
                          logical_symbol):
        wrapper.write(self._var_print_name(logical_ct.binary_var))
        wrapper.write("=")
        wrapper.write("%d" % logical_ct.active_value)
        wrapper.write(logical_symbol)
        self._print_binary_ct(wrapper, num_printer, var_name_map, logical_ct.linear_constraint)


    def _print_quadratic_ct(self, wrapper, num_printer, var_name_map, qct):
        q = self._print_qexpr_iter(wrapper, num_printer, var_name_map, qct.iter_net_quads())
        # force a '+' ?
        has_quads = q > 0
        self._print_binary_ct(wrapper, num_printer, var_name_map, qct, allow_empty=has_quads,
                              force_first_sign=has_quads)

    def _print_pwl_ct(self, wrapper, num_printer, var_name_map, pwl):
        """
        Prints a PWL ct in LP
        :param wrapper:
        :param pwl
        :return:
        """
        num2string_fn = num_printer.to_string
        wrapper.write('%s = %s' % (var_name_map[pwl.y._index], var_name_map[pwl.expr._index]))
        pwl_func = pwl.pwl_func
        pwl_def = pwl_func.pwl_def_as_breaks
        wrapper.write('%s' % num2string_fn(pwl_def.preslope))
        for pair in pwl_def.breaksxy:
            wrapper.write('(%s, %s)' % (num2string_fn(pair[0]), num2string_fn(pair[1])))
        wrapper.write('%s' % num2string_fn(pwl_def.postslope))

    def _print_constraint_label(self, wrapper, ct, name_map):
        if self._mangle_names:
            wrapper.set_indent('')
        else:
            indent_str, ct_label = self._print_ct_name(ct, name_map=name_map)
            wrapper.set_indent(indent_str)
            if ct_label is not None:
                wrapper.write(ct_label)

    def _print_constraint(self, wrapper, num_printer, var_name_map, ct):
        if isinstance(ct, PwlConstraint):
            # Pwl constraints are printed in a separate section (names 'PWL')
            return

        wrapper.begin_line()
        if isinstance(ct, LinearConstraint):
            self._print_constraint_label(wrapper, ct, name_map=self._linct_name_map)
            self._print_binary_ct(wrapper, num_printer, var_name_map, ct)
        elif isinstance(ct, RangeConstraint):
            self._print_constraint_label(wrapper, ct, name_map=self._linct_name_map)
            self._print_ranged_ct(wrapper, num_printer, var_name_map, ct)
        elif ct.is_logical():
            is_eq = ct.is_equivalence()
            logical_symbol = '<->' if is_eq else '->'
            self._print_constraint_label(wrapper, ct, name_map=self._lc_name_map)
            self._print_logical_ct(wrapper, num_printer, var_name_map, ct,
                                   logical_symbol=logical_symbol,
                                   # avoid printing active value for equivalence with active=1
                                   )
        elif isinstance(ct, QuadraticConstraint):
            self._print_constraint_label(wrapper, ct, name_map=self._qc_name_map)
            self._print_quadratic_ct(wrapper, num_printer, var_name_map, ct)
        else:
            ct.error("ERROR: unexpected constraint not printed: {0!s}".format(ct))  # pragma: no cover

        wrapper.flush(print_newline=True, reset=True)

    def _print_pwl_constraint(self, wrapper, num_printer, var_name_map, ct):
        wrapper.begin_line()
        self._print_constraint_label(wrapper, ct, name_map=self._pwl_name_map)
        self._print_pwl_ct(wrapper, num_printer, var_name_map, ct)
        wrapper.flush(print_newline=True, reset=True)

    def _print_var_block(self, wrapper, iter_vars, header):
        wrapper.begin_line()
        printed_header = False
        self_indent = self._indent_space
        for v in iter_vars:
            lp_name = self._var_print_name(v)
            if not printed_header:
                wrapper.newline()
                wrapper.write(header)
                printed_header = True
                wrapper.set_indent(self_indent)
                # Configure indent for next lines
                wrapper.flush(print_newline=True)
            wrapper.write(lp_name)
        if printed_header:
            wrapper.flush(print_newline=True)

    def _print_var_bounds(self, out, num_printer, varname, lb, ub, varname_indent=5 * ' ',
                          le_symbol='<=',
                          free_symbol='Free'):
        if lb is None and ub is None:
            # try to indent with space of '0 <= ', that is 5 space
            out.write("%s %s %s\n" % (varname_indent, varname, free_symbol))
        elif lb is None:
            out.write("%s %s %s %s\n" % (varname_indent, varname, le_symbol, num_printer.to_string(ub)))
        elif ub is None:
            out.write("%s %s %s\n" % (num_printer.to_string(lb), le_symbol, varname))
        elif lb == ub:
            out.write("%s %s %s %s\n" % (varname_indent, varname, "=", num_printer.to_string(lb)))
        else:
            out.write("%s %s %s %s %s\n" % (num_printer.to_string(lb),
                                            le_symbol, varname, le_symbol,
                                            num_printer.to_string(ub)))

    TRUNCATE = 200

    @staticmethod
    def _non_compliant_lp_name_stop_here(name):
        pass

    def fix_name(self, mobj, prefix, local_index_map, hide_names):
        raw_name = mobj.name

        # anonymous constraints must be named in a LP (we follow CPLEX here)
        if hide_names or not raw_name or mobj.is_generated():
            return self._make_prefix_name(mobj, prefix, local_index_map, offset=1)
        elif not self._is_lp_compliant(raw_name):
            if raw_name[0] in 'eE':
                # fixing eE non-LP names
                fixed_name = '_' + raw_name
                if  self._is_lp_compliant(fixed_name):
                    return fixed_name
            # -- stats
            self._nb_noncompliant_ids += 1
            if not self._noncompliant_justifier:
                self._noncompliant_justifier = raw_name
            # --
            self._non_compliant_lp_name_stop_here(raw_name)
            return self._make_prefix_name(mobj, prefix, local_index_map, offset=1)
        else:
            # swap blanks with underscores
            fixed_name = self._translate_chars(raw_name)
            # truncate if necessary, again this does nothing if name is too short
            return fixed_name[:self.TRUNCATE]

    def _print_model_name(self, out, model):
        model_name = None
        if model.name:
            # make sure model name is ascii
            encoded = model.name.encode('ascii', 'backslashreplace')
            if sys.version_info[0] == 3:
                # in python 3, encoded is a bytes at this point. Make it a string again
                encoded = encoded.decode('ascii')
            model_name = encoded.replace('\\\\', '_').replace('\\', '_')
        printed_name = model_name or 'CPLEX'
        out.write("\\Problem name: %s\n" % printed_name)

    @staticmethod
    def _is_lp_compliant(name, _lpname_regexp=_lp_re):
        if name is None:
            return True  # pragma: no cover
        # PUT THIS SOMEWHERE ELSE
        fixed_name = LPModelPrinter.fix_whitespace(name)
        lp_match = _lpname_regexp.match(fixed_name)
        return lp_match and lp_match.start() == 0 and lp_match.end() == len(fixed_name)

    @staticmethod
    def _is_injective(name_map):
        nb_keys = len(name_map)
        nb_different_names = len(set(name_map.values()))
        return nb_different_names == nb_keys

    @staticmethod
    def _update_variables_set(expr_iter, unreferenced_variables):   # pragma: no cover
        for (v, coeff) in expr_iter:
            if not coeff:
                continue  # pragma: no cover
            unreferenced_variables.discard(v)

    @staticmethod
    def _add_to_variables_set(expr_iter, var_set):
        for (v, coeff) in expr_iter:
            var_set.add(v)


    def _get_forced_predeclared_variables(self, model):
        # compute predeclared variables
        predeclared_variables = set()
        for sos in model.iter_sos():
            for sos_var in sos.iter_variables():
                predeclared_variables.add(sos_var)
        for pwl in model.iter_pwl_constraints():
            predeclared_variables.add(pwl.y)
            for pwv in pwl.expr.iter_variables():
                predeclared_variables.add(pwv)
        return predeclared_variables

    def _get_all_referenced_variables(self, model, objlin):
        referenced_variables = set()
        for ct in model.iter_constraints():
            if isinstance(ct, LinearConstraint):
                if not ct.is_trivial_feasible():
                    iter_diff_coeffs = ct.iter_net_linear_coefs()
                    self._add_to_variables_set(iter_diff_coeffs, referenced_variables)
            elif isinstance(ct, RangeConstraint):
                term_iter = ct.expr.iter_sorted_terms()
                self._add_to_variables_set(term_iter, referenced_variables)
            elif isinstance(ct, IndicatorConstraint):
                binary_var = ct.binary_var
                referenced_variables.add(binary_var)
                iter_diff_coeffs = ct.linear_constraint.iter_net_linear_coefs()
                self._add_to_variables_set(iter_diff_coeffs, referenced_variables)
            elif isinstance(ct, QuadraticConstraint):
                iter_quads = ct.iter_net_quads()
                for vp, _ in iter_quads:
                    referenced_variables.add(vp.first)
                    referenced_variables.add(vp.second)
                iter_diff_coeffs = ct.iter_net_linear_coefs()
                self._add_to_variables_set(iter_diff_coeffs, referenced_variables)
            elif isinstance(ct, PwlConstraint):
                # Pwl constraints are not handled later on
                pass
            else:
                ct.error("ERROR: unexpected constraint not handled: {0!s}".format(ct))  # pragma: no cover
        # Second: add variables that occur in SOS and PWL constraints
        for sos in model.iter_sos():
            for sos_var in sos.iter_variables():
                referenced_variables.add(sos_var)
        for pwl in model.iter_pwl_constraints():
            referenced_variables.add(pwl.y)
            referenced_variables.add(pwl.expr)
        return referenced_variables

    @staticmethod
    def _has_sos_or_pwl_constraints(model):
        return model.number_of_sos > 0 or model.number_of_pwl_constraints > 0

    def _iter_completed_linear_obj_terms(self, model, objlin):
        obj_variables = set(v for v, _ in objlin.iter_terms()) # unsorted
        predeclared_variables = self._get_forced_predeclared_variables(model)
        variables_to_display = predeclared_variables | obj_variables
        # sort by index
        sorted_display_vars = list(variables_to_display)
        sorted_display_vars.sort(key=lambda dv: dv._index)
        for v in sorted_display_vars:
            yield v, objlin.unchecked_get_coef(v)

    def _iter_full_linear_obj_terms(self, model, objlin):
        # INTERNAL: print all variables and their coef in the linear part of the objective
        for v in model.iter_variables():
            yield v, objlin.unchecked_get_coef(v)

    def post_print_hook(self, model):
        nb_non_compliants = self._nb_noncompliant_ids
        if nb_non_compliants:
            try:
                model.warning('Some identifiers are not valid LP identifiers: %d (e.g.: "%s")',
                                   nb_non_compliants, self._noncompliant_justifier)
            except UnicodeEncodeError:
                model.warning('Some identifiers are not valid LP identifiers: %d (e.g.: "%s")',
                                   nb_non_compliants, self._noncompliant_justifier.encode('utf-8'))

    #  @profile
    def print_model_to_stream(self, out, model):
        # reset noncompliant stats
        self._nb_noncompliant_ids = 0
        self._noncompliant_justifier = None

        if not self._is_injective(self._var_name_map):
            # use indices to differentiate names
            sys.__stdout__.write("\DOcplex: refine variable names\n")
            k = 0
            for dv, lp_varname in iteritems(self._var_name_map):
                refined_name = "%s#%d" % (lp_varname, k)
                self._var_name_map[dv] = refined_name
                k += 1

        TextModelPrinter.prepare(self, model)
        self_num_printer = self._lp_num_printer
        var_name_map = self._var_name_map

        self._print_signature(out)
        self._print_encoding(out)
        self._print_model_name(out, model)
        self._newline(out)

        # ---  print objective
        out.write(model.objective_sense.name)
        self._newline(out)
        wrapper = _ExportWrapper(out, indent_str=self.__expr_indent)
        wrapper.write(' obj:')
        objexpr = model.objective_expr

        if objexpr.is_quad_expr():
            objlin = objexpr.linear_part
        else:
            objlin = objexpr


        if self._print_full_obj:
            iter_linear_terms = self._iter_full_linear_obj_terms(model, objlin)

        elif self._has_sos_or_pwl_constraints(model):
            iter_linear_terms = self._iter_completed_linear_obj_terms(model, objlin)


        else:
            # write the linear part first
            # prints an expr to a stream
            iter_linear_terms = objlin.iter_sorted_terms()

        printed= self._print_expr_iter(wrapper, self_num_printer, var_name_map, iter_linear_terms,
                              allow_empty=True, accept_zero=True)


        if objexpr.is_quad_expr() and objexpr.has_quadratic_term():
            self._print_qexpr_obj(wrapper, self_num_printer, var_name_map,
                                  quad_expr=objexpr,
                                  force_initial_plus=printed)
            printed = True

        obj_offset = objexpr.get_constant()
        if obj_offset:
            if printed and obj_offset > 0:
                wrapper.write(u'+')
            wrapper.write(self._num_to_string(obj_offset))
        # ---

        wrapper.flush(print_newline=True)

        out.write("Subject To\n")

        for ct in model.iter_constraints():
            self._print_constraint(wrapper, self_num_printer, var_name_map, ct)
        for lct in model.iter_implicit_equivalence_cts():
            wrapper.begin_line(True)
            self._print_logical_ct(wrapper, self_num_printer, var_name_map, lct, '<->')
            wrapper.flush(print_newline=True, reset=True)

        out.write("\nBounds\n")
        symbolic_num_printer = self._num_printer
        print_var_bounds_fn = self._print_var_bounds
        var_print_name_fn = self._var_print_name
        for dvar in model.iter_variables():
            lp_varname = var_print_name_fn(dvar)
            var_lb = dvar._get_lb()
            var_ub = dvar._get_ub()
            if dvar.is_binary():
                print_var_bounds_fn(out, self_num_printer, lp_varname, var_lb, var_ub)
            else:

                free_lb = dvar.has_free_lb()
                free_ub = dvar.has_free_ub()
                if free_lb and free_ub:
                    print_var_bounds_fn(out, self_num_printer, lp_varname, lb=None, ub=None)
                elif free_ub:
                    # avoid zero lb
                    if 0 != var_lb:
                        print_var_bounds_fn(out, symbolic_num_printer, lp_varname, var_lb, ub=None)
                    else:
                        # lb is zero, ub is infinity, we dont print anything
                        pass
                else:
                    # save the lb if is zero
                    printed_lb = None if 0 == var_lb else var_lb
                    print_var_bounds_fn(out, symbolic_num_printer, lp_varname, lb=printed_lb, ub=var_ub)

        # add ranged cts vars
        for rng in model.iter_range_constraints():
            (varname, _, ub) = self._rangeData[rng]
            self._print_var_bounds(out, self_num_printer, varname, 0, ub)

        iter_semis = chain(model.iter_semicontinuous_vars(), model.iter_semiinteger_vars())

        self._print_var_block(wrapper, model.iter_binary_vars(), 'Binaries')
        self._print_var_block(wrapper, chain(model.iter_integer_vars(), model.iter_semiinteger_vars()), 'Generals')
        self._print_var_block(wrapper, iter_semis, 'Semi-continuous')
        self._print_sos_block(wrapper, model)
        self._print_pwl_block(wrapper, model, self_num_printer, var_name_map)
        out.write("End\n")

    def _print_sos_block(self, wrapper, mdl):
        if mdl.number_of_sos > 0:
            wrapper.write('SOS')
            wrapper.flush(print_newline=True)
            name_fn = self._var_print_name
            for sos in mdl.iter_sos():
                sos_name = sos.get_name()
                if sos_name:
                    wrapper.write('%s:' % sos_name)
                wrapper.write('S%d ::' % sos.sos_type.value)  # 1 or 2
                ranks = sos.get_ranks()
                for rank, sos_var in izip(ranks, sos._variables):
                    wrapper.write('%s : %d' % (name_fn(sos_var), rank))
                wrapper.flush(print_newline=True)

    def _print_pwl_block(self, wrapper, mdl, self_num_printer, var_name_map):
        if mdl.number_of_pwl_constraints > 0:
            wrapper.write('Pwl')
            wrapper.flush(print_newline=True)
            # name_fn = self._var_print_name
            for pwl in mdl.iter_pwl_constraints():
                self._print_pwl_constraint(wrapper, self_num_printer, var_name_map, pwl)
