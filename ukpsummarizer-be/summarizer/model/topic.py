import codecs
import json
import os
from os import path


class Topic(object):
    """
    A topic is expected to reside as a subdirectory within a dataset.
    """
    docs_directory_name = "docs"
    models_directory_name = "summaries"

    parsed_docs_directory_name = "docs.parsed"
    parsed_models_directory_name = "summaries.parsed"

    language_descriptor = "../index.json"
    task_filename = "task.json"

    def __init__(self, base_path):
        if not path.isdir(base_path):
            raise BaseException("base_path %s is not a directory (or not existing)" % base_path)
        if not path.isdir(path.join(base_path, Topic.docs_directory_name)):
            raise BaseException("%s subdirectory is missing" % (path.join(base_path, Topic.docs_directory_name)))
        if not path.isdir(path.join(base_path, Topic.models_directory_name)):
            raise BaseException("%s subdirectory is missing" % (path.join(base_path, Topic.models_directory_name)))

        self.base_path = base_path
        self.docs_dir = path.join(self.base_path, Topic.docs_directory_name)
        self.models_dir = path.join(self.base_path, Topic.models_directory_name)
        self.parsed_docs_dir = path.join(self.base_path, Topic.parsed_docs_directory_name)
        self.parsed_models_dir = path.join(self.base_path, Topic.parsed_models_directory_name)
        self.dataset_info = json.load(open(path.join(self.base_path, Topic.language_descriptor)))

    def get_name(self):
        (_, f) = path.split(self.base_path)
        return f

    def get_docs(self, parsed=False):
        if parsed:
            return self.read(self.parsed_docs_dir)
        return self.read(self.docs_dir)

    def get_models(self, parsed=False):
        if parsed:
            return self.read(self.parsed_models_dir)
        return self.read(self.models_dir)

    def get_language(self):
        return self.dataset_info["language"]

    def get_dataset(self):
        return self.dataset_info["dataset"]

    @staticmethod
    def read(location):
        """

        :param location:
        :return: list[tuple[str, list[str]]]: list of tuples with file -> sentences[]
        """
        documents = []
        for file_name in os.listdir(location):
            absolute_file = path.normpath(path.join(location, file_name))
            with codecs.open(absolute_file, 'r', 'utf-8') as fp:
                text = fp.read().splitlines()
            documents.append((absolute_file, text))
        return documents

    def get_parse_info(self, m_idx):
        parse_docs = self.read(self.parsed_docs_dir)
        parse_models = self.read(self.parsed_models_dir)
        return [parse_docs, [parse_models[m_idx]]]

    def get_summary_size(self):
        return self.dataset_info["summary_length"]
