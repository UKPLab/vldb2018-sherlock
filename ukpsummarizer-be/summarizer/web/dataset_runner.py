import json
import os
import re
from os import path

from utils.data_helpers import load_w2v_embeddings
from web.single_iteration_runner import SingleTopicRunner


class DatasetRunner(object):
    def __init__(self, iobasedir, rouge_dir, out="/tmp/afile.json"):
        self.iobasedir = iobasedir
        # resolved_rouge_dir = path.normpath(path.expanduser(rouge_dir))
        self.rouge_dir = rouge_dir
        self.out = out

    def run(self, dataset_path, size=250, summarizer="SUME", summary_idx=None, parser=None,
            oracle="accept", weights_file=None, propagation=False, max_iteration_count=25):

        # relativize the dataset_path!
        if dataset_path.startswith("/"):
            relative_path = re.search('^(/)(.*)$', dataset_path).group(2)
        else:
            relative_path = dataset_path
        ds = path.join(self.iobasedir, relative_path)

        dataset_info = json.load(open(path.normpath(path.join(ds, "index.json"))))

        embeddings_path = path.normpath(path.join(self.iobasedir, "embeddings"))
        if propagation is True:
            embeddings = load_w2v_embeddings(embeddings_path, dataset_info["language"], "active_learning")
        else:
            embeddings = load_w2v_embeddings(embeddings_path, dataset_info["language"], oracle)

        (out_dir, out_file) = path.split(self.out)
        for file_name in os.listdir(ds):
            if not os.path.isdir(path.normpath(path.join(ds, file_name))):
                continue
            topic = relative_path + "/" + file_name

            sir = SingleTopicRunner(self.iobasedir, self.rouge_dir, path.join(out_dir, topic + ".json"))
            sir.run(topic_path=topic, size=size, summarizer=summarizer, summary_idx=summary_idx,
                    parser=parser, oracle=oracle, feedback_log=weights_file,
                    max_iteration_count=max_iteration_count, preload_embeddings=embeddings)
