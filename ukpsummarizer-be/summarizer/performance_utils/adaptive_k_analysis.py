from tqdm import tqdm as bar
from mreader import MeasurementReader
from combine_funcs import get_sorted_topics, read_logs
import log_constants as c

stat_folder = "/home/orkan/acl2017-interactive_summarizer/summarizer/performance_utils/corpora stats/"

dataset = "DUC06"
# path = "/home/orkan/Dropbox/measurements/sherlock/naive adaptive k/" + dataset + "/"
path = "/home/orkan/Dropbox/measurements/sherlock/naive adaptive k 2/" + dataset + "/"

topics = get_sorted_topics(path)

labels = ["0", "0.05", "0.1", "0.15", "0.2", "0.25"]
# labels = ["0", "0.25", "0.5", "0.75", "1"]
data = {r: {'k': [], 't': [], 'k_norm': [], 'no_of_iterations': []} for r in labels}
atts = ['k', 't']

iterations = 10
##############################################################

for topic in bar(topics):
    reader = MeasurementReader()

    if topic.startswith("D0611B"):
        continue
    # k Bereich pro adaptive k
    # durchschn. k pro adaptive k
    # zeit pro Iteration, durchschn.

    read_logs(reader, path, topic)

    reader.read_corpora_stats(stat_folder)
    reader.set_topic_rid()
    corpus_size = reader.get_corpus_stat("Corpus Size after")
    for k in reader.run_log['k']:
        for att in atts:
            [data[k][att].append(datapoint) for datapoint in reader.iteration_log[k][att][:iterations]]

        for datapoint in reader.iteration_log[k]['k']:
            # An Korpusgröße normalisiertes k
            entry = datapoint / corpus_size
            if entry > 1:
                entry = 1
            [data[k]['k_norm'].append(entry) for datapoint in reader.iteration_log[k]['k'][:iterations]]

        data[k]['no_of_iterations'].append(len(reader.iteration_log[k]['k']))


print("")
for key, val_dict in sorted(data.items(), key=lambda x: float(x[0])):
    print(key)
    for att, values in val_dict.items():
        avg = sum(values) / len(values)
        print("--", att, "--")
        print("Avg.:", avg)
        print("Min.:", min(values))
        print("Max.:", max(values))
