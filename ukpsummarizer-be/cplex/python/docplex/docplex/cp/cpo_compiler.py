# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------
# Author: Olivier OUDOT, IBM Analytics, France Lab, Sophia-Antipolis

"""
Compiler converting internal model representation to CPO file format.
"""

from docplex.cp.expression import *
from docplex.cp.solution import *
from docplex.cp.utils import *
import docplex.cp.config as config

import sys


###############################################################################
## Constants
###############################################################################

# Map of CPO names for each array type
_ARRAY_TYPES = {Type_IntArray: 'intArray', Type_FloatArray: 'floatArray',
                Type_IntExprArray: 'intExprArray', Type_FloatExprArray: 'floatExprArray',
                Type_CumulExprArray: 'cumulExprArray',
                Type_IntervalVarArray: 'intervalVarArray', Type_SequenceVarArray: 'sequenceVarArray',
                Type_CumulAtomArray: '_cumulAtomArray'}


# Set of CPO types represented by an integer
_INTEGER_TYPES = frozenset((Type_Int, Type_PositiveInt, Type_TimeInt))


###############################################################################
## Public classes
###############################################################################

class CpoCompiler(object):
    """ Compiler to CPO file format """
    __slots__ = ('model',                      # Source model
                 'parameters',                 # Solving parameters
                 'add_source_location',        # Indicator to add location traces in generated output
                 'min_name_length_for_alias',  # Minimum variable name length to use an alias
                 'min_name_length_for_rename', # Minimum variable name length to replace it by an alias
                 'identifier_strings',         # Dictionary of printable string for each identifier
                 'last_location',              # Last source location (file, line)
                 'rename_map',                 # Dictionary of variables rename (key=new name, value = original name)
                 'list_vars',                  # List of variables
                 'list_exprs',                 # List of model expressions
                 'list_phases',                # List of search phases
                 )

    def __init__(self, model, **kwargs):
        """ Create a new compiler

        Args:
            model:  Source model
        Optional args:
            context:             Global solving context. If not given, context is the default context that is set in config.py.
            params:              Solving parameters (CpoParameters) that overwrites those in solving context
            add_source_location: Add source location into generated text
            length_for_alias:    Minimum name length to use shorter alias instead
            (others):            All other context parameters that can be changed
        """
        super(CpoCompiler, self).__init__()

        # Build effective context
        if model:
            mparams = model.get_parameters()
            if mparams:
                pparams = kwargs.get('params')
                if pparams:
                    mparams = mparams.clone()
                    mparams.add(pparams)
                kwargs['params'] = mparams
        context = config._get_effective_context(**kwargs)

        # Initialize processing
        self.model = model
        self.parameters = context.params

        self.min_name_length_for_alias = None
        self.min_name_length_for_rename = None
        self.identifier_strings = {}
        self.rename_map = None

        # Set model parameters
        mctx = context.model
        if mctx is not None:
            ma = mctx.length_for_alias
            mr = mctx.length_for_rename
            if (mr is not None) and (ma is not None) and (ma >= mr):
                ma = None
            self.min_name_length_for_alias = ma
            self.min_name_length_for_rename = mr

        # Initialize source location
        if (self.parameters is not None) and (self.parameters.UseFileLocations is not None):
            self.add_source_location = (self.parameters.UseFileLocations in ('On', True))
        elif (mctx is not None) and (mctx.add_source_location is not None):
            self.add_source_location = mctx.add_source_location
        else:
            self.add_source_location = True
        self.last_location = None


    def print_model(self, out=None):
        """ Compile the model and print the CPO file format in a given output.

        If the given output is a string, it is considered as a file name that is opened by this method
        using 'utf-8' encoding.

        Args:
            out: Target output, stream or file name. Default is sys.stdout.
        """
        # Check file name
        if out is None:
            out = sys.stdout
        if is_string(out):
            with open_utf8(os.path.abspath(out), mode='w') as f:
                self._write_model(f)
        else:
            self._write_model(out)


    def get_as_string(self):
        """ Compile the model in CPO file format into a string

        Returns:
            String containing the model
        """
        # Print the model into a string
        out = StringIO()
        self._write_model(out)
        res = out.getvalue()
        out.close()

        # Convert in unicode if required
        if IS_PYTHON_2 and (type(res) is str):
            res = unicode(res)
        return res


    def get_rename_map(self):
        """ Get the map of variables that has been renamed.

        This map is required to retrieve the original variable name from the name returned in the solution.

        Returns:
            Map of variable renamings. Key is new variable name, Value is original variable name.
            None if no renaming has been done.
        """
        return self.rename_map


    def _write_model(self, out):
        """ Compile the model

        Args:
            out: target output
        """
        # Expand model expressions (share sub-expressions)
        model = self.model
        list_vars, list_exprs, list_phases = self._expand_all_expressions()

        self.last_location = None

        # Write header
        banner = u"/" * 79 + "\n"
        mname = model.get_name()
        sfile = model.get_source_file()
        out.write(banner)
        if mname:
            out.write(u"// CPO file generated for model: {}\n".format(mname))
        else:
            out.write(u"// CPO file generated for anonymous model\n")
        if sfile:
            out.write(u"// Source file: {}\n".format(sfile))
        out.write(banner)

        # Write version if any
        ver = model.get_format_version()
        if ver is not None:
            out.write(u"\n//--- Internals ---\n")
            out.write(u"internals {\n")
            out.write(u"   version({});\n".format(ver))
            out.write(u"}\n")

        # Get working variables
        idstrings = self.identifier_strings

        # Rename variables if required
        mnl = self.min_name_length_for_rename
        if mnl is not None:
            # Rename variables whose name is too long
            rename_gen = IdAllocator('_R_', IdAllocator.LETTERS_AND_DIGITS)
            renmap = self.rename_map if self.rename_map else {}
            for lx in list_vars:
                v = lx[0]
                # Compute CPO printable variable name
                vname = v.name
                vpname = self._get_id_string(vname)
                if (len(vpname) > mnl) and (vpname[0] != '_'):
                    # Rename variable
                    vpname = rename_gen.allocate()
                    renmap[vpname] = vname
                    idstrings[vname] = vpname
            self.rename_map = None if len(renmap) == 0 else renmap

        # Write variables
        out.write(u"\n//--- Variables ---\n")
        for lx in list_vars:
            self._write_expression(out, lx)

        # If aliases are requested, print as comment list of aliases
        mnl = self.min_name_length_for_alias
        if mnl is not None:
            # Preload string map with aliases when relevant
            aliasfound = False
            alias_gen = IdAllocator('_A_', IdAllocator.LETTERS_AND_DIGITS)
            for lx in list_vars:
                v = lx[0]
                # Compute CPO printable variable name
                vname = v.name
                vpname = strname = self._get_id_string(vname)
                if (len(vpname) > mnl) and (vpname[0] != '_'):
                    # Replace by an alias
                    vpname = alias_gen.allocate()
                    # Trace alias
                    if not aliasfound:
                        aliasfound = True
                        out.write(u"\n//--- Aliases ---\n")
                        out.write(u"// To reduce CPO file size, the following aliases have been used to replace variable names longer than " + str(mnl) + "\n")
                    out.write(vpname + " = " + strname + ";\n")
                    idstrings[vname] = vpname

        # Write expressions
        out.write(u"\n//--- Expressions ---\n")
        self.last_location = None
        for lx in list_exprs:
            self._write_expression(out, lx)

        # Write search phases 
        if list_phases:
            out.write(u"\n//--- Search phases ---\n")
            out.write(u"search {\n")
            for lx in list_phases:
                self._write_expression(out, lx)
            out.write(u"}\n")

        # Write starting point
        spoint = model.get_starting_point()
        if spoint is not None:
            out.write(u"\n//--- Starting point ---\n")
            if self.last_location is not None:
                out.write(u"#line off\n")
            out.write(u"startingPoint {\n")
            for var in spoint.get_all_var_solutions():
                self._write_starting_point(out, var)
            out.write(u"}\n")

        # Write parameters
        if self.parameters and (len(self.parameters) > 0):
            out.write(u"\n//--- Parameters ---\n")
            if self.last_location is not None:
                out.write(u"#line off\n")
            out.write(u"parameters {\n")
            for k in sorted(self.parameters.keys()):
                v = self.parameters[k]
                if v is not None:
                    out.write(u"   {} = {};\n".format(k, v))
            out.write(u"}\n")

        # Flush stream (required on Linux rhel6.7)
        out.flush()


    def _write_expression(self, out, locexpr):
        """ Write model expression

        Args:
            out:   Target output
            locexpr: Located expression, tuple (expr, loc, sub_expressions, already_compiled)
        """
        # Retrieve expression elements
        expr, loc, sexprs, compiled = locexpr

        # Trace location if required
        lloc = self.last_location
        if self.add_source_location and (loc is not None) and (loc != lloc):
            (file, line) = loc
            lline = u"#line " + str(line)
            if (lloc is None) or (file != lloc[0]):
                lline += u' "' + file + '"'
            out.write(lline + u"\n")
            self.last_location = loc

        # Write sub-expressions if any
        for sx in sexprs:
            self._write_sub_expression(out, sx)

        # Write expression
        if not compiled:
            self._write_sub_expression(out, expr)

        # Write expression label (for constraints)
        if expr.name and expr.type in (Type_Constraint, Type_BoolExpr, Type_SearchPhase):
            out.write(self._get_id_string(expr.name) + u";\n")

    def _write_sub_expression(self, out, expr):
        """ Write model expression

        Args:
            out:   Target output
            expr:  Expression to write
        """
        # Write expression
        id = expr.name
        if id is not None:
            out.write(self._get_id_string(id) + u" = ")
        out.write(self._compile_expression(expr))
        out.write(u";\n")


    def _write_starting_point(self, out, var):
        """ Write a starting point variable

        Args:
            out:  Target output
            var:  Variable solution
        """
        # Build starting point declaration
        cout = []
        if isinstance(var, CpoIntVarSolution):
            self._compile_int_var_starting_point(var, cout)
        elif isinstance(var, CpoIntervalVarSolution):
            self._compile_interval_var_starting_point(var, cout)
        else:
            raise CpoException("Internal error: unsupported starting point variable: " + str(var))
        # Write variable starting point
        out.write(self._get_id_string(var.name) + u" = " + u''.join(cout) + u";\n")


    def _get_id_string(self, id):
        """ Get the string representing an identifier

        Args:
            id: Identifier name
        Returns:
            CPO identifier string, including double quotes and escape sequences if needed if not only chars and integers
        """
        # Check if already converted
        res = self.identifier_strings.get(id)
        if res is None:
            # Convert id into string and store result for next call
            res = to_printable_symbol(id)
            self.identifier_strings[id] = res
        return res


    def _compile_expression(self, expr, root=True):
        """ Compile an expression in a string in CPO format

        Args:
            expr: Expression to compile
            root: Root expression indicator
        Returns:
            String representing this expression in CPO format
        """
        # Initialize working variables
        cout = []  # Result list of strings
        estack = [[expr, -1, False]]  # Expression stack [Expression, child index, parenthesis]

        # Loop while expression stack is not empty
        while estack:
            # Get expression to compile
            edscr = estack[-1]
            e = edscr[0]

            # Check if expression is named and not root (named expression and variable)
            if (not root or (e is not expr)) and e.name:
                cout.append(self._get_id_string(e.name))
                estack.pop()
                continue

            # Check constant expressions
            t = e.type
            if t.is_constant:
                estack.pop()
                if t.is_array:
                    vals = e.value
                    if len(vals) == 0:
                        cout.append(_ARRAY_TYPES[t])
                        cout.append("[]")
                    else:
                        cout.append('[')
                        self._compile_var_domain(vals, cout)
                        #cout.append(', '.join(str(v) for v in vals))
                        cout.append(']')
                elif (t is Type_Bool):
                    cout.append("true()" if e.value else "false()")
                elif (t in _INTEGER_TYPES):
                    cout.append(_number_value_string(e.value))
                elif (t is Type_TransitionMatrix):
                    self._compile_transition_matrix(e, cout)
                elif (t is Type_TupleSet):
                    self._compile_tuple_set(e.value, cout)
                elif (t is Type_StepFunction):
                    self._compile_step_function(e, cout)
                elif (t is Type_SegmentedFunction):
                    self._compile_segmented_function(e, cout)
                else:
                    cout.append(_number_value_string(e.value))

            # Check variables
            elif t.is_variable:
                estack.pop()
                if (t is Type_IntVar):
                    self._compile_int_var(e, cout)
                elif (t is Type_IntervalVar):
                    self._compile_interval_var(e, cout)
                elif (t is Type_SequenceVar):
                    self._compile_sequence_var(e, cout)
                elif (t is Type_StateFunction):
                    self._compile_state_function(e, cout)

            # Check expression array
            elif t.is_array:
                oprnds = e.children
                alen = len(oprnds)
                if alen == 0:
                    cout.append(_ARRAY_TYPES[t])
                    cout.append("[]")
                    estack.pop()
                else:
                    cnx = edscr[1]
                    if (cnx < 0):
                        cout.append("[")
                    cnx += 1
                    if (cnx >= alen):
                        cout.append("]")
                        estack.pop()
                    else:
                        edscr[1] = cnx
                        if (cnx > 0):
                            cout.append(", ")
                        estack.append([oprnds[cnx], -1, False])

            # General expression
            else:
                # Get operation elements
                oper = e.operation
                prio = oper.priority
                oprnds = e.children
                oplen = len(oprnds)
                cnx = edscr[1]

                # Check if function call
                if (prio < 0):
                    # Check first call
                    if (cnx < 0):
                        cout.append(oper.keyword)
                        cout.append("(")
                    cnx += 1
                    if (cnx >= oplen):
                        cout.append(")")
                        estack.pop()
                    else:
                        edscr[1] = cnx
                        if (cnx > 0):
                            cout.append(", ")
                        estack.append([oprnds[cnx], -1, False])

                # Write operation
                else:
                    # Check parenthesis required
                    parents = edscr[2]

                    # Write operation
                    if (cnx < 0):
                        if (oplen == 1):
                            cout.append(oper.keyword)
                        if parents:
                            cout.append("(")
                    cnx += 1
                    if (cnx >= oplen):
                        # All operands have been processed
                        if parents:
                            cout.append(")")
                        estack.pop()
                    else:
                        # Process operand
                        edscr[1] = cnx
                        if (cnx > 0):
                            # Add operator
                            cout.append(" " + oper.keyword + " ")
                        # Check if operand will require to have parenthesis
                        arg = oprnds[cnx]
                        nprio = arg.priority
                        # Parenthesis required if priority is greater than parent node, or if this node is not first child
                        chparnts = (nprio > prio) \
                                  or (nprio >= 5) \
                                  or ((nprio == prio) and (cnx > 0)) \
                                  or ((oplen == 1) and not parents and oprnds[0].children)
                        # Put operand on stack
                        estack.append([arg, -1, chparnts])

        # Check output exists
        if not cout:
            raise CpoException("Internal error: unable to compile expression: " + str(expr))
        return u''.join(cout)


    def _compile_int_var(self, v, cout):
        """ Compile a IntVar in a string in CPO format
        Args:
            v:    Variable
            cout: Output string list
        """
        cout.append("intVar(")
        self._compile_var_domain(v.get_domain(), cout)
        cout.append(")")


    def _compile_int_var_starting_point(self, v, cout):
        """ Compile a starting point IntVar in a string in CPO format
        Args:
            v:    Variable
            cout: Output string list
        """
        cout.append("(")
        self._compile_var_domain(v.value, cout)
        cout.append(")")


    def _compile_interval_var(self, v, cout):
        """ Compile a IntervalVar in a string in CPO format
        Args:
            v:    Variable
            cout: Output string list
        """
        cout.append("intervalVar(")
        args = []
        if v.is_absent():
            args.append("absent")
        elif v.is_optional():
            args.append("optional")
        if (v.start != DEFAULT_INTERVAL):
            args.append("start=" + _interval_var_domain_string(v.start))
        if (v.end != DEFAULT_INTERVAL):
            args.append("end=" + _interval_var_domain_string(v.end))
        if (v.length != DEFAULT_INTERVAL):
            args.append("length=" + _interval_var_domain_string(v.length))
        if (v.size != DEFAULT_INTERVAL):
            args.append("size=" + _interval_var_domain_string(v.size))
        if (v.intensity is not None):
            args.append("intensity=" + self._compile_expression(v.intensity, root=False))
        if (v.granularity is not None):
            args.append("granularity=" + str(v.granularity))
        cout.append(", ".join(args) + ")")


    def _compile_interval_var_starting_point(self, v, cout):
        """ Compile a starting IntervalVar in a string in CPO format
        Args:
            v:    Variable
            cout: Output string list
        """
        if v.is_absent():
            cout.append("absent")
            return
        cout.append("(")
        cout.append("present" if v.is_present() else "optional")
        rng = v.get_start()
        if rng is not None:
            cout.append(", start=")
            self._compile_var_domain([rng], cout)
        rng = v.get_end()
        if rng is not None:
            cout.append(", end=")
            self._compile_var_domain([rng], cout)
        rng = v.get_size()
        if rng is not None:
            cout.append(", size=")
            self._compile_var_domain([rng], cout)
        cout.append(")")


    def _compile_sequence_var(self, sv, cout):
        """ Compile a SequenceVar in a string in CPO format
        Args:
            sv:   Sequence variable
            cout: Output string list
        """
        cout.append("sequenceVar(")
        lvars = sv.get_interval_variables()
        if len(lvars) == 0:
            cout.append("intervalVarArray[]")
        else:
            cout.append("[" + ", ".join(self._get_id_string(v.name) for v in lvars) + "]")
        types = sv.get_types()
        if (types is not None):
            if len(lvars) == 0:
                cout.append(", intArray[]")
            else:
                cout.append(", [" + ", ".join(str(t) for t in types) + "]")
        cout.append(")")


    def _compile_state_function(self, stfct, cout):
        """ Compile a State in a string in CPO format

        Args:
           stfct: Segmented function
           cout:  Output string list
        """
        cout.append("stateFunction(")
        trmx = stfct.get_transition_matrix()
        if trmx is not None:
            cout.append(self._compile_expression(trmx, root=False))
        cout.append(")")


    def _compile_transition_matrix(self, tm, cout):
        """ Compile a TransitionMatrix in a string in CPO format

        Args:
            tm:   Transition matrix
            cout: Output string list
        """
        cout.append("transitionMatrix(")
        cout.append(", ".join(str(v) for v in tm.get_all_values()))
        cout.append(")")


    def _compile_tuple_set(self, tplset, cout):
        """ Compile a TupleSet in a string in CPO format

        Args:
           tplset: Tuple set as tuple of tuples
           cout:   Output string list
        """
        cout.append("[")
        for i, tpl in enumerate(tplset):
            if i > 0:
                cout.append(", ")
            cout.append("[")
            cout.append(", ".join(str(x) for x in tpl))
            #self._compile_list_of_integers(tpl, cout)
            cout.append("]")
        cout.append("]")


    def _compile_var_domain(self, dom, cout):
        """ Compile a variable domain in CPO format

        Args:
            dom:   Variable domain
            cout:  Output string list
        """
        if is_array(dom):
            for i, d in enumerate(dom):
                if i > 0:
                    cout.append(", ")
                if (isinstance(d, (list, tuple))):
                    cout.append(_int_var_domain_string(d))
                else:
                    cout.append(_number_value_string(d))
        else:
            cout.append(_number_value_string(dom))


    def _compile_list_of_integers(self, lint, cout):
        """ Compile a list of integers in CPO format

        Args:
            lint:  List of integers
            cout:  Output string list
        """
        llen = len(lint)
        i = 0
        while i < llen:
            if i > 0:
                cout.append(", ")
            j = i + 1
            while (j < llen) and (lint[j] == lint[j - 1] + 1):
                j += 1
            if (j > i + 1):
                cout.append(str(lint[i]) + ".." + str(lint[j - 1]))
            else:
                cout.append(str(lint[i]))
            i = j


    def _compile_step_function(self, stfct, cout):
        """ Compile a StepFunction in a string in CPO format

        Args:
           stfct: Step function
           cout:  Output string list
        """
        cout.append("stepFunction(")
        for i, s in enumerate(stfct.get_step_list()):
            if i > 0:
                cout.append(", ")
            cout.append('(' + _number_value_string(s[0]) + ", " + str(s[1]) + ')')
        cout.append(")")


    def _compile_segmented_function(self, sgfct, cout):
        """ Compile a SegmentedFunction in a string in CPO format

        Args:
           sgfct: Segmented function
           cout:  Output string list
        """
        cout.append("segmentedFunction(")
        cout.append(", ".join(map(to_string, sgfct.get_segment_list())))
        cout.append(")")


    def _expand_expression(self, expr, doneset):
        """ Get the list of all sub-expressions required to compile an expression.

        Sub-expressions are named expressions. Cause may be:
         * used multiple times,
         * explicitly named by end-user

        Args:
            expr:    Expression to compile
            doneset: Set of already done expressions
        Returns:
            List of sub-expressions to compile, in compilation order.
            Root expression is NOT included in this list
        """
        # Expand children expressions
        estack = []
        for e in expr.children:
            if not id(e) in doneset:
                estack.append(e)
        enx = 0
        while enx < len(estack):
            for e in estack[enx].children:
                if not id(e) in doneset:
                    estack.append(e)
            enx += 1

        # Scan list reversely
        subexpr = []  # Result list of expressions
        while estack:
            e = estack.pop()
            eid = id(e)
            if not eid in doneset:  # Test again is mandatory because same sub-expr can be used twice
                if e.name:
                   subexpr.append(e)
                   doneset.add(eid)
                elif e.reference_count > 1:
                    if not e.name:
                        e.name = e._generate_name()
                    subexpr.append(e)
                    doneset.add(eid)

        return subexpr


    def _expand_all_expressions(self):
        """ Expand model expressions and sort them in variables, expressions and phases.

        Sub-expressions are named expressions. Cause may be:
         * used multiple times,
         * explicitly named by end-user

        Result is a tuple (list_vars, list_exprs, list_phases) where each element is a tuple
        (expre, loc, sub_exprs)
        """
        # Initialize result
        list_vars = []
        list_exprs = []
        list_phases = []

        # Inirialize set of expressions already expanded
        doneset = set()

        # Expand expressions
        for expr, loc in self.model.get_all_expressions():
            lsexpr = self._expand_expression(expr, doneset)
            nlsexpr = []
            # Process variables in children
            for v in lsexpr:
                xtyp = v.type
                if xtyp in (Type_IntVar, Type_IntervalVar):
                    self.model._add_named_expr(v)
                    list_vars.append((v, None, (), False))
                elif xtyp == Type_StepFunction:
                    list_vars.append((v, None, (), False))
                else:
                    nlsexpr.append(v)
            # Add to list of expressions
            if expr.type in (Type_IntVar, Type_IntervalVar):
                self.model._add_named_expr(expr)
                list_vars.append((expr, None, (), id(expr) in doneset))
            else:
                list_exprs.append((expr, loc, nlsexpr, id(expr) in doneset))
            doneset.add(id(expr))

        # Expand phases
        for expr, loc in self.model.get_search_phases():
            lsexpr = self._expand_expression(expr, doneset)
            nlsexpr = []
            # Process variables in children
            for v in lsexpr:
                xtyp = v.type
                if xtyp in (Type_IntVar, Type_IntervalVar, Type_StepFunction):
                    self.model._add_named_expr(v)
                    list_vars.append((v, None, (), False))
                elif xtyp == Type_StepFunction:
                    list_vars.append((v, None, (), False))
                else:
                    nlsexpr.append(v)
            # Add to list of phases
            list_phases.append((expr, loc, nlsexpr, id(expr) in doneset))
            doneset.add(id(expr))

        # Return final result
        return (list_vars, list_exprs, list_phases)


###############################################################################
## Public functions
###############################################################################

def get_cpo_model(model, **kwargs):
    """ Convert a model into a string with CPO file format.

    Args:
        model:  Source model
    Optional args:
        context:             Global solving context. If not given, context is the default context that is set in config.py.
        params:              Solving parameters (CpoParameters) that overwrites those in solving context
        add_source_location: Add source location into generated text
        length_for_alias:    Minimum name length to use shorter alias instead
        (others):            All other context parameters that can be changed
    Returns:
        String of the model in CPO file format
    """
    cplr = CpoCompiler(model, **kwargs)
    return cplr.get_as_string()


###############################################################################
## Private functions
###############################################################################

_NUMBER_CONSTANTS = {INT_MIN: "intmin", INT_MAX: "intmax",
                     INTERVAL_MIN: "intervalmin", INTERVAL_MAX: "intervalmax"}

def _number_value_string(val):
    """ Build the string representing a number value

    This methods checks for special values INT_MIN, INT_MAX, INTERVAL_MIN and INTERVAL_MAX.

    Args:
        val: Integer value
    Returns:
        String representation of the value
    """
    try:
        s = _NUMBER_CONSTANTS.get(val)
    except:
        # Case where value is not hashable, like numpy.ndarray that can be the value type
        # when numpy operand appears in the left of an overloaded operator.
        s = None
    return s if s else str(val)


def _int_var_value_string(ibv):
    """ Build the string representing an integer variable domain value

    This methods checks for special values INT_MIN and INT_MAX.

    Args:
        ibv: Integer value value
    Returns:
        String representation of the value
    """
    if (ibv == INT_MIN):
        return ("intmin")
    elif (ibv == INT_MAX):
        return ("intmax")
    else:
        return str(ibv)


def _int_var_domain_string(intv):
    """ Build the string representing an interval domain

    Args:
       intv: Domain interval (list or tuple of 2 integers)
    Returns:
        String representation of the interval
    """
    return _int_var_value_string(intv[0]) + ".." + _int_var_value_string(intv[1])


def _interval_var_value_string(ibv):
    """ Build the string representing an interval variable domain value

    This methods checks for special values INTERVAL_MIN and INTERVAL_MAX.

    Args:
        ibv: Interval value
    Returns:
        String representation of the value
    """
    if (ibv == INTERVAL_MIN):
        return ("intervalmin")
    elif (ibv == INTERVAL_MAX):
        return ("intervalmax")
    else:
        return str(ibv)


def _interval_var_domain_string(intv):
    """ Build the string representing an interval_var domain

    Args:
        intv: Domain interval
    Returns:
        String representation of the domain
    """
    smn = intv[0]
    smx = intv[1]
    if (smn == smx):
        return _interval_var_value_string(smn)
    return _interval_var_value_string(smn) + ".." + _interval_var_value_string(smx)

