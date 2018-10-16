import matplotlib.pyplot as plt
import pandas as pd

from mreader import MeasurementReader
from combine_funcs import get_sorted_topics, read_logs
import log_constants as c
import seaborn as sns


# Baselines
dataset = "DUC07"
# path = '/home/orkan/Dropbox/sherlock/Ranking-Baseline-alt/' + dataset + '/'
# path = '/home/orkan/Dropbox/sherlock/adaptive k max entr/' + dataset + '/'
# path = '/home/orkan/Dropbox/sherlock/naive adaptive k/' + dataset + '/'

# path = '/home/orkan/Dropbox/sherlock/Time Prediction Accuracy/' + dataset + '/'
path = '/home/orkan/Dropbox/sherlock/WEntropy/' + dataset + '/'

columns = ["k", "t"]

d = {key: [] for key in columns}
df = pd.DataFrame(data=d)

rows = []
print(path)
topics = get_sorted_topics(path)
for topic in topics:
    reader = MeasurementReader()

    read_logs(reader, path, topic)
    # corpus_size = reader.get_corpus_stat('Corpus Size after')

    # t distribution for time prediction accuracy
    for k in reader.run_log['k']:
        for t in reader.iteration_log[k]['t']:
            row = []
            row.append(k)
            row.append(t)
            rows.append(row)

    # k distribution
    # row = []
    # # Baseline hack
    # for k in reader.run_log['k']:
    #     for size in reader.iteration_log[k]['sentences']:
    #         rows.append(size / corpus_size * 100)

df = pd.DataFrame(rows, columns=columns)

print(df)


sns.distplot(df.loc[df['k'] == '100']['t'], label='k = 100')
sns.distplot(df.loc[df['k'] == '200']['t'], label='k = 200')
sns.distplot(df.loc[df['k'] == '500']['t'], label='k = 500', color='forestgreen')
# sns.distplot(df['t'], label='Zielzeit t = 1000ms')
plt.xlabel('Zeit pro Iteration t in ms')
plt.legend()
plt.show()
