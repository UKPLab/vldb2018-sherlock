from collections import defaultdict

from mreader import MeasurementReader
from combine_funcs import read_logs, get_sorted_topics
import log_constants as c
import pandas as pd


class ResultCombiner(object):
    """docstring for ResultCombiner"""
    def __init__(self, to_compare=None, compare_base=None):
        self.topics = get_sorted_topics(to_compare)
        # if self.topics != get_sorted_topics(compare_base):
        #     raise ValueError('not the same number of topics in path 1 and 2')
        self.base_paths = [to_compare, compare_base]

        self.routines = {
            'get diffs': {
                'aggregate': self.get_diffs,
                'post_aggregate': self.diff_results,
            },
            'get results': {
                'aggregate': self.get_result_routine,
                'post_aggregate': self.get_results_end,
            },
            'winner histogram': {
                'aggregate': self.aggregate_histogram_data,
                'post_aggregate': self.plot_histogram,
            },
            'test': {
                'aggregate': lambda: None,
                'post_aggregate': lambda: None
            }
        }

        # analyse baselines routine
        self.best_slice = 3
        self.bound_counter = defaultdict(lambda: defaultdict(lambda: 0))
        self.diff = defaultdict(lambda: defaultdict(list))

        # get results routine
        self.sorted_results = defaultdict(list)

        # histogram routine
        self.histogram_data = []

    def execute_routine(self, routine):
        inner_routine = self.routines[routine]['aggregate']
        end_routine = self.routines[routine]['post_aggregate']

        for folder in self.topics:
            self.readers = [MeasurementReader() for i in self.base_paths]
            for reader, path in zip(self.readers, self.base_paths):
                read_logs(reader, path, folder)
            inner_routine()

        end_routine()

# #### diff routine ####
    def get_diffs(self, normalized=True):
        # def get_diffs(readers, bound_counter, best_slice, diff, normalized=False):
        ranked_reader, baseline_reader = self.readers
        best_k = [x[1] for x in ranked_reader.add_max(label='k', att='r2', it=10, best_slice=self.best_slice)]
        # colour = ['forestgreen', 'salmon']

        r2_values = []
        for k in baseline_reader.run_log['k']:
            try:
                val = float(baseline_reader.iteration_log[k]['r2'][9])
            except IndexError:
                val = float(baseline_reader.iteration_log[k]['r2'][-1])

            # if val == 0:
                # continue
            r2_values.append(val)

        # for val in r2_values:
        #     if val == 0:
        #         raise ValueError('{}'.format(r2_values))

        bound_ge = []
        delim = min(self.best_slice, len(best_k))
        for r2 in r2_values:
            tup = []
            for i in range(0, delim):
                tup.append(r2 >= best_k[i])
            bound_ge.append(tuple(tup))

        for i, (flags, r2_val) in enumerate(zip(bound_ge, r2_values)):
            for j, flag in enumerate(flags):
                if flag:
                    self.bound_counter[i][j] += 1
                base = 1
                if normalized:
                    base = best_k[j]
                val_diff = (best_k[j] - r2_val) / base
                self.diff[i][j].append(val_diff)
            pass

    def diff_results(self):
        for i in range(0, len(self.bound_counter.keys())):
            header = ""
            for j in range(0, len(self.bound_counter[0])):
                header += "{}>={}.best\t\t".format(i + 1, j + 1)
            print(header)
            for j in range(0, len(self.bound_counter[0])):
                print("{}\t\t\t\t".format(self.bound_counter[i][j]), end="")
            print("")

        print("\n----")
        floaty = "%.4f"
        for i in range(0, len(self.bound_counter.keys())):
            for k in range(0, len(self.bound_counter[0])):
                print("Diff Best {}".format(i + 1), end=" ")
                print(k + 1)
                # print("\tMin value:\t", floaty % min(diff[i][k]))
                # print("\tMax value:\t", floaty % max(diff[i][k]))
                print("\tAverage:\t", floaty % (sum(self.diff[i][k]) / len(self.diff[i][k])))
                # TODO show normalized values
            print("")

# #### result routine ####
    def get_result_routine(self):
        for i, reader in enumerate(self.readers):
            self.sorted_results[i].append(self.get_results(reader))

    def get_results(self, reader, at_iteration=10, get_sorted=False):
        measurement = []
        for k in reader.run_log['k']:
            try:
                r2 = float(reader.iteration_log[k]['r2'][at_iteration - 1])
            except IndexError:
                r2 = float(reader.iteration_log[k]['r2'][-1])
            measurement.append((r2, k))
        # measurement = [(float(x), k) for x, k in zip(reader.run_log['r2'], reader.run_log['k'])]
        if get_sorted:
            measurement.sort(key=lambda x: (-x[0], x[1]))
        return measurement

    def get_results_end(self, is_sorted=False):
        for i, reader in zip(self.sorted_results, self.readers):
            len_rank = defaultdict(lambda: 0)
            for mlist in self.sorted_results[i]:
                for j in range(len(mlist)):
                    len_rank[j] += 1
            avgs = defaultdict(lambda: 0)
            for mlist in self.sorted_results[i]:
                for j, (r2, k) in enumerate(mlist):
                    avgs[j] += ((r2) / len_rank[j])
                    # avgs[j] += ((r2) / len(sorted_results[i]))
            print("Reader", i)
            for rank, avg_r2 in avgs.items():
                if is_sorted:
                    prefix = rank + 1
                else:
                    prefix = reader.run_log['k'][rank]
                print(prefix, ":", "%.4f" % avg_r2, "k", end=" ")
            print("")

# #### histogram routine ####
    def aggregate_histogram_data(self):
        best_k = self.count_best_ks()
        self.histogram_data.append(best_k)
        pass

    def count_best_ks(self):
        to_compare_reader, compare_base_reader = self.readers
        best_k, best_k_score = to_compare_reader.add_max(label='k', att='r2', it=10, best_slice=1)

    def plot_histogram(self):
        print("WTF")
        pass

# #### Getters ####
    def get_topics(self):
        return self.topics

    def get_number_of_topics(self):
        return len(self.topics)
