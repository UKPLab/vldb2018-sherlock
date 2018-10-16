# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

# gendoc: ignore
from docplex.mp.utils import is_number


class StaticTypeChecker(object):

    @staticmethod
    def typecheck_as_power(mdl, e, power):
        # INTERNAL: checks <power> is 0,1,2
        if power < 0 or power > 2:
            mdl.fatal("Cannot raise {0!s} to the power {1}. A variable's exponent must be 0, 1 or 2.", e, power)

    @staticmethod
    def cannot_be_used_as_denominator_error(mdl, denominator, numerator):
        mdl.fatal("{1!s} / {0!s} : operation not supported, only numbers can be denominators", denominator, numerator)

    @classmethod
    def typecheck_as_denominator(cls, mdl, denominator, numerator):
        if not is_number(denominator):
            cls.cannot_be_used_as_denominator_error(mdl, denominator, numerator)
        else:
            float_e = float(denominator)
            if 0 == float_e:
                mdl.fatal("Zero divide on {0!s}", numerator)

    @classmethod
    def typecheck_discrete_constraint(cls, logger, ct, msg):
        if not ct.is_discrete():
            logger.fatal('{0}, not {1!s}', msg, ct)
