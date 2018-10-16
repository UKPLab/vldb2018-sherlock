import logging
from collections import Counter

import networkx as nx
from nltk import ngrams

from algorithms.feedback.FeedbackStore import FeedbackStore


class PageRankFeedbackGraph(FeedbackStore):
    """
    PageRankFeedbackGraph uses the pagerank algorithm to infer node weights, which act as
    """
    logger = logging.getLogger("PageRankFeedbackGraph")

    def __init__(self, stemmer, language, N=2, G=None):
        if G is None:
            self.G = nx.DiGraph()
        else:
            self.G = G

        self.stemmer = stemmer
        self.language = language
        self.N = N

        self.counter = Counter()
        self.pr = nx.pagerank(G)

    def get_config(self):
        return {
            "language": self.language,
            "N": self.N
        }

    def add_sentences(self, sentences):
        """
        @type sentences: list[Sentence]
        :param sentences:
        :return:
        """
        counter = self.counter
        G = self.G
        for sentence in sentences:
            G.add_nodes_from(sentence.concepts)
            counter.update(ngrams(sentence.concepts, self.N))

        for (keys, value) in counter.items():
            for i in range(0, len(keys) - 1):
                for j in range(1, len(keys)):
                    G.add_edge(keys[i], keys[j], weight=value)
                    # counter.update((keys[i], keys[j]))

        # for (key, value) in counter.items():
        #     G.add_edge(key[0], key[1], attr={"weight": value})

        print("V := (N,E), |N| = %s, |E| = %s" % (len(G.nodes()), len(G.edges())))

        self.pr = nx.pagerank(G)

    def get_weights(self):
        G = self.G
        pr = self.pr
        max_pagerank = max(pr.itervalues())
        # get the largest count to scale weights between 0 and 1.

        # dump_details(G)

        for (k, v) in pr.iteritems():
            yield (k, float(v / max_pagerank))

    def incorporate_feedback(self, flightrecorder):
        """

        :param flightrecorder:
        :return:
         @type flightrecorder: FlightRecorder
        """
        G = self.G
        print("V := (N,E), |N| = %s, |E| = %s" % (len(G.nodes()), len(G.edges())))

        # use the pagerank personalization feature to incorporate flightrecorder feedback

        union = flightrecorder.union()

        for rejected in union.reject:
            if (G.has_node(rejected)):
                G.remove_node(rejected)

        print("V := (N,E), |N| = %s, |E| = %s" % (len(G.nodes()), len(G.edges())))

        self.pr = nx.pagerank(G)