from __future__ import print_function
from sortedcollections import ValueSortedDict
from collections import defaultdict
from copy import deepcopy
import pandas as pd
from math import log
from summarizer.algorithms.cost_model import CostModel

# Strategies
BY_TIME = 'time_based'
BY_ENTROPY_INIT = 'hw_init'
BY_ENTROPY_ADAPT = 'hw_adapt'
BY_WINDOW = 'adaptive_window'
BY_SWEEP = 'redundancy_sweep'
BY_POS_LINK = 'positive_link'
STRATIFIED = 'stratified'


class SentenceRanker(object):
    """SentenceRanker manages a ranked list of sentences. Sentences are ranked based on
    a heuristic. Currently there is one heuristic named "concept density", but this can
    be expanded on in the future.

    sent_id = (doc_id, position)

    dict sentences_dict: Maps sent_id : ref to Sentence
    dict concept_to_sentences: Maps a concept to the sentences in which it occurs
                        Concept : set([sent_ids])
    ValueSortedDict ranks_to_sentences: [{sent_id : metric_value}] where list index corresponds to rank
    dict concept_weights: original concept weights
    k: Parameter that sets how many sentences are fed into ILP
    """

    def __init__(self, sentences, concept_weights, summary_length, k, options):
        '''
        :param sentences: List of Sentence objects
        :param concept_weights: Dictionary of weights for concept
        :param k: Number of k sentences that is fed into the ILP per feedback iteration
        '''

        # Sets sentences_dict, concept_to_sent dict and ranked_to_sent dict

        if options['strategy'] == STRATIFIED:
            self.all_concept_weights = concept_weights
        else:
            self.all_concept_weights = deepcopy(concept_weights)  # keep original concept weights
        self.initialize_sentences(sentences, concept_weights)

        self.k = k
        if options['relative_k']:
            self.k = int(k * self.get_corpus_size())

        self.k_is_dynamic = options['dynamic_k']
        self.summary_length = summary_length

        self.seen_sentences = set()
        self.important_concepts = set()

        # Interface for feedback
        self.get_input_sentences = self.get_top_k_sentences

        if options['strategy']:
            self.init_strategy(options)

    def initialize_sentences(self, sentences, concept_weights):
        '''Initializes sentences based on metric "concept density".
            creates:
            sentences_dict, concept_to_sentences, ranks_to_sentences
        '''
        self.sentences_dict = {}
        self.concept_to_sentences = defaultdict(set)
        # Highest value --> first rank
        self.ranks_to_sentences = ValueSortedDict(lambda x: -x)

        for sent in sentences:
            # Create sentences_dict
            sent_id = (sent.doc_id, sent.position)
            self.sentences_dict[sent_id] = sent

            # Calculate concept density per sentence
            concept_density = 0
            for concept in sent.concepts:
                concept_density += concept_weights[concept]
                # Create concept2sent dic
                self.concept_to_sentences[concept].add(sent_id)

            concept_density /= float(sent.length)
            # create ranks_to_sentences
            self.ranks_to_sentences[sent_id] = concept_density

    def update_ranking(self, new_accepts, new_rejects, new_implicits):
        ''' Changes top k sentences after feedback based on changed concept weights.'''
        # TODO: implement implicits
        changed_concepts = new_accepts + new_rejects
        for concept in changed_concepts:
            # Update affected sentences
            for sent_id in self.concept_to_sentences[concept]:
                sentence = self.sentences_dict[sent_id]
                concept_density = 0
                for c in sentence.concepts:
                    concept_density += self.all_concept_weights[c]

                concept_density /= float(sentence.length)

                # Update the metric and rank
                self.ranks_to_sentences[sent_id] = concept_density

        for concept in new_accepts:
            self.important_concepts.add(concept)
        if self.k_is_dynamic:
            self.set_k()
            self.k_history.append(self.k)
        return

    def update_weights(self, updated_weights):
        '''Update weights that have changed after weights have been recalculated.'''
        for key, value in updated_weights.items():
            self.all_concept_weights[key] = value
        return

    def filter_concepts_of_top_k_sentences(self, new_accepts=[], new_rejects=[], k=None, sentences=None):
        ''' This method aggregates all relevant concepts based on top k sents and returns those.
            The ILP should only receive those concepts that are also in the subset of sentences
            which is passed to the ILP.
            For the intermediate summary, which is generated for oracle types 'feedback_ilp',
            and 'active_learning', the weights of the accepts and rejects should also be available
            in the returned dictionary (they might not be part of top k sentences anymore).
            '''

        if sentences is None:
            sentences = self.get_top_k_sentences(k)

        concept_weights = {}
        for sent in sentences:
            for concept in sent.concepts:
                concept_weights[concept] = self.all_concept_weights[concept]

        if new_accepts + new_rejects:
            for concept in new_accepts + new_rejects:
                concept_weights[concept] = self.all_concept_weights[concept]

        return concept_weights

    def init_strategy(self, options):
        if options['strategy'] == BY_TIME:
            self.cost_model = CostModel('./algorithms/')
            self.cost_model.k_to_constraints = self.k_to_constraint_size
            self.determine_k = self.set_k_by_target_time

        elif options['strategy'] == BY_ENTROPY_INIT:
            self.k = self.set_k_by_entropy()
            self.determine_k = lambda: self.k

        elif options['strategy'] == BY_ENTROPY_ADAPT:
            self.determine_k = self.set_k_by_entropy

        elif options['strategy'] == BY_WINDOW:
            self.adaptive_window_size = options['adaptive_window_size']
            self.determine_k = self.get_important_sentences

        elif options['strategy'] == BY_SWEEP:
            self.sweep_threshold = options['sweep_threshold']
            self.get_input_sentences = self.get_distinct_top_k_sentences
            return

        elif options['strategy'] == BY_POS_LINK:
            self.get_input_sentences = self.get_sents_with_accepted_concepts

        if options['dynamic_k']:
            self.k_history = []
            self.set_k()
            self.k_history.append(self.k)

    def set_k(self, k=None):
        if k is None and self.k_is_dynamic:
            chosen_k = self.determine_k()
            # Set k so there are enough concepts to fill L
            minimum_k = self.set_k_by_L()
            self.k = max(chosen_k, minimum_k)
        elif k is not None:
            self.k = k

    # Entropy Methods #
    def get_entropy(self, sentences):
        num_concepts = 0
        concept_count = defaultdict(lambda: 0)
        for sent in sentences:
            for c in sent.concepts:
                num_concepts += 1
                concept_count[c] += 1

        S = num_concepts  # Size of Sample
        self.num_of_concepts_in_summary = len(concept_count.keys())

        base = len(self.all_concept_weights.keys())

        entropy = 0.0
        for concept, count in concept_count.items():
            entropy -= (count / S) * log((count / S), base) * self.all_concept_weights[concept]

        return entropy

    def set_k_by_entropy(self):
        k_to_entropy = []
        top_k_sents = self.get_top_k_sentences(self.get_corpus_size())

        num_concepts = 0
        concept_count = defaultdict(lambda: 0)

        base = len(self.all_concept_weights.keys())
        for k, sent in enumerate(top_k_sents, 1):
            for c in sent.concepts:
                num_concepts += 1
                concept_count[c] += 1

            entropy = 0.0
            S = num_concepts
            for concept, count in concept_count.items():
                entropy -= (count / S) * log((count / S), base) * self.all_concept_weights[concept]
            k_to_entropy.append((k, entropy))

        return max(k_to_entropy, key=lambda x: x[1])[0]

    def set_k_by_L(self):
        # TODO sequentially
        for k in range(1, self.get_corpus_size() + 1):
            bigram_count = len(set([c for sent in self.get_top_k_sentences(k)
                                    for c in sent.concepts]))
            # We count unique bigrams, hence / 2
            if bigram_count / 2 >= self.summary_length:
                break
        return k

    def get_baseline_entropies(self):
        # Max, min baselines
        print(self.k_to_entropy)
        return [m(self.k_to_entropy, key=lambda x: x[1])[0] for m in [max, min]]

    def get_number_of_concepts(self):
        return self.num_of_concepts_in_summary

    # Time based strategy #
    def set_k_by_target_time(self):
        # TODO parametrize max_time
        max_time = 1
        target_constraint_size = int(self.time_to_ilp_constraints(max_time))

        occurences = 0
        concepts = set()

        k = 0
        while ((occurences + len(concepts) + 1) < target_constraint_size or
               len(concepts) < self.summary_length / 2):
            sent_id = self.ranks_to_sentences.iloc[k]
            k += 1
            for c in set(self.sentences_dict[sent_id].concepts):
                occurences += 1
                concepts.add(c)
        return k

    def k_to_constraint_size(self, k):
        if type(k) is pd.core.series.Series:
            s = {}
            se = []
            unique_k = set(k)
            for i in unique_k:
                # s.append(self.get_constraint_size_for(i))
                s[i] = self.get_constraint_size_for(i)
            for i in k:
                se.append(s[i])
            return pd.Series(se)
        else:
            return self.get_constraint_size_for(int(k))

    def time_to_ilp_constraints(self, t):
        return self.cost_model.constraints(t)

    def get_constraint_size_for(self, k):
        top_k_sent_ids = self.get_sentence_ids_for(k)
        occurences = 0
        concepts = set()

        for s_id in top_k_sent_ids:
            for ci in set(self.sentences_dict[s_id].concepts):
                occurences += 1
                concepts.add(ci)
        return occurences + len(concepts) + 1

    # Adaptive Window #
    def get_important_sentences(self):
        num_of_important_sents = 0
        # Get number of consecutive sentences that have at least one important concept
        for i, (sent_id, metric_value) in enumerate(self.ranks_to_sentences.items()):
            for concept in self.sentences_dict[sent_id].concepts:
                if concept in self.important_concepts:
                    num_of_important_sents += 1
                    break
            if (i + 1) != num_of_important_sents:
                break
        return int(num_of_important_sents * (1 + self.adaptive_window_size))

    # Redundancy Sweep #
    def get_distinct_top_k_sentences(self):
        top_k_sent_ids = list(self.ranks_to_sentences.keys())
        distinct_sentences = []
        seen_concepts = defaultdict(lambda: False)

        i = 0
        while len(distinct_sentences) < self.k and i < self.get_corpus_size():
            skip = False
            sent = self.sentences_dict[top_k_sent_ids[i]]

            counter = 0
            for c in set(sent.concepts):
                if seen_concepts[c]:
                    # +1 -> threshold = 0 means no overlap; 1 means one concept-overlap allowed
                    if counter >= self.sweep_threshold + 1:
                        skip = True
                        break
                    counter += 1

            if not skip:
                distinct_sentences.append(sent)
                for c in sent.concepts:
                    seen_concepts[c] = 1
            i += 1

        return distinct_sentences

    def get_sents_with_accepted_concepts(self):
        input_sentences = self.get_top_k_sentences()

        for sent_id in self.ranks_to_sentences.iloc[self.k:]:
            sent = self.sentences_dict[sent_id]
            for c in sent.concepts:
                if c in self.important_concepts:
                    input_sentences.append(sent)
                    break

        return input_sentences

    def get_top_k_sentences(self, k=None):
        print('### Top k %f' % self.k)
        if k is None:
            k = self.k
        # Statistic purposes:
        top_k_sent_ids = [sent_id for sent_id in self.ranks_to_sentences.iloc[:k]]
        self.seen_sentences |= set(top_k_sent_ids)
        return [self.sentences_dict[sent_id] for sent_id in self.ranks_to_sentences.iloc[:k]]

    def get_top_k_sentence_ids(self):
        return self.ranks_to_sentences.iloc[:self.k]

    def get_sentence_ids_for(self, k):
        return self.ranks_to_sentences.iloc[:k]

    def bisect_rank_by_value(self, value):
        '''workaround method as ValueSortedDict bisects by key (not sensible for sort order by value).'''
        self.ranks_to_sentences["bisect"] = value
        index = self.ranks_to_sentences.index("bisect")
        del self.ranks_to_sentences["bisect"]
        return index

    def get_corpus_size(self):
        return len(self.sentences_dict)

    # following: stuff for test purposes
    def rank_to_metric(self, rank):
        return self.ranks_to_sentences[self.ranks_to_sentences.iloc[rank]]

    def print_ranked_sentences(self, n=10, full_sentence=False):
        for rank, (doc_id, position) in enumerate(self.get_top_k_sentence_ids()[:n], 1):
            print("Rank: ", rank)
            if full_sentence:
                # self.sentences_dict[sent_id] returns the desired sentence
                print(self.sentences_dict[(doc_id, position)].untokenized_form)
            else:
                print("doc_id: {}, sentence_pos: {}".format(doc_id, position))
            # self.ranks_to_sentences[sent_id] returns the metric
            print("Heuristic value: ", self.ranks_to_sentences[(doc_id, position)])
            print("#-----#")

    def print_concept_map(self):
        i = 0
        for key, value in self.concept_to_sentences.items():
            print(key, ":", self.concept_to_sentences[key])
            for entry in self.concept_to_sentences[key]:
                print(self.sentences_dict[entry].untokenized_form)
            i += 1
            print("-------")
            if i >= 10:
                break
