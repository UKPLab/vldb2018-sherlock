import collections
import logging

import networkx as nx
from algorithms.feedback import print_graph_stats
from scipy.spatial.distance import cosine

from algorithms.feedback.ConceptEmbedder import ConceptEmbedder
from algorithms.feedback.FeedbackStore import FeedbackStore

log = logging.getLogger("WordEmbeddingGaussianFeedbackGraph")

class WordEmbeddingGaussianFeedbackGraph(FeedbackStore):
    logger = logging.getLogger("WordEmbeddingGaussianFeedbackGraph")

    def __init__(self,
                 embedder,
                 cut_off_threshold=0.4,
                 G=None,
                 mass_accept=1.0,
                 mass_reject=-1.0,
                 iterations_accept=1,
                 iterations_reject=1):
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

    def get_config(self):
        config = {
            "type": "WordEmbeddingGaussianFeedbackGraph",
            "mass_accept": self.mass_accept,
            "iterations_accept": self.iterations_accept,
            "mass_reject": self.mass_reject,
            "iterations_reject": self.iterations_reject,
            "cutoff_threshold": self.cut_off_threshold
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
        log.debug("not known in feedbackgraph:", len(set(weights.keys()) - set(G.nodes())), " in G:", len(set(G.nodes())),
              "in total:", len(set(weights.keys())))
        # print("not known in feedbackgraph:", set(weights.keys()) - set(G.nodes()))

        log.debug("%s unique vectors, %s contained unknown words" % (
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
        print_graph_stats(G, "WordEmbeddingGaussianFeedbackGraph")

        # dump_details(G)

    def incorporate_feedback(self, flightrecorder):
        G = self.G
        records = flightrecorder.latest()
        for item in records.accept:
            self.freeze_node(item, 1.0)
            self.__gaussian_blur_based_propagation__(item, mass=self.mass_accept, num_iteration=self.iterations_accept)
        for item in records.reject:
            self.freeze_node(item, 0.0)
            self.__gaussian_blur_based_propagation__(item, mass=self.mass_reject, num_iteration=self.iterations_reject)

    def __gaussian_blur_based_propagation__(self, focus_node, mass=1, num_iteration=1):
        """
        either propagate 0 in case of negative feedback, or propagate 1 in case of positive feedback.
        If value of a node is 0 or 1, its fixated, if 0 < value < 1, it is updated.

        :param focus_node:
        :param mass:
        :param num_iteration:
        :return:
        """
        # print("propagating ", focus_node, "with mass", mass)
        G = self.G

        c = []
        for _ in range(num_iteration):
            G.node[focus_node]["feedback"].append(mass)

            q = collections.deque()
            q.append(focus_node)

            t = nx.bfs_tree(G, focus_node)

            while len(q) > 0:
                v = q.pop()

                v_mass = G.node[v]["feedback"][-1:][0]
                if 0 < v_mass < 1:
                    v_mass_new = 0
                    # only update current node feedback mass if allowed
                    if G.degree(v) > 0:
                        for n in G.successors(v):
                            v_n_sim = G[v][n]["similarity"]
                            n_mass = G.node[n]["feedback"][-1:][0]
                            v_mass_new += v_n_sim * n_mass
                        v_mass_new /= (G.degree(v) + 1)

                    if not self.is_frozen(v):
                        G.node[v]["feedback"].append(v_mass_new)

                q.extend(t.successors(v))
                # dump_details(G)
                # print_graph_stats(G)
                # print("Finished with node feedback for node %s" % (focus_node))

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


