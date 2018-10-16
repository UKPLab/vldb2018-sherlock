from mreader import MeasurementReader
from mplotter import MeasurementPlotter
from result_combiner import ResultCombiner
from collections import defaultdict
from tqdm import tqdm as bar

from combine_funcs import (read_logs, plot_summary_lengths_per_k, get_hu_k, plot_k_distribution,
                           plot_iteration, count_best_k_entropy, beat_counts_vanilla_best_k,
                           plot_iteration_combination_02_and_11, count_best_ks, plot_bar_chart,
                           compute_mse, compute_mse_best_k, plot_error_step_graph, compute_mse_error_steps,
                           plot_outliers, has_same_gold_standard)

import matplotlib.pyplot as plt

dataset = "DUC06"
length = " L400"

paths = {
    '0_2': '/home/orkan/Dropbox/sherlock/accept_reject/' + dataset + ' 0.2/',
    '1_1': '/home/orkan/Dropbox/sherlock/accept_reject/' + dataset + ' 1.1/',

    '0_2_length': '/home/orkan/Dropbox/sherlock/accept_reject/' + dataset + ' 0.2' + length + '/',
    '1_1_length': '/home/orkan/Dropbox/sherlock/accept_reject/' + dataset + ' 1.1' + length + '/',

    'standard_entropy_1_1': '/home/orkan/Dropbox/sherlock/Entropy/' + dataset + '/',
    'weighted_entropy_1_1': '/home/orkan/Dropbox/sherlock/WEntropy/' + dataset + '/',

    'entropy_on_summary_BL': '/home/orkan/Dropbox/sherlock/NB/' + dataset + '/',
    'entropy_on_ranking_BL': '/home/orkan/Dropbox/sherlock/Ranking-Baseline-alt/' + dataset + '/',
    'adaptive_k_entropy_BL': '/home/orkan/Dropbox/sherlock/adaptive k max entr/' + dataset + '/',
    'adaptive_k_window': '/home/orkan/Dropbox/sherlock/naive adaptive k/' + dataset + '/',
    'adaptive_k_window_timelimit': '/home/orkan/Dropbox/sherlock/limited naive adaptive k/' + dataset + '/',
    'relative_k': '/home/orkan/Dropbox/sherlock/Relative k/' + dataset + '/',
    'relative_k_2': '/home/orkan/Dropbox/sherlock/Relative k 2/' + dataset + '/',
    'time_predict': '/home/orkan/Dropbox/sherlock/Time Prediction Accuracy/' + dataset + '/',
    'boudin': '/home/orkan/Dropbox/sherlock/boudin/' + dataset + '/',
}

# save_path = "/home/orkan/Dropbox/measurements/sherlock/adaptive k max entr/" # + dataset + " qit/"
save_path = "/home/orkan/Dropbox/sherlock/"

path1, path2 = paths['0_2'], paths['boudin']

path2 = '/home/orkan/Dropbox/measurements/11-07-2018 12:04/'

# combiner = ResultCombiner(to_compare=paths['0_2'], compare_base=paths['weighted_entropy_1_1'])
combiner = ResultCombiner(to_compare=path1, compare_base=path2)
topics = combiner.get_topics()
num_topics = combiner.get_number_of_topics()
model_summaries = []

# combiner.execute_routine('get diffs')
# combiner.execute_routine('get results')
# combiner.execute_routine('winner histogram')
# exit()
tie_breaker = False

# ############################################
baseline_values = []
cherry_picks = []
k_wise_values = defaultdict(list)
beat_counts = 0

best_k_high_H = []

mp = MeasurementPlotter()

sorted_results = defaultdict(list)
hu_ks = []
relative_ks = []
errors = defaultdict(lambda: defaultdict(list))
best_k_errors = defaultdict(list)
accumulate_errors = defaultdict(lambda: defaultdict(list))

for folder in combiner.get_topics():
    readers = [MeasurementReader() for i in range(0, 2)]
    paths = [path1, path2]

    for reader, path in zip(readers, paths):
        read_logs(reader, path, folder)

    # if not has_same_gold_standard(readers):
    #     raise("WTF!")

    model_summaries.append(readers[1].run_log['model_id'][0])
    # plt.clf()
    # Combined Iteration plots
    # plot_iteration(mp, readers, save_path)
    # if folder in ['D0631D_2', 'D0616G_1']:
    # if folder in ['D0639C_1', 'D0616G_1']:
        # continue

    # if folder in ['D0631D_2', 'D0645I_0']:
        # continue

    # Combined Entropy plots
    # mp.plot_compare_entropy_per_iteration(save_path, readers=readers, top=3)

    # Count highest entropy for best k
    # count_best_k_entropy(readers, best_k_high_H)

    # Beat count Vanilla vs Best k
    # beat_counts += beat_counts_vanilla_best_k(readers, baseline_values, cherry_picks)

    # plot_summary_lengths_per_k(readers)

    # Best k bar plots
    # best_ks = count_best_ks(readers, tie_breaker=tie_breaker, base_only=True)
    # for best_k in best_ks:
    #     relative_ks.append(best_k)


    # compute error
    compute_mse(readers, errors)
    label, diff = compute_mse_best_k(readers)
    # best_k_errors[label].append(diff)

    # for step function
    compute_mse_error_steps(readers, accumulate_errors)

    # distplot
    # get_hu_k(readers, hu_ks)

# plot_k_distribution(hu_ks)
plt.show()

if best_k_high_H:
    rank_count = defaultdict(lambda: 0)
    for r in best_k_high_H:
        rank_count[r] += 1

    print(rank_count)
    for key in sorted(rank_count.keys()):
        print(key)
        summe = 0
        for i in range(1, key + 1):
            summe += rank_count[i]
        print(summe)
        print("---")

if beat_counts:
    print("Beaten: {} of {} times".format(beat_counts, len(topics)))
    print("Averages")
    print("Baseline:", sum(baseline_values) / len(baseline_values))
    print(len(baseline_values))
    print("Cherry Pick Ranked:", sum(cherry_picks) / len(cherry_picks))
    for k, v in sorted(k_wise_values.items(), key=lambda x: int(x[0])):
        print(k, sum(v) / len(v))

if relative_ks:
    plot_bar_chart(relative_ks, num_topics, dataset, tie_breaker)

if errors:
    # new method hack
    new_method = defaultdict(lambda: 0)
    lens = defaultdict(lambda: 0)
    for k, d in sorted(errors.items()):
        for label, values in d.items():
            mse = sum(values) / len(values)
            new_method[label] += sum(values)
            lens[label] += len(values)
            print(k, label, "%.4f" % mse, len(values))
        print()
    print("---")
    for label, mse in new_method.items():
        print(label, lens[label])
        print(label, mse / lens[label])

if accumulate_errors:
    k_to_diffs = defaultdict(list)
    for k, d in sorted(accumulate_errors.items()):
        for label, values in sorted(d.items(), reverse=True):
            # print(label, values)
            # plot_error_step_graph(k, label, values, dataset)
            max_index = values.index(max(values))
            print(k, label, max_index, combiner.get_topics()[max_index])
            print("max_index diff:", values[max_index])

            if not k_to_diffs[k]:
                k_to_diffs[k] = values[:]
            else:
                for i, val in enumerate(values):
                    k_to_diffs[k][i] = k_to_diffs[k][i] - val
            # bin_only = [val for val in values if val != 0 and val <= 0.2]
            # print(len(bin_only))
            # print(sum(bin_only) / len(bin_only))
        print()

    # plot the outlier things
    # Topic + Gold Standard Summary
    #i have for each k -> diffs

    x = [_id[:5] + _id[-3:] for _id in model_summaries]
    x = model_summaries

    # new method hack
    diffs = []
    for k, vals in k_to_diffs.items():
        diffs += vals

    print(diffs)
    plot_outliers(dataset, 'none', x, diffs)
    exit()
    for k, diffs in sorted(k_to_diffs.items()):
        plot_outliers(dataset, k, x, diffs)

if best_k_errors:
    for label, values in best_k_errors.items():
        mse = sum(values) / len(values)
        print(label, "%.4f" % mse, len(values))
