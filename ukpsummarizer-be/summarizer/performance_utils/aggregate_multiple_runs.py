import os
from mreader import MeasurementReader
from combine_funcs import read_logs
from collections import defaultdict
import pandas as pd

path_base_system = '/home/orkan/Dropbox/sherlock/accept_reject multiple/'
path_ranking = '/home/orkan/Dropbox/sherlock/Relative k multiple/DUC06/'

threshold = 1
path_redundancy_sweep = '/home/orkan/Dropbox/sherlock/Redundancy sweep/thr {}/'.format(threshold)

# path = path_base_system
# path = path_ranking
path = path_redundancy_sweep

data = defaultdict(dict)
count = 0
for folder, subfolders, files in os.walk(path):
    if subfolders:
        continue

    reader = MeasurementReader()
    read_logs(reader, folder)

    model_id = reader.run_log['model_id'][0]
    parent_directory = os.path.basename(os.path.dirname(folder))

    # For loop: get value after 10 iterations
    for k in reader.run_log['k']:
        header = "{}".format(parent_directory)
        if len(reader.run_log['k']) > 1:
            header += "-{}".format(k)
        data[model_id][header] = reader.get_value_at(k, att='r2', it=9)
        data['k'][header] = k

    # for k in reader.run_log['k']:
    #     for i, (r2, t, sents) in enumerate(zip(reader.iteration_log[k]['r2'],
    #                                            reader.iteration_log[k]['t'],
    #                                            reader.iteration_log[k]['sentences'])):
    #         header = "{}-{}-{}".format(parent_directory, k, i)
    #         data[model_id][header] = r2
    #         data['k'][header] = k
    #         data['i'][header] = int(i)
    #         data['t'][header] = t
    #         data['sents'][header] = sents

df = pd.DataFrame(data=data)
df.to_csv(path + "all.csv")
# df.to_csv(path + "iterations.csv")

# import pprint
# pp = pprint.PrettyPrinter(indent=4)
# pp.pprint(data)
