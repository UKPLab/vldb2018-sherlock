from __future__ import print_function

import itertools
import os.path as path
import re
import sys
from abc import ABCMeta, abstractmethod

from algorithms.feedback.BaselineFeedbackStore import BaselineFeedbackStore
from algorithms.oracle.OldOracle import OldOracle
from algorithms.sentence_ranker import SentenceRanker
sys.path.append(path.dirname(path.dirname(path.dirname(path.abspath(__file__)))))

import tempfile
from sets import Set
import logging

import numpy as np
import pulp
from nltk.corpus import stopwords
from nltk.stem.snowball import SnowballStemmer
from nltk.stem import WordNetLemmatizer
from sklearn import svm

from algorithms.flight_recorder import FlightRecorder
from summarizer.baselines import sume
from summarizer.baselines.sume_wrap import SumeWrap
from summarizer.utils.data_helpers import prune_ngrams, extract_ngrams2, get_parse_info, \
    prune_phrases

from constants import *
import copy

log = logging.getLogger("SimulatedFeedback")


class ISvmSuff():
    def __init__(self):
        pass


class IRecommender():
    __metaclass__ = ABCMeta

    def __init__(self):
        pass

    @abstractmethod
    def get_recommendations(self, samples):
        pass


class DefaultRecommender(IRecommender):
    def __init__(self):
        IRecommender.__init__(self)

    def get_recommendations(self, samples):
        return samples


class HighestWeightRecommender(IRecommender):
    def __init__(self, graph, flight_recorder, stoplist, N):
        IRecommender.__init__(self)
        self.graph = graph
        self.flight_recorder = flight_recorder
        self.stoplist = stoplist
        self.N = N

    def get_recommendations(self, samples):
        return self.recommend_highest_weight(samples)

    def recommend_highest_weight(self, samples, limit=1, prune=True):
        w = dict(self.graph.get_weights())
        s = sorted(w, key=w.get, reverse=True)
        s = [item for item in s if
             item not in self.flight_recorder.union().reject
             and item not in self.flight_recorder.union().accept
             and item not in self.flight_recorder.union().implicit_reject]

        pruned = prune_ngrams(s, self.stoplist, self.N)
        result = []
        for concept in s:
            if concept in samples:
                # log.debug ("adding %s with weight %s to result" % (concept, w[concept]))
                result.append(concept)

        return result[:limit]


class IScoringFunction():
    def __init__(self):
        pass

    @abstractmethod
    def get_scores(self, text):
        pass


class UbEnhancedRougeScorer():
    def __init__(self, rouge, models, parse_info, language, stemmer, summary_length=100, N=2, stopwords=None,
                 ub_score=None,
                 ub_summary=None, summarizer=None, parser_type=None):
        self.rouge = rouge
        self.models = models
        self.language = language
        self.stopwords = stopwords or Set()
        self.summary_length = summary_length

        self.ref_ngrams = Set()  # set of ngrams that are in the reference summaries (for the feedback to peek)
        self.ref_phrases = Set()  # set of phrases that are in the reference summaries (for the feedback to peek)
        self.__ub_summary__ = ub_summary or []
        self.__ub_score__ = ub_score or (0.0, 0.0, 0.0)

        # this only deals with the reference summaries
        parse_info = parse_info or []
        for model_name, model in models:
            y = Set(extract_ngrams2(model, stemmer, language, N))
            self.ref_ngrams = self.ref_ngrams.union(y)
            if parser_type == PARSE_TYPE_PARSE:
                for _, parse_sents in parse_info[1]:
                    for parse_sent in parse_sents:
                        _, phrases = get_parse_info(parse_sent, stemmer, language,
                                                    stopwords)
                        y = Set(prune_phrases(phrases, stopwords, stemmer, language))
                        self.ref_phrases = self.ref_phrases.union(y)

        if summarizer is not None:
            if parser_type is None or parser_type == PARSE_TYPE_NGRAMS:
                concept_match = [key for key in summarizer.weights if key in self.ref_ngrams]
                log.debug('Total uniq ref concepts (ngr):   %s' % (len(self.ref_ngrams)))
            elif parser_type == PARSE_TYPE_PARSE:
                concept_match = [key for key in summarizer.weights if key in self.ref_phrases]
                log.debug('Total uniq ref concepts (phr):   %s' % (len(self.ref_phrases)))
            else:
                raise ValueError("parse_type '%s' is invalid, should be %s or %s" %
                                 (parser_type, None, PARSE_TYPE_PARSE))
            log.debug('UB Accept concepts:  %s' % (len(concept_match)))

    def get_scores(self, text):
        return self.rouge(text, self.models, self.summary_length)

    def get_upper_bound_text(self):
        return self.__ub_summary__

    def get_upper_bound_scores(self):
        return self.__ub_score__


class ISentenceUnwrapper():
    def __init__(self, summarizer,
                 input_parse_type,
                 ref_phrases,
                 ref_ngrams):
        self.summarizer = summarizer
        self.parse_type = input_parse_type
        self.ref_phrases = ref_phrases
        self.ref_ngrams = ref_ngrams

    def unwrap(self, subset):
        summarizer = self.summarizer
        summary = [summarizer.sentences[j].untokenized_form for j in subset]
        # log.debug('Feedback-optimal summary:', summary)
        # # Use the untokenized_concepts to extract the concepts from the sentences.
        # sum_co = [summarizer.sentences[j].untokenized_concepts for j in subset]
        # sam= list(itertools.chain(*sum_co))
        if self.parse_type == 'parse':
            # log.debug('feedback on phrases')
            summary_phrases = [summarizer.sentences[j].phrases for j in subset]

            samples = list(itertools.chain(*summary_phrases))
            references = self.ref_phrases

        else:
            # log.debug('feedback on ngrams')
            summary_concepts = [summarizer.sentences[j].concepts for j in subset]

            samples = list(itertools.chain(*summary_concepts))
            references = self.ref_ngrams

        return references, samples


class SimulatedFeedback():
    def __init__(self, language, rouge, embeddings=None, svm_fvector=None, ngrams_size=2, top_n=100,
                 dump_base_dir=tempfile.mkdtemp(prefix="simufee-"), recommender=None, oracle=None, docs=None,
                 models=None, summary_length=None, oracle_type=None, ub_score=None,
                 ub_summary=None, parser_type=None, parse_info=None, max_iteration_count=25,
                 flightrecorder=None, magic_stats=False, feedbackstore=None, solver='cplex',
                 k=0.1, adaptive_window_size=None, run_config={}, sweep_threshold=1, clusters=None):

        self.language = language  # document language. relevant for stemmer, embeddings, stopwords, parsing
        sumewrap = SumeWrap(
            self.language)  # only used to load the sentences and push them into self.summarizer
        self.summarizer = sume.ConceptBasedILPSummarizer(" ", self.language)

        if self.language == "english":
            self.stemmer = WordNetLemmatizer()
            # elf.stemmer = WordNetLemmatizer()
        else:
            self.stemmer = SnowballStemmer(self.language)

        self.stopwordlist = Set(stopwords.words(self.language))

        if embeddings is None:
            self.embeddings = {}
        else:
            self.embeddings = embeddings  # word2vec embeddings

        if svm_fvector is None:
            self.svm_fvector = []
        else:
            self.svm_fvector = svm_fvector  # List of support vectors for active learning SVM

        self.ngrams_size = ngrams_size  # how many words an should the ngrams consist of

        self.top_n = top_n  # currently unused

        self.recommender = recommender or DefaultRecommender()

        self.cluster_size = len(docs)
        self.MAX_WEIGHT = len(docs)  # int with # of documents (i.e. largest possible DF value)

        self.flight_recorder = flightrecorder or FlightRecorder()  # The flight-recorder stores all interactions wrt to concepts (eg. accepted, and rejected)

        self.log_info_data = []  # stats for the pipeline. The only thing that leaves this class
        self.log_sir_info_data = []  # stats for the single_iteration_runner.

        self.summary_length = summary_length  # target summary length.

        # Extra arguments for the sampling strategies
        self.k = k
        self.adaptive_window_size = adaptive_window_size
        self.sweep_threshold = sweep_threshold
        self.run_config = run_config
        self.clusters = clusters

        # TODO move into actual summarizer class (?)
        # initialization of the self.new_summarizer instance
        self.summarizer.sentences = sumewrap.load_sume_sentences(docs, parser_type, parse_info)

        # extract bigrams as concepts
        if parser_type == PARSE_TYPE_PARSE:
            log.debug('Get concept types Phrases')
            self.summarizer.extract_ngrams2(concept_type='phrase')
        if parser_type == None or parser_type=='ngram':
            log.debug('Get concept types ngrams')
            self.summarizer.extract_ngrams2(concept_type='ngrams')

        # compute document frequency as concept weights
        self.summarizer.compute_document_frequency()
        self.summarizer.compute_word_frequency()

        # from all concepts that are going to be pruned, keep only those that also appear elsewhere
        log.debug('Total concepts before sentence pruning: %s' % (len(self.summarizer.weights)))
        old_sentences = self.summarizer.sentences
        self.summarizer.prune_sentences(remove_citations=True, remove_redundancy=True, imp_list=[])
        retained_concepts = [concept for s in self.summarizer.sentences for concept in s.concepts]
        for sentence in Set(old_sentences).difference(self.summarizer.sentences):
            for concept in sentence.concepts:
                if concept not in retained_concepts and self.summarizer.weights.has_key(concept):
                    del self.summarizer.weights[concept]
        log.debug('Total concepts found: %s ' % (len(self.summarizer.weights)))

        # TODO move to other classes: ##################################################################################

        # TODO move to "recommender"
        self.svm_pos_hash = {}  # active learning // SVM
        self.svm_concept_vec_idx = {}  # active learning // SVM
        self.svm_index_vec_concept = {}  # active learning // SVM
        self.svm_fvector_data = None  # np.array(self.fvector)   # active learning // SVM TODO rename self.data to somehting that contains svm...
        self.svm_labels = None  # active learning // SVM
        self.svm_uncertainity = {}  # active learning // SVM

        # TODO move to recommender or oracle...
        self.MOVE_allowed_number_of_feedback_per_iteration = 5

        # TODO open for deletion: ######################################################################################
        self.new_debug_weights_history = []  # store the evolution of weights over time. Needed for propagation methods that use *random* walks
        self.new_oracle_type = oracle_type
        # ----------------------------------------------------------------------------------------------

        self.scorer = UbEnhancedRougeScorer(rouge, models, parse_info, self.language, self.stemmer,
                                            N=self.ngrams_size,
                                            stopwords=self.stopwordlist, ub_score=ub_score, ub_summary=ub_summary,
                                            summarizer=self.summarizer, parser_type=parser_type)
        # self.new_oracle = oracle or IOracle(recommender=self.new_new_recommender)  # oracle
        self.oracle = oracle or OldOracle(parser_type, self.summarizer, self.flight_recorder,
                                          ref_phrases=self.scorer.ref_phrases, ref_ngrams=self.scorer.ref_ngrams,
                                          stoplist=self.stopwordlist,
                                          ngrams_size=self.ngrams_size)

        if oracle_type.startswith(ORACLE_TYPE_ACTIVE_LEARNING):
            self.get_feature_vector()
            self.svm_fvector_data = np.array(self.svm_fvector)
            self.svm_model = svm.SVC(kernel='linear', C=1.0, probability=True, class_weight='balanced')

        if run_config['rank_subset']:
            # Filter the sentences here w.r.t. the top-k sentences idea
            self.initialize_sentence_ranking()
        elif run_config['strategy'] in ['random', 'stratified']:
            self.initialize_sampling()

        self.new_initial_weights = self.summarizer.weights

        # graph based propagation settings
        self.feedbackstore = feedbackstore or BaselineFeedbackStore(MAX_WEIGHT=self.MAX_WEIGHT,
                                                                    interpretation_mode=oracle_type,
                                                                    parse_type=parser_type,
                                                                    project_phrase_ngrams=self.project_phrase_ngrams,
                                                                    initial_weights=self.new_initial_weights)

        log.debug("FeedbackStore of type %s" % (type(feedbackstore)))

        # self.summarizer.prune_sentences(remove_citations=False, remove_redundancy=True, imp_list=[])

        # create the coocurence graph
        self.feedbackstore.add_sentences(sentences=self.summarizer.sentences,
                                         weights=self.summarizer.weights,
                                         max_weight=self.MAX_WEIGHT)

        self.new_debug_dump_target_dir = dump_base_dir
        dump_dir = tempfile.mkdtemp(dir = self.new_debug_dump_target_dir)

        self.sentence_unwrapper = ISentenceUnwrapper(self.summarizer, parser_type, ref_phrases=self.scorer.ref_phrases,
                                                     ref_ngrams=self.scorer.ref_ngrams)

        self.solver = solver or 'glpk'

        log.info('Summarizing %s sentences down to %s words' %
                 (len(self.summarizer.sentences), self.summary_length))

    ####################################################################################################################

    def get_implicit_feedback(self, summ_ngrams, list_concepts):
        feedback_keys = []
        for key in summ_ngrams:
            for phrase in list_concepts:
                if re.search(u'(\s|^)%s([\s]|$)' % (key), u'%s' % (phrase)) or re.search(u'(\s|^)%s([\s]|$)' % (phrase),
                                                                                         u'%s' % (key)):
                    # log.debug(key, phrase)
                    feedback_keys.append(key)
        implicit_feedback = Set(summ_ngrams) - Set(feedback_keys)
        return implicit_feedback

    def recalculate_weights(self, oracle_type, graph=None, weights=None,
                            max_weight=None, recorder=None):
        """
            Set new weights in self.summarizer.weights according to the currently selected feedbcack method.

            This method basically interprets the feedback. if propagation is False, its using the default model, which
            is changing weights based on the FlightRecorder feedback. If propagation is True, changing weights is based
            on graph traversal...

        :param oracle_type:
        """
        G = graph or self.feedbackstore
        weights = weights or self.summarizer.weights
        max_weight = max_weight or self.MAX_WEIGHT
        recorder = recorder or self.flight_recorder

        if G is None:
            raise StandardError("Set to propagation, but no FeedbackStore is available")

        G.incorporate_feedback(recorder)

        for (concept, weight) in G.get_weights():
            if concept in weights:
                if weight > 1:
                    # log.debug("ignoring   known key: %s  with weight %s " % (concept, weight))
                    weights[concept] = max_weight
                    continue
                weights[concept] = weight * max_weight
            else:
                log.debug("ignoring unknown key:  %s  with weight %s " % (concept, weight))

    def get_summary_details(self, iteration, summary_length):
        """
            Get details about an ilp iteration. It does actually recalc the weights, solve the ilp, extract the
            relevant information, and resets the weights to the previous value.
        :param iteration:
        :param summary_length:
        :return:
        """

        fr = self.flight_recorder
        # solve the ilp model
        value, subset = self.summarizer.solve_ilp_problem(summary_size=int(summary_length), units="WORDS")
        summary = [self.summarizer.sentences[j].untokenized_form for j in subset]

        summary_text = '\n'.join(summary)
        score = self.scorer.get_scores(summary_text)

        # score = self.new_rouge(summary_text)

        return summary, score, subset

    def check_break_condition(self, summary, iteration, max_iteration_count, current_score, prev_score):
        if not self.flight_recorder.latest().accept \
                and not self.flight_recorder.latest().reject \
                and len(self.flight_recorder.records) > 1:
            log.info("BREAKING HERE: Stopping because last flight_recorder is basically empty")
            return 1
        if current_score == self.scorer.get_upper_bound_scores():
            log.info("BREAKING HERE: score is equal to UB score")
            return 1
        if current_score[1] >= self.scorer.get_upper_bound_scores()[1]:  # ROUGE2 score> Upper-bound
            log.info("BREAKING HERE: current summary is BETTER than UB")
            return 1
        if summary == self.scorer.get_upper_bound_text():
            log.info("BREAKING HERE: Found UB summary")
            return 1
        if iteration + 1 >= max_iteration_count:
            log.info("BREAKING HERE: max num of iterations found")
            return 1
        return 0

    def get_feature_vector(self):
        """
        assign each concept a vector in word2vec space that is the mean of its constituting words

        :return:
        """
        '''
        corpus = [' '.join(doc) for _, doc in docs]
        vectorizer = TfidfVectorizer(min_df=1)
        X = vectorizer.fit_transform(corpus)
        idf = vectorizer._tfidf.idf_
        tf_idf = dict(zip(vectorizer.get_feature_names(), idf))
        print tf_idf
        '''
        index = 0
        self.svm_uncertainity, self.svm_concept_vec_idx = {}, {}
        self.svm_fvector = []
        unknown_l, hit_l = [], []

        for i in range(len(self.summarizer.sentences)):
            '''
            print(self.summarizer.sentences[i].concepts)
            print(self.summarizer.sentences[i].untokenized_form)
            print(self.summarizer.sentences[i].tokens_pos)
            '''
            # for each concept
            for concept in self.summarizer.sentences[i].concepts:
                # log.debug(self.summarizer.sentences[i].untokenized_form)
                if concept not in self.svm_concept_vec_idx:
                    ngram = concept.split(' ')
                    is_capital, stopword, pos_list, concept_tf, embd = 0, 0, [], [], []
                    # log.debug()
                    for token in ngram:
                        try:
                            word, pos = self.summarizer.sentences[i].tokens_pos[token].split('::')
                        except:
                            token = re.sub(u'[-\.](\s|$)', u'\\1', token)
                            try:
                                word, pos = self.summarizer.sentences[i].tokens_pos[token].split('::')
                            except:
                                if token.isnumeric():
                                    word, pos = token, 'CD'
                                else:
                                    word, pos = token, 'NN'
                        if word.istitle():
                            is_capital += 1
                        pos_list.append(pos)
                        # log.debug(token,)
                        if token in self.stopwordlist:
                            stopword += 1
                        if token in self.summarizer.word_frequencies:
                            concept_tf.append(self.summarizer.word_frequencies[token])
                        if token not in self.stopwordlist:
                            word_l = word.lower()
                            if word_l in self.embeddings.vocab_dict:
                                embd_val = self.embeddings.W[self.embeddings.vocab_dict[word_l]]
                                hit_l.append(word_l)
                                embd.append(embd_val.tolist())
                            else:
                                joint_words = word_l.split('-')
                                for j_word in joint_words:
                                    j_word = unicode(j_word)
                                    if j_word in self.embeddings.vocab_dict:
                                        embd_val = self.embeddings.W[self.embeddings.vocab_dict[j_word]]
                                        hit_l.append(j_word)
                                        embd.append(embd_val.tolist())
                                    else:
                                        if self.language == "english":
                                            embd_val = self.embeddings.W[self.embeddings.vocab_dict[u"unk"]]
                                        if self.language == "german":
                                            embd_val = self.embeddings.W[
                                                self.embeddings.vocab_dict[u"unknown"]]
                                        unknown_l.append(unicode(word_l))
                            embd.append(embd_val.tolist())

                    pos_key = '_'.join(pos_list)
                    if pos_key in self.svm_pos_hash:
                        pos_val = self.svm_pos_hash[pos_key]
                    else:
                        pos_val = len(self.svm_pos_hash) + 1
                        self.svm_pos_hash[pos_key] = pos_val
                    if concept_tf == []:
                        concept_tf = [1]

                    # calculate concept vector as the mean of its constituent word vectors.
                    if concept not in self.svm_concept_vec_idx:
                        if not embd:
                            log.debug("%s - %s" % (embd, concept))
                            continue
                        vector = np.mean(np.array(embd), axis=0)
                        vector = np.append(vector, np.array([-1]), axis=0)
                        self.svm_fvector.append(vector.tolist())

                        '''
                        self.fvector.append([1.0 * self.summarizer.weights[concept]/self.cluster_size,
                                    is_capital,
                                    pos_val,
                                    stopword,
                                    np.mean(np.array(concept_tf)),
                                    -1])
                        '''
                        self.svm_uncertainity[concept] = 1.0
                        self.svm_concept_vec_idx[concept] = index
                        self.svm_index_vec_concept[index] = concept
                        index += 1

        hit_l, unknown_l = Set(hit_l), Set(unknown_l)
        hit, unknown = len(hit_l), len(unknown_l)
        # log.debug('size of the feature vector: %d' % len(self.svm_fvector))
        # log.debug('hit concepts: %d, unknown concepts: %d' % (hit, unknown))
        # log.debug('hit ratio: %f, unknown ratio: %f' % (1.0 * hit / (hit + unknown), 1.0 * unknown / (hit + unknown)))
        # log.debug('Unknown words %s' % (unknown_l))

    def change_labels(self, feedback_list, label):
        for concept in feedback_list:
            # log.debug(concept, self.summarizer.weights[concept])
            if concept in self.svm_concept_vec_idx:
                vec_index = self.svm_concept_vec_idx[concept]
                self.svm_fvector_data[vec_index, -1] = label
            else:
                log.debug(
                    "accessing illegal concept vector index %s while labelling items for active_learning" % (concept))

    def project_phrase_ngrams(self, concept_list):
        feedback_keys = []
        for phrase in concept_list:
            for key in self.summarizer.weights:
                if re.search(u'(\s|^)%s([\s]|$)' % (key), u'%s' % (phrase)) or re.search(u'(\s|^)%s([\s]|$)' % (phrase),
                                                                                         u'%s' % (key)):
                    # log.debug(key, phrase)
                    feedback_keys.append(key)
        return feedback_keys

    def get_uncertainity_labels(self, model):

        '''
        if self.parse_type == PARSE_TYPE_PARSE:
            #log.debug('Accept keys:', self.recorder.total_accept_keys)
            self.change_labels(self.recorder.total_accept_keys, label=1)
            self.change_labels(self.recorder.union().reject_keys, label=0)
            self.change_labels(self.recorder.union().implicit_reject, label=0)
        if self.parse_type == None:
        '''
        self.change_labels(self.flight_recorder.union().accept, label=1)
        self.change_labels(self.flight_recorder.union().reject, label=0)

        Y = self.svm_fvector_data[:, -1]
        X = self.svm_fvector_data[:, 1:-1]

        UL_indexes = np.where(Y == -1)
        L_indexes = np.where(Y > -1)

        X_train, Y_train = X[L_indexes], Y[L_indexes]
        X_unlabeled, _ = X[UL_indexes], Y[UL_indexes]

        flag = 0
        try:
            model.fit(X_train, Y_train)
            UL_probs = model.predict_proba(X_unlabeled)
            UL = model.predict(X_unlabeled)
        except:  # If there are no Accepts [training data has only one class]
            flag = 1

        concept_u, concept_labels = {}, {}

        index = 0
        for vec_index in self.svm_index_vec_concept:
            concept = self.svm_index_vec_concept[vec_index]
            if vec_index not in UL_indexes[0]:
                concept_u[concept] = 0.0
                concept_labels[concept] = self.svm_fvector_data[vec_index, -1]
            else:
                if flag == 0:
                    prob = UL_probs[index]
                    concept_u[concept] = 1 - prob.max()
                    concept_labels[concept] = UL[index]
                else:  # If there are no Accepts [training data has only one class]
                    concept_u[concept] = 1.0
                    concept_labels[concept] = 1.0
                index += 1

        return concept_u, concept_labels

    # def __replay_flightrecorder_logbook__(self, oracle_type, G):
    #     copiedRecorder = FlightRecorder()
    #     for record in self.flight_recorder.records:
    #         copiedRecorder.add_record(record)
    #         self.recalculate_weights(oracle_type, graph=G, recorder=copiedRecorder)

    def run_full_simulation(self, max_iteration_count=11):
        """
        This starts of the simualted feedback for a single cluster of documents, towards a list of models. i.e. the
        models get united, and then the feedback loop is simulated.

        """

        svm_flag = 0
        # get_details is the personalizedSummary function which gets updated weights in every iteration.
        # Starting with boudin as starting weights (except in case of weights_override != None).

        # # replay the existing logbook so that we can continue where we left of...
        # self.__replay_flightrecorder_logbook__(self.new_oracle_type, self.new_feedbackstore);

        # self.__apply_initial_weights_override__(weights_override, clear_before_override)

        # initial iteration


        previous_score = (0.0, 0.0, 0.0)
        previous_summary = ''
        recommendations = []
        break_condition_met = 0
        iteration = 0
        while (break_condition_met == 0):
            # for iteration in range(0, max_iteration_count):
            # let the summary be evaluated by the oracle
            # should be Step 1 in any iterations.
            score, summary, summary_sentences, recommendations, recomm_sentence = self.single_iteration(
                iteration, recommendations, svm_flag)

            break_condition_met = self.check_break_condition(summary, iteration, max_iteration_count,
                                                             score, previous_score)
            self.__print_iteration_info__(summary_sentences, iteration, summary, score,
                                          recommendations, recommendations_sentences = recomm_sentence)
            iteration += 1
            print('Iteration %d' % (iteration))
        return (summary, summary_sentences, recomm_sentence)

    def single_iteration(self, iteration, samples, svm_flag):
        new_accepts, new_rejects, new_implicits = self.oracle.get_labels(samples)
        self.flight_recorder.record(new_accepts, new_rejects, new_implicits)
        self.recalculate_weights(self.new_oracle_type)
        current_summary, current_score, current_summary_sentence_ids = self.get_summary_details(iteration,
                                                                                                self.summary_length)
        self.__add_weights_to_history__(self.new_debug_dump_target_dir, iteration)
        # _, recommendations = self.sentence_unwrapper.unwrap(current_summary_sentence_ids)
        # samples = self.__convert_subset_to_concepts__(current_summary_sentence_ids, self.new_summarizer,
        #                                               self.input_parse_type,
        #                                               self.ref_phrases,
        #                                               self.ref_ngrams)  # from all samples, use a sub-set
        # current_summary, current_score, _ = self.get_summary_details(iteration, self.new_input_summary_length)
        recommendations, recomm_sentence = self.get_recommendations(svm_flag)
        return current_score, current_summary, current_summary_sentence_ids, recommendations, recomm_sentence

    def __print_iteration_info__(self, subset, iteration=-1, text=None, score=(-1.0, -1.0, -1.0), recommendations=None, recommendations_sentences = None):
        text = text or [""]
        recommendations = recommendations or []
        recommendations_sentences = recommendations or set([])
        fr = self.flight_recorder

        log.debug("flight rec: (T: %s = A: %s + R: %s ), (L: %s = A: %s + R: %s)" %
                  (len(fr.union().accept | fr.union().reject),
                   len(fr.union().accept),
                   len(fr.union().reject),
                   len(fr.latest().accept | fr.latest().reject),
                   len(fr.latest().accept),
                   len(fr.latest().reject)))

        fb_sent_cnt = len([self.summarizer.sentences[j].concepts for j in subset])
        fb_conc = [c for j in subset for c in self.summarizer.sentences[j].concepts]
        log.info(
            'Current summary %s , with a total of %s concepts ( %s  unique). Unknown concepts:  %s ' %
            (subset,
             len(fb_conc),
             len(Set(fb_conc)),
             len(Set(fb_conc).difference(self.flight_recorder.union().accept).difference(
                 self.flight_recorder.union().reject))))

        accepted = fr.latest().accept
        rejected = fr.latest().reject

        row = [str(iteration), score[0], score[1], score[2], len(accepted), len(rejected),
               '\n'.join(text)]

        # self.summarizer.weights = old_weights

        log.debug(row[:-1])
        # log.debug(summary_text.encode('utf-8'))
        self.log_info_data.append(row)
        self.log_sir_info_data.append({
            "iteration": iteration,
            "ROUGE-1 R score": score[0],
            "ROUGE-2 R score": score[1],
            "ROUGE-SU* R score": score[2],
            "accepted": list(accepted),
            "accept_count": len(accepted),
            "rejected": list(rejected),
            "reject_count": len(rejected),
            "summary": text,
            "requested_feedback_recommendations": list(set(recommendations))
        })

    def __add_weights_to_history__(self, dump_dir=tempfile.mkdtemp(), iteration=0):
        """

        :param dump_dir: directory (has to exist) where the weight map should be stored.
        :param iteration: current iteration
        @type dump_dir: str
        @type iteration: int

        """

        self.new_debug_weights_history.append(copy.deepcopy(self.summarizer.weights))
        # prefix = "weights-%s-" % iteration
        # handle, file = tempfile.mkstemp(suffix=".json", prefix=prefix, dir=dump_dir)
        # os.close(handle)
        # log.debug("Dumping weights to %s" % file)
        # write_to_file(json_content, file)

    def get_recommendations(self, svm_flag=0):
        if self.new_oracle_type.startswith(ORACLE_TYPE_ACTIVE_LEARNING):
            self.svm_uncertainity, self.svm_labels = self.get_uncertainity_labels(self.svm_model)

        subset_of_optimal_feedback = self.generate_optimal_feedback_summary(
            summarizer=self.summarizer,
            flight_recorder=self.flight_recorder,
            flag=svm_flag,
            oracle_type=self.new_oracle_type,
            summary_length=self.summary_length,
            svm_labels=self.svm_labels,
            svm_uncertainity=self.svm_uncertainity)
        _, recommendations = self.sentence_unwrapper.unwrap(subset_of_optimal_feedback)

        #log.debug("new recommendations: %s" % (recommendations))
        recommendations = [i for i in recommendations if i
                           not in self.flight_recorder.union().implicit_reject
                           and i not in self.flight_recorder.union().reject
                           and i not in self.flight_recorder.union().accept]

        return (Set(recommendations), subset_of_optimal_feedback)

    def __solve_joint_ilp__(self, feedback, non_feedback, summarizer, summary_length,
                            uncertainity={}, labels={}, unique=False, excluded_solutions=[], solver='cplex'):
        """
        :param summary_length: The size of the backpack. i.e. how many words are allowed in the summary.
        :param feedback:
        :param non_feedback:
        :param unique: if True, an boudin_2015 eq. (5) is applied to enforce a unique solution.
        :param solver: default glpk
        :param excluded_solutions:
        :return:
        """
        w = summarizer.weights
        if self.run_config['rank_subset']:
            w = self.sentence_ranker.all_concept_weights

        u = uncertainity
        L = summary_length
        NF = len(non_feedback)
        F = len(feedback)
        S = len(summarizer.sentences)

        if not summarizer.word_frequencies:
            summarizer.compute_word_frequency()

        tokens = summarizer.word_frequencies.keys()
        f = summarizer.word_frequencies
        T = len(tokens)

        # HACK Sort keys
        # concepts = sorted(self.weights, key=self.weights.get, reverse=True)

        # formulation of the ILP problem
        prob = pulp.LpProblem(summarizer.input_directory, pulp.LpMaximize)

        # initialize the concepts binary variables
        nf = pulp.LpVariable.dicts(name='nf',
                                   indexs=range(NF),
                                   lowBound=0,
                                   upBound=1,
                                   cat='Integer')

        f = pulp.LpVariable.dicts(name='F',
                                  indexs=range(F),
                                  lowBound=0,
                                  upBound=1,
                                  cat='Integer')

        # initialize the sentences binary variables
        s = pulp.LpVariable.dicts(name='s',
                                  indexs=range(S),
                                  lowBound=0,
                                  upBound=1,
                                  cat='Integer')

        # initialize the word binary variables
        t = pulp.LpVariable.dicts(name='t',
                                  indexs=range(T),
                                  lowBound=0,
                                  upBound=1,
                                  cat='Integer')

        # OBJECTIVE FUNCTION
        if labels:
            # log.debug('solve for Active learning 2')
            prob += pulp.lpSum(
                w[non_feedback[i]] * (1.0 - u[non_feedback[i]]) * labels[non_feedback[i]] * nf[i] for i in range(NF))
        if not labels:
            if uncertainity:
                # log.debug('solve for Active learning')
                if feedback:
                    # In this phase, we force new concepts to be chosen, and not those we already have feedback on, and
                    # therefore non_feedback is added while feedback is substracted from the problem. I.e. by
                    # substracting the feedback, those sentences will disappear from the solution.
                    prob += pulp.lpSum(w[non_feedback[i]] * u[non_feedback[i]] * nf[i] for i in range(NF)) - pulp.lpSum(
                        w[feedback[i]] * u[feedback[i]] * f[i] for i in range(F))
                else:
                    prob += pulp.lpSum(w[non_feedback[i]] * u[non_feedback[i]] * nf[i] for i in range(NF))
            if not uncertainity:
                # log.debug('solve for ILP feedback')
                if feedback:
                    prob += pulp.lpSum(w[non_feedback[i]] * nf[i] for i in range(NF)) - pulp.lpSum(
                        w[feedback[i]] * f[i] for i in range(F))
                else:
                    prob += pulp.lpSum(w[non_feedback[i]] * nf[i] for i in range(NF))

        if unique:
            prob += pulp.lpSum(w[non_feedback[i]] * nf[i] for i in range(NF)) - pulp.lpSum(w[feedback[i]] * f[i] for i in range(F)) + \
                    10e-6 * pulp.lpSum(f[tokens[k]] * t[k] for k in range(T))

        # CONSTRAINT FOR SUMMARY SIZE
        prob += pulp.lpSum(s[j] * summarizer.sentences[j].length for j in range(S)) <= L

        # INTEGRITY CONSTRAINTS
        for i in range(NF):
            for j in range(S):
                if non_feedback[i] in summarizer.sentences[j].concepts:
                    prob += s[j] <= nf[i]

        for i in range(NF):
            prob += pulp.lpSum(s[j] for j in range(S)
                        if non_feedback[i] in summarizer.sentences[j].concepts) >= nf[i]

        for i in range(F):
            for j in range(S):
                if feedback[i] in summarizer.sentences[j].concepts:
                    prob += s[j] <= f[i]

        for i in range(F):
            prob += pulp.lpSum(s[j] for j in range(S)
                        if feedback[i] in summarizer.sentences[j].concepts) >= f[i]

        # WORD INTEGRITY CONSTRAINTS
        if unique:
            for k in range(T):
                for j in summarizer.w2s[tokens[k]]:
                    prob += s[j] <= t[k]

            for k in range(T):
                prob += pulp.lpSum(s[j] for j in summarizer.w2s[tokens[k]]) >= t[k]

        # CONSTRAINTS FOR FINDING OPTIMAL SOLUTIONS
        for sentence_set in excluded_solutions:
            prob += pulp.lpSum([s[j] for j in sentence_set]) <= len(sentence_set) - 1

        # prob.writeLP('test.lp')

        # solving the ilp problem
        try:
            #print('Solving using CPLEX')
            prob.solve(pulp.CPLEX(msg=0))
        except:
            #print('Fallback to mentioned solver')
            if solver == 'gurobi':
                prob.solve(pulp.GUROBI(msg=0))
            elif solver == 'glpk':
                prob.solve(pulp.GLPK(msg=0))
            else:
                sys.exit('no solver specified')

        # retreive the optimal subset of sentences
        solution = Set([j for j in range(S) if s[j].varValue == 1])

        # returns the (objective function value, solution) tuple
        return (pulp.value(prob.objective), solution)

    def initialize_sentence_ranking(self):
        self.run_config['adaptive_window_size'] = self.adaptive_window_size
        self.run_config['sweep_threshold'] = self.sweep_threshold
        self.sentence_ranker = SentenceRanker(
            self.summarizer.sentences, self.summarizer.weights, self.summary_length, self.k, self.run_config)
        self.change_sentence_subset()

    def update_sentence_ranking(self, new_accepts=[], new_rejects=[], new_implicits=[]):
        self.sentence_ranker.update_weights(self.summarizer.weights)
        # # New accepts / Rejects / Implicits are known; use them to update ranks
        self.sentence_ranker.update_ranking(new_accepts, new_rejects, new_implicits)
        self.change_sentence_subset(new_accepts, new_rejects)

    def change_sentence_subset(self, new_accepts=[], new_rejects=[], new_implicits=[]):
        self.summarizer.sentences = self.sentence_ranker.get_input_sentences()
        self.summarizer.weights = self.sentence_ranker.filter_concepts_of_top_k_sentences(
            new_accepts, new_rejects, sentences=self.summarizer.sentences)
        self.summarizer.compute_word_frequency() # recomputation only necessary if unique=True

    def initialize_sampling(self):
        if self.run_config['strategy'] == 'random':
            self.sampling = RandomSampling(self.summarizer.sentences, self.k)
        elif self.run_config['strategy'] == 'stratified':
            self.sampling = StratifiedSampling(self.clusters,
                                               self.summarizer.sentences,
                                               self.summarizer.weights,
                                               self.summary_length,
                                               self.k,
                                               self.run_config)
        else:
            raise ValueError('Configuration error: Unknown sampling strategy: {}'.format(self.run_config['strategy']))

        self.summarizer.sentences = self.sampling.get_sentences()
        self.summarizer.weights = self.sampling.get_concept_weights(self.summarizer.sentences)

    def update_sampling(self, new_accepts, new_rejects, new_implicits, subset, i):
        self.sampling.update_weights(self.summarizer.weights)
        self.sampling.update_clusters(new_accepts, new_rejects, new_implicits)
        if self.run_config['adaptive_sampling'] in ['epsilon', 'heuristic'] or (
                self.run_config['adaptive_sampling'] == 'epsilon_exploration' and i > 4) or (
                self.run_config['adaptive_sampling'] == 'heuristic_exploration' and i > 4):
            summary_sents = [self.summarizer.sentences[j] for j in subset]
            self.sampling.update_cluster_weights(summary_sents, new_accepts)
        self.sampling.log()
        self.summarizer.sentences = self.sampling.get_sentences()
        self.summarizer.weights = self.sampling.get_concept_weights(self.summarizer.sentences,
                                                                    new_accepts, new_rejects)

    def set_stats(self, stats, new_stats):
        for k, v in new_stats.items():
            stats[k].append(v)

    def tick(self, dynamic, i=None):
        if dynamic:
            print(self.sentence_ranker.k, end=" ", flush=True)
        else:
            print(i, end=" ", flush=True)

    def generate_optimal_feedback_summary(self, summarizer, flight_recorder, flag, oracle_type, summary_length,
                                          svm_uncertainity=None, svm_labels=None):
        """
            Generates a summary which is optimal for getting feedback on. This is done by increasing the probability of
            generating a summary with unknown concepts in it. This is achieved by setting the concept weights of known
            concepts (either positivly or negativly rated) to ZERO.

        :param flag:
        :param oracle_type:
        :return:
        """

        if oracle_type == ORACLE_TYPE_ILP_FEEDBACK or oracle_type.startswith(ORACLE_TYPE_ACTIVE_LEARNING):

            feedback = flight_recorder.union().accept | flight_recorder.union().reject
            """
            if self.parse_type == PARSE_TYPE_PARSE:
                feedback = self.project_phrase_ngrams(feedback)
            """

            non_feedback = summarizer.weights.viewkeys() - feedback
            # log.debug("GeOpFeSu: Feedback Size:", len(feedback), len(non_feedback),
            #       'Total:', len(summarizer.weights.keys()))
            if not ((flight_recorder.latest().accept == 0 or len(feedback) == 0) and flag == 0):
                if oracle_type == ORACLE_TYPE_ILP_FEEDBACK:
                    _, subset = self.__solve_joint_ilp__(list(feedback),
                                                         list(non_feedback),
                                                         summarizer=summarizer,
                                                         summary_length=summary_length)
                if oracle_type == ORACLE_TYPE_ACTIVE_LEARNING:
                    _, subset = self.__solve_joint_ilp__(list(feedback),
                                                         list(non_feedback),
                                                         summarizer=summarizer,
                                                         summary_length=summary_length,
                                                         uncertainity=svm_uncertainity)
                if oracle_type == ORACLE_TYPE_ACTIVE_LEARNING2:
                    _, subset = self.__solve_joint_ilp__(list(feedback),
                                                         list(non_feedback),
                                                         summarizer=summarizer,
                                                         summary_length=summary_length,
                                                         uncertainity=svm_uncertainity,
                                                         labels=svm_labels)
                    print('Subset after AL2', subset)
                if not subset:
                    flag = 1
                    print('Solving regular ILP with flag %s (no subset)' % (flag))
                    _, subset = summarizer.solve_ilp_problem(summary_size=int(self.summary_length), units="WORDS")
            else:
                print('Solving regular ILP with flag %s (else)' % (flag))
                _, subset = summarizer.solve_ilp_problem(summary_size=int(self.summary_length), units="WORDS")
        # elif oracle_type == ORACLE_TYPE_CUSTOM_WEIGHT:
        #     log.debug("oracle == ", ORACLE_TYPE_CUSTOM_WEIGHT)
        else:
            # solve the ilp model
            _, subset = summarizer.solve_ilp_problem(summary_size=int(self.summary_length), units="WORDS")
        return subset
