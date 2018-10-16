from __future__ import print_function

from time import localtime, strftime
import os
import errno
import logging
from summarizer.performance_utils.notifier import DropboxNotifier, EmailNotifier

LOG_FOLDER = "performance_utils/measurements/"
LOGGED_PARAM = "k"
PERF_DATA_SET = "DUC_PERF"

BREAK_COND = {1: "flight_recorder empty",
              2: "current summary > UB",
              3: "Found UB summary",
              4: "score = UB score",
              0: "> maximum iteration count", }
LOGFILE_RUN_SUFFIX = "run"
LOGFILE_IT_SUFFIX = "iterations"

NOTIFIERS = {
    'email': EmailNotifier,
    'dropbox': DropboxNotifier,
}


class MeasurementLogger(object):
    def __init__(self, notify_by=None):
        self.time_of_run = strftime("%d-%m-%Y %H:%M", localtime())
        self.log_folder = self.create_folder(LOG_FOLDER + self.time_of_run + "/")

        # EmailNotifier by default
        self.notifier = NOTIFIERS.get(notify_by, EmailNotifier)(self.time_of_run, self.log_folder)
        return

    def set_run_info(self, data_set, topic, summary_len, num_models, oracle_type, measured_k, max_iteration_count, run_version, max_weight):
        if data_set == PERF_DATA_SET:
            self.data_set = PERF_DATA_SET
        else:
            self.data_set = data_set
        self.topic = topic
        self.summary_len = summary_len
        self.num_models = num_models
        self.oracle_type = oracle_type
        self.measured_k = measured_k
        self.max_iteration_count = max_iteration_count
        self.run_version = run_version
        self.max_weight = max_weight
        return

    def create_folder(self, path):
        try:
            os.makedirs(path)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
        return path

    def create_working_dir(self, m_idx):
        working_dir = self.log_folder + self.topic + "_" + str(m_idx) + "/"
        self.working_dir = self.create_folder(working_dir)

        self.logfile_prefix = "{}{}-".format(self.working_dir, self.topic)
        self.logfile = "{}{}".format(self.logfile_prefix, LOGFILE_RUN_SUFFIX)
        self.logfile_it = "{}{}".format(self.logfile_prefix, LOGFILE_IT_SUFFIX)
        self.write_log_header()

        # For Stratified Sampling
        logger = logging.getLogger('stratified')
        hdlr = logging.FileHandler(self.working_dir + 'clustering')
        formatter = logging.Formatter('')
        hdlr.setFormatter(formatter)
        logger.handlers = []
        logger.addHandler(hdlr)
        logger.setLevel(logging.INFO)

    def write_log_header(self):
        '''writes a header for a measurement run'''
        header_values = [self.time_of_run, self.data_set, self.topic, self.oracle_type,
                         self.summary_len, self.max_iteration_count, self.run_version, self.max_weight]
        header = '''{} - On dataset {}, topic {}, oracle_type {}, L = {}, max iteration count = {}, algorithm version = {}, max_weight = {}\n'''.format(
            *header_values)
        table_header = "k | t | r2 | r1 | r4 | r | upper bound| summary length| model id| break condition\n"
        with open(self.logfile, 'a+') as file:
            for string in [header, table_header]:
                file.write(string)
        return

    def log_iteration(self, k, r, run_time, run_r1, run_r2, run_r4, ub_score, result_length, model_id, break_condition, stats, k_history, x, r1, r2, r4):
        '''Plot graph for Quality per time; one graph per k'''

        self.log_iteration_results(k, x, r1, r2, r4, k_history, stats)
        self.log_measurement(k, run_time, run_r2, run_r1, run_r4, r, ub_score, result_length, model_id, BREAK_COND[break_condition])

    def log_iteration_results(self, k, x, r1, r2, r4, k_history, stats):
        it_header = "k={}\niteration | time| r2 | r1 | r4 | number_of_ilp_constraints| concepts_size| sentences_size| accepts | rejects| entropy| total concept size |ranking entropy| k\n".format(k)
        constraints, c, s = stats['constraints'], stats['C'], stats['S']
        acc, rej = stats['accepts'], stats['rejects']
        H = stats['entropy']
        c_t = stats['total concept size']
        Hrank = stats['ranking entropy']
        
        if type(k_history) is list:
            ks = k_history
        else:
            ks = [k for _ in H]

        with open(self.logfile_it, 'a+') as file:
            file.write(it_header)
        for i, (xi, r1_i, r2_i, r4_i, con, ci, si, ai, ri, Hi, c_ti, Hri, ki) in enumerate(zip(x, r1, r2, r4, constraints, c, s, acc, rej, H, c_t, Hrank, ks)):
            self.log_measurement(i + 1, "%.4f" % xi, r2_i, r1_i, r4_i, con, ci, si, ai, ri, "%.4f" % Hi, c_ti, "%.4f" % Hri, ki, filename=self.logfile_it)
        return

    def end_run(self):
        self.notifier.send_payload()

    # Helpers #
    def table_log_string(self, args):
        output = ""
        for arg in args:
            output += "{} | ".format(arg)
        return output + "\n"

    def log_measurement(self, *args, **kwargs):
        if 'filename' in kwargs:
            filename = kwargs['filename']
        else:
            filename = self.logfile
        with open(filename, 'a+') as file:
            file.write(self.table_log_string(args))
        return

    def parseNum(self, num):
        try:
            num = int(num)
            return num
        except ValueError:
            return float(num)
