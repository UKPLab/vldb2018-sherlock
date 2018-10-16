# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------
# Author: Olivier OUDOT, IBM Analytics, France Lab, Sophia-Antipolis

"""
Miscellaneous utility functions. Some of theme are here to prevent possible
port problems between the different versions of Python.
"""

import os
import time
import logging
import sys
import threading
import io
import inspect
from collections import deque
import json

try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO


###############################################################################
## Constants
###############################################################################

# Python 2 indicator
IS_PYTHON_2 = (sys.version_info[0] == 2)

# Check if numpy is available
try:
    import numpy
    IS_NUMPY_AVAILABLE = True
    NUMPY_NDARRAY = numpy.ndarray
except:
    IS_NUMPY_AVAILABLE = False
    NUMPY_NDARRAY = False

# Check if panda is available
try:
    import pandas
    from pandas import Series as PandaSeries
    IS_PANDA_AVAILABLE = True
except:
    IS_PANDA_AVAILABLE = False

# Indicator that program is running inside a notebook
IS_IN_NOTEBOOK = 'ipykernel' in sys.modules

# Constant used to indicate to set a parameter to its default value
# Useful if default value is not static
DEFAULT = "default"

# Infinity
POSITIVE_INFINITY = float('inf')
NEGATIVE_INFINITY = float('-inf')

###############################################################################
## Public classes
###############################################################################

class CpoException(Exception):
    """ Exception thrown in case of CPO errors
    """
    def __init__(self, msg):
        """ Create a new exception

        Args:
            msg: Error message
        """
        super(Exception, self).__init__(msg)


class CpoNotSupportedException(CpoException):
    """ Exception thrown when a CPO function is not supported.
    """
    def __init__(self, msg):
        """ Create a new exception

        Args:
            msg: Error message
        """
        super(CpoException, self).__init__(msg)


class Context(dict):
    """ Class handling miscellaneous list of parameters """
    def __init__(self, **kwargs):
        """ Create a new context

        Args:
            List of key=value to initialize context with.
        """
        super(Context, self).__init__()
        vars(self)['parent'] = None
        for k, v in kwargs.items():
            self.set_attribute(k, v)


    def __setattr__(self, name, value):
        """ Set a context parameter.

        Args:
            name:  Parameter name
            value: Parameter value
        """
        self.set_attribute(name, value)


    def __getattr__(self, name):
        """ Get a context parameter.

        Args:
            name:  Parameter name
        Return:
            Parameter value, None if not set
        """
        return self.get_attribute(name)


    def set_attribute(self, name, value):
        """ Set a context attribute.

        Args:
            name:  Attribute name
            value: Attribute value
        """
        self[name] = value
        if isinstance(value, Context):
            vars(value)['parent'] = self


    def get_attribute(self, name, default=None):
        """ Get a context attribute.

        This method search first attribute in this context. If not found, it moves up to
        parent context, and continues as long as not found or root is reached.

        Args:
            name:    Attribute name
            default: Optional, default value if attribute is absent
        Return:
            Attribute value, default value if not found
        """
        if name.startswith('__'):
            raise AttributeError
        ctx = self
        while True:
            if name in ctx:
               return(ctx.get(name))
            ctx = ctx.get_parent()
            if ctx is None:
                return default


    def del_attribute(self, name):
        """ Remove a context attribute.

        Args:
            name:    Attribute name
        Return:
            True if attribute has been removed, False if it was not present
        """
        if name in self:
            value = self.get(name)
            if isinstance(value, Context):
                vars(value)['parent'] = None
            del(self[name])
            return True
        else:
            return False


    def get_by_path(self, path, default=None):
        """ Get a context attribute using its path.

        Attribute path is a sequence of attribute names separated by dots.

        Args:
            path:    Attribute path
            default: Optional, default value if attribute is not found
        Return:
            Attribute value, default value if not found
        """
        res = self
        for k in path.split('.'):
            if k:
                res = res.get_attribute(k)
                if res is None:
                    return default
        return res


    def search_and_replace_attribute(self, name, value, path=""):
        """ Replace an existing attribute.

        The attribute is searched first as a value in this context node.
        If not found, it is searched recursively in children contexts, in alphabetical order.

        Args:
            name:  Attribute name
            value: Attribute value, None to remove attribute
            path:  Path of the current node. default is ''
        Return:
            Full path of the attribute that has been found and replaced, None if not found
        """
        sctxs = []  # List of subcontexts
        # Search first in atomic values
        for k in sorted(self.keys()):
            v = self[k]
            if isinstance(v, Context):
                sctxs.append((k, v))
            else:
                npath = path + "." + k
                if k == name:
                    ov = self.get_attribute(name)
                    if (ov is not None):
                        if isinstance(value, Context):
                            if not isinstance(ov, Context):
                                raise Exception("Attribute '" + npath + "' is a Context and can only be replaced by a Context")
                    self.set_attribute(name, value)
                    return npath
        # Search then in sub-contexts
        for (k, v) in sctxs:
            apth = v.search_and_replace_attribute(name, value, path=npath)
            if apth:
                return apth
        return None


    def get_parent(self):
        """ Get the parent context.

        Each time a context attribute is set to a context, its parent is assigned to the context where it is stored.

        Return:
            Parent context, None if this context is root
        """
        return vars(self)['parent']


    def get_root(self):
        """ Get the root context (last parent with no parent).

        Return:
            Root context
        """
        res = self
        pp = res.get_parent()
        while pp is not None:
            res = pp
            pp = pp.get_parent()
        return res


    def clone(self):
        """ Clone this context and all sub-contexts recursively.

        Return:
            Cloned copy of this context.
        """
        res = type(self)()
        for k, v in self.items():
            if isinstance(v, Context):
                v = v.clone()
            res.set_attribute(k, v)
        return res


    def add(self, ctx):
        """ Add another context to this one.

        All attributes of given context are set in this one, replacing previous value if any.
        If one value is another context, it is cloned before being set.

        Args:
            ctx:  Other context to add to this one.
        """
        for k, v in ctx.items():
            if isinstance(v, Context):
                v = v.clone()
            self.set_attribute(k, v)


    def is_log_enabled(self, vrb):
        """ Check if log is enabled for a given verbosity

        This method get this context 'log_output' attribute to retrieve the log output, and the
        attribute 'verbose' to retrieve the current verbosity level.

        Args:
            vrb:  Required verbosity level, None for always
        """
        return self.log_output and ((vrb is None) or (self.verbose and (self.verbose >= vrb)))


    def get_log_output(self):
        """ Get this context log output

        This method returns the log_output defined in attribute 'log_output' and convert
        it to sys.stdout if its value is True

        Returns:
            Log output stream, None if none
        """
        out = self.log_output
        return sys.stdout if out == True else out


    def log(self, vrb, *msg):
        """ Log a message if log is enabled with enough verbosity

        This method get this context 'log_output' attribute to retrieve the log output, and the
        attribute 'verbose' to retrieve the current verbosity level.

        Args:
            vrb:  Required verbosity level, None for always
            msg:  Message elements to log (concatenated on one line)
        """
        if self.is_log_enabled(vrb):
            out = self.get_log_output()
            prfx = self.log_prefix
            if prfx:
                out.write(str(prfx))
            out.write(''.join([str(m) for m in msg]) + "\n")
            out.flush()


    def print_context(self, out=None, indent=""):
        """ Print this context.

        At each level, atomic values are printed first, then sub-contexts, in alphabetical order.

        Args:
            out:    Print output. stdout by default.
            indent: Start line indentation. Default is empty
        """
        if out is None:
            out = sys.stdout
        sctxs = []  # List of subcontexts
        # Print atomic values
        for k in sorted(self.keys()):
            v = self[k]
            if isinstance(v, Context):
                sctxs.append((k, v))
            else:
                if is_string(v):
                    # Check if value must be masked
                    if (k in ("key", "secret")):
                        v = "**********" + v[-4:]
                    vstr = '"' + v + '"'
                else:
                    vstr = str(v)
                out.write(indent + str(k) + " = " + vstr + "\n")
        # Print sub-contexts
        for (k, v) in sctxs:
            out.write(indent + str(k) + ' =\n')
            v.print_context(out, indent + " : ")
        out.flush()


class IdAllocator(object):
    """ Allocator of identifiers

    This implementation is not thread-safe.
    Use SafeIdAllocator for a usage in a multi-thread environment.
    """
    __slots__ = ('prefix',  # Id prefix
                 'count',   # Allocated id count
                 'bdgts',   # Count printing base digits
                 )
    DIGITS = "0123456789"
    LETTERS_AND_DIGITS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    def __init__(self, prefix, bdgts="0123456789"):
        """ Create a new id allocator

        Args:
            prefix:  Prefix of all ids
            bdgts:   List of digit characters to be use for counter conversion
        """
        super(IdAllocator, self).__init__()
        self.prefix = prefix
        self.count = 0
        self.bdgts = bdgts

    def get_prefix(self):
        """ Get the name prefix used by this allocator.

        Returns:
            Name prefix
        """
        return self.prefix

    def get_count(self):
        """ Get the number of id that has been allocated by this allocator.

        Returns:
            Number of id that has been allocated by this allocator.
        """
        return self.count

    def allocate(self):
        """ Allocate a new id

        Returns:
            Next id for this allocator
        """
        self.count += 1
        cnt = self.count
        res = []
        bdgts = self.bdgts
        blen = len(bdgts)
        while cnt > 0:
           res.append(bdgts[cnt % blen])
           cnt //= blen
        res.reverse()
        return(self.prefix + ''.join(res))


class SafeIdAllocator(object):
    """ Allocator of identifiers

    This implementation uses a lock to protect the increment of the counter,
    allowing to use as shared between multiple threads.
    """
    __slots__ = ('prefix',  # Id prefix
                 'count',   # Allocated id count
                 'bdgts',   # Count printing base digits
                 'lock',    # Lock to protect counter
                 )
    def __init__(self, prefix, bdgts="0123456789"):
        """ Create a new id allocator

        Args:
            prefix:  Prefix of all ids
            bdgts:   List of digit characters to be use for counter conversion
        """
        super(SafeIdAllocator, self).__init__()
        self.prefix = prefix
        self.count = 0
        self.bdgts = bdgts
        self.lock = threading.Lock()

    def get_prefix(self):
        """ Get the name prefix used by this allocator.

        Returns:
            Name prefix
        """
        return self.prefix

    def get_count(self):
        """ Get the number of id that has been allocated by this allocator.

        Returns:
            Number of id that has been allocated by this allocator.
        """
        return self.count

    def allocate(self):
        """ Allocate a new id

        Returns:
            Next id for this allocator
        """
        with self.lock:
            self.count += 1
            cnt = self.count
        res = []
        bdgts = self.bdgts
        blen = len(bdgts)
        while cnt > 0:
           res.append(bdgts[cnt % blen])
           cnt //= blen
        res.reverse()
        return(self.prefix + ''.join(res))


class KeyIdDict(object):
    """ Dictionary using id of the key objects as key.

    This object allows to use any Python object as key (with no __hash__() function),
    and to map a value on the physical instance of the key.
    """
    __slots__ = ('kdict',  # Dictionary of objects
                 )

    def __init__(self):
        super(KeyIdDict, self).__init__()
        self.kdict = {}

    def set(self, key, value):
        """ Set a value in the dictionary

        Args:
            key:   Key
            value: Value
        """
        kid = id(key)
        # Store value and original key, to not garbage it and preserve its id
        self.kdict[kid] = (key, value)

    def get(self, key, default=None):
        """ Get a value from the dictionary

        Args:
            key:     Key
            default: Default value if not found. Default is None.
        Returns:
            Value corresponding to the key, default value (None) if not found
        """
        kid = id(key)
        v = self.kdict.get(kid, None)
        return default if v is None else v[1]

    def keys(self):
        """ Get the list of all keys """
        return [k for (k, v) in self.kdict.values()]

    def values(self):
        """ Get the list of all values """
        return [v for (k, v) in self.kdict.values()]

    def clear(self):
        """ Clear all dictionary content """
        self.kdict.clear()

    def __len__(self):
        """ Returns the number of elements in this dictionary """
        return len(self.kdict)


class ObjectCache(object):
    """ Limited size object cache.

    This object allows to associate an object to a key.
    This cache is limited in size. This means that, if the max size is reached, adding a new
    object removes the oldest.
    """
    __slots__ = ('obj_dict',  # Dictionary of objects
                 'max_size',  # Max cache size
                 'key_list',  # Ordered list of objects keys in the cache
                 )

    def __init__(self, maxsize):
        super(ObjectCache, self).__init__()
        self.obj_dict = {}
        self.max_size = maxsize
        self.key_list = deque()

    def set(self, key, value):
        """ Set a value in the cache

        Args:
            key:   Key
            value: Value
        """
        # Check if key already in cache
        if key in self.obj_dict:
            # Just replace the value
            self.obj_dict[key] = value
        else:
            # Remove older object if max size is reached
            if len(self.key_list) == self.max_size:
                self.obj_dict.pop(self.key_list.popleft())
            # Store new object
            self.obj_dict[key] = value
            self.key_list.append(key)

    def get(self, key):
        """ Get a value from the cache

        Args:
            key:  Key
        Returns:
            Value corresponding to the key, None if not found
        """
        return self.obj_dict.get(key)

    def keys(self):
        """ Get the list of all keys """
        return list(self.key_list)

    def values(self):
        """ Get the list of all values """
        return list(self.obj_dict.values())

    def clear(self):
        """ Clear all dictionary content """
        self.obj_dict.clear()
        self.key_list.clear()

    def __len__(self):
        """ Returns the number of elements in this dictionary """
        return len(self.obj_dict)


class ObjectCacheById(object):
    """ Object cache that uses object id as key.

    This object allows to associate an object to a key.
    It is implemented using a dict that uses the id of the key as dict key.
    This allows to:
     * Use any Python object as key, even if it does not implement __hash__() function,
     * Use different key objects that are logically equal

    This cache is limited in size. This means that, if the max size is reached, adding a new
    object will remove the oldest.
    """
    __slots__ = ('obj_dict',  # Dictionary of objects
                 'max_size',  # Max cache size
                 'key_list',  # Ordered list of objects keys in the cache
                 )

    def __init__(self, maxsize):
        super(ObjectCacheById, self).__init__()
        self.obj_dict = {}
        self.max_size = maxsize
        self.key_list = deque()

    def set(self, key, value):
        """ Set a value in the cache

        Args:
            key:   Key
            value: Value
        """
        kid = id(key)
        # Check if key already in cache
        if kid in self.obj_dict:
            # Just replace the value
            self.obj_dict[kid] = value
        else:
            # Remove older object if max size is reached
            if len(self.key_list) == self.max_size:
                self.obj_dict.pop(id(self.key_list.popleft()))
            # Store new object
            self.obj_dict[kid] = value
            # Append key in deque (side effect is that key is preserved to preserve its id)
            self.key_list.append(key)

    def get(self, key):
        """ Get a value from the cache

        Args:
            key:  Key
        Returns:
            Value corresponding to the key, None if not found
        """
        return self.obj_dict.get(id(key))

    def keys(self):
        """ Get the list of all keys """
        return list(self.key_list)

    def values(self):
        """ Get the list of all values """
        return list(self.obj_dict.values())

    def clear(self):
        """ Clear all dictionary content """
        self.obj_dict.clear()
        self.key_list.clear()

    def __len__(self):
        """ Returns the number of elements in this dictionary """
        return len(self.obj_dict)


class IdentityAccessor(object):
    """ Object implementing a __getitem__ that returns the key as value """
    def __getitem__(self, key):
        return key


class Chrono(object):
    """ Chronometer """
    __slots__ = ('startTime',  # Chrono start time
                 )
    def __init__(self):
        """ Create a new chronometer initialized with current time
        """
        super(Chrono, self).__init__()
        self.restart()

    def get_start(self):
        """ Get the chrono start time

        Returns:
            Time when chronometer has been started
        """
        return self.startTime

    def get_elapsed(self):
        """ Get the chrono elapsed time

        Returns:
            Time spent from chronometer start time (float), in seconds
        """
        return time.time() - self.startTime

    def restart(self):
        """ Restart chrono to current time
        """
        self.startTime = time.time()

    def __str__(self):
        """ Convert this chronometer into a string

        Returns:
            String of the chrono elapsed time
        """
        return str(self.get_elapsed())

class Barrier(object):
    """ Barrier blocking multiple threads

    This class implements a simple barrier with no timeout.
    Implemented here because not available in Python 2
    """
    __slots__ = ('parties',  # Chrono start time
                 'count',    # Number of waiting parties
                 'lock',     # Counters protection lock
                 'barrier'   # Threads blocking lock
                 )
    def __init__(self, parties):
        """ Create a new barrier
        Args:
            parties:  Number of parties required before unlocking the barrier
        """
        self.parties = parties
        self.count = 0
        self.lock = threading.Lock()
        self.barrier = threading.Lock()
        self.barrier.acquire()

    def wait(self):
        """ Wait for the barrier
        This method blocks the calling thread until required number of threads has called this method.
        """
        with self.lock:
            self.count += 1
        if self.count < self.parties:
           self.barrier.acquire()
        self.barrier.release()


class FunctionCache(object):
    """ Object caching the result of a function.
    Future calls with same parameters returns a result stored in a cache dictionary.

    This object is not thread-safe.
    """
    __slots__ = ('values',  # Function returned values. Key is the parameters passed to the function,
                            # value is function result if function has already been called.
                 'lock',    # Lock protecting values dictionary
                 'funct',   # Function whose results is cached.
                 )
    def __init__(self, f):
        """
        Args:
            f:  Function whose results should be cached.
        """
        self.values = {}
        self.funct = f
        self.lock = threading.Lock()

    def get(self, *param):
        """ Get the function result for a given parameter
        Args:
            param:  Function parameters
        Returns:
            Function result
        """
        res = self.values.get(param)
        if res is None:
            # First check if None is the actual result
            if param in self.values:
                return None
            # Call function to get actual result
            res = self.funct(*param)
            # Add result to the cache
            self.values[param] = res
        return res

    def clear(self):
        """ Clear all dictionary content """
        self.values.clear()

    def __len__(self):
        """ Returns the number of elements in this cache """
        return len(self.values)


class InfoDict(dict):
    """ Dictionary of various informations.
    """

    def __init__(self):
        super(dict, self).__init__()


    def incr(self, key, val):
        """ Increment the value of an attribute by a given value

        If the attribute was not set, it is set to the value.
        Args:
            key:  Attribute key
            val:  Value to add
        """
        self[key] = self.get(key, 0) + val


    def clone(self):
        """ Clone this information dictionary

        Returns:
            New information dictionary, cloned copy of this one.
        """
        res = InfoDict()
        res.update(self)
        return res


    def print_infos(self, out=None, indent=""):
        """ Print this information structure.

        Attributes are sorted in alphabetical order

        Args:
            out:    Print output. stdout by default.
            indent: Start line indentation. Default is empty
        """
        if out is None:
            out = sys.stdout

        # Get sorted list of keys
        keys = sorted(list(self.keys()))
        if keys:
            # Print attributes
            mxlen = max(len(k) for k in keys)
            for k in keys:
                out.write(indent + k + (" " * (mxlen - len(k))) + " : " + str((self.get(k))) + "\n")
        else:
            out.write(indent + "none\n")
        out.flush()



###############################################################################
## Public functions
###############################################################################

def check_default(val, default):
    """ Check that an argument value is DEFAULT and returns the default value if so.

    This method has to be used in conjunction with usage of the DEFAULT constant as
    default value of a parameter. It allows to assign a parameter to a default value
    that can be computed dynamically.

    Args:
        val:     Value to check
        default: Default value to return if val is DEFAULT
    Returns:
        val if val is different from DEFAULT, default otherwise
    """
    return default if (val is DEFAULT) else val


def assert_arg_int_interval(val, mn, mx, name=None):
    """ Check that an argument is an integer in a given interval

    Args:
        val:  Argument value
        mn:   Minimal possible value (included)
        mx:   Maximal possible value (excluded)
        name: Name of the parameter (optional), used in raised exception.
    Raises:
      TypeError exception if wrong argument type
    """
    assert is_int(val) and (val >= mn) and (val < mx), \
           "Argument '" + name + "' should be an integer in [" + str(mn) + ".." + str(mx) + ")"


def to_string(val):
    """ Convert a value into a string, recursively for lists and tuples

    Args:
        val: Value to convert value
    Returns:
        String representation of the value
    """
    # Check tuple
    if (isinstance(val, tuple)):
        if (len(val) == 1):
            return "(" + to_string(val[0]) + ",)"
        return "(" + ", ".join(map(to_string, val)) + ")"

    # Check list
    if (isinstance(val, list)):
        return "[" + ", ".join(map(to_string, val)) + "]"

    # Default
    return str(val)


def _get_vars(obj):
    """ Get the list variable names of an object
    """
    # Check if a dictionary is present
    if hasattr(obj, '__dict__'):
        res = getattr(obj, '__dict__').keys()
    # Check if slot is defined
    elif hasattr(obj, '__slots__'):
        slts = getattr(obj, '__slots__')
        if is_array(slts):
            res = list(slts)
        else:
            res = [slts]
        # Go upper in the class hierarchy
        obj = super(obj.__class__, obj)
        while hasattr(obj, '__slots__'):
            slts = getattr(obj, '__slots__')
            if is_array(slts):
                res.extend(slts)
            else:
                res.append(slts)
            obj = super(obj.__class__, obj)
        return res
    # No attributes
    else:
        res = ()
    return sorted(res)


def _equals_lists(l1, l2):
    """ Utility function for equals() to check two lists.
    """
    return (len(l1) == len(l2)) and all(equals(v1, v2) for v1, v2 in zip(l1, l2))


def equals(v1, v2):
    """ Check that two values are logically equal, i.e. with the same attributes with the same values, recursively

    This method does NOT call __eq__ and is then proof to possible overloads of '=='

    Args:
       v1: First value
       v2: Second value
    Returns:
        True if both values are identical, false otherwise
    """
    # Check same object (also covers some primitive types as int, float and strings, but not guarantee)
    if v1 is v2:
        return True

    # Check same type
    t = type(v1)
    if not (t is type(v2)):
        return False

    # Check basic types
    if (t in BASIC_TYPES):
        return (v1 == v2)

    # Check list or tuple
    if isinstance(v1, (list, tuple, bytes, bytearray)):
        return _equals_lists(v1, v2)

    # Check dictionary
    if isinstance(v1, dict):
        # Compare keys
        k1 = sorted(tuple(v1.keys()))
        if not _equals_lists(k1, sorted(tuple(v2.keys()))):
            return False
        # Compare values
        for k in k1:
            if not equals(v1.get(k), v2.get(k)):
                return False
        return True

    # Check sets
    if isinstance(v1, (set, frozenset)):
        # Compare values
        return _equals_lists(sorted(tuple(v1)), sorted(tuple(v2)))

    # Compare object attributes
    dv1 = _get_vars(v1)
    if not _equals_lists(dv1, _get_vars(v2)):
        return False
    for k in dv1:
        if not equals(getattr(v1, k), getattr(v2, k)):
           return False
    return True


def make_directories(path):
    """ Ensure a directory path exists

    Args:
        path: Directory path to check or create
    Raises:
        Any IO exception if directory creation is not possible
    """
    if (path != "") and (not os.path.isdir(path)):
        os.makedirs(path)


def create_stdout_logger(name):
    """ Create a default logger on stdout with default formatter printing time at the beginning
        of the line.

    Args:
        name:  Name of the logger
    """
    logger = logging.getLogger(name)
    logger.setLevel(logging.DEBUG)
    ch = logging.StreamHandler(sys.stdout)
    ch.setFormatter(logging.Formatter('%(asctime)s  %(message)s'))
    logger.addHandler(ch)
    return logger


def get_file_name_only(file):
    """ Get the name of a file, without directory or extension
    Args:
        file:  File name
    Returns:
        Name of the file, without directory or extension
    """
    return os.path.splitext(os.path.basename(file))[0]


def read_string_file(file):
    """ Read a file as a string
    Args:
        file:  File name
    Returns:
        File content as a string
    """
    with open(file, "r") as f:
        str = f.read()
    return str


def write_string_file(file, str):
    """ Write a string into a file.
    Args:
        file:  File name
        str:   String to write
    """
    with open(file, "w") as f:
        f.write(str)


def format_text(txt, size):
    """ Format a given text in multiple lines
    Args:
        txt:  Text to format
        size: Line size
    Returns:
        List of lines.
    """
    res = []
    sepchars = ' \t\n\r'
    txt = txt.strip(sepchars)
    while (len(txt) > size):
        # Search end of line
        enx = size
        while (enx > 0) and (txt[enx] not in sepchars):
            enx -= 1
        # Check no separator in the line
        if (enx == 0):
            enx = size
        # Check for a end of line in the line
        x = txt.find('\n', 0, enx)
        if (x > 0):
            enx = x
        # Append line
        res.append(txt[:enx])
        # Remove line from source
        txt = txt[enx:].strip(sepchars)
    # Add last line
    if txt != "":
        res.append(txt)
        return res


def open_utf8(file, mode='r'):
    """ Open a stream with UTF-8 encoding

    Args:
        file:  File to open
        mode:  Open mode
    """
    encd = 'utf-8-sig' if mode.startswith('r') else 'utf-8'
    return io.open(file, mode=mode, encoding=encd)


def list_module_public_functions(mod, excepted=()):
    """ Build the list of all public functions of a module.

    Args:
        mod:  Module to parse
        excepted:  List of function names to not include. Default is none.
    Returns:
        List of public functions declared in this module
    """
    return [t[1] for t in inspect.getmembers(mod, inspect.isfunction) if not t[0].startswith('_') and inspect.getmodule(t[1]) == mod and not t[0] in excepted]


#-----------------------------------------------------------------------------
# Checking of object types
#-----------------------------------------------------------------------------

# Determine list of types representing the different python scalar types
BOOL_TYPES    = {bool}
INTEGER_TYPES = {int}
FLOAT_TYPES   = {float}
STRING_TYPES  = {str}

# Add Python2 specific unicode if any
if IS_PYTHON_2:
    INTEGER_TYPES.add(long)
    STRING_TYPES.add(unicode)

# Add numpy types if any
if IS_NUMPY_AVAILABLE:
    BOOL_TYPES.add(numpy.bool_)
    BOOL_TYPES.add(numpy.bool)

    INTEGER_TYPES.add(numpy.int_)
    INTEGER_TYPES.add(numpy.intc)
    INTEGER_TYPES.add(numpy.intp)

    INTEGER_TYPES.add(numpy.int8)
    INTEGER_TYPES.add(numpy.int16)
    INTEGER_TYPES.add(numpy.int32)
    INTEGER_TYPES.add(numpy.int64)

    INTEGER_TYPES.add(numpy.uint8)
    INTEGER_TYPES.add(numpy.uint16)
    INTEGER_TYPES.add(numpy.uint32)
    INTEGER_TYPES.add(numpy.uint64)

    FLOAT_TYPES.add(numpy.float_)
    FLOAT_TYPES.add(numpy.float16)
    FLOAT_TYPES.add(numpy.float32)
    FLOAT_TYPES.add(numpy.float64)

# Build all number type sets
INTEGER_TYPES = frozenset(INTEGER_TYPES)
FLOAT_TYPES   = frozenset(FLOAT_TYPES)
BOOL_TYPES    = frozenset(BOOL_TYPES)
NUMBER_TYPES  = frozenset(INTEGER_TYPES.union(FLOAT_TYPES))
STRING_TYPES  = frozenset(STRING_TYPES)
BASIC_TYPES   = frozenset(NUMBER_TYPES.union(BOOL_TYPES).union(STRING_TYPES))


def is_bool(val):
    """ Check if a value is a boolean, including numpy variants if any

    Args:
        val: Value to check
    Returns:
        True if value is a boolean.
    """
    return type(val) in BOOL_TYPES

if IS_NUMPY_AVAILABLE:

    def is_int(val):
        """ Check if a value is an integer, including numpy variants if any

        Args:
            val: Value to check
        Returns:
            True if value is an integer.
        """
        return (type(val) in INTEGER_TYPES) or numpy.issubdtype(type(val), numpy.integer)


    def is_float(val):
        """ Check if a value is a float, including numpy variants if any

        Args:
            val: Value to check
        Returns:
            True if value is a float
        """
        return (type(val) in FLOAT_TYPES) or numpy.issubdtype(type(val), numpy.float)


    def is_number(val):
        """ Check if a value is a number, including numpy variants if any

        Args:
            val: Value to check
        Returns:
            True if value is a number
        """
        return (type(val) in NUMBER_TYPES) or numpy.issubdtype(type(val), numpy.number)


    def is_array(val):
        """ Check if a value is an array (list or tuple)

        Args:
            val: Value to check
        Returns:
            True if value is an array (list or tuple)
        """
        if isinstance(val, (list, tuple)):
            return True
        return isinstance(val, numpy.ndarray) and val.shape

else:

    def is_int(val):
        """ Check if a value is an integer, including numpy variants if any

        Args:
            val: Value to check
        Returns:
            True if value is an integer.
        """
        return type(val) in INTEGER_TYPES


    def is_float(val):
        """ Check if a value is a float, including numpy variants if any

        Args:
            val: Value to check
        Returns:
            True if value is a float
        """
        return type(val) in FLOAT_TYPES


    def is_number(val):
        """ Check if a value is a number, including numpy variants if any

        Args:
            val: Value to check
        Returns:
            True if value is a number
        """
        return type(val) in NUMBER_TYPES


    def is_array(val):
        """ Check if a value is an array (list or tuple)

        Args:
            val: Value to check
        Returns:
            True if value is an array (list or tuple)
        """
        return isinstance(val, (list, tuple))


if IS_PANDA_AVAILABLE:

    def is_panda_series(val):
        """ Check if a value is a panda serie

        Args:
            val: Value to check
        Returns:
            True if value is an panda serie
        """
        return isinstance(val, PandaSeries)

else:

    def is_panda_series(val):
        """ Check if a value is a panda serie

        Args:
            val: Value to check
        Returns:
            False
        """
        return False


def is_string(val):
    """ Check if a value is a string or a variant

    Args:
        val: Value to check
    Returns:
        True if value is a string
    """
    return type(val) in STRING_TYPES


def is_int_array(val):
    """ Check that a value is an array of integers

    Args:
        val: Value to check
    Returns:
        True if value is an array where all elements are integers
    """
    return is_array(val) and (all(is_int(x) for x in val))


def is_array_of_type(val, typ):
    """ Check that a value is an array with all elements instances of a given type

    Args:
        val: Value to check
        typ: Expected element type
    Returns:
        True if value is an array with all elements with expected type
    """
    return is_array(val) and (all(isinstance(x, typ) for x in val))


#-----------------------------------------------------------------------------
# String conversion functions
#-----------------------------------------------------------------------------

# Dictionary of special characters conversion
_FROM_SPECIAL_CHARS = {'n': "\n", 't': "\t", 'r': "\r", 'f': "\f", 'b': "\b", '\\': "\\", '"': "\""}

# Dictionary of special characters conversion
_TO_SPECIAL_CHARS = {'\n': "\\n", '\t': "\\t", '\r': "\\r", '\f': "\\f", '\b': "\\b", '\\': "\\\\", '\"': "\\\""}

# Set of symbol characters
_SYMBOL_CHARS = frozenset(x for x in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_")

# Set of digit characters
_DIGIT_CHARS = frozenset(x for x in "0123456789")


def is_symbol_char(c):
    """ Check whether a character can be used in a symbol

    Args:
        c: Character
    Returns:
        True if character in 0..9, a..z, A..Z, _ or .
    """
    # return ((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z')) or ((c >= '0') and (c <= '9')) or (c == '_')
    # Following is 25% faster
    return (c in _SYMBOL_CHARS)


def to_printable_symbol(id):
    """ Build a printable string from raw string (add escape sequences and quotes if necessary)

    Args:
        id: Identifier string
    Returns:
        Unicode CPO identifier string, including double quotes and escape sequences if needed if not only chars and integers
    """
    # Check empty string
    if len(id) == 0:
        return u'""'
    # Check is string can be used as it is
    if (all((c in _SYMBOL_CHARS) for c in id)) and not id[0] in _DIGIT_CHARS:
        return make_unicode(id)
    # Build result string
    return(u'"' + ''.join(_TO_SPECIAL_CHARS.get(c, c) for c in id) + u'"')


def to_internal_string(strg):
    """ Convert string (with enclosing quotes) into internal string (interpret escape sequences)

    Args:
        strg: String to convert
    Returns:
        Raw string corresponding to source
    """
    res = []
    i = 1
    slen = len(strg) - 1
    while i < slen:
        c = strg[i]
        if (c == '\\'):
            i += 1
            c = _FROM_SPECIAL_CHARS.get(strg[i], None)
            if c is None:
                raise SyntaxError("Unknown special character '\\" + strg[i] + "'")
        res.append(c)
        i += 1
    return u''.join(res)


if IS_PYTHON_2:
    def make_unicode(s):
        """ Convert a string in unicode

        Args:
            s: String to convert
        Returns:
            String in unicode
        """
        return s if type(s) is unicode else unicode(s)
else:
    def make_unicode(s):
        """ Convert a string in unicode

        Args:
            s: String to convert
        Returns:
            String in unicode
        """
        return s


def int_to_base(val, bdgts):
    """ Convert an integer into a string with a given base

    Args:
        val:   Integer value to convert
        bdgts: List of base digits
    Returns:
        String corresponding to the integer
    """
    # Check zero
    if val == 0:
        return bdgts[0]
    # Check negative number
    if (val < 0):
        isneg = True
        val = -val
    else:
        isneg = False
    # Fill list of digits
    res = []
    blen = len(bdgts)
    while val > 0:
        res.append(bdgts[val % blen])
        val //= blen
    # Add negative sign if necessary
    if isneg:
        res.append('-')
    # Return
    res.reverse()
    return ''.join(res)


def search_file_in_path(f):
    """ Search a given file in system path

    Args:
        f:  File name
    Returns:
        First file found with full path, None if not found
    """
    if os.path.isfile(f):
        return f
    for d in os.getenv('PATH').split(os.pathsep):
        nf = os.path.join(d, f)
        if os.path.isfile(nf):
            return nf
    return None


def parse_json_string(jstr):
    """ Parse a JSON string

    Args:
        jstr: String containing JSON document
    Returns:
        Python representation of JSON document
    """
    return json.loads(jstr, parse_constant=True)


#-----------------------------------------------------------------------------
# Zip iterator functions to scan lists simultaneously
#-----------------------------------------------------------------------------

import itertools
if IS_PYTHON_2:
    zip = itertools.izip
    zip_longest = itertools.izip_longest
else:
    # For Python 3.
    zip = zip
    zip_longest = itertools.zip_longest


#-----------------------------------------------------------------------------
# Retrieve builtin functions that are overwritten
#-----------------------------------------------------------------------------

try:
    import __builtin__ as builtin  # Python 2
except ImportError:
    import builtins as builtin     # Python 3

builtin_min   = builtin.min
builtin_max   = builtin.max
builtin_sum   = builtin.sum
builtin_abs   = builtin.abs
builtin_range = builtin.range
builtin_all   = builtin.all
builtin_any   = builtin.any


#-----------------------------------------------------------------------------
# Set warning filter to default (print warnings)
#-----------------------------------------------------------------------------

# import warnings
# warnings.simplefilter("default", DeprecationWarning)
# warnings.simplefilter("default", PendingDeprecationWarning)
