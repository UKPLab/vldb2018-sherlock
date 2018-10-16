import logging
from collections import Counter

import networkx as nx
from nltk import ngrams

from algorithms.feedback.FeedbackStore import FeedbackStore


class SimpleNgramFeedbackGraph(FeedbackStore):
    logger = logging.getLogger("SimpleNgramFeedbackGraph")

    def __init__(self, stemmer, language, N=2, G=None, factor_accept=None, factor_reject=None):
        """
        This class does some simple propagation based feedback on a token-graph. Nodes represent Tokens (stemmed), and
        edges represent N-Grams. As default, :param N is "2", and the Graph is directed.
        """

        if G is None:
            self.G = nx.DiGraph()
        else:
            self.G = G
        """ the graph which has encoded the concept weight along its edges. """

        self.counter = Counter()
        self.language = language
        self.stemmer = stemmer
        self.N = N
        if factor_accept is None:
            self.factor_accept = 2.0
        else:
            self.factor_accept = factor_accept

        if factor_reject is None:
            self.factor_reject = 0.5
        else:
            self.factor_reject = factor_reject

    def get_config(self):
        return {
            "language": self.language,
            "N": self.N,
            "type": "SimpleNgramFeedbackGraph",
            "multiplier_reject": self.factor_reject,
            "multiplier_accept": self.factor_accept
        }

    def add_sentences(self, sentences, weights=None, max_weight=None):
        """ 
        @type sentences: list[Sentence]
        """
        counter = self.counter
        G = self.G
        for sent in sentences:
            counter.update(ngrams(sent.tokens, self.N))
            G.add_nodes_from(sent.tokens)

        updated_edges = []
        for v in counter.elements():
            s = v[0]
            t = v[1]
            c = counter[v]
            updated_edges.append((s, t, c))

        G.add_weighted_edges_from(updated_edges)

    def incorporate_feedback(self, flightrecorder):
        # print("Incorporating feedback into graph")
        G = self.G
        maxweight = self.counter.most_common()[0][1]
        latest_feedback = flightrecorder.latest()

        for edge_as_string in latest_feedback.accept:
            splitted = edge_as_string.split(" ")

            u = splitted[0]
            v = splitted[1]
            if not G.has_edge(u, v):
                continue

            edge = G.get_edge_data(u, v)
            weight = edge.get("weight")

            # set the edge itself to max weight
            G.add_edge(u, v, {"weight": maxweight})

            # multiply adjacent edges by a constant
            for n in G.successors(u):
                if n is v:
                    continue
                w = G.get_edge_data(u, n).get("weight")
                newweight = min(maxweight, max(w * self.factor_accept,0.0))
                G.add_edge(u, n, {"weight": newweight})

            for n in G.successors(v):
                if n is u:
                    continue
                w = G.get_edge_data(v, n).get("weight")
                newweight = min(maxweight, max(0.0, w * self.factor_accept))
                G.add_edge(v, n, {"weight": newweight})

        for edge_as_string in latest_feedback.reject:
            splitted = edge_as_string.split(" ")

            u = splitted[0]
            v = splitted[1]
            if not G.has_edge(u, v):
                continue

            edge = G.get_edge_data(u, v)
            weight = edge.get("weight")

            # set the edge itself to min weight
            G.add_edge(u, v, weight=0.0)

            # multiply adjacent edges by 2
            for n in G.successors(u):
                if n is v:
                    continue
                w = G.get_edge_data(u, n).get("weight")
                newweight = min(max(0.0, w * self.factor_reject), maxweight)
                G.add_edge(u, n, weight=newweight)

            for n in G.successors(v):
                if n is u:
                    continue
                w = G.get_edge_data(v, n).get("weight")
                newweight = min(max(0.0, w * self.factor_reject), maxweight)
                G.add_edge(v, n, weight=newweight)

    def get_weights(self):
        G = self.G
        maxweight = self.counter.most_common()[0][1]
        # get the largest count to scale weights between 0 and 1.
        for (u, v, attr) in G.edges_iter(data="weight"):
            weight = attr
            ngram = u + " " + v
            assert(float(weight / maxweight) <= 1.0)
            yield (ngram, float(weight / maxweight))