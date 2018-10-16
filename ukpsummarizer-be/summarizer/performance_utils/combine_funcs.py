import os
import glob
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import numpy as np

stat_folder = "/home/orkan/acl2017-interactive_summarizer/summarizer/performance_utils/corpora stats/"


def get_sorted_topics(path):
    for folder, subfolders, files in os.walk(path):
        return sorted(subfolders)


def read_logs(reader, path, folder=''):
    try:
        run_log = glob.glob(path + folder + "/*-run")[0]
        iteration_log = glob.glob(path + folder + "/*-iterations")[0]
    except IndexError as e:
        print("\n", e, path, folder)
    reader.read_run_log(run_log)
    reader.read_iteration_log(iteration_log)
    reader.read_corpora_stats(stat_folder)
    reader.set_topic_rid()


def has_same_gold_standard(readers):
    return readers[0].run_log['model_id'][0] == readers[1].run_log['model_id'][0]

def plot_summary_lengths_per_k(readers):
    _, baseline_reader = readers

    hu_k, hl_k = [float(x) for x in baseline_reader.run_log['k']]
    hu_len, hl_len = [float(x) for x in baseline_reader.run_log['summary length']]

    plt.plot(hu_k, hu_len, color="blue", marker=".")
    plt.plot(hl_k, hl_len, color="blue", marker=".")

def get_hu_k(readers, hu_ks):
    _, baseline_reader = readers
    hu_ks.append(float(baseline_reader.run_log['k'][1]))

def plot_k_distribution(ks):
    f = sns.distplot(ks)
    f.set(xticks=range(0, int(max(ks)) + 51, 50))
    plt.show()

def plot_iteration(mp, readers, save_path):
    for reader in readers:
        reader.read_corpora_stats(stat_folder)
        reader.set_topic_rid()

    standard_reader, baseline_reader = readers
    corpus_size = standard_reader.get_corpus_stat("Corpus Size")
    best_k = standard_reader.add_max(label='k', att='r2', it=10, best_slice=3)

    plot_iteration = True
    for k, _ in best_k:
        mp.info = standard_reader.info
        mp.plot_iteration_results(k, data=standard_reader.iteration_log, plot_iteration=plot_iteration, corpus_size=corpus_size)

    # colour = ['forestgreen', 'salmon']
    best_k = baseline_reader.add_max(label='k', att='r2', it=10, best_slice=3)
    max_entr, min_entr = baseline_reader.run_log['k']
    for i, (k, _) in enumerate(best_k):
        if k == max_entr:
            colour = 'forestgreen'
        elif k == min_entr:
            colour = 'salmon'
        else:
            raise(ValueError('Something is not right here'))
        mp.plot_iteration_results(k, data=baseline_reader.iteration_log, marker='v',
                                  linestyle="--", plot_iteration=plot_iteration, corpus_size=corpus_size)
        score = baseline_reader.iteration_log[k]['r2'][-1]
        mp.plot_upperbound(score, colour)

    mp.readers.append(standard_reader)
    mp.save_it_plots(save_path)

def plot_iteration_combination_02_and_11(mp, readers, save_path):
    plt.clf()
    reader02, reader11 = readers
    mp.readers.append(reader02)
    mp.info = reader02.info
    k = reader02.run_log['k'][0]
    mp.plot_iteration_results(k, data=reader02.iteration_log, marker='.',
                              linestyle="--", cut=10)

    mp.readers.append(reader11)
    mp.info = reader11.info
    for k in reader11.run_log['k'][2:-1]:
        mp.plot_iteration_results(k, data=reader11.iteration_log, marker='.',
                                  linestyle="-")
    mp.save_it_plots(save_path)


def count_best_k_entropy(readers, entropy_rank):
    standard_reader, _ = readers
    best_k = standard_reader.add_max(label='k', att='r2', it=10, best_slice=1)[0]

    start = 1
    if standard_reader.info['dataset'] == 'DUC2004':
        start = 0
    entropies = []
    for k in standard_reader.run_log['k'][start:]:
        h = standard_reader.iteration_log[k]['entropy'][-1]
        entropies.append((k, h))

    entropies.sort(key=lambda x: -x[1])

    for i, (k, h) in enumerate(entropies, 1):
        if k == best_k:
            entropy_rank.append(i)
            break


def beat_counts_vanilla_best_k(readers, baseline_values, cherry_picks):
    iteration = 10
    beat_count = 0
    orig_reader, rank_reader = readers

    measured_iterations = len(orig_reader.iteration_log['original']['r2'])
    # if there are less than 10 iterations, get the maximum
    if measured_iterations < iteration:
        # print(iteration, measured_iterations)
        iteration = measured_iterations

    # Base system score
    baseline = orig_reader.iteration_log['original']['r2'][iteration - 1]
    baseline_values.append(baseline)
    beaten = False

    for k in rank_reader.iteration_log.keys():
        # iteration = measured_iterations
        iteration = 10
        ranked_measured_iteration = len(rank_reader.iteration_log[k]['r2'])
        if ranked_measured_iteration < iteration:
            # print(iteration, ranked_measured_iteration)
            iteration = ranked_measured_iteration
            # continue
        val = rank_reader.iteration_log[k]['r2'][iteration - 1]
        k_wise_values[k].append(val)
        if baseline <= rank_reader.iteration_log[k]['r2'][iteration - 1]:
            beaten = True
            # break
    if beaten:
        beat_count = 1

    # Cherry pick
    cherry_picks.append(max([val for k in rank_reader.iteration_log.keys() for val in rank_reader.iteration_log[k]['r2'][:iteration]]))

    # print(folder, "\t", beaten, "\t", iteration, "\t", val, "\t", baseline)
    return beat_count


def count_best_ks(readers, tie_breaker=True, base_only=False):
    to_compare_reader, comp_base_reader = readers
    manual_ks = []

    # Baseline
    # Ranking baseline: [0] -> max entr, [1] -> min entr
    k = to_compare_reader.run_log['k'][0]
    # score = to_compare_reader.get_value_at(k, att='r2', it=9)

    # Window strategy: [2] -> +0.5
    # k = to_compare_reader.run_log['k'][2]
    score = to_compare_reader.get_value_at(k, att='r2', it=9)

    if not base_only:
        manual_ks.append(("0.0", score))

    # Manual k
    for k in comp_base_reader.run_log['k']:
        r2 = comp_base_reader.get_value_at(k, att='r2', it=9)
        manual_ks.append((k, r2))
    manual_ks.sort(key=lambda x: (-x[1], x[0]))

    if manual_ks[0][0] == "0.0":
        print(manual_ks)
    # exit()
    best_ks = []
    for i, (k, r2) in enumerate(manual_ks):
        try:
            best_ks.append(float(k))
        except ValueError:
            best_ks.append(k)
        if r2 == manual_ks[i + 1][1]:
            continue
        else:
            break
    if tie_breaker:
        return [best_ks[0]]
    else:
        return best_ks
    # if best_k_score >= score:
    #     return [float(best_k)]
    # else:
    #     return ["BASELINE"]
    # # return [float(best_k)]


def plot_bar_chart(relative_ks, num_topics, dataset, tie_breaker):
    bar_number = 10
    categorized = {i / bar_number: [] for i in range(1, bar_number + 1)}
    baseline_label = "    Window strategy"
    baseline_index = 100
    categorized[baseline_index] = []
    counts = {"label": [], "count": []}
    for k in relative_ks:
        for key in sorted(categorized.keys()):
            if k == 0.0:
                categorized[baseline_index].append(k)
                break
            if k == key:
                categorized[key].append(k)
                break
    keys = list(sorted(categorized.keys()))
    for i, key in enumerate(sorted(categorized.keys())):
        if key == baseline_index:
            label = baseline_label
        else:
            label = "{} * |D|".format(key)
        if len(categorized[key]) / num_topics != 0:
            counts["label"].append(label)
            counts["count"].append(len(categorized[key]) / num_topics)

    df = pd.DataFrame(data=counts)
    print(df)
    ax = sns.barplot(x="label", y="count", data=counts)
    # plt.title("How often is score of baseline 'initial highest entropy' \n>= score of base system, on {}".format(dataset))
    # plt.title("How often is score of baseline 'initial highest entropy' >= score of best manual k, on {}".format(dataset))
    # plt.title("How often is R2 score of static entropy baseline >= R2 score of best manual k, on {}".format(dataset))
    tie_label = "no tie breaker"
    if tie_breaker:
        # tie_label = 'tie breaker: r2(baseline) >= r2(manual k), then smaller k'
        tie_label = 'tie breaker: smaller k'
    plt.title('''Which manual set k (relative to corpus size) performs best, on {}
              \n 10 iterations, {}'''.format(dataset, tie_label))
    # plt.xlabel('Interval in which (k / corpus size) is')
    plt.ylabel('Frequency k performs best')
    plt.xlabel('k, set manually')
    # plt.ylabel('Frequency a strategy performs best')
    plt.show()


def compute_mse(readers, errors):
    base_system_reader, topk_reader = readers

    r2_base = base_system_reader.get_value_at('original', att='r2', it=9)
    baseline_labels = ["max", "min"]
    # baseline_labels = ["1s"]
    # for k, baseline in zip(topk_reader.run_log['k'], baseline_labels):
    for k in topk_reader.run_log['k']:
        r2 = topk_reader.get_value_at(k, att='r2', it=9)
        label = 'decline'

        try:
            diff = (r2 - r2_base) / r2_base
        except ZeroDivisionError:
            r2s = [x for x in base_system_reader.iteration_log['original']['r2'] if x > 0]
            r2_base = r2s[-1]
            diff = (r2 - r2_base) / r2_base
        if diff > 0:
            label = 'improve'
        elif diff == 0:
            label = 'equal'

        errors[k][label].append(diff**2)
        # errors[baseline][label].append(diff**2)

    # print(diff)
    return

def compute_mse_best_k(readers):
    base_system_reader, topk_reader = readers
    manual_ks = []
    label = 'decline'

    r2_base = base_system_reader.get_value_at('original', att='r2', it=9)
    for k in topk_reader.run_log['k']:
        r2 = topk_reader.get_value_at(k, att='r2', it=9)
        manual_ks.append((k, r2))
    manual_ks.sort(key=lambda x: (-x[1], x[0]))

    try:
        diff = (manual_ks[0][1] - r2_base) / r2_base
    except ZeroDivisionError:
        r2s = [x for x in base_system_reader.iteration_log['original']['r2'] if x > 0]
        r2_base = r2s[-1]
        diff = (manual_ks[0][1] - r2_base) / r2_base
    if diff > 0:
        label = 'improve'
        # print("__")

    # print(diff)

    return label, diff**2


def compute_mse_error_steps(readers, acc_errors):
    base_system_reader, topk_reader = readers

    r2_base = base_system_reader.get_value_at('original', att='r2', it=9)
    # baseline_labels = ["max", "min"]
    # baseline_labels = ["1s"]
    # for k, baseline in zip(topk_reader.run_log['k'], baseline_labels):
    for k in topk_reader.run_log['k']:
        r2 = topk_reader.get_value_at(k, att='r2', it=9)
        try:
            diff = (r2 - r2_base) / r2_base
        except ZeroDivisionError:
            r2s = [x for x in base_system_reader.iteration_log['original']['r2'] if x > 0]
            r2_base = r2s[-1]
            diff = (r2 - r2_base) / r2_base
        if diff > 0:
            improvement = diff**2
            decline = 0
        elif diff == 0:
            improvement = 0
            decline = 0
        else:
            improvement = 0
            decline = diff**2

        acc_errors[k]['improve'].append(improvement)
        acc_errors[k]['decline'].append(decline)
    return


def plot_error_step_graph(k, label, values, dataset):
    xi = 0.0
    yi = 0
    x = []
    y = []
    fig = plt.figure(1, figsize=(12, 8), dpi=80)
    for delta in values:
        xi += delta
        yi += 1
        x.append(xi)
        y.append(yi)

    if label == 'improve':
        color = 'green'
        title = 'Improvement'
    else:
        color = 'red'
        title = 'Decline'

    plt.xlabel(label)
    plt.ylabel('# of clusters')
    plt.grid(True, which="both")
    # TODO Dataset
    plt.title("{} - Sum over each cluster on {} for k = {}".format(title, dataset, k))
    plt.plot(x, y, label=label, color=color)  # uninterpolated

    plt.legend()
    filename = '{}-{}.png'.format(label, k)
    fig.savefig('/home/orkan/Dropbox/sherlock/Relative k/Ausreisser/' + dataset + '/' + filename)
    plt.clf()

def plot_outliers(dataset, k, x, y, color='blue', linestyle='-'):
    fig = plt.figure(1, figsize=(50, 14), dpi=80)
    plt.figure(1)

    plt.xlabel('Topic.Model_Summary')
    plt.ylabel('MSE on cluster')
    plt.grid(True, which="both")
    # TODO Dataset
    plt.title("MSE, compared to base system, of each cluster on {} for k = {}".format(dataset, k))
    plt.plot(range(len(x)), y, label=k, color=color, linestyle=linestyle)

    plt.xticks(np.arange(len(x)), x, rotation='vertical')
    plt.legend()
    filename = '{}-{}.png'.format('outliers', k)
    # fig.savefig('/home/orkan/Dropbox/sherlock/Relative k/Ausreisser/' + dataset + '/' + filename)
    fig.savefig('/home/orkan/Dropbox/sherlock/Relative k multiple/' + dataset + '/' + filename)
    # plt.show()
    # plt.clf()
