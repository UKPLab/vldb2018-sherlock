# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

from docplex.mp.linear import Var
from docplex.mp.basic import Expr
from docplex.mp.operand import Operand

from docplex.mp.utils import is_number, is_string, is_function, str_holo


class KPI(object):
    """ Abstract class for key performance indicators (KPIs).

    Each KPI has a unique name. A KPI is attached to a model instance and can compute a numerical value,
    using the :func:`compute` method.

    Some KPIs require a valid solution of the model, while others do not. Use :func:`requires_solution` to
    check whether a given KPI requires a solution.
    """

    def __init__(self):
        pass

    def get_name(self):
        """
        Returns:
            string: The published name of the KPI, a non-empty, unique string.
        """
        raise NotImplementedError  # pragma: no cover

    def set_name(self, new_name):
        pass  # pragma: no cover

    def get_model(self):
        """
        Returns:
           The model instance on which the KPI is defined.
        :rtype: :class:`docplex.mp.model.Model`
        """
        raise NotImplementedError   # pragma: no cover

    def compute(self, s=None):
        raise NotImplementedError   # pragma: no cover

    def _get_solution_value(self, s=None):  # pragma: no cover
        return self.compute(s)

    @property
    def solution_value(self):
        return self._get_solution_value()

    def _check_name(self, name_arg):
        if not is_string(name_arg) or not name_arg:
            self.get_model().fatal("KPI.set_name() expects a non-empty string, got: {0!r}", name_arg)

    def requires_solution(self):
        """ KPIs based on decision expressions or variables require a successful solution
        to be computed.

        Returns:
           Boolean: True if the KPI requires a valid solution.
        """
        raise NotImplementedError   # pragma: no cover

    def copy(self, new_model, var_map):
        raise NotImplementedError   # pragma: no cover

    def clone(self):
        raise NotImplementedError  # pragma: no cover

    @staticmethod
    def new_kpi(model, kpi_arg, kpi_name):
        # static factory method to build a new concrete instance of KPI

        if isinstance(kpi_arg, Expr):
            return DecisionKPI(kpi_arg, kpi_name)
        elif isinstance(kpi_arg, Var):
            return DecisionKPI(kpi_arg, kpi_name)
        elif isinstance(kpi_arg, KPI):
            if not kpi_name:
                return kpi_arg
            else:
                cloned = kpi_arg.clone()
                cloned.name = kpi_name
                return cloned
        elif is_function(kpi_arg):
            return FunctionalKPI(kpi_arg, model, kpi_name)
        elif is_number(kpi_arg):
            kpi_fn = lambda _: kpi_arg
            return FunctionalKPI(kpi_fn, model, kpi_name)
        else:
            model.fatal("Cannot interpret this as a KPI: {0!r}. expecting expression, variable or function", kpi_arg)

    def notify_removed(self):
        pass

class DecisionKPI(KPI):
    """ Specialized class of Key Performance Indicator, based on expressions.

    This subclass is built from a decision variable or a linear expression.
    The :func:`compute` method returns the solution value after a successful solve.

    """
    def __init__(self, kpi_op, name=None):
        KPI.__init__(self)
        if is_number(kpi_op):
            self._expr = self.get_model().linear_expr(arg=kpi_op)
            self._name = name
        elif not isinstance(kpi_op, Operand):
            self.get_model().fatal('cannot interpret this as kpi: {0!r}', kpi_op)
        else:
            self._expr = kpi_op
            self._expr.notify_used(self)  # kpi is a subscriber
            self._name = name or kpi_op.name
        self._check_name(self._name)


    def notify_expr_modified(self, expr, event):
        # do nothing
        pass

    def notify_removed(self):
        self._expr.notify_unsubscribed(self)

    def get_name(self):
        return self._name

    def set_name(self, new_name):
        self._check_name(new_name)
        self._name = new_name

    name = property(get_name, set_name)

    def get_model(self):
        return self._expr.model

    def compute(self, s=None):
        """ Redefinition of the abstract `compute()` method.

        Returns:
            float: The value of the decision expression at the solution.

        Raises:
            Evaluating this KPI raises an exception if the underlying model has not been solved successfully.
        """
        return self._expr._get_solution_value(s)

    def requires_solution(self):
        return True

    def to_expr(self):
        return self._expr

    as_expression = to_expr

    def to_linear_expr(self):
        return self._expr.to_linear_expr()

    def copy(self, new_model, var_map):
        expr_copy = self._expr.copy(new_model, var_map)
        return DecisionKPI(kpi_op=expr_copy, name=self.name)

    def clone(self):
        return DecisionKPI(self._expr, self._name)

    def __repr__(self):
        return "{0}(name={1},expr={2!s})".format(self.__class__.__name__, self.name, str_holo(self._expr, maxlen=64))


class FunctionalKPI(KPI):
    # Functional KPIs store a function that takes a model to compute a number
    # Functional KPIs do not require a successful solve.

    def __init__(self, fn, model, name):
        KPI.__init__(self)
        self._name = name
        self._function = fn
        self._model = model
        self._check_name(self._name)

    def get_name(self):
        return self._name

    def set_name(self, new_name):
        self._name = new_name

    name = property(get_name, set_name)

    def get_model(self):
        return self._model

    def compute(self, s=None):
        return self._function(self._model, s)

    def requires_solution(self):
        return False

    def copy(self, new_model, var_map):
        return FunctionalKPI(fn=self._function, model=self._model, name=self._name)

    def clone(self):
        return FunctionalKPI(fn=self._function, model=self._model, name=self._name)
