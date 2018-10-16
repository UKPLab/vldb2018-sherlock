import collections
import logging
import random

import networkx as nx
import numpy as np
from scipy.spatial.distance import cosine

from algorithms.feedback.FeedbackStore import FeedbackStore
from algorithms.feedback import print_graph_stats

from algorithms.feedback.ConceptEmbedder import ConceptEmbedder


class WordEmbeddingEgoPrFeedbackGraph(FeedbackStore):
    logger = logging.getLogger("WordEmbeddingEgoPrFeedbackGraph")

    def __init__(self, embedder, cut_off_threshold=0.4, G=None, mass_accept=1.0,
                 mass_reject=-1.0, iterations_accept=1, iterations_reject=1, ego_pr_depth_accept=1,
                 ego_pr_depth_reject=1, tax=0.5):
        if G is None:
            self.G = nx.DiGraph()
        else:
            self.G = G
        self.cut_off_threshold = cut_off_threshold
        # self.pr = nx.pagerank(G)
        self.vectors = {}
        self.concept_embedder = embedder

        self.mass_accept = mass_accept
        self.mass_reject = mass_reject
        self.iterations_accept = iterations_accept
        self.iterations_reject = iterations_reject
        self.ego_pr_depth_accept = ego_pr_depth_accept
        self.ego_pr_depth_reject = ego_pr_depth_reject
        self.tax = tax

    def get_config(self):
        config = {
            "type": "WordEmbeddingEgoPrFeedbackGraph",
            "depth_accept": self.ego_pr_depth_accept,
            "depth_reject": self.ego_pr_depth_reject,
            "mass_accept": self.mass_accept,
            "mass_reject": self.mass_reject,
            "tax": self.tax
        }
        return config

    def add_sentences(self, sentences=None, weights=None, max_weight=None):
        if weights is None:
            weights = {}

        G = self.G
        # for (k,v) in weights.items():
        #     G.add_node(k, label=k, concept=k, feedback=[float(v) / float(max_weight)], df= v)

        for sentence in sentences:
            for concept in sentence.concepts:
                untokenized_concept = sentence.untokenized_concepts[sentence.concepts.index(concept)]
                # words = unstem_ngram(concept, sentence)
                words = untokenized_concept.split(" ")

                vectorized = self.concept_embedder(words)
                self.vectors[concept] = vectorized

                # if there are initial weights given, normalize them and store.
                if weights.has_key(concept):
                    df = weights[concept] / float(max_weight)
                else:
                    df = 1.0 / float(max_weight)

                G.add_node(concept, label=concept, concept=concept, feedback=[df], df=weights[concept], frozen=False)
        print("not known in feedbackgraph:", len(set(weights.keys()) - set(G.nodes())), " in G:", len(set(G.nodes())),
              "in total:", len(set(weights.keys())))
        # print("not known in feedbackgraph:", set(weights.keys()) - set(G.nodes()))

        print("%s unique vectors, %s contained unknown words" % (
            len(self.concept_embedder.cache), self.concept_embedder.errorcount))

        # add edges
        for u in G.nodes():
            for v in G.nodes():
                if u is v:
                    continue
                sim = 1 - cosine(self.vectors[u], self.vectors[v])
                if sim > self.cut_off_threshold:
                    G.add_edge(u, v, similarity=sim, dissimilarity=1 - sim)
        # self.pr = nx.pagerank(G, weight="similarity")

        # initial graph stats
        print_graph_stats(G, "WordEmbeddingEgoPrFeedbackGraph")

        # dump_details(G)

    def incorporate_feedback(self, flightrecorder):
        G = self.G
        records = flightrecorder.union()
        for item in records.accept:
            self.freeze_node(item, 1.0)
            self.__pagerank_based_distribution__(item,
                                                 depth=self.ego_pr_depth_accept,
                                                 num_iteration=self.iterations_accept,
                                                 tax=self.tax)
        for item in records.reject:
            self.freeze_node(item, 0.0)
            self.__pagerank_based_distribution__(item,
                                                 depth=self.ego_pr_depth_reject,
                                                 num_iteration=self.iterations_reject,
                                                 tax=self.tax)

    def __pagerank_based_distribution__(self, node_id, mass=1.0, depth=1, tax=0.5):
        # t = nx.bfs_tree(self.G, node_id)
        ego = nx.ego_graph(self.G, node_id, radius=depth)

        ego_pr = nx.pagerank(ego, weight="similarity")
        # sum of all pr items == 1 => pr is the share of all money :)
        feedbacks = nx.get_node_attributes(ego, 'feedback')
        for (k, v) in ego_pr.items():
            if k is node_id:
                appendable_mass = mass * tax
            else:
                appendable_mass = mass * tax * v
            for (n, d) in self.G.nodes(data=True):
                if n is k:
                    # print(d["feedback"])
                    if self.is_frozen(n):
                        self.logger.debug("           Frozen")
                    else:
                        self.logger.debug("       Not frozen")
                        if len(d["feedback"]) == 0:
                            v = 0.0
                        else:
                            v = d["feedback"][-1:][0]
                        d["feedback"].append(v + appendable_mass)
                        # G.node[source_node]["feedback"].append(source_mass + mass_addable_to_source)

    def get_weights(self):
        G = self.G
        # pr = self.pr
        # max_pagerank = max(pr.itervalues())
        # get the largest count to scale weights between 0 and 1.

        # dump_graph_gexf(G)

        # find the maximum feedback

        for (n, d) in G.nodes(data=True):
            if len(d["feedback"]) == 0:
                yield (n, 0.0)
            else:
                yield (n, d["feedback"][-1:][0])

                # avg = np.average(d["feedback"])
                # mx = max(d["feedback"])
                # mn = min(d["feedback"])
                #
                # if mx >= 1.0:
                #     yield (n, 1.0)
                # elif mn <= 0.0:
                #     yield (n, 0.0)
                # else:
                #     yield (n, avg)

                # yield (n, np.sum(d["feedback"]))

                # for n, d in nx.get_node_attributes(G, "feedback"):
                #     yield (n, np.sum(d))
                #
                # for (k, v) in pr.iteritems():
                #     yield (k, float(v / max_pagerank))

    def freeze_node(self, item, weight):
        g = self.G
        g.node[item]["frozen"] = True
        g.node[item]["feedback"].append(weight)

    def is_frozen(self, item):
        return self.G.node[item]["frozen"]
