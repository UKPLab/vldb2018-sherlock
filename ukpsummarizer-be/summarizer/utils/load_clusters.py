import os
from collections import defaultdict


def get_clusters(path, data_set):
    print(path)
    base_dir = os.path.normpath(os.path.join(path, data_set)) + '/'
    print(base_dir)
    files = sorted(os.listdir(base_dir))

    clusters = defaultdict(dict)
    for file in files:
        with open(base_dir + file, 'r') as f:
            lines = f.read()
            for line in lines.split('\n')[1:]:
                if not line:
                    break
                doc_id, sent_pos, cluster = [int(el) for el in line.split(',')]
                clusters[file][(doc_id, sent_pos)] = cluster
    return clusters
