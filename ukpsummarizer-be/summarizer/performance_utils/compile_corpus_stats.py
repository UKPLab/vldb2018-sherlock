import pandas as pd


def compile_corpora_stats(path, dataset):
    df = pd.read_csv(path + dataset, sep="|")
    stats = {}
    size = "Corpus Size"
    stats['Number of Topics'] = df[size].count()
    stats['Number of Documents'] = df["Number of documents"].unique()
    stats['max_size'] = df[size].max()
    stats['median'] = df[size].median()
    stats['min_size'] = df[size].min()
    stats['mean'] = df[size].mean()

    print("dataset", ":", dataset)
    for k, v in stats.items():
        print(k, ":", v)


path = "/home/orkan/acl2017-interactive_summarizer/summarizer/performance_utils/corpora stats/"
dataset = "DUC2007"

compile_corpora_stats(path, dataset)
