import re
from collections import defaultdict
import pandas as pd
import numpy as np
import csv

try:
    import log_constants as c
except ImportError:
    import summarizer.performance_utils.log_constants as c


class MeasurementReader(object):
    """docstring for MeasurementReader"""

    def __init__(self):
        self.info = {c.DATASET: '', c.TOPIC: '', c.ORACLE_TYPE: '', c.SUMMARY_LEN: '',
                     c.ITERATIONS: '', c.VERSION: '', c.MAX_WEIGHT: ''}
        self.run_log_values = ["k", "t", 'r2', 'r1', 'r4', "r",
                               "upper bound", "summary length", "model_id", "break condition"]
        self.iteration_log_values = ['iteration', 't', 'r2', 'r1', 'r4', 'constraints', 'concepts',
                                     'sentences', 'accepts', 'rejects', 'entropy', 'total concepts', 'ranking entropy', "k"]

        self.run_log = {key: [] for key in self.run_log_values}
        self.iteration_log = defaultdict(lambda: defaultdict(list))

        self.aggregated_data = defaultdict(list)
        self.aggregated_iteration_data = defaultdict(list)
        self.agg_max_iteration = defaultdict(list)

    def read_run_log(self, file):
        with open(file) as f:
            text = f.read()

            # get run info for plot
            for att in self.info:
                match = re.search("{} (\w+.?\w+?)".format(att), text)
                if match is None:
                    continue
                self.info[att] = match.groups()[0]

            mlines = text.split('\n')
            start = "k | t | r2"
            for line in mlines:
                if line.startswith(start):
                    i0 = mlines.index(line) + 1
                    break
            measurements = mlines[i0:-1]
            for i, m in enumerate(measurements):
                tokens = m.replace(" ", '').split('|')
                for key, token in zip(self.run_log_values, tokens):
                    # in case of double ks
                    # if key is 'k':
                    #     if token in self.run_log[key]:
                    #         token = token + "_" + str(i)
                    self.run_log[key].append(token)

    def read_iteration_log(self, file):
        with open(file) as f:
            text = f.read()
            measurements = text.split('k=')
            for i, m in enumerate(measurements[1:]):
                lines = m.splitlines()
                k = lines[0]
                # in case of double ks, which might happen in dynamic testing with rank ranges
                # if k in self.iteration_log:
                #     k = k + "_" + str(i)
                for line in lines[2:]:
                    tokens = line.replace(" ", '').split('|')
                    for i in range(0, len(tokens) - 1):
                        try:
                            entry = float(tokens[i])
                        except ValueError as e:
                            entry = tokens[i]
                        self.iteration_log[k][self.iteration_log_values[i]].append(entry)

    def read_corpora_stats(self, folder):
        path = folder + self.info[c.DATASET]
        self.corpus_stats_df = self.csv_to_df(path, sep="|")

    def aggregate_data(self):
        for key, value_list in self.run_log.items():
            for value in value_list:
                try:
                    self.aggregated_data[key].append(float(value))
                except ValueError:
                    self.aggregated_data[key].append(value)
        # extra stuff
        # for k in self.run_log['k']:
        #     number_of_iterations = len(self.iteration_log[k]['t'])
        #     avg_time_per_iteration = sum(
        #         self.iteration_log[k]['t']) / number_of_iterations
        #     self.aggregated_data['number_of_iterations'].append(
        #         number_of_iterations)
        #     self.aggregated_data['avg_time_per_iteration'].append(
        #         avg_time_per_iteration)

    def aggregate_iteration_data(self):
        keys = self.iteration_log[self.run_log['k'][0]].keys()
        for k in self.run_log['k']:
            for key in keys:
                for val in self.iteration_log[k][key]:
                    self.aggregated_iteration_data[key].append(val)
            # add ks
            for key in keys:
                for val in self.iteration_log[k][key]:
                    self.aggregated_iteration_data['k'].append(k)
                break

        del self.iteration_log
        self.iteration_log = defaultdict(lambda: defaultdict(list))

        # corpus_size = self.get_corpus_stat("Corpus Size")
        # for i in range(0, len(self.iteration_log[k][key])):
        #     self.aggregated_iteration_data['k'].append(float(k))
        #     self.aggregated_iteration_data['length_constraint'].append(float(self.info[c.SUMMARY_LEN]))
        #     self.aggregated_iteration_data['corpus_size'].append(int(corpus_size))

    def get_corpus_stat(self, column):
        df = self.corpus_stats_df
        value = df.loc[df['Topic'] == self.topic_rid][column]
        return value.values[0]

    def csv_to_df(self, path, sep=','):
        df = pd.read_csv(path, sep=sep)
        return df

    def write_to_csv(self, path):
        df = pd.DataFrame(data=self.aggregated_iteration_data)
        df.to_csv(path + "iterations.csv", index=False)

    def get_number_of_iterations(self):
        for k in sorted(self.iteration_log.keys()):
            self.agg_max_iteration[k].append(max(self.iteration_log[k]['iteration']))

    def maxit_to_csv(self, path):
        d = {}
        # TODO: Rotate columns to row
        for k, v in self.agg_max_iteration.items():
            d[k] = sum(v) / len(v)
        with open(path + 'maxit.csv', 'w') as f:
            w = csv.DictWriter(f, d.keys())
            w.writeheader()
            w.writerow(d)

    def set_topic_rid(self, rid=None):
        if rid is None:
            rid = self.info[c.TOPIC]
        try:
            self.topic_rid = int(rid)
        except ValueError:
            self.topic_rid = rid

    def clear_aggregate_data(self):
        self.aggregated_data = defaultdict(list)
        self.aggregated_iteration_data = defaultdict(list)

    def add_max(self, label='r', att='r2', it=10, best_slice=1, max_weight=None):
        if max_weight is None:
            max_weight = self.info[c.MAX_WEIGHT]

        values = []
        # measured_k = self.run_log['k']
        for k, la in zip(self.run_log['k'], self.run_log[label]):
            if label is 'k':
                if k in ['10', '20']:
            #     # if k in ['10', '20', '50']:
                    continue
            if label is 'r':
                la = float(la) * max_weight
            try:
                values.append((la, self.iteration_log[k][att][it - 1]))
            except IndexError:
                values.append((la, self.iteration_log[k][att][-1]))
                # continue
        if not values:
            return None

        # heck = [x for x in values if x == max(values, key=lambda x: x[1])]
        # if len(heck) > 1:
        #     print(heck)
        #     exit()

        try:
            return_values = sorted(values, key=lambda x: (-float(x[1]), int(x[0])))
        except ValueError:
            return_values = sorted(values, key=lambda x: (-float(x[1]), x[0]))
        # return_values = sorted(values, reverse=True, key=lambda x: x[1])
        # exit()
        if best_slice == 1:
            return return_values[0]
        else:
            return return_values[:best_slice]

    def get_k_attribute_pairs(self, att='entropy', it=1):
        for k in self.run_log['k']:
            try:
                yield (k, self.iteration_log[k][att][it - 1])
            except IndexError:
                yield (k, self.iteration_log[k][att][-1])

    def get_value_at(self, k, att='r2', it=0):
        try:
            val = float(self.iteration_log[k][att][it])
        except IndexError:
            val = float(self.iteration_log[k][att][-1])
        return val
