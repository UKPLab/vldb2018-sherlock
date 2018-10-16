import hashlib
import itertools
import logging
import os
import re
from os import path
from random import random

from nltk import SnowballStemmer

from algorithms.feedback.ConceptEmbedder import ConceptEmbedder
from algorithms.feedback.SimpleNgramFeedbackGraph import SimpleNgramFeedbackGraph
from algorithms.feedback.WordEmbeddingGaussianFeedbackGraph import WordEmbeddingGaussianFeedbackGraph
from algorithms.feedback.WordEmbeddingRandomWalkDiffusionFeedbackGraph import \
    WordEmbeddingRandomWalkDiffusionFeedbackGraph
from model.topic import Topic
from utils.data_helpers import load_w2v_embeddings
from web.single_iteration_runner import SingleTopicRunner


class GridSearch(object):
    def __init__(self, iobasedir, rouge_dir, scores_dir="C:\\users\\hatieke\\.ukpsummarizer\\grid_scores"):
        self.iobasedir = path.normpath(path.expanduser(iobasedir))
        # resolved_rouge_dir = path.normpath(path.expanduser(rouge_dir))
        self.rouge = rouge_dir
        self.scores_dir = path.normpath(path.join(self.iobasedir, "scores_grid"))
        if not path.exists(self.scores_dir):
            os.mkdir(self.scores_dir)

        self.embeddings = {
            "english": None,
            "german": None
        }

    def run(self, topic_path, size=None, max_iteration_count=25):
        log = logging.getLogger("GridSearch")

        interpretation_types = [
            'SimpleNgramFeedbackGraph',
            'WordEmbeddingGaussianFeedbackGraph',
            'BaselineFeedbackStore',
            'WordEmbeddingRandomWalkDiffusionFeedbackGraph',
            #            'WordEmbeddingEgoPrFeedbackGraph',
            #            'PageRankFeedbackGraph',
        ]

        random.shuffle(interpretation_types)

        if topic_path.startswith("/"):
            relative_path = re.search('^(/)(.*)$', topic_path).group(2)
        else:
            relative_path = topic_path

        topic = Topic(path.join(self.iobasedir, path.normpath(relative_path)))

        embeddings = self.__get_embeddings__(topic.get_language())
        run_id = hashlib.sha224(topic_path).hexdigest()
        outputdir = path.join(self.scores_dir, run_id)
        try:
            os.mkdir(outputdir)
        except:
            pass

        concept_embedder = ConceptEmbedder(embeddings)
        for itype in interpretation_types:
            if itype == 'WordEmbeddingGaussianFeedbackGraph':
                mass_reject = [4.0, 1.0, 0.0, -1.0, -4.0]
                mass_accept = [4.0, 1.0, 0.0, -1.0, -4.0]
                iterations_accept = [16, 128, 1024]
                iterations_reject = [2, 4, 8, 16, 64]
                cut_off_threshold = [0.998, 0.98, 0.9, 0.6, 0.4]

                combinations = list(itertools.product(mass_reject, mass_accept, iterations_accept,
                                                      iterations_reject, cut_off_threshold))
                random.shuffle(combinations)

                for (mr, ma, ia, ir, co) in combinations:
                    log.info("WordEmbeddingGaussianFeedbackGraph: %s %s %s %s %s" % (mr, ma, ia, ir, co))
                    g = WordEmbeddingGaussianFeedbackGraph(concept_embedder,
                                                           cut_off_threshold=co,
                                                           mass_reject=mr,
                                                           mass_accept=ma,
                                                           iterations_reject=ir,
                                                           iterations_accept=ia)

                    sir = SingleTopicRunner(self.iobasedir, self.rouge, scores_dir=outputdir)
                    sir.run(topic_path, size, feedbackstore=g, summarizer="PROPAGATION", preload_embeddings=embeddings)

            elif itype == 'WordEmbeddingRandomWalkDiffusionFeedbackGraph':
                mass_reject = [4.0, 1.0, 0.0, -1.0, -4.0]
                mass_accept = [4.0, 1.0, 0.0, -1.0, -4.0]
                iterations_accept = [128, 1024, 10000]
                iterations_reject = [64, 200, 5000]
                cut_off_threshold = [0.998, 0.98, 0.9, 0.6, 0.4]
                propagation_abort_threshold = [0.01, 0.1, 0.25, 0.5, 0.75, 0.9]

                combinations = list(itertools.product(mass_reject, mass_accept, iterations_accept,
                                                      iterations_reject, cut_off_threshold,
                                                      propagation_abort_threshold))
                random.shuffle(combinations)

                for (mr, ma, ia, ir, co, pat) in combinations:
                    log.info(
                        "WordEmbeddingRandomWalkDiffusionFeedbackGraph: %s %s %s %s %s %s" % (mr, ma, ia, ir, co, pat))
                    g = WordEmbeddingRandomWalkDiffusionFeedbackGraph(concept_embedder,
                                                                      mass_accept=ma,
                                                                      mass_reject=mr,
                                                                      iterations_accept=ia,
                                                                      iterations_reject=ir,
                                                                      cut_off_threshold=co,
                                                                      propagation_abort_threshold=pat)

                    sir = SingleTopicRunner(self.iobasedir, self.rouge, scores_dir=outputdir)
                    sir.run(topic_path, size, feedbackstore=g, summarizer="PROPAGATION", preload_embeddings=embeddings)

            elif itype == "BaselineFeedbackStore":
                log.info("BaselineFeedbackStore")

                sir = SingleTopicRunner(self.iobasedir, self.rouge, scores_dir=outputdir)
                sir.run(topic_path, size, summarizer="PROPAGATION", preload_embeddings=embeddings)

            elif itype == "PageRankFeedbackGraph":
                log.warning("interpretationtype not implementend. type: %s" % (itype))
            elif itype == "SimpleNgramFeedbackGraph":
                window_size = [2, 3, 4, 5]
                factor_rejects = [1, 0, 0.05, 0.25, 0.5, 2, 4, 8]
                factor_accepts = [1, 0, 0.05, 0.25, 0.5, 2, 4, 8]
                stemmer = SnowballStemmer(topic.get_language())
                combinations = list(itertools.product(window_size, factor_rejects, factor_accepts))
                random.shuffle(combinations)

                for (ws, fr, fa) in combinations:
                    log.info("SimpleNgramFeedbackGraph: (ws %s, fr %s, fa %s)" % (ws, fr, fa))
                    g = SimpleNgramFeedbackGraph(stemmer,
                                                 topic.get_language(),
                                                 N=ws,
                                                 factor_reject=fr,
                                                 factor_accept=fa)

                    sir = SingleTopicRunner(self.iobasedir, self.rouge, scores_dir=outputdir)
                    sir.run(topic_path, size, feedbackstore=g, summarizer="PROPAGATION",
                            preload_embeddings=embeddings)
            else:
                log.warning("Got wrong interpretationtype. ignoring type %s" % (itype))

    def __get_embeddings__(self, language):
        embeddings_path = path.normpath(path.join(self.iobasedir, "embeddings"))

        if self.embeddings[language] is None:
            self.embeddings[language] = load_w2v_embeddings(embeddings_path, language, "active_learning")
        return self.embeddings[language]
