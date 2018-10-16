# -------------------------------------------------------------------------
# File: slmtag.py
# -------------------------------------------------------------------------
# Licensed Materials - Property of IBM
# 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5655-Y21
# Copyright IBM Corporation 2014. All Rights Reserved.
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with
# IBM Corp.
# -------------------------------------------------------------------------
"""
Reads a CPLEX Studio ILMT Logging database and writes it to an SLMTag file.

Note: This script is meant to work with Python >= 2.6.  If you want to use
      it with Python 3, you can safely convert it using 2to3.

Usage: python slmtag.py [-c ilmt_config_file.xml]

Required Arguments: NONE

Valid Options:
  -c [--config]   : The location of an ILMT configuration file.  If this option
                    is not given, then an attempt to read it from the
                    CPLEX_STUDIO_ILMT_CONFIG_FILE environment variable will be
                    made.
  -h [--help]     : Print this usage summary.

Example:
  $ python slmtag.py # Read from CPLEX_STUDIO_ILMT_CONFIG_FILE
  or
  $ python slmtag.py -c ilmt_config.xml
"""
from __future__ import print_function

import getopt
import hashlib # NB: Requires Python 2.5!
import os
import sqlite3
import sys
import urllib
import xml.etree.ElementTree as ET # NB: Requires Python 2.5!
from datetime import datetime

__version__ = '1.0.0'

CPX_ILMT_SCHEMA_VERSION = '2.1.1'
CPX_ILMT_SOFTWARE_NAME = \
    'IBM ILOG CPLEX Optimization Studio Developer Edition'

CPX_ILMT_PERSISTENT_ID = '85775252b3224d50adcfb6c1401b506e'
CPX_ILMT_DATE_FORMAT = '%Y-%m-%dT%H:%M:%S%z'
CPX_ILMT_DB_EXT = '.db'
CPX_ILMT_SLMTAG_EXT = '.slmtag'
CPX_ILMT_FILE_PREFIX = 'CPLEX_Studio_'

ILMT_DEFAULT_INSTANCE_ID = 'CPLEX_Studio_1280'
ILMT_DEFAULT_MAX_FILE_SIZE = 1024
ILMT_DEFAULT_LOG_PERIOD = 12
ILMT_DEFAULT_MAX_FILE_COUNT = 10

ILMT_CONFIG_LICENSE_METRIC = 'LicenseMetric'
ILMT_CONFIG_LOCATION = 'Location'
ILMT_CONFIG_INSTANCE_ID = 'InstanceID'
ILMT_CONFIG_MAX_FILE_SIZE = 'MaximumFileSize'
ILMT_CONFIG_LOG_PERIOD = 'LogPeriod'
ILMT_CONFIG_MAX_FILE_COUNT = 'MaximumFileCount'

ILMT_AUTHORIZED_USER_STR = 'AUTHORIZED_USER'
ILMT_FLOATING_USER_STR = 'FLOATING_USER'

CPXILMTMETRIC_AUTHORIZED_USER = 1
CPXILMTMETRIC_FLOATING_USER = 2

ILMT_SCHEMA_VERSION_BLOCK = """\
<SchemaVersion>{0}</SchemaVersion>
<SoftwareIdentity>
  <Name>{1}</Name>
  <PersistentId>{2}</PersistentId>
  <InstanceId>{3}</InstanceId>
</SoftwareIdentity>
"""

ILMT_METRIC_BLOCK = """\
<Metric logTime="{0}">
  <Type>{1}</Type>
  <Value>{2}</Value>
  <Period>
    <StartTime>{3}</StartTime>
    <EndTime>{4}</EndTime>
  </Period>
</Metric>
"""

class TimeWindow(object):
    """Class used to calculate the overlapping time windows."""

    def __init__(self, start, stop):
        self.start = start
        self.stop = stop
        self.num_overlaps = 1

    def overlaps(self, that):
        """Does this time window overlap with that?"""
        (maxstart, minstop) = self._get_max_min(that)
        return (maxstart <= minstop and
                self.start <= maxstart and
                self.stop >= minstop)

    def merge(self, that):
        """Merge this time window with that."""
        (self.start, self.stop) = self._get_max_min(that)
        assert self.start <= self.stop, "start: {0}, stop: {1}".format(
            self.start, self.stop)
        self.num_overlaps += 1

    def _get_max_min(self, that):
        """Get the max start and min stop from two time windows."""
        maxstart = max(self.start, that.start)
        minstop = min(self.stop, that.stop)
        return (maxstart, minstop)

class SqliteCursor(object):
    """Thin wrapper around sqlite3 so we can use 'with' statements."""

    def __init__(self, dbpath, commit_on_exit=True):
        self.conn = None
        self.cur = None
        self.dbpath = dbpath
        self.commit_on_exit = commit_on_exit

    def __enter__(self):
        self.conn = sqlite3.connect(self.dbpath)
        self.conn.row_factory = sqlite3.Row
        self.cur = self.conn.cursor()
        return self.cur

    def __exit__(self, type_, value, traceback):
        # NB: We don't commit until exit!!!  This could bite you if you're
        #     not careful!
        if self.commit_on_exit:
            self.conn.commit()
        if self.cur is not None:
            self.cur.close()
        if self.conn is not None:
            self.conn.close()

class Result(object):
    """An object that contains the results of an operation."""

    def __init__(self, success=True, msg=None):
        self.success = success
        self.msg = msg

    def is_successful(self):
        """Get the success boolean."""
        return self.success

    def get_message(self):
        """Get the result message."""
        return self.msg

class IlmtConfig(object):
    """An object to contain the ILMT configuration."""

    def __init__(self):
        self.license_metric = None
        self.location = None
        self.instance_id = ILMT_DEFAULT_INSTANCE_ID
        self.max_file_size = ILMT_DEFAULT_MAX_FILE_SIZE
        self.log_period = ILMT_DEFAULT_LOG_PERIOD
        self.max_file_count = ILMT_DEFAULT_MAX_FILE_COUNT

    def __str__(self):
        result = "ILMT Config:\n"
        result += "License Metric {0}\n".format(self.license_metric)
        result += "Location {0}\n".format(self.location)
        result += "InstanceID {0}\n".format(self.instance_id)
        result += "MaximumFileSize {0}\n".format(self.max_file_size)
        result += "LogPeriod {0}\n".format(self.log_period)
        result += "MaximumFileCount {0}\n".format(self.max_file_count)
        return result

    def validate(self):
        """Validates the configuration."""
        msgformat = 'Invalid {0} detected in ILMT configuration file!'
        if (self.license_metric is None or
            self.license_metric != ILMT_AUTHORIZED_USER_STR and
            self.license_metric != ILMT_FLOATING_USER_STR):
            return Result(success=False,
                          msg=msgformat.format(ILMT_CONFIG_LICENSE_METRIC))
        elif (self.location is None or
              not os.path.isdir(self.location)):
            return Result(success=False,
                          msg=msgformat.format(ILMT_CONFIG_LOCATION))
        else:
            # NB: These are ignored for now, so no need to validate.
            # ILMT_CONFIG_MAX_FILE_SIZE
            # ILMT_CONFIG_LOG_PERIOD
            # ILMT_CONFIG_MAX_FILE_COUNT
            return Result()

def usage():
    """Print this module's docstring."""
    print(__doc__)

def init(argv):
    """
    This procedure uses the getopt class to pull options and their
    values out of the commandline argument vector.
    """
    config = None
    # Parse commandline options
    try:
        opts, args = getopt.getopt(argv, "c:h", ["config=", "help"])
    except getopt.GetoptError as msg:
        print(msg)
        usage()
        sys.exit()
    # Get commandline options
    for opt, arg in opts:
        if opt in ("-c", "--config"):
            config = arg
        elif opt in ("-h", "--help"):
            usage()
            sys.exit()
        else:
            raise RuntimeError("Unknown option!")
    if len(args) != 0:
        print("ERROR: Unexpected commandline arguments!")
        usage()
        sys.exit()
    return config

def get_from_env_var():
    """Attempts to read the ILMT config file from an environment variable."""
    config = None
    try:
        config = os.environ['CPLEX_STUDIO_ILMT_CONFIG_FILE']
    except KeyError:
        pass
    return config

def check_result(result):
    """Check the result of an operation.

    If it fails then print the message and exit.
    """
    if not result.is_successful():
        print(result.get_message())
        sys.exit()

def check_file(path):
    """Validate the file path."""
    if path is None:
        return Result(success=False,
                      msg="ILMT configuration file not found!")
    elif os.path.isfile(path):
        return Result()
    else:
        return Result(success=False,
                      msg="Not a valid file!: %s" % path)

def get_ilmt_config(config):
    """Attemps to parse the ILMT configuration file."""
    tree = ET.parse(config)
    root = tree.getroot()
    ilmtconfig = IlmtConfig()
    # find will return an Element instance or None.
    element = root.find(ILMT_CONFIG_LICENSE_METRIC)
    if element is not None:
        ilmtconfig.license_metric = element.text
    element = root.find(ILMT_CONFIG_LOCATION)
    if element is not None:
        ilmtconfig.location = element.text
    element = root.find(ILMT_CONFIG_INSTANCE_ID)
    if element is not None:
        ilmtconfig.instance_id = element.text
    element = root.find(ILMT_CONFIG_MAX_FILE_SIZE)
    if element is not None:
        ilmtconfig.max_file_size = element.text
    element = root.find(ILMT_CONFIG_LOG_PERIOD)
    if element is not None:
        ilmtconfig.log_period = element.text
    element = root.find(ILMT_CONFIG_MAX_FILE_COUNT)
    if element is not None:
        ilmtconfig.max_file_count = element.text
    return ilmtconfig

def _safe_hash_update(hashalg, strarg):
    """Call hashalg.update(strarg) in fail-safe way.

    Should work with either Python 2 or 3.
    """
    try:
        hashalg.update(strarg)
    except TypeError:
        hashalg.update(strarg.encode('utf-8'))

def generate_md5_name(instance_id):
    """Given the instance ID, generate an md5 hash."""
    assert instance_id is not None
    hashalg = hashlib.md5()
    _safe_hash_update(hashalg, CPX_ILMT_PERSISTENT_ID)
    _safe_hash_update(hashalg, instance_id)
    return hashalg.hexdigest()

def get_db_path(ilmtconfig):
    """Get the database path from the ILMT config."""
    md5name = generate_md5_name(ilmtconfig.instance_id)
    dbname = '{0}{1}{2}'.format(CPX_ILMT_FILE_PREFIX, md5name,
                                CPX_ILMT_DB_EXT)
    return os.path.join(ilmtconfig.location, dbname)

def get_slmtag_path(ilmtconfig):
    """Get the SLMTag file location from the ILMT config."""
    md5name = generate_md5_name(ilmtconfig.instance_id)
    slmtagfile = '{0}{1}{2}'.format(CPX_ILMT_FILE_PREFIX, md5name,
                                    CPX_ILMT_SLMTAG_EXT)
    return os.path.join(ilmtconfig.location, slmtagfile)

def calculate_peak_usage(cur):
    """Calculate peak usage given a set of start, stop records."""
    lhs = []
    rhs = []
    for row in cur:
        lhs.append(TimeWindow(row['start'], row['stop']))
        rhs.append(TimeWindow(row['start'], row['stop']))
    numrecords = len(lhs)
    assert len(rhs) == numrecords
    for i in xrange(numrecords):
        for j in xrange(numrecords):
            # There is no need to compare time windows that are the same
            # nor to compare in both directions (i, j) and (j, i).
            if i <= j:
                continue
            twlhs = lhs[i]
            twrhs = rhs[j]
            if twlhs.overlaps(twrhs):
                twlhs.merge(twrhs)
            if twlhs.num_overlaps == numrecords:
                # If we encounter the case where all time windows overlap,
                # then we're done.  Just return the result.
                return numrecords
    return max(twlhs.num_overlaps for twlhs in lhs)

def get_user_info(dbpath, user_type):
    """Get user info from the ILMT logging database.

    After the information is retreived, we delete it.
    """
    with SqliteCursor(dbpath, False) as cur:
        cur.execute("SELECT COUNT(*) FROM ilmt "
                    "WHERE lic_metric_id = ? AND count = 0",
                    (user_type,))
        numrecords = cur.fetchone()[0]
        if numrecords == 0:
            peakusage = 0
        else:
            cur.execute("SELECT start, stop FROM ilmt "
                        "WHERE lic_metric_id = ? AND count = 0",
                        (user_type,))
            peakusage = calculate_peak_usage(cur)
        cur.execute("SELECT name FROM license_metric WHERE id = ?",
                    (user_type,))
        utname = cur.fetchone()[0]
        cur.execute("SELECT MIN(start) FROM ilmt "
                    "WHERE lic_metric_id = ? AND count = 0",
                    (user_type,))
        start = cur.fetchone()[0]
        cur.execute("SELECT MAX(stop) FROM ilmt "
                    "WHERE lic_metric_id = ? AND count = 0",
                    (user_type,))
        stop = cur.fetchone()[0]
    with SqliteCursor(dbpath, True) as cur:
        cur.execute("DELETE FROM ilmt "
                    "WHERE lic_metric_id = ? AND count = 0",
                    (user_type,))
    return (peakusage, utname, start, stop)

def get_auth_user_info(dbpath):
    """Get AUTHORIZED_USER info from the ILMT logging database.

    After the information is retreived, we delete it.
    """
    return get_user_info(dbpath, CPXILMTMETRIC_AUTHORIZED_USER)

def get_float_user_info(dbpath):
    """Get FLOATING_USER info from the ILMT logging database.

    After the information is retreived, we delete it.
    """
    return get_user_info(dbpath, CPXILMTMETRIC_FLOATING_USER)

def format_slmtag_info(logtime, usertype, value, start, stop):
    """Format SLMTag info as XML string."""
    # No URL encoding here.
    return ILMT_METRIC_BLOCK.format(logtime, usertype, value, start, stop)

def format_slmtag_header(instance_id):
    """Format SLMTag XML header as string."""
    # We URL encode the instance ID only.
    instance_id_enc = urllib.quote(instance_id)
    return ILMT_SCHEMA_VERSION_BLOCK.format(
        CPX_ILMT_SCHEMA_VERSION, CPX_ILMT_SOFTWARE_NAME,
        CPX_ILMT_PERSISTENT_ID, instance_id_enc)

def get_readable_time(timestamp):
    """Convert Unix timestamp into readable time."""
    return datetime.fromtimestamp(
        int(timestamp)).strftime(CPX_ILMT_DATE_FORMAT)

def get_start_stop_strings(start, stop):
    """Convert Unix timestamps and ensure start != stop."""
    # If the start time is equal to the stop time, then increment the stop
    # time by 1 second.  This to comply with the SLMTag rule that these
    # should not be the same value.
    if start == stop:
        stop += 1
    startstr = get_readable_time(start)
    stopstr = get_readable_time(stop)
    return (startstr, stopstr)

def get_slmtag_str(dbpath, timestamp=None):
    """Read the database and return an SLMTag string.

    dbpath    - the path to the sqlite database.
    timestamp - to be used for testing only, where we want to ensure that
                log times are always the same.
    """
    (authval, authtype, authstart, authstop) = get_auth_user_info(dbpath)
    (fltval, flttype, fltstart, fltstop) = get_float_user_info(dbpath)
    if timestamp is None:
        logtime = datetime.now().strftime(CPX_ILMT_DATE_FORMAT)
    else:
        logtime = get_readable_time(timestamp)
    slmtagstr = ''
    if authval > 0:
        (authstart, authstop) = get_start_stop_strings(authstart, authstop)
        slmtagstr += format_slmtag_info(logtime, authtype, authval,
                                        authstart, authstop)
    if fltval > 0:
        (fltstart, fltstop) = get_start_stop_strings(fltstart, fltstop)
        slmtagstr += format_slmtag_info(logtime, flttype, fltval,
                                        fltstart, fltstop)
    return slmtagstr

def write_slmtag(ilmtconfig, slmtagstr):
    """Writes information to the SLMTag file."""
    slmtagfile = get_slmtag_path(ilmtconfig)
    slmtagexists = os.path.isfile(slmtagfile)
    if not slmtagexists:
        with open(slmtagfile, 'w') as outfile:
            header = format_slmtag_header(ilmtconfig.instance_id)
            outfile.write(header)
    with open(slmtagfile, 'a') as outfile:
        outfile.write(slmtagstr)

def main():
    """The main function."""
    config = init(sys.argv[1:])
    if config is None:
        config = get_from_env_var()
    check_result(check_file(config))
    ilmtconfig = get_ilmt_config(config)
    check_result(ilmtconfig.validate())
    dbpath = get_db_path(ilmtconfig)
    check_result(check_file(dbpath))
    slmtagstr = get_slmtag_str(dbpath)
    write_slmtag(ilmtconfig, slmtagstr)
    print('SLMTag file successfully written/updated.')

if __name__ == "__main__":
    main()
