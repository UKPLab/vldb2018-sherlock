from __future__ import print_function
from sets import Set


class FlightRecorder(object):
    """
    Keeps a list of records, plus the union of all of them, too.
    """

    def __init__(self, records=None, total=None):
        self.records = records or []
        self.total = total or Record()

    def add_record(self, record):
        if type(record) is not Record:
            raise BaseException("IllegalArgumentException")

        self.record(record.accept, record.reject, record.implicit_reject)

    def record(self, accept=None, reject=None, implicit_reject=None):
        accept = Set(accept)
        reject = Set(reject)
        implicit_reject = Set(implicit_reject)
        self.records.append(Record(accept, reject, implicit_reject))

        if accept is not None:
            self.total.accept |= accept
            # self.accepted_concepts=accept
            # self.total_accept += accept
        if reject is not None:
            self.total.reject |= reject
            # self.rejected_concepts=reject
            # self.total_reject += reject
        if implicit_reject is not None:
            self.total.implicit_reject |= implicit_reject
            # self.implicit_reject = implicit_reject
            # self.total_implicit_reject += implicit_reject

    def latest(self):
        """
            Returns the last added Record, or an empty Record
        :return:
        """
        if len(self.records) == 0:
            return Record()
        return self.records[-1:][0]

    def union(self):
        """

        :return: dict
        """
        return self.total

    def clear(self):
        self.__init__()


class Record(object):
    def __init__(self, accept=None, reject=None, implicit_reject=None):
        """
        @type accept: set[str]
        @type reject: set[str]
        @type implicit_reject: set[str]
        """
        if accept is None:
            accept = Set()
        if reject is None:
            reject = Set()
        if implicit_reject is None:
            implicit_reject = Set()
        self.accept = accept
        self.reject = reject
        self.implicit_reject = implicit_reject
