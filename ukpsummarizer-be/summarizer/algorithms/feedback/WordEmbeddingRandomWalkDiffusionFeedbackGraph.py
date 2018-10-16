import collections
import logging
import random

import networkx as nx
import numpy as np
from scipy.spatial.distance import cosine

from algorithms.feedback.FeedbackStore import FeedbackStore
from algorithms.feedback import print_graph_stats

from algorithms.feedback.ConceptEmbedder import ConceptEmbedder


log = logging.getLogger("WordEmbeddingRandomWalkDiffusionFeedbackGraph")
class WordEmbeddingRandomWalkDiffusionFeedbackGraph(FeedbackStore):

    def __init__(self, embedder,
                 G=None,
                 mass_accept=1.0,
                 mass_reject=-1.0,
                 iterations_accept=1,
                 iterations_reject=1,
                 cut_off_threshold=0.4,
                 propagation_abort_threshold=0.1):
        if G is None:
            self.G = nx.DiGraph()
        else:
            self.G = G

        # self.pr = nx.pagerank(G)
        self.vectors = {}
        self.concept_embedder = embedder

        self.mass_accept = mass_accept
        self.mass_reject = mass_reject
        self.iterations_accept = iterations_accept
        self.iterations_reject = iterations_reject
        self.cut_off_threshold = cut_off_threshold
        self.propagation_abort_threshold = propagation_abort_threshold

    def get_config(self):
        config = {
            "type": "WordEmbeddingRandomWalkDiffusionFeedbackGraph",
            "mass_accept": self.mass_accept,
            "mass_reject": self.mass_reject,
            "iterations_accept": self.iterations_accept,
            "iterations_reject": self.iterations_reject,
            "cut_off_threshold": self.cut_off_threshold,
            "propagation_abort_threshold": self.propagation_abort_threshold
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
        print_graph_stats(G, "WordEmbeddingRandomWalkDiffusionFeedbackGraph")

        # dump_details(G)

    def incorporate_feedback(self, flightrecorder):
        G = self.G

        records = flightrecorder.latest()
        for item in records.accept:
            self.freeze_node(item, 1.0)
            self.__random_walk_diffusion_propagation__(item,
                                                       mass=self.mass_accept,
                                                       num_iteration=self.iterations_accept,
                                                       abort_thres=self.propagation_abort_threshold)
        for item in records.reject:
            self.freeze_node(item, 0.0)
            self.__random_walk_diffusion_propagation__(item,
                                                       mass=self.mass_reject,
                                                       num_iteration=self.iterations_reject,
                                                       abort_thres=self.propagation_abort_threshold)

    def __random_walk_diffusion_propagation__(self, focus_node, mass=1, num_iteration=1, abort_thres=0.1):
        """
        Propagate the given mass from to focus_node to its neighbors using random walks for num_iteration times. 
        I.e. in every iteration, the mass is distributed using a random walk. 
        So, whenever a node is reached, it keeps some part of the share, and redistributes the rest.
        While moving mass to the next node, each edge eats some toll that depends on the edge weight.

        :param focus_node: 
        :param mass: 
        :param num_iteration: 
        :return: 
        """

        G = self.G
        log.debug("Starting from node '%s' (%s) with distributable total mass of %s with %s walks" % (
            focus_node, G.node[focus_node]["feedback"], mass, num_iteration))
        mass = float(mass) / float(num_iteration)
        threshold = float(abort_thres) / float(num_iteration)
        for _ in range(num_iteration):
            # start from the focus_node
            source_node = focus_node
            distributable_mass = mass

            while abs(distributable_mass) > abs(threshold):
                try:
                    target_node = self.__choose_random_successor__(source_node)
                except:
                    # no distribution possible. incorporate all feedback locally.
                    source_mass = G.node[source_node]["feedback"][-1:][0]
                    if not self.is_frozen(source_node):
                        G.node[source_node]["feedback"].append(source_mass + distributable_mass)
                        log.debug("no neighbors: %s (%s + %s)" % (source_node, source_mass, distributable_mass))
                    break

                # retain the mass in the current source node
                source_mass = G.node[source_node]["feedback"][-1:][0]

                mass_addable_to_source = distributable_mass / 2
                if not self.is_frozen(source_node):
                    log.debug("       Not frozen")
                    G.node[source_node]["feedback"].append(source_mass + mass_addable_to_source)
                else:
                    log.debug("           Frozen")

                # reduce the rest of the distributable_mass
                edge_weight = G[source_node][target_node]["similarity"]
                distributable_mass = edge_weight * (distributable_mass - mass_addable_to_source)
                log.debug("   neighbors: %s (%s + %s) -- %s --> %s (%s)" % (
                    source_node, source_mass, mass_addable_to_source, edge_weight, target_node, distributable_mass))

                # advance to next node
                source_node = target_node

    def __choose_random_successor__(self, focus_node):
        g = self.G
        total = sum((g[focus_node][n]["similarity"] for n in g.successors(focus_node)))
        # print "total:", total
        psum = 0
        r = random.uniform(0, total)
        # print "r:" , r
        # print "aborting as soon as some value > ", r, "has been found"
        for n in g.successors(focus_node):
            cw = g[focus_node][n]["similarity"]
            psum += cw
            # print "psum", psum, "r", "r < psum?", r < psum
            if r <= psum:
                return n
        raise BaseException("nothing found")

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
