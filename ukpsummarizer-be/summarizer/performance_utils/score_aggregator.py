import pandas as pd

from mreader import MeasurementReader
from combine_funcs import get_sorted_topics, read_logs
import log_constants as c

paths = []

# # DUC 2001
# dataset = "DUC01"
# version = "0.2"
# lens = ["L50", "L100", "L200", "L400"]

# # # DUC 2002
# dataset = "DUC02"
# version = "0.2"
# lens = ["L50", "L100", "L200"]

# # DUC 2004
# dataset = "DUC04"
# version = "0.2"
# lens = []

# # DBS
# dataset = "DBS"
# version = "0.2"
# lens = []

# DUC 06
dataset = "DUC06"
version = "1.1"
lens = []

# path = "/home/orkan/Dropbox/sherlock/accept_reject/"
# path = "/home/orkan/Dropbox/sherlock/ilp_feedback/"
path = "/home/orkan/Dropbox/sherlock/active_learning/"
# path = "/home/orkan/Dropbox/sherlock/active_learning2/"

# DUC 06
# dataset = "DUC06"
# version = ""
# path = "/home/orkan/Dropbox/sherlock/Relative k/"

# Baselines
# dataset = "DUC07"
# version = ""
# path = '/home/orkan/Dropbox/sherlock/Ranking-Baseline-alt/'
# path = '/home/orkan/Dropbox/sherlock/adaptive k max entr/'
# path = '/home/orkan/Dropbox/sherlock/naive adaptive k/'
# path = '/home/orkan/Dropbox/sherlock/NB/'

run_bases = [path]

for base in run_bases:
    if not version:
        path = base + dataset + "/"
        paths.append(path)
        break
    for l in lens:
        path = base + "{} {} {}/".format(dataset, version, l)
        paths.append(path)
    if not lens and version:
        path = base + "{} {}/".format(dataset, version)
        paths.append(path)


attributes = ["t", "r1", "r2", "r4"]
columns = ["k"] + attributes

d = {key: [] for key in columns}
df = pd.DataFrame(data=d)

rows = []
for path in paths:
    print(path)
    topics = get_sorted_topics(path)
    for topic in topics:
        reader = MeasurementReader()

        read_logs(reader, path, topic)

        row = []
        if reader.info[c.ITERATIONS] == 10:
            for r in columns:
                row.append(reader.run_log[r])
        else:
            for k in reader.run_log['k']:
                try:
                    row = [int(k)]
                except ValueError:
                    row = [k]
                for r in attributes:
                    try:
                        row.append(reader.iteration_log[k][r][9])
                    except IndexError:
                        row.append(reader.iteration_log[k][r][-1])
                rows.append(row)

df = pd.DataFrame(rows, columns=columns)
print(df.groupby(["k"]).mean().round(4))
# print(df.mean().round(4))
# print(df['r2'].max().round(4))
# print(df['r2'].min().round(4))
# print(df['r2'].quantile(.95))
# print(df['r2'].quantile(.05))
# print(df.groupby(["k"]).max().round(4))
# print(df.groupby(["k"]).min().round(4))
# print(df.groupby(["k"]).quantile(.95).round(4))
# print(df.groupby(["k"]).quantile(.05).round(4))
