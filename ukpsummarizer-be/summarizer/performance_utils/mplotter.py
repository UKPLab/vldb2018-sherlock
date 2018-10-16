import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import matplotlib.ticker as ticker
from collections import defaultdict
import statsmodels.formula.api as sm

from mreader import MeasurementReader
import log_constants as c


class MeasurementPlotter(object):
    """docstring for MeasurementPlotter"""

    def __init__(self):
        self.colors = ['#ffad01', '#042e60', '#11875d', '#7a5901', '#f7022a',
                       '#29e8e8', '#ff7fa7', '#800ed1', '#51b73b', '#730039', '#63b365']
        self.color_i = 0

        self.readers = []
        self.max_values = []
        self.corr = []
        self.categorized_corr = defaultdict(list)
        self.categorized_values = defaultdict(list)

        self.H = defaultdict(list)
        self.H['cat'] = defaultdict(list)

    def plot_run_results(self, i, folder):
        ''' Plot graph for time per k'''
        self.fig_runtime = plt.figure(1, figsize=(16, 12), dpi=80)
        plt.figure(1)
        for xi, yi in zip(self.measured_k, self.y):
            # plot actual data points when interpolation)
            plt.plot(xi, yi, marker='x',
                     color=self.colors[1], figure=self.fig_runtime)

        # interpolate and plot
        # Graph 1
        # graph info
        plot_header = "Time per k for {} - {}, oracle_type {}, L ={}, max_iteration_count={}, algorithm version ={}".format(
            self.info[c.DATASET], self.info[c.TOPIC], self.info[c.ORACLE_TYPE], self.info[c.SUMMARY_LEN], self.info[c.ITERATIONS], self.info[c.VERSION])
        plt.title(plot_header)
        plt.xlabel('k')
        plt.ylabel('time in seconds')

        plt.plot(self.measured_k, self.y, marker='x', color=self.colors[1 + i],
                 figure=self.fig_runtime, label=self.info[c.VERSION])  # uninterpolated
        plt.grid()
        plt.legend(title='version')
        self.fig_runtime.savefig(
            folder + '/' + self.info[c.TOPIC] + '-runtime' + '.png')
        return

    def plot_run_results_quality(self, folder):
        self.fig_runtime = plt.figure(1, figsize=(16, 12), dpi=80)
        plt.figure(1)

        x = self.readers[-1].run_log['k']
        y = self.readers[-1].run_log['r2']

        plt.plot(x, y, marker='x', figure=self.fig_runtime)
        self.fig_runtime.savefig(folder + '-runquality' + '.png')
        plt.clf()

    def plot_iteration_results(self, k, data=None, plot_iteration=False, marker='.',
                               linestyle='-', corpus_size=0, cut=None):
        '''Plot graph for Quality per time; one graph per k'''
        x = []
        t = 0.0
        if data is None:
            data = self.readers[-1].iteration_log

        for i, ti in enumerate(data[k]['t']):
            t += ti
            if plot_iteration:
                x.append(i + 1)
            else:
                x.append(t)
        x = x[:cut]
        y = data[k]['r2'][:cut]
        self.fig_quality = plt.figure(2, figsize=(10, 6), dpi=80)
        plt.figure(2)

        color = self.colors[self.color_i]

        # plt.axvline(x=300, color='salmon')
        # # # # #
        # Plot graph for quality/time per k
        # graph info
        # plt.title("R2-Wert pro Zeit für Datensatz:" +
        #           self.info[c.DATASET] + "; Topic: " + self.info[c.TOPIC] +
        #           "; Referenzzusammenfassung: " + self.readers[-1].run_log['model_id'][0])
        if plot_iteration:
            plt.xlabel('Iteration t')
        else:
            plt.xlabel('Zeit in s')
        plt.ylabel('Qualität in ROUGE-2')
        plt.grid(True, which="both")

        if k == 'original':
            k = 'Basis'
        plt.plot(x, y, marker=marker, color=color, linestyle=linestyle,
                 figure=self.fig_quality, label=k)  # uninterpolated

        # needs to be called last
        plt.legend(title='k')

        self.color_i = (self.color_i + 1) % len(self.colors)
        pass

    def plot_qt_ratio_per_k(self, folder):
        self.fig_ratio = plt.figure(3, figsize=(16, 12), dpi=80)
        plt.figure(3)
        x = [int(k) for k in self.measured_k]
        y = []
        threshold = 0.1
        for k in self.measured_k:
            val = 0.0
            yk = self.iterations_data[k]['r2']
            for i in range(0, len(yk)):
                if yk[i] > threshold:
                    val = yk[i]
                    break
            y.append(val)

        plot_header = "Q/t per k for {} - {}, L ={}, max_iteration_count={}, algorithm version ={}".format(
            self.info[c.DATASET], self.info[c.TOPIC], self.info[c.SUMMARY_LEN], self.info[c.ITERATIONS], self.info[c.VERSION])
        plt.title(plot_header)
        plt.xlabel(self.info[c.VERSION])
        plt.ylabel("Quality / time")

        plt.plot(x, y, marker='x', color=self.colors[1])

        self.fig_ratio.savefig(
            folder + '/' + self.info[c.TOPIC] + '-qt-ratio' + '.png')

    def plot_number_of_iterations_per_k(self):
        x = [int(k) for k in self.measured_k]
        y = []
        for k in self.measured_k:
            y.append(len(self.iterations_data[k]['t']))

        self.fig_num_it = plt.figure(5, figsize=(16, 12), dpi=80)
        plt.figure(5)

        plt.title("Number of iterations per k for " +
                  self.info[c.DATASET] + " " + self.info[c.TOPIC])
        plt.xlabel('k')
        plt.ylabel('# of iterations')
        plt.grid(True, which="both")

        plt.plot(x, y, marker='x', color=self.colors[self.color_i],
                 figure=self.fig_num_it, label=k)

        self.fig_num_it.savefig(
            folder + '/' + self.info[c.TOPIC] + '-num_it' + '.png')

    def plot_summary_length_per_k(self):
        self.fig_sum_len = plt.figure(4, figsize=(16, 12), dpi=80)
        x = [int(k) for k in self.measured_k]
        y = self.summary_lengths

        plt.figure(4)

        plt.title("Resulting summary length per k for " +
                  self.info[c.DATASET] + " " + self.info[c.TOPIC])
        plt.xlabel('k')
        plt.ylabel('Summary length')
        plt.grid(True, which="both")

        plt.plot(x, y, marker='x', color=self.colors[self.color_i],
                 figure=self.fig_sum_len)

        self.fig_sum_len.savefig(
            folder + '/' + self.info[c.TOPIC] + '-sum_len' + '.png')

    def plot_accept_reject_ratio(self):

        plt.figure(6)
        for k in self.measured_k:
            df = pd.DataFrame(data=self.iterations_data[k])
            df['ratio'] = df['accepts'] / (df['accepts'] + df['rejects'])
            df['ratio'].fillna(0, inplace=True)
            x = self.iterations_data[k]['iteration']
            y = df['ratio'].tolist()
            plt.plot(x, y, label=k)

        plt.grid(True, which="both")
        plt.legend(title='k')
        plt.show()
        # x = [self.iterations_data]
        pass

    def plot_upperbound(self, r2score, color='salmon'):
        plt.figure(2)
        plt.axhline(y=r2score, color=color)
        return

    def plot_vanilla_timebound(self):
        plt.figure(1)
        plt.axhline(y=self.timebound, color='salmon')

    def save_it_plots(self, folder):
        '''Save plot as png'''
        self.fig_quality.savefig(
            folder + self.readers[-1].run_log['model_id'][0] + '-q-per-it' + '.png')

        self.color_i = 0

    def boxplot_number_of_iterations(self, folder):
        df = pd.DataFrame(data=self.readers[-1].aggregated_data)
        sns.set(style="ticks")

        # Select * from where
        # print(df.loc[df['k'] == 100.0])
        # df = df.loc[~df['k'].isin([200.0, 300.0, 500.0, 750.0])]

        # print(df.loc[df['avg_time_per_iteration'] > 6])
        # Initialize the figure with a logarithmic x axis
        f, ax = plt.subplots(figsize=(12, 8))

        # ax.xaxis.set_major_locator(ticker.MultipleLocator(0.5))
        # ax.xaxis.set_major_formatter(ticker.ScalarFormatter())

        sns.boxplot(x="number_of_iterations", y="k", data=df,
                    palette="vlag", orient='h')
        # Add in points to show each observation
        sns.swarmplot(x="number_of_iterations", y="k", data=df,
                      size=2, color=".3", linewidth=0, orient='h')

        sns.set(color_codes=True)
        lmp = sns.lmplot(x="k", y="number_of_iterations", data=df, order=2, ci=None, scatter_kws={"s": 2}, size=9)
        lmp.set_axis_labels("k", "number_of_iterations")
        lmp.set(xticks=range(0, 1001, 100))

        # sns.distplot(df.loc[df['k'] == 500.0]['avg_time_per_iteration'])

        # Tweak the visual presentation
        ax.xaxis.grid(True)
        ax.set(ylabel="k")
        sns.despine(trim=True, left=True)

        # plt.show()
        f.savefig(folder + "boxplot number of iterations.png")
        lmp.savefig(folder + "lmplot number of iterations.png")

    def boxplot_avg_time_per_iteration(self, folder):
        df = pd.DataFrame(data=self.readers[-1].aggregated_data)
        sns.set(style="ticks")
        f, ax = plt.subplots(figsize=(12, 8))

        # ax.xaxis.set_major_locator(ticker.MultipleLocator(0.5))
        # ax.xaxis.set_major_formatter(ticker.ScalarFormatter())

        ax.set_xscale("log")
        sns.boxplot(x="avg_time_per_iteration", y="k", data=df,
                    palette="husl", orient='h')
        # Add in points to show each observation
        sns.swarmplot(x="avg_time_per_iteration", y="k", data=df,
                      size=2, color=".3", linewidth=0, orient='h')

        sns.set(color_codes=True)
        lmp = sns.lmplot(x="k", y="avg_time_per_iteration", data=df, order=3, ci=None, scatter_kws={"s": 2}, size=5)
        # lmp.set(xticks=range(0, 1050,100))

        # Tweak the visual presentation
        ax.xaxis.grid(True)
        ax.set(ylabel="average time per iteration")
        sns.despine(trim=True, left=True)
        # plt.show()
        f.savefig(folder + "boxplot avg time per iterations.png")
        lmp.savefig(folder + "lmplot number of iterations.png")
        f.close()

    def time_per_constraints(self, folder):
        # df = pd.DataFrame(data=self.readers[-1].aggregated_iteration_data)
        df = self.get_aggregate_data()
        # Data points only
        point_size = 0.6
        # with regression
        point_size = 0.4

        sns.set(color_codes=True)
        # gr = sns.lmplot(x="constraints", y="t", data=df, ci=None, scatter_kws={"s": point_size}, size=9)
        # gr = sns.lmplot(x="constraints", y="t", data=df, order=2,
        #                 ci=None, scatter_kws={"s": point_size}, size=5, truncate=True)
        #                 # ci=None, scatter_kws={"s": point_size, 'color': 'blue'}, size=9, truncate=True)

        # Data points only
        gr = sns.lmplot(x="constraints", y="t", data=df, order=1, fit_reg=False, ci=None, scatter_kws={"s": point_size}, size=6, truncate=True)

        max_tick = df['t'].max() + 1
        gr.set(yticks=np.arange(0, max_tick, 1))
        gr.set_axis_labels("Anzahl der ILP-Constraints", "Zeit pro Iteration")
        # sns.residplot(x="constraints", y="t", data=df, order=1, scatter_kws={"s": point_size})
        gr.savefig(folder + "time per constraints.png")
        plt.show()

    def regression(self, folder):

        df = pd.DataFrame()
        for reader in self.readers:
            tmp = pd.DataFrame(data=reader.aggregated_iteration_data)
            df = pd.concat([df, tmp], ignore_index=True)

        print(df[['entropy', 't']].corr())

        # print(df)
        # result = sm.ols(formula="t ~ constraints", data=df).fit()
        # print(result.summary())

        pass

    def accept_reject_ratio(self, folder):
        df = pd.DataFrame(data=self.readers[-1].aggregated_iteration_data)
        df['accept_reject_ratio'] = df['accepts'] / (df['accepts'] + df['rejects'])
        df['accept_reject_ratio'].fillna(0, inplace=True)

        point_size = 2
        order = 2
        sns.set(color_codes=True)
        gr = sns.lmplot(x="iteration", y="accept_reject_ratio", data=df, order=order, ci=None, scatter_kws={"s": point_size}, size=9)
        gr.set_axis_labels("k", "accept / reject ratio")
        gr.set(ylim=(0, 1))
        gr.savefig(folder + "accept reject ratio.png")

    def boxplot_estimate_time(self):
        base = "/home/orkan/Dropbox/measurements/"
        dataset = "bbc"

        file = "iterations.csv"
        df = self.csv_to_df(base + dataset + "/time to ilp 1s/" + file)
        df['max_t'] = pd.Series(1, index=df.index)

        sns.set(style="ticks")
        f, ax = plt.subplots(figsize=(12, 8))

        ax.xaxis.set_major_locator(ticker.MultipleLocator(0.5))
        ax.xaxis.set_major_formatter(ticker.ScalarFormatter())
        ax.set_xlim(0, 7)
        # ax.set_xscale("log")
        sns.boxplot(x="t", y="max_t", data=df,
                    palette="husl", orient='h', bootstrap=1000)
        # Add in points to show each observation
        # sns.swarmplot(x="t", y="max_t", data=df,
        #               size=2, color=".3", linewidth=0, orient='h')

        sns.set(color_codes=True)

        # Tweak the visual presentation
        ax.xaxis.grid(True)
        ax.set(ylabel="target time per i")
        sns.despine(trim=True, left=True)
        # plt.show()
        f.savefig("/home/orkan/Dropbox/measurements/boxplot_estimate_time" + dataset +".png")
        return

    def errorbar_estimate_time(self):
        base = "/home/orkan/Dropbox/measurements/"
        dataset = "bbc"
        file = "iterations.csv"

        # df = self.csv_to_df(base + dataset + "/time to ilp 1s/" + file)
        # df['max_t'] = pd.Series(1, index=df.index)

        # df1 = self.csv_to_df(base + dataset + "/time to ilp 05s/" + file)
        # df1['max_t'] = pd.Series(0.5, index=df1.index)
        # df2 = self.csv_to_df(base + dataset + "/time to ilp 1s/" + file)
        # df2['max_t'] = pd.Series(1, index=df2.index)
        # df3 = self.csv_to_df(base + dataset + "/time to ilp 2s/" + file)
        # df3['max_t'] = pd.Series(2, index=df3.index)
        # df4 = self.csv_to_df(base + dataset + "/time to ilp 3s/" + file)
        # df4['max_t'] = pd.Series(3, index=df4.index)

        # df5 = self.csv_to_df(base + dataset + "/time to ilp 5s/" + file)
        # df5['max_t'] = pd.Series(3, index=df5.index)

        # df = df2
        # print(df.sort_values('t'))
        # df = pd.concat([df1, df2, df3, df4], ignore_index=True)
        # df = pd.concat([df1, df2, df3, df4, df5], ignore_index=True)

        p025 = df.groupby('max_t')['t'].quantile(0.025)
        p975 = df.groupby('max_t')['t'].quantile(0.975)
        mean = df.groupby('max_t')['t'].mean()
        std = df.groupby('max_t')['t'].std()

        print(std)

        plt.errorbar(mean, mean.index, yerr=0.2, xerr=[mean - p025, p975 - mean], linestyle='', capsize=3, marker=".")
        # plt.errorbar(mean, mean.index, yerr=0.2, xerr=2 * std, linestyle='', capsize=3, marker=".")
        plt.yticks((0, 0.5, 1, 2, 3, 5, 6), ('', '.5', '1', '2', "3", '5', ''))
        # plt.show()
        plt.savefig(base + dataset + "-errrorbars2.png")

    def best_k_entropy_boxplot(self, save_path):
        sns.set(style="ticks")
        f, ax = plt.subplots(figsize=(12, 8))

        ax.xaxis.set_major_locator(ticker.MultipleLocator(0.05))
        ax.xaxis.set_major_formatter(ticker.ScalarFormatter())
        ax.set_xlim(0.4, 0.75)

        # key = "< 300"
        # key = "300-700"
        key = ">700"
        # df = pd.DataFrame(self.H['cat'][key], columns=['k', 'H'])
        df = pd.DataFrame(self.H['k'], columns=['k', 'H'])
        # ax.set_xscale("log")
        range_max = int(df['k'].max() + 1)
        order = [20.0, 50.0] + list(range(100, range_max, 100))
        sns.boxplot(x="H", y="k", data=df, palette="husl", orient='h',
                    order=reversed(order))
        # Add in points to show each observation
        # sns.swarmplot(x="H", y="k", data=df,
        #               size=2, color=".3", linewidth=0, orient='h')

        # print(df.corr())
        sns.set(color_codes=True)

        # Tweak the visual presentation
        ax.xaxis.grid(True)
        ax.set(xlabel="Entropy")
        sns.despine(trim=True, left=True)
        # plt.show()
        f.savefig(save_path + "boxplot entropy {} {}.png".format(self.ent_it, self.ent_points))
        # f.savefig(save_path + "boxplot entropy {}.png".format(key))

        pass

    def plot_entropy_per_iteration(self, save_path):
        '''Plot graph for Quality per time; one graph per k'''

        for reader in self.readers:
            print(reader.info[c.TOPIC])
            fig = plt.figure(1, figsize=(10, 6), dpi=80)
            plt.figure(1)
            # plt.title("Entropy per iteration for " + reader.info[c.DATASET] + " " +
            #           reader.info[c.TOPIC])
            plt.xlabel('Iteration t')
            plt.ylabel('gew. Entropie Hw')
            plt.grid(True, which="both")

            best_k = reader.add_max(label='k', att='r2', it=10, max_weight=None)[0]
            self.color_i = 0
            for k in reader.run_log['k']:
                # if k not in ['0.7', '0.8', '0.9']:
                #     continue
                entr = 'entropy'
                # entr = 'ranking entropy'
                y = reader.iteration_log[k][entr]
                # y = reader.iteration_log[k][]
                x = list(range(1, len(y) + 1))

                if k == best_k:
                    linestyle = '-.'
                    marker = '^'
                else:
                    linestyle = '-'
                    marker = '.'
                linestyle = '-'
                marker = '.'
                color = self.colors[self.color_i]

                plt.plot(x, y, marker=marker, color=color, linestyle=linestyle, label=k)  # uninterpolated

                self.color_i = (self.color_i + 1) % len(self.colors)

            # needs to be called last
            plt.legend(title='k')

            fig.savefig(save_path + reader.run_log['model_id'][0] + "-{}.png".format(entr))
            plt.clf()
        pass

    def plot_initial_entropy_per_corpus_size(self, save_path):

        plt.figure(1)
        plt.xlabel('Corpus size')
        plt.ylabel('Entropy')
        plt.grid(True, which="both")
        for reader in self.readers:
            plt.title("Initial Entropy per corpus size for " + reader.info[c.DATASET] + " ")

            corpus_size = reader.get_corpus_stat('Corpus Size after')

            max_entr, min_entr = [float(x) for x in reader.run_log['k']]
            max_entr_r2, min_entr_r2 = [float(x) for x in reader.run_log['r2']]

            plt.plot(corpus_size, max_entr, marker="x")

        plt.show()

    def entropy_per_k(self, save_path):

        plt.figure(1)
        plt.xlabel('Corpus size')
        plt.ylabel('Entropy')
        plt.grid(True, which="both")
        for reader in self.readers:
            plt.title("Initial Entropy per corpus size for " + reader.info[c.DATASET] + " ")

            x = reader.run_log['k']
            y = [reader.iteration_log[k]['entropy'][0] for k in reader.run_log['k']]

            plt.plot(x, y, marker="x")

            plt.show()

    def plot_compare_entropy_per_iteration(self, save_path, top=3, readers=None):
        '''Plot graph for Quality per time; one graph per k'''
        if readers is None:
            readers = self.readers

        plt.clf()
        for i, reader in enumerate(readers):
            f = plt.figure(1, figsize=(20, 20), dpi=80)
            plt.figure(1)
            plt.title("Entropy per iteration for " + reader.info[c.DATASET] + " " +
                      reader.info[c.TOPIC])
            plt.xlabel('Iteration')
            plt.ylabel('Entropy')
            plt.grid(True, which="both")

            markers = {
                0: ['*', 'x', '.'],
                1: ['.']
            }
            linestyles = {
                0: ['-'],
                1: ['--']
            }

            best_k = reader.add_max(label='k', att='r2', it=10, best_slice=top)
            for num, (k, _) in enumerate(best_k):
                y = reader.iteration_log[k]['entropy']
                x = list(range(1, len(y) + 1))

                marker = markers[i][num % len(markers[i])]
                linestyle = linestyles[i][num % len(linestyles[i])]

                color = self.colors[self.color_i]

                plt.plot(x, y, marker=marker, color=color, linestyle=linestyle, label=k)  # uninterpolated

                self.color_i = (self.color_i + 1) % len(self.colors)

            # needs to be called last
            plt.legend(title='k')

        f.savefig(save_path + reader.run_log['model_id'][0] + "-entr.png")
        pass

    def histogram(self, label='k'):
        vals = [float(x[0]) for x in self.max_values]
        print(sorted(set(vals)))
        # gr = sns.distplot(vals)
        sns.countplot(vals)
        if label is 'r':
            ticks = [float(x) * 25 for x in self.readers[-1].run_log[label]]
        if label is 'k':
            ticks = [float(x) for x in self.readers[-1].run_log[label]]
            # for x in [20, 50]:
            #     ticks.remove(x)
        # gr.set(xticks=ticks)
        plt.show()

        # for key, value in self.categorized_values.items():
        #     vals = [float(x[0]) for x in value]

        #     gr = sns.distplot(vals)
        #     # sns.countplot(vals)
        #     ticks = [float(x) for x in self.readers[-1].run_log[label]]
        #     print(ticks)
        #     for x in [20, 50]:
        #         ticks.remove(x)
        #     gr.set(xticks=ticks)
        #     plt.title(key)
        #     plt.show()

    def correlation(self, stat_labels):
        df = pd.DataFrame()
        best_k = []
        stats = []
        self.corr.sort(key=lambda x: x[1])

        stats = defaultdict(list)
        for entry in self.corr:
            value, *stat = entry
            best_k.append(float(value[0]))
            for label, st in zip(stat_labels, stat):
                stats[label].append(st)

        df['best_k'] = best_k
        for label in stat_labels:
            column = label.replace(' ', '')
            df[column] = stats[label]
        # print(df)
        print(df.corr())

        # print(df.loc[df['best_k'] == 20.0])

        # result = sm.ols(formula="best_k ~ LexicalDiversity", data=df).fit()
        # result = sm.ols(formula="best_k ~ CorpusSize + LDwithoutstopwords + Avgwordspersentence", data=df).fit()
        # print(result.summary())

        # from scipy.stats.stats import pearsonr
        # print(pearsonr(best_k, stats))

    def categorized_correlation(self, stat_labels):
        values = []
        statistics = defaultdict(list)
        for key, entries in sorted(self.categorized_corr.items(), key=lambda x: x[0]):
            for entry in entries:
                value, *stats = entry
                values.append(float(value[0]))
                for label, st in zip(stat_labels, stats):
                    statistics[label].append(st)

            df = pd.DataFrame()
            df['best_k'] = values
            for label in stat_labels:
                column = label.replace(' ', '')
                df[column] = statistics[label]
            print("For", key)
            print(len(entries))
            # print(df)
            print(df.corr())
        pass

    def add_reader(self):
        self.readers.append(MeasurementReader())
        self.info = self.readers[-1].info
        pass

    def read_logs(self, run_log, iteration_log):
        self.readers[-1].read_run_log(run_log)
        self.readers[-1].read_iteration_log(iteration_log)
        self.readers[-1].set_topic_rid()

    def aggregate_data(self):
        self.readers[-1].aggregate_data()
        self.readers[-1].aggregate_iteration_data()

    def get_aggregate_data(self):
        df = pd.DataFrame()
        for reader in self.readers:
            tmp = pd.DataFrame(data=reader.aggregated_iteration_data)
            df = pd.concat([df, tmp], ignore_index=True)
        return df

    def read_corpora_stats(self, stat_folder):
        self.readers[-1].read_corpora_stats(stat_folder)

    def add_max(self, label='r', att='r2', it=10, max_weight=None):
        value = self.readers[-1].add_max(label=label, att=att, it=it, max_weight=max_weight)
        corpus_size = self.readers[-1].get_corpus_stat('Corpus Size')
        if value is not None:
            # 0 - 400
            # 400 - 700
            # 700 - 900
            # 900 - x
            self.max_values.append(value)
            for threshold in [250, 500, 750]:
                if corpus_size <= threshold:
                    self.categorized_values[threshold].append(value)
                    return
            self.categorized_values[1000].append(value)
            return

    def add_correlation_pairs(self, corpus_stat, label='r', att='r2', it=10, max_weight=None):
        if type(corpus_stat) is list:
            stat = [self.readers[-1].get_corpus_stat(cs) for cs in corpus_stat]
        else:
            stat = [self.readers[-1].get_corpus_stat(corpus_stat)]

        value = self.readers[-1].add_max(label=label, att=att, it=it, max_weight=max_weight)
        self.corr.append((value, *stat))

        corpus_size = self.readers[-1].get_corpus_stat('Corpus Size')
        for threshold in [700]:
            if corpus_size <= threshold:
                self.categorized_corr[threshold].append((value, *stat))
                return
        self.categorized_corr[1000].append((value, *stat))

    def entropy_analysis(self, it=1, all_data_points=False):
        self.ent_it, self.ent_points = it, all_data_points
        categories = {
            0: "< 300",
            300: "300-700",
            700: ">700",
            # 700: "700-1000",
            # 1000: "> 1000",
            5000: "for measure",
        }

        thresholds = sorted(categories.keys())

        # for each k, what is the initial/final entropy?
        pair_gen = self.readers[-1].get_k_attribute_pairs(att='entropy', it=it)
        pairs = []
        for pair in pair_gen:
            pairs.append(pair)

        # for best k, what is the initial/final entropy?
        best_k, r2_score = self.readers[-1].add_max(label='k', att='r2', it=10, max_weight=None)
        for k, H in pairs:
            if k == best_k or all_data_points:
                # best_k_pair = pair
                self.H['k'].append([float(x) for x in (k, H)])

                # what initial entropy value results in highest quality?'''
                self.H['r2'].append([r2_score, H])

                # for entropy, what is the best k?

                for i, thr in enumerate(thresholds):
                    comp = self.readers[-1].get_corpus_stat("Corpus Size after")
                    if comp >= thr and comp < thresholds[i + 1]:
                        self.H['cat'][categories[thr]].append([float(x) for x in (k, H)])
                        break

    def write_aggregated_data(self, path):
        df = self.get_aggregate_data()
        df.to_csv(path + "iterations.csv", index=False)
