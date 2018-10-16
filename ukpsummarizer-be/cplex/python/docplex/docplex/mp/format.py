# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

# gendoc: ignore


class ExchangeFormat(object):
    """ The ExchangeFormat class groups data and attributes of model formats.

        This class is not meant to be instantiated by users.
    """

    def __init__(self, name, extension, is_binary=False):
        assert isinstance(name, str)
        assert isinstance(extension, str)
        self._name = name
        self._extension = extension if extension.startswith(".") else ".%s" % extension
        self._is_binary = is_binary

    @property
    def name(self):
        """ Returns a string qualifying the format to be used in messages.

        Example:
            LP format name is "LP"
        """
        return self._name

    @property
    def is_binary(self):
        """ Returns True if the format is binary, False for a text format.

        Example:
            LP is a text format and returns False.

            SAV is a binary format and returns TRue
        """
        return self._is_binary

    @property
    def extension(self):
        """ Returns the full file extension of the format, including "."

        Example:
            LP format extension is ".lp". SAV format extension is ".sav"
        """
        return self._extension

    def to_string(self):
        return "%s_format" % self.name

    def __str__(self):
        return self.to_string()

    def __repr__(self):
        return self.to_string()

    @staticmethod
    def fromstring(definition):
        if definition:
            definition = definition.lower()
        return _FORMAT_MAPPER.get(definition, None)


""" The global LP format object."""
# noinspection PyPep8
LP_format  = ExchangeFormat("LP", "lp")
SAV_format = ExchangeFormat("SAV", "sav", is_binary=True)
OPL_format = ExchangeFormat("OPL", ".mod")

_FORMAT_MAPPER = {"lp": LP_format,
                  "opl": OPL_format
                  }
