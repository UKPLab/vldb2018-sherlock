import sys
import os
import glob
from collections import defaultdict
import matplotlib.pyplot as plt
from mplotter import MeasurementPlotter
from tqdm import tqdm
# num of args
possible_args = {'-h': 'help',
                 '-s': 'show plot afterwards',
                 '-r': 'plot run',
                 '-i': 'plot iterations',
                 '-qt': 'plot quality/time ratio',
                 '-ni': 'plot number of iterations per k',
                 '-len': 'plot summary length per k',
                 '-a': 'one plot for all topics',
                 '-ai': 'one plot for all topics, but not tho',
                 '-ar': 'plot single accept_reject_ratio',
                 '-maxit': 'get number of maximum iteration for each run per k',
                 '-e': 'errorbar of time estimate measurement',
                 '-hist': 'histogram',
                 '-corr': 'correlation',
                 '-entropy': 'entropy',
                 }

# Print help
if '-h' in sys.argv:
    for flag in sorted(possible_args.keys()):
        print("{} --- {}".format(flag, possible_args[flag]))
    exit()

flags = defaultdict(lambda: False)
argv = sys.argv[1:]
path = sys.argv[-1]
# Set flags and count number
for key in possible_args.keys():
    if key in argv:
        flags[key] = True

if not os.path.isdir(path):
    raise OSError("{} not a valid path".format(path))

# determine starting index for folder
# passing multiple folders as parameters plots those in the same

stat_folder = "/home/orkan/acl2017-interactive_summarizer/summarizer/performance_utils/corpora stats/"
mp = MeasurementPlotter()
if flags['-e']:
    mp.boxplot_estimate_time()
    # mp.errorbar_estimate_time()
    exit()

# correlation_stats = ["Corpus Size after", "LD without stopwords after"]
# correlation_stats = ["Corpus Size", "LD without stopwords"]
# correlation_stats = ["Corpus Size", "Corpus Size after", "Lexical Diversity", "Lexical Diversity after"]
correlation_stats = ["Corpus Size", "Concept Size", "Avg words per sentence", "Lexical Diversity",
                     "LD without stopwords", "Corpus Size after", "Concept Size after",
                     "Avg words per sentence after", "Lexical Diversity after", "LD without stopwords after"]
hist_label = 'k'
hist_att = 'r2',
hist_it = 10
max_weight = 25

for folder, subfolders, files in os.walk(path):
    # TODO:
    # determine when in "parent" folder of a run collection
    # if so: new aggregate data objects

    if not glob.glob(folder + "/*-run") or not glob.glob(folder + "/*-iterations"):
        print("skipped.")
        if flags['-a'] or flags['-ai']:
            mp.add_reader()
        continue
    if not flags['-a'] or flags['ai']:
        mp.add_reader()

    if not os.path.isdir(folder):
        raise(OSError("There's no directory named " + folder))

    run_log = glob.glob(folder + "/*-run")[0]
    iteration_log = glob.glob(folder + "/*-iterations")[0]

    # TODO: MODEL IDs
    print(folder)
    mp.read_logs(run_log, iteration_log)
    mp.read_corpora_stats(stat_folder)

    if flags['-a'] or flags['-ai']:
        mp.aggregate_data()
        if flags['-ai']:
            mp.time_per_constraints(folder)
            plt.close('all')
            mp.readers[-1].clear_aggregate_data()

    if flags['-ar']:
        mp.plot_accept_reject_ratio()

    if flags['-r']:
        # mp.plot_run_results(1, folder)
        mp.plot_run_results_quality(folder)

    if flags['-i']:
        plt.clf()
        # for k in mp.measured_k:
        print(mp.readers[-1].run_log['k'])
        for k in mp.readers[-1].run_log['k']:
            # if k in ['0.7', '0.8', '0.9']:
            mp.plot_iteration_results(k, plot_iteration=True)

        mp.save_it_plots(path)

    if flags['-hist']:
        mp.add_max(label='k', att="r2", it=10, max_weight=25)
        pass

    if flags['-corr']:
        # mp.add_correlation_pairs("LD without stopwords after", label='k', att="r2", it=10, max_weight=25)
        # mp.add_correlation_pairs("Avg words per sentence", label='k', att="r2", it=10, max_weight=25)
        mp.add_correlation_pairs(correlation_stats, label='k', att="r2", it=10, max_weight=25)

    if flags['-entropy']:
        mp.entropy_analysis(it=1, all_data_points=False)

    if flags['-qt']:
        mp.plot_qt_ratio_per_k(folder)

    if flags['-len']:
        mp.plot_summary_length_per_k()
        pass

    if flags['-ni']:
        mp.plot_number_of_iterations_per_k()
        pass

    if flags['-maxit']:
        mp.get_number_of_iterations()

if flags['-hist']:
    mp.histogram(label='k')

if flags['-corr']:
    mp.correlation(correlation_stats)
    # mp.categorized_correlation(correlation_stats)

if flags['-entropy']:
    # mp.best_k_entropy_boxplot(path)
    mp.plot_entropy_per_iteration(path)
    # mp.entropy_quality_scatterplot(path)
    # mp.plot_initial_entropy_per_corpus_size(path)
    # mp.entropy_per_k(path)

if flags['-a']:
    # mp.boxplot_number_of_iterations(path)
    # mp.boxplot_avg_time_per_iteration(path)
    mp.time_per_constraints(path)
    # mp.time_per_parameter(path)
    # mp.accept_reject_ratio(path)
    # mp.write_aggregated_data(path)
    # mp.regression(path)
    pass

if flags['-maxit']:
    mp.maxit_to_csv(path)

if flags['-s']:
    plt.show()
