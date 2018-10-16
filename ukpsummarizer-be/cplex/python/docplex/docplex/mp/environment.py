# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

import platform
import os
import sys


def env_is_64_bit():
    return sys.maxsize > 2**32


# noinspection PyPep8
class Environment(object):
    """ This class detects and contains information regarding other modules of interest, such as
        whether CPLEX, `numpy`, and `matplotlib` are installed.
    """
    _default_env = None  # The default env singleton

    """ This class detects and contains information regarding other modules of interest, such as
        whether CPLEX, `numpy`, and `matplotlib` are installed.
    """
    def __init__(self, start_auto_configure=True):
        """
        __init__(self)
        """
        self._found_cplex = False
        self._cplex_version = ''
        self._cplex_location = None

        self._found_numpy = None
        self._numpy_version = None
        self._numpy_hook = None

        self._found_pandas = None
        self._pandas_version = ''
        self._found_matplotlib = None
        self._matplotlib_version = None

        self._python_version = platform.python_version()
        self._system = platform.system()
        self._bitness = platform.architecture()[0]
        self._is64bit = sys.maxsize > 2**32

        if start_auto_configure:
            self.auto_configure()

    # class variable
    env_is_python36 = platform.python_version() >= '3.6'

    def _get_numpy_hook(self):
        return self._numpy_hook

    def _set_numpy_hook(self, hook):
        self._numpy_hook = hook
        if hook is not None:
            if self.has_numpy: # now that we have set a hook, do check for numpy
                hook()         # if numpy is present, do call the hook

    numpy_hook = property(_get_numpy_hook,  _set_numpy_hook)


    def equals(self, other):
        if type(other) != Environment:
            return False
        if self.has_cplex != other.has_cplex:
            return False
        if self.cplex_version != other.cplex_version:
            return False

        if self.has_numpy != other.has_numpy:
            return False
        if self.has_matplotlib != other.has_matplotlib:
            return False
        if self.has_pandas != other.has_pandas:
            return False

        return True


    @property
    def has_cplex(self):
        """True if the CPLEX libraries are available.
        """
        return self._found_cplex


    def hash_cplex_with_version_min(self, min_version):
        return self._found_cplex and self._cplex_version >= min_version

    @property
    def cplex_version(self):
        return self._cplex_version

    @property
    def has_matplotlib(self):
        """True if the `matplotlib` libraries are available.
        """
        if self._found_matplotlib is None:
            self.__check_matplotlib()
        return self._found_matplotlib

    @property
    def has_pandas(self):
        """True if the `pandas` libraries are available.
        """
        self.__check_pandas()
        return self._found_pandas

    @property
    def pandas_version(self):
        self.__check_pandas()
        return self._pandas_version

    @property
    def cplex_location(self):
        """The system path where CPLEX is located, if present. Otherwise, returns None.
        """
        return self._cplex_location

    @property
    def has_numpy(self):
        """True if the `numpy` libraries are available.
        """
        self._check_numpy()
        return self._found_numpy

    def is_64bit(self):
        """True if running on a 64-bit platform.
        """
        return self._is64bit

    def auto_configure(self):
        self._check_cplex()
        # do not check for numpy, done lazily or on setting a hook

    def _check_cplex(self):
        # detecting CPLEX
        try:
            import cplex
            self._found_cplex = True
            cpx = cplex.Cplex()
            # format: MM.mm.rr.ff e.g.e 12.6.2.0
            self._cplex_version = cpx.get_version()
            cplex_module_file  = cplex.__file__
            if cplex_module_file:
                self._cplex_location = os.path.dirname(os.path.dirname(cplex_module_file))
            del cpx
        except ImportError:
            self._found_cplex = False

    def _check_numpy(self):
        if self._found_numpy is None:
            try:
                import numpy.version as npv
                self._found_numpy = True
                self._numpy_version = npv.version

                self_numpy_hook = self._numpy_hook
                if self_numpy_hook is not None:
                    # lazy call the hook once at first check time.
                    self_numpy_hook()

            except ImportError:
                self._found_numpy = False

        return self._found_numpy

    def __check_matplotlib(self):
        try:
            from matplotlib import __version__ as matplotlib_version
            self._found_matplotlib = True
            self._matplotlib_version = matplotlib_version
        except ImportError:
            self._found_matplotlib = False


    def __check_pandas(self):
        if self._found_pandas is None:
            try:
                import pandas
                self._found_pandas = True
                self._pandas_version = pandas.__version__
            except ImportError:
                self._found_pandas = False

    @staticmethod
    def _display_feature(is_present, feature_name, feature_version, location=None):
        safe_feature_version = feature_version or "?"
        if is_present is None:
            pass  # we dont know yet
        elif is_present:
            if location:
                print("* {0} is present, version is {1}, located at: {2}".format(feature_name, safe_feature_version, location))
            else:
                print("* {0} is present, version is {1}".format(feature_name, safe_feature_version))
        else:
            print("* {0} is not available".format(feature_name))

    @property
    def max_nb_digits(self):
        # source: https://en.wikipedia.org/wiki/IEEE_floating_point
        return 17 if self.is_64bit() else 9

    @property
    def bitness(self):
        return 64 if self.is_64bit() else 32

    def print_information(self):
        print("* system is: {0} {1}".format(self._system, self._bitness))
        from sys import version_info
        from docplex.mp import __version_info__
        self._display_feature(True, "Python", str(version_info[0])+"."+str(version_info[1])+"."+str(version_info[2]))
        self._display_feature(True, "docplex", __version_info__)
        self._display_feature(self._found_cplex, "CPLEX wrapper", self._cplex_version, self._cplex_location)
        self._display_feature(self._found_numpy, "Numpy", self._numpy_version)
        self._display_feature(self._found_matplotlib, "Matplotlib", self._matplotlib_version)


    @staticmethod
    def closed_env():
        return Environment(start_auto_configure=False)

    @staticmethod
    def make_new_configured_env():
        # returns a fresh new environment
        return Environment(start_auto_configure=True)

    @staticmethod
    def get_default_env():
        if not Environment._default_env:
            Environment._default_env = Environment.make_new_configured_env()
        return Environment._default_env


    # for pickling: recreate a fresh environment at the other end of pickle.
    def __reduce__(self):
        return Environment.make_new_configured_env, ()


def get_closed_environment():
    # This instance assumes nothing is found, CPLEX, numpy, etc, to be used for tests
    env = Environment(start_auto_configure=False)
    # force matplotlib absent
    env._found_matplotlib = False
    env._found_numpy = False
    env._found_pandas = False
    return env
