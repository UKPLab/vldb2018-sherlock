from __future__ import print_function, unicode_literals

import json

import os
from os import path

from model.topic import Topic


class DataSet:
    def __init__(self, location):
        if not os.path.isdir(location):
            raise ValueError("directory location not resolveable")

        self.root = location

        try:
            self.dataset_info = json.load(open(path.normpath(path.join(self.root, "index.json"))))
        except:
            raise ValueError("Error while trying to parse dataset info from %s " % (
                path.normpath(path.join(self.root, "index.json"))))

    def get_language(self):
        if not self.dataset_info.has_key("language"):
            return "unknown"
        return self.dataset_info["language"]

    def get_name(self):
        if not self.dataset_info.has_key("dataset"):
            return "unknown_data_set"

        return self.dataset_info["dataset"]

    def get_summary_size(self):
        return self.dataset_info.get("summary_length")

    def get_topics(self):
        for file_name in os.listdir(self.root):
            topic_location = path.normpath(path.join(self.root, file_name))
            if not os.path.isdir(topic_location):
                continue
            yield Topic(topic_location)
