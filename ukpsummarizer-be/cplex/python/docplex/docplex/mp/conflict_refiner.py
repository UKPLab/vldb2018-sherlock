# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

from collections import namedtuple, Iterable
from docplex.mp.constants import ComparisonType
from docplex.mp.context import check_credentials
from docplex.mp.cloudutils import context_must_use_docloud
import warnings


TConflictConstraint = namedtuple("_TConflictConstraint", ["name", "element", "status"])


class VarUbConstraintWrapper(object):
    """
    This class is a wrapper for a model variable and its associated upper bound.

    Instances of this class are created by the ``refine_conflict`` method when the conflict involves
    a variable upper bound. Each of these instances is then referenced by a ``TConflictConstraint`` namedtuple
    in the conflict list returned by ``refine_conflict``.
    
    To check whether the upper bound of a variable causes a conflict, wrap the variable and
    include the resulting constraint in a ConstraintsGroup.
    """
    def __init__(self, var):
        self._var = var

    def get_var(self):
        return self._var

    def get_ub(self):
        return self._var._get_ub()

    def get_index(self):
        return self._var.get_index()

    def get_name(self):
        return self._var.get_name()

    index = property(get_index)
    name = property(get_name)

    def get_constraint(self):
        var_ub = self.get_ub()
        op = ComparisonType.cplex_ctsense_to_python_op('L')
        ct = op(self.get_var(), var_ub)
        return ct


class VarLbConstraintWrapper(object):
    """
    This class is a wrapper for a model variable and its associated lower bound.

    Instances of this class are created by the ``refine_conflict`` method when the conflict involves
    a variable lower bound. Each of these instances is then referenced by a ``TConflictConstraint`` namedtuple
    in the conflict list returned by ``refine_conflict``.

    To check whether the lower bound of a variable causes a conflict, wrap the variable and
    include the resulting constraint in a ConstraintsGroup.
    """
    def __init__(self, var):
        self._var = var

    def get_var(self):
        return self._var

    def get_lb(self):
        return self._var._get_lb()

    def get_index(self):
        return self._var.get_index()

    def get_name(self):
        return self._var.get_name()

    index = property(get_index)
    name = property(get_name)

    def get_constraint(self):
        var_lb = self.get_lb()
        op = ComparisonType.cplex_ctsense_to_python_op('G')
        ct = op(self.get_var(), var_lb)
        return ct


class ConstraintsGroup(object):
    """
    This class is a container for the definition of a group of constraints.
    A preference for conflict refinement is associated to the group.

    Groups may be assigned preference. A group with a higher preference is more likely to be included in the conflict.
    A negative value specifies that the corresponding group should not be considered in the computation
    of a conflict. In other words, such groups are not considered part of the model. Groups with a preference of 0 (zero)
    are always considered to be part of the conflict.

    Args:
        preference: A floating-point number that specifies the preference for the group. The higher the number, the
                    higher the preference.
    """

    def __init__(self, preference=1.0):
        self._preference = preference
        self._cts = []

    def get_preference(self):
        return self._preference

    def set_preference(self, value):
        self._preference = value

    preference = property(get_preference, set_preference)

    def add_constraint(self, ct):
        self._cts.append(ct)

    def add_constraints(self, cts):
        if isinstance(cts, Iterable):
            for ct in cts:
                self.add_constraint(ct)
        else:
            self.add_constraint(cts)

    def get_group_constraints(self):
        return self._cts


class ConflictRefiner(object):
    ''' This class is an abstract algorithm; it operates on interfaces.

    A conflict is a set of mutually contradictory constraints and bounds within a model.
    Given an infeasible model, the conflict refiner can identify conflicting constraints and bounds
    within it. CPLEX refines an infeasible model by examining elements that can be removed from the
    conflict to arrive at a minimal conflict.
    '''

    def __init__(self):
        pass

    def refine_conflict(self, mdl, preferences=None, groups=None, **kwargs):
        """ Starts the conflict refiner on the model.

        If CPLEX is available, the conflict refinement operation will be performed using the native CPLEX.
        If CPLEX is not available, the conflict refinement operation will be started on DOcplexcloud.
        The DOcplexcloud connection parameters are checked in the following order:

            - If ``kwargs`` contains valid ``url`` and ``key`` values, they are used.
            - If ``kwargs`` contains a ``context`` and that context contains a
              valid ``solver.docloud.url`` and ``solver.docloud.key`` values,
              those values are used. Other attributes of ``solver.docloud``
              can also be used. See :class:`docplex.mp.context.Context`.
            - Finally, the model's attribute ``context`` is used. This ``context``
              is set at model creation time.

        If CPLEX is not available and the model has no valid credentials, an error is raised, because there is
        no way to perform the conflict refinement.

        Note that if the ``url`` and ``key`` parameters are present and the
        values of the parameters are not in the ignored url or key list,
        the conflict refinement operation will be started on DOcplexcloud even if CPLEX is
        available.

        Example::

            # forces the conflict refiner on DOcplexcloud with the specified url and keys
            crefiner.refine_conflict(url='https://foo.com', key='bar')

        Example::

            # set some DOcplexcloud credentials, but depend on another
            # method to decide if conflict refiner is local or not
            ctx.solver.docloud.url = 'https://foo.com'
            ctx.solver.docloud.key = 'bar'
            agent = 'local' if method_that_decides_if_solve_is_local() or 'docloud'
            crefiner.conflict_refiner(context=ctx, agent=agent)

        Args:
            mdl: The model to be relaxed.
            preferences: A dictionary defining constraint preferences.
            groups: A list of ConstraintsGroups.
            kwargs: Accepts named arguments similar to solve.

        Returns:
            A list of ``TConflictConstraint`` namedtuples, each tuple corresponding to a constraint that is
            involved in the conflict.
            The fields of the ``TConflictConstraint`` namedtuple are:

                - the name of the constraint or None if the constraint corresponds to a variable lower or upper bound.
                - a reference to the constraint or to a wrapper representing a Var upper or lower bound.
                - a ``docplex.mp.constants.ConflictStatus`` object that indicates the
                  conflict status type (Excluded, Possible_member, Member...).

            This list is empty if no conflict is found by the conflict refiner.
        """

        # take into account local argument overrides
        context = mdl.prepare_actual_context(**kwargs)

        # log stuff
        saved_context_log_output = mdl.context.solver.log_output
        saved_log_output_stream = mdl.get_log_output()

        try:
            mdl.set_log_output(context.solver.log_output)

            forced_docloud = context_must_use_docloud(context, **kwargs)

            have_credentials = False
            if context.solver.docloud:
                have_credentials, error_message = check_credentials(context.solver.docloud)
                if error_message is not None:
                    warnings.warn(error_message, stacklevel=2)
            if forced_docloud:
                if have_credentials:
                    return self._refine_conflict_cloud(mdl, context, preferences, groups)
                else:
                    mdl.fatal("DOcplexcloud context has no valid credentials: {0!s}",
                               context.solver.docloud)
            # from now on docloud_context is None
            elif mdl.environment.has_cplex:
                # if CPLEX is installed go for it
                return self._refine_conflict_local(mdl, context, preferences, groups)
            elif have_credentials:
                # no context passed as argument, no Cplex installed, try model's own context
                return self._refine_conflict_cloud(mdl, context, preferences, groups)
            else:
                # no way to solve.. really
                return mdl.fatal("CPLEX DLL not found: please provide DOcplexcloud credentials")
        finally:
            if saved_log_output_stream != mdl.get_log_output():
                mdl.set_log_output_as_stream(saved_log_output_stream)
            if saved_context_log_output != mdl.context.solver.log_output:
                mdl.context.solver.log_output = saved_context_log_output

    def _refine_conflict_cloud(self, mdl, context, preferences=None, groups=None):
        """ This method handles invocation of the conflict refiner feature by configuring and submitting a job
        to DOcplexcloud and then parsing the result file that is returned.

        :param context:
        :return:
        """
        docloud_context = context.solver.docloud
        parameters = context.cplex_parameters
        # see if we can reuse the local docloud engine if any?
        docloud_engine = mdl._engine_factory.new_docloud_engine(model=mdl,
                                                                docloud_context=docloud_context,
                                                                log_output=context.solver.log_output_as_stream)

        mdl.notify_start_solve()
        mdl._fire_start_solve_listeners()
        conflict = docloud_engine.refine_conflict(mdl, preferences=preferences, groups=groups, parameters=parameters)

        mdl._fire_end_solve_listeners(conflict is not None, None)
        #
        return conflict

    def _refine_conflict_local(self, mdl, context, preferences=None, groups=None):
        """ TODO: add documentation

        :param context:
        :return:
        """
        parameters = context.cplex_parameters
        self_engine = mdl.get_engine()
        return self_engine.refine_conflict(mdl, preferences, groups, parameters)

    @staticmethod
    def display_conflicts(conflicts):
        """
        This method displays a formatted representation of the conflicts that are provided.

        Args:
           conflicts: A list of ``TConflictConstraint`` namedtuples, one that was returned
                      by the `refine_conflict()` method.
        """
        print('Conflict set:')
        for conflict in conflicts:
            st = conflict.status
            ct = conflict.element
            label = type(conflict.element)
            if isinstance(conflict.element, VarLbConstraintWrapper) \
                    or isinstance(conflict.element, VarUbConstraintWrapper):
                ct = conflict.element.get_constraint()

            if conflict.name is None:
                print("\t{} (status code: {}): {}".format(label, st, ct))
            else:
                print("\t{} (status code: {}) - {}: {}".format(label, st, conflict.name, ct))
