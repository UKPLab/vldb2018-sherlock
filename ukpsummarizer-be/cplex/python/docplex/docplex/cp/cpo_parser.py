# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------
# Author: Olivier OUDOT, IBM Analytics, France Lab, Sophia-Antipolis

"""
Parser converting a CPO file to internal model representation.
"""

from docplex.cp.cpo_tokenizer import *
from docplex.cp.expression import *
from docplex.cp.expression import _create_operation
from docplex.cp.function import *
from docplex.cp.catalog import *
from docplex.cp.solution import *
from docplex.cp.model import CpoModel
import math


###############################################################################
## Constants
###############################################################################

# Minimum CPO format version number
MIN_CPO_VERSION_NUMBER = "12.6.3.0"

# Map of all operators. Key is operator, value is list of corresponding operation descriptors
_ALL_OPERATORS = {}

# Map of all operations. Key is CPO operation name, value is list corresponding operation descriptor
_ALL_OPERATIONS = {}

# Initialization code
for op in ALL_OPERATIONS:
    if op.priority >= 0:
        _ALL_OPERATORS[op.keyword] = op
    _ALL_OPERATIONS[op.cpo_name] = op
_ALL_OPERATIONS['alldiff'] = Oper_all_diff
_ALL_OPERATIONS['allDiff'] = Oper_all_diff

# Known identifiers
_KNOWN_IDENTIFIERS = {"intmax": INT_MAX, "intmin": INT_MIN,
                      "inf": INFINITY,
                      "intervalmax": INTERVAL_MAX, "intervalmin": INTERVAL_MIN,
                      "no": False}

# Map of array types for each CPO name
_ARRAY_TYPES = {'intArray': Type_IntArray, 'floatArray': Type_FloatArray,
                'intExprArray': Type_IntExprArray, 'floatExprArray': Type_FloatExprArray,
                'intervalVarArray': Type_IntervalVarArray, 'sequenceVarArray': Type_SequenceVarArray,
                '_cumulAtomArray' : Type_CumulAtomArray}

# Fake operator '..' to read intervals properly
_OPER_INTERVAL = CpoOperation("_interval", "_interval", "..", 9, (CpoSignature(Type_IntExprArray, (Type_IntExpr, Type_IntExpr)),))
_ALL_OPERATORS[".."] = _OPER_INTERVAL
#for kwd in _ALL_OPERATORS:
#    print("Operator '{}': {}".format(kwd, _ALL_OPERATORS[kwd]))


###############################################################################
## Public classes
###############################################################################

class CpoParserException(CpoException):
    """ The base class for exceptions raised by the CPO parser
    """
    def __init__(self, msg):
        """ Create a new exception
        Args:
            msg: Error message
        """
        super(CpoParserException, self).__init__(msg)


class CpoUnsupportedFormatVersionException(CpoParserException):
    """ Exception raised when a version mismatch is detected
    """
    def __init__(self, msg):
        """ Create a new exception
        Args:
            msg: Error message
        """
        super(CpoUnsupportedFormatVersionException, self).__init__(msg)



class CpoParser(object):
    """ Reader of CPO file format """
    __slots__ = ('model',          # Read model
                 'source_file',    # Source file
                 'tokenizer',      # Reading tokenizer
                 'token',          # Last read token
                 'pushtoken',      # Pushed token
                 )
    
    def __init__(self, mdl=None):
        """ Create a new CPO format parser

        Args:
            mdl:  Model to fill, None (default) to create a new one.
        """
        super(CpoParser, self).__init__()
        self.model = mdl if mdl is not None else CpoModel()
        self.source_file = None
        self.tokenizer = None
        self.token = None
        self.pushtoken = None

        # Do not store location information (would store parser instead of real lines)
        self.model.source_loc = False

        # TODO: parse and include source information

    def get_model(self):
        """ Get the model that have been parsed

        Return:
            CpoModel result of the parsing
        """
        return self.model
            
    def parse(self, cfile):
        """ Parse a CPO file

        Args:
            cfile: CPO file to read
        Raises:
            CpoParserException: Parsing exception
            CpoVersionMismatchException: Read CPO format is under MIN_CPO_VERSION_NUMBER
        """
        # Store file name if first file
        self.source_file = cfile
        if self.model.source_file is None:
            self.model.source_file = cfile
        with open_utf8(cfile, mode='r') as f:
            self.tokenizer = CpoTokenizer(cfile, f)
            while self._read_statement():
                pass
            self.tokenizer = None

    def parse_string(self, str):
        """ Parse a string

        Args:
            str: String to parse
        """
        self.tokenizer = CpoTokenizer("String", str)
        while self._read_statement():
            pass
        self.tokenizer = None

    def _read_statement(self):
        """ Read a statement or a section

        This functions reads the first token and exits with current token that is
        the last of the statement.

        Returns:
            True if something has been read, False if end of input
        """
        try:
            tok1 = self._next_token()
            if tok1 is TOKEN_EOF:
                return False
            tok2 = self._next_token()
            if tok1 is TOKEN_HASH:
                self._read_directive(tok2.value)
            elif tok2 is TOKEN_EQUAL:
                self._read_assignment(tok1.get_string())
            elif tok2 is TOKEN_SEMICOLON:
                # Get existing expression and re-add it to the model
                expr = self.model.get_expression(tok1.get_string())
                if expr is None:
                    self._raise_exception("Expression '{}' not found in the model".format(tok1.get_string()))
                self.model.add(expr)
            elif tok2 is TOKEN_BRACE_OPEN:
                self._read_section(tok1.value)
            else:
                # Read expression
                self._push_token(tok1)
                expr = self._read_expression()
                # print("Expression read in statement: Type: " + str(expr.type) + ", val=" + str(expr))
                self.model.add(expr)
                self._check_token(self.token, TOKEN_SEMICOLON)
        except Exception as e:
            #if isinstance(e, CpoParserException):
            #    raise e
            import traceback
            traceback.print_exc()
            self._raise_exception(str(e))
        return True

    def _read_directive(self, name):
        """ Read a directive

        Args:
            name:  Directive name
        """
        if name == "include":
            # Get file name
            fname = self._check_token_string(self._next_token())
            if (os.path.dirname(fname) == "") and (self.source_file is not None):
                fname = os.path.dirname(self.source_file) + "/" + fname
            # Push current context
            old_ctx = (self.source_file, self.tokenizer, self.token)
            # Parse file
            self.parse(fname)
            # Restore context
            self.source_file, self.tokenizer, self.token = old_ctx

        elif name == "line":
            # Skip line
            self.tokenizer.get_line_reminder()
        else:
            self._raise_exception("Unknown directive '" + name + "'")
        
    def _read_assignment(self, name):
        """ Read an assignment

        Args:
            name:  Assignment name
        """
        tok = self._next_token()
        if tok.value in ("intVar", "_intVar"):
            v = self._read_int_var(name)
            self.model.add(v)

        elif tok.value == "intervalVar":
            v = self._read_interval_var(name)
            self.model.add(v)

        elif tok.value == "sequenceVar":
            v = self._read_sequence_var(name)
            self.model.add(v)

        elif tok.value == "stateFunction":
            v = self._read_state_function(name)
            self.model.add(v)

        else:
            # Read expression
            expr = self._read_expression()
            self._push_token('')
            # Build CPO expression if needed
            expr = build_cpo_expr(expr)
            # Add expression to model
            expr.set_name(name)
            self.model._add_named_expr(expr)
        self._check_token(self._next_token(), TOKEN_SEMICOLON)
            
    def _read_int_var(self, name):
        """ Read a int_var declaration

        Args:
            name:  Variable name
        Returns:
            CpoIntVar variable expression
        """
        # Read arguments
        self._check_token(self._next_token(), TOKEN_PARENT_OPEN)
        args = self._read_expression_list(TOKEN_PARENT_CLOSE)
        return CpoIntVar(tuple(args), name)
            
    def _read_interval_var(self, name):
        """ Read a interval_var declaration
        Args:
            name:  Variable name
        Returns:
            CpoIntervalVar variable expression
        """
        self._check_token(self._next_token(), TOKEN_PARENT_OPEN)
        self._next_token()
        return self._read_interval_var_params(name, TOKEN_PARENT_CLOSE)

    def _read_interval_var_params(self, name, etok):
        """ Read a interval_var declaration
        Args:
            name:  Variable name
            etok:  Ending token
        Returns:
            CpoIntervalVar variable expression
        """
        res = interval_var(name=name)
        tok = self.token
        while tok is not etok:
            # Read argument name
            self._check_token_string(tok)
            aname = tok.value
            if (aname == "present"):
                res.set_present()
                self._next_token()
            elif (aname == "absent"):
                res.set_absent()
                self._next_token()
            elif (aname == "optional"):
                res.set_optional()
                self._next_token()
            else:
                self._check_token(self._next_token(), TOKEN_EQUAL)
                if (aname in ("start", "end", "length", "size")):
                    # Read interval
                    self._next_token()
                    intv = self._read_expression()
                    if isinstance(intv, int):
                        intv = (intv, intv)
                    elif not isinstance(intv, (list, tuple)):
                        self._raise_exception("'start', 'end', 'length' or 'size' should be an integer or an interval")
                    setattr(res, aname, intv)
                elif (aname == "intensity"):
                    self._next_token()
                    res.set_intensity(self._read_expression())
                elif (aname == "granularity"):
                    tok = self._next_token()
                    self._check_token_integer(tok)
                    res.set_granularity(int(tok.value))
                    self._next_token()
                else:
                    self._raise_exception("Unknown IntervalVar attribute argument '" + aname + "'")
            # Read comma
            tok = self.token
            if tok.value == ',':
                tok = self._next_token()
        return res

    def _read_sequence_var(self, name):
        """ Read a sequence_var declaration
        Args:
            name:  Variable name
        Returns:
            CpoSequenceVar variable expression
        """
        self._check_token(self._next_token(), TOKEN_PARENT_OPEN)
        args = self._read_expression_list(TOKEN_PARENT_CLOSE)
        if (len(args) == 1):
            lvars = args[0]
            ltypes = None
        else:
            if (len(args) != 2):
                self._raise_exception("'sequenceVar' should have 1 or 2 arguments")
            lvars = args[0]
            ltypes = args[1]
        return CpoSequenceVar(lvars, ltypes, name)

    def _read_state_function(self, name):
        """ Read a state function declaration
        Args:
            name:  Variable name
        Returns:
            CpoStateFunction variable expression
        """
        self._check_token(self._next_token(), TOKEN_PARENT_OPEN)
        args = self._read_expression_list(TOKEN_PARENT_CLOSE)
        nbargs = len(args)
        if (nbargs == 0):
            trmx = None
        elif (nbargs == 1):
            trmx = args[0]
        else:
            self._raise_exception("'stateFunction' should have 0 or 1 argument")
        return CpoStateFunction(trmx, name)

    def _read_expression(self):
        """ Read an expression

        First expression token is already read.
        Function exits with current token following the last expression token

        Returns:
            Expression that has been read
        """
        # Read first sub-expression
        expr = self._read_sub_expression()
        tok = self.token
        if tok.type is not TOKEN_OPERATOR:
            return expr

        # Initialize elements stack
        stack = [expr]
        while tok.type is TOKEN_OPERATOR:
            op = self._get_and_check_operator(tok)
            self._next_token()
            expr = self._read_sub_expression()
            tok = self.token

            # Reduce stack if possible
            while (len(stack) > 1) and op.priority >= stack[-2].priority:
                oexpr = stack.pop()
                oop = stack.pop()
                stack[-1] = self._create_operation_expression(oop, (stack[-1], oexpr))

            stack.append(op)
            stack.append(expr)

        # Build final expression
        expr = stack.pop()
        while stack:
            op = stack.pop()
            expr = self._create_operation_expression(op, (stack.pop(), expr))
        return expr

    def _read_sub_expression(self):
        """ Read a sub-expression

        First expression token is already read.
        Function exits with current token following the last expression token
        Return:
            Expression that has been read
        """

        tok = self.token

        # Check int constant
        if (tok.type == TOKEN_INTEGER):
            self._next_token()
            return int(tok.value)
        
        # Check float constant
        if (tok.type == TOKEN_FLOAT):
            self._next_token()
            return float(tok.value)

        # Check known identifier
        if (tok.value in _KNOWN_IDENTIFIERS):
            self._next_token()
            return _KNOWN_IDENTIFIERS[tok.value]

        # Check unary operator
        if tok.type is TOKEN_OPERATOR:
            # Retrieve operation descriptor
            op = self._get_and_check_operator(tok)
            # Read next expression
            self._next_token()
            expr = self._read_sub_expression()
            return self._create_operation_expression(op, (expr,))
        
        # Check symbol
        if (tok.type == TOKEN_SYMBOL):
            ntok = self._next_token()
            if ntok is TOKEN_PARENT_OPEN:
                # Read function arguments
                if tok.value == "transitionMatrix":
                    # Check arguments list to support presolved version
                    tok = self._next_token()
                    self._push_token(tok)
                    if tok.value == "matrixSize":
                        args = self._read_arguments_list(TOKEN_PARENT_CLOSE)
                        for name, mtrx in args:
                            if name == 'matrix':
                                break
                    else:
                        mtrx = self._read_expression_list(TOKEN_PARENT_CLOSE)
                    self._next_token()
                    slen = len(mtrx)
                    size = int(math.sqrt(slen))
                    if size * size != slen:
                        raise CpoParserException("Length of transition matrix values should be a square")
                    return CpoTransitionMatrix(values=(mtrx[i * size : (i+1) * size] for i in range(size)))
                else:
                    args = self._read_expression_list(TOKEN_PARENT_CLOSE)
                    self._next_token()
                    # Check predefined functions
                    if tok.value == "stepFunction":
                        return CpoStepFunction(args)
                    if tok.value == "segmentedFunction":
                        return CpoSegmentedFunction(args[0], args[1:])
                    if tok.value == "sequenceVar":
                        return CpoSequenceVar(*args)
                    # General function call, retrieve operation descriptor
                    opname = tok.value
                    op = _ALL_OPERATIONS.get(opname, None)
                    if op is None:
                        self._raise_exception("Unknown operation '" + str(tok.value) + "'")
                    return self._create_operation_expression(op, args)

            elif ntok is TOKEN_HOOK_OPEN:
                # Read typed array
                expr = self._read_expression_list(TOKEN_HOOK_CLOSE)
                self._next_token()
                # Search type
                typ = _ARRAY_TYPES.get(tok.value)
                if typ is None:
                    self._raise_exception("Unknown array type '" + str(tok.value) + "'")
                # Check empty array
                if len(expr) == 0:
                    return CpoValue((), typ)
                # Compute best type if array not empty
                res = build_cpo_expr(expr)
                if not res.type.is_kind_of(typ):
                    res.type = typ
                return res

            else:
                # Token is an expression id
                return self._get_identifier_value(tok.value)
        
        # Check expression in parenthesis
        if tok is TOKEN_PARENT_OPEN:
            expr = self._read_expression_list(TOKEN_PARENT_CLOSE)
            self._next_token()
            if len(expr) == 1:
                return expr[0]
            return expr

        # Check array with no type
        if tok is TOKEN_HOOK_OPEN:
            expr = self._read_expression_list(TOKEN_HOOK_CLOSE)
            self._next_token()
            return expr
            
        # Check reference to a model expression or variable
        if tok.type is TOKEN_STRING:
            self._next_token()
            return self._get_identifier_value(tok.get_string())
                
        # Unknown expression
        self._raise_exception("Invalid start of expression: '" + str(tok) + "'")
            
    def _read_expression_list(self, etok):
        """ Read a list of expressions

        This method supposes that the list start token is read (for example '(' or '[').
        When returning, current token is list ending token
        Args:
           etok: Expression list ending token string (for example ')' or ']')
        Returns:
            Array of expressions
        """
        lxpr = []
        self._next_token()
        while self.token is not etok:
            lxpr.append(self._read_expression())
            if self.token is TOKEN_COMMA:
                self._next_token()
        return tuple(lxpr)
        
    def _read_arguments_list(self, etok):
        """ Read a list of arguments that are possibly named

        This method supposes that the list start token is read (for example '(' or '[').
        When returning, current token is list ending token
        Args:
           etok: Expression list ending token (for example ')' or ']')
        Returns:
            Array of couples (name, expression)
        """
        lxpr = []
        self._next_token()
        while self.token is not etok:
            if self.token.type is TOKEN_SYMBOL:
                name = self.token
                if self._next_token() is TOKEN_EQUAL:
                    self._next_token()
                else:
                    self._push_token(name)
                    name = None
            else:
                name = None
            lxpr.append((name, self._read_expression()))
            if self.token is TOKEN_COMMA:
                self._next_token()
        return lxpr

    def _read_section(self, name):
        """ Read a section.
        Current token is the opening brace
        Args:
            name:  Section name
        """
        if (name == "parameters"):
            self._read_section_parameters()
        elif (name == "internals"):
            self._read_section_internals()
        elif (name == "search"):
            self._read_section_search()
        elif (name == "startingPoint"):
            self._read_section_starting_point()
        else:
            self._raise_exception("Unknown section '" + name + "'")
            
    def _read_section_parameters(self):
        """ Read a parameters section
        """
        params = CpoParameters()
        tok = self._next_token()
        while not tok.value == '}':
            vname = self._check_token_string(tok)
            self._check_token(self._next_token(), TOKEN_EQUAL)
            value = self._next_token()
            self._check_token(self._next_token(), TOKEN_SEMICOLON)
            params.set_attribute(vname, value.get_string())
            tok = self._next_token()
        if params:
            self.model.set_parameters(params)


    def _read_section_internals(self):
        """ Read a internals section
        """
        # Skip all until section end
        tok = self._next_token()
        while (tok is not TOKEN_EOF) and (tok.value != '}'):
            if self.token.value == "version":
                self._check_token(self._next_token(), TOKEN_PARENT_OPEN)
                ver = self.tokenizer.get_up_to(')')
                self.model.format_version = ver
                # if ver < MIN_CPO_VERSION_NUMBER:
                #     raise CpoUnsupportedFormatVersionException("Can not parse a CPO file with version {}, lower than {}"
                #                                                .format(ver, MIN_CPO_VERSION_NUMBER))
                self._check_token(self._next_token(), TOKEN_PARENT_CLOSE)
            tok = self._next_token()


    def _read_section_search(self):
        """ Read a search section
        """
        # Read statements up to end of section
        tok = self._next_token()
        while (tok is not TOKEN_EOF) and (tok is not TOKEN_BRACE_CLOSE):
            self._push_token(tok)
            self._read_statement()
            tok = self._next_token()


    def _read_section_starting_point(self):
        """ Read a starting point section
        """
        sp = CpoModelSolution()
        # Read statements up to end of section
        tok = self._next_token()
        while (tok is not TOKEN_EOF) and (tok is not TOKEN_BRACE_CLOSE):
            # Get and check the variable that is concerned
            vname = self._check_token_string(tok)
            var = self.model.get_expression(vname)
            if var is None:
                self._raise_exception("There is no variable named '{}' in this model".format(vname))
            self._check_token(self._next_token(), TOKEN_EQUAL)
            tok = self._next_token()
            is_parent = False

            # Process integer variable
            if var.type == Type_IntVar:
                if tok.value == "intVar":
                    tok = self._next_token()
                is_parent = (tok is TOKEN_PARENT_OPEN)
                if is_parent:
                    self._next_token()
                # Read domain
                dom = self._read_expression()
                # Add solution to starting point
                vsol = CpoIntVarSolution(vname, (dom, ))

            elif var.type == Type_IntervalVar:
                if tok.value == "absent":
                    vsol = CpoIntervalVarSolution(vname, presence=False)
                    self._check_token(self._next_token(), TOKEN_SEMICOLON)
                else:
                    if tok.value == "intervalVar":
                        tok = self._next_token()
                    is_parent = (tok is TOKEN_PARENT_OPEN)
                    if is_parent:
                        self._next_token()
                    ivar = self._read_interval_var_params(vname, TOKEN_PARENT_CLOSE if is_parent else TOKEN_SEMICOLON)
                    vsol = CpoIntervalVarSolution(vname,
                                                  presence=True if ivar.is_present() else False if ivar.is_absent() else None,
                                                  start=ivar.get_start() if ivar.get_start() != DEFAULT_INTERVAL else None,
                                                  end=ivar.get_end() if ivar.get_end() != DEFAULT_INTERVAL else None,
                                                  size=ivar.get_size() if ivar.get_size() != DEFAULT_INTERVAL else None)

            else:
                self._raise_exception("The section 'startingPoint' should contain only integer and interval variables.")

            # Add variable solution to starting point
            sp.add_var_solution(vsol)

            # Read end of variable starting point
            if is_parent:
                self._check_token(self.token, TOKEN_PARENT_CLOSE)
                self._next_token()
            self._check_token(self.token, TOKEN_SEMICOLON)
            tok = self._next_token()

        # Add starting point to the model
        self.model.set_starting_point(sp)


    def _check_token(self, tok, etok):
        """ Check that a read token is a given one an raise an exception if not
        Args:
            tok: Read token
            etok: Expected token
        """
        if tok is not etok:
            self._raise_unexpected_token(etok, tok)


    def _get_and_check_operator(self, tok):
        """ Get an operator descriptor and raise an exception if not found
        Args:
            tok:  Operator token
        Returns:
            List of Operation descriptor for this keyword
        Raises:
            CpoException if operator does not exists
        """
        op = _ALL_OPERATORS.get(tok.value)
        if op is None:
            self._raise_exception("Unknown operator '" + str(tok.value) + "'")
        return op


    def _check_token_string(self, tok):
        """ Check that a token is a string and raise an exception if not
        Args:
            tok: Token
        Returns:
            String value of the token            
        """
        if tok.type is TOKEN_SYMBOL:
            return tok.value
        if tok.type is TOKEN_STRING:
            return tok.get_string()
        self._raise_exception("String expected")


    def _check_token_integer(self, tok):
        """ Check that a token is an integer and raise an exception if not
        Args:
            tok: Token
        Returns:
            integer value of the token
        """
        if tok.type is TOKEN_INTEGER:
            return(int(tok.value))
        if tok.value in _KNOWN_IDENTIFIERS:
            return _KNOWN_IDENTIFIERS[tok.value]
        self._raise_exception("Integer expected instead of '" + tok.value + "'")


    def _get_identifier_value(self, eid):
        """ Get an expression associated to an identifier
        Args:
            eid:  Expression identifier
        """
        expr = self.model.get_expression(eid)
        if expr is None:
            self._raise_exception("Unknown identifier '" + str(eid) + "'")
        return(expr)


    def _create_operation_expression(self, op, args):
        """ Create a model operation

        Args:
            op:    Operation descriptor
            args:  Operation arguments
        Returns:
            Model expression
        Raises:
            Cpo exception if error
        """
        # Check interval operator
        if op is _OPER_INTERVAL:
            return tuple(args)
        # Check unary minus on constant value
        if (op is Oper_minus) and (len(args) == 1) and is_number(args[0]):
            return -args[0]
        if (op is Oper_plus) and (len(args) == 1) and is_number(args[0]):
            return args[0]

        try:
            return _create_operation(op, args)
        except Exception as e:
            lastex = Exception("No valid operation found for {}: {}".format(op.cpo_name, e))
            self._raise_exception(str(lastex))


    def _raise_unexpected_token(self, expect=None, tok=None):
        """ Raise a "Unexpected token" exception
        Args:
            tok:  Unexpected token
        """
        if tok is None: 
            tok = self.token
        if expect is None:
            self._raise_exception("Unexpected token '" + str(tok) + "'")
        self._raise_exception("Read '" + str(tok) + "' instead of expected '" + str(expect) + "'")


    def _raise_exception(self, msg):
        """ Raise a Parsing exception
        Args:
            msg:  Exception message
        """
        raise CpoParserException(self.tokenizer.build_error_string(msg))


    def _next_token(self):
        """ Read next token
        Returns:
            Next read token, None if end of input
        """
        # Check if a token has been pushed
        if self.pushtoken is not None:
            tok = self.pushtoken
            self.pushtoken = None
        else:
            tok = self.tokenizer.next_token()
        self.token = tok
        #print("Tok='" + str(tok) + "'")
        return tok


    def _push_token(self, tok):
        """ Push current token
        Args:
            tok: New current token 
        """
        self.pushtoken = self.token
        self.token = tok
        
