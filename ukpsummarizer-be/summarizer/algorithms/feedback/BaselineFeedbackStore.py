import copy
import logging

from algorithms.feedback.FeedbackStore import FeedbackStore
from constants import ORACLE_TYPE_ACCEPT_ALL, ORACLE_TYPE_REJECT_ALL, CHANGE_WEIGHT_MODE_REJECT, PARSE_TYPE_PARSE, \
    CHANGE_WEIGHT_MODE_ACCEPT, ORACLE_TYPE_ACCEPT_REJECT, ORACLE_TYPE_ILP_FEEDBACK, ORACLE_TYPE_ACTIVE_LEARNING, \
    ORACLE_TYPE_KEEPTRACK, ORACLE_TYPE_TOP_N


class BaselineFeedbackStore(FeedbackStore):
    logger = logging.getLogger("BaselineFeedbackStore")

    def __init__(self, MAX_WEIGHT=0, interpretation_mode=ORACLE_TYPE_ACCEPT_ALL, parse_type=None, project_phrase_ngrams=None,
                 initial_weights=None):
        self.oracle_type = interpretation_mode
        self.MAX_WEIGHT = MAX_WEIGHT
        self.parse_type = parse_type
        self.weights = {}
        self.project_phrase_ngrams = project_phrase_ngrams
        if initial_weights is None:
            self.initial_weights = {}
        else:
            self.initial_weights = initial_weights

    def get_config(self):
        if self.initial_weights is {}:
            had_intitials = False
        else:
            had_intitials = True

        return {
            "oracle": self.oracle_type,
            "max_weight": self.MAX_WEIGHT,
            "parse_type": self.parse_type,
            "had_initial_weights": had_intitials,
            "type": "BaselineFeedbackStore"

        }

    def add_sentences(self, sentences=None, weights=None, max_weight=None):
        """
        @type sentences: list[Sentence]
        """

        self.weights = copy.deepcopy(weights)
        for (k, v) in self.weights.items():
            self.weights[k] = float(v) / float(max_weight)

    def incorporate_feedback(self, flightrecorder):
        oracle_type = self.oracle_type
        parse_type = self.parse_type
        # we dont need to do anything but keep track of the flightrecorder :)

        if oracle_type == ORACLE_TYPE_REJECT_ALL:
            self.__change_weights__(flightrecorder.union().reject, CHANGE_WEIGHT_MODE_REJECT)
            if parse_type == PARSE_TYPE_PARSE:
                self.__change_weights__(flightrecorder.union().implicit_reject, CHANGE_WEIGHT_MODE_REJECT)
        if oracle_type == ORACLE_TYPE_ACCEPT_ALL:
            self.__change_weights__(flightrecorder.union().accept, CHANGE_WEIGHT_MODE_ACCEPT)
        if oracle_type == ORACLE_TYPE_ACCEPT_REJECT \
                or oracle_type == ORACLE_TYPE_ILP_FEEDBACK \
                or oracle_type.startswith(ORACLE_TYPE_ACTIVE_LEARNING):
            if parse_type is None:
                self.logger.debug('Weight change', oracle_type)
                self.__change_weights__(flightrecorder.latest().reject, CHANGE_WEIGHT_MODE_REJECT)
                self.__change_weights__(flightrecorder.latest().accept, CHANGE_WEIGHT_MODE_ACCEPT)
            if parse_type == PARSE_TYPE_PARSE:
                self.__change_weights__(self.project_phrase_ngrams(flightrecorder.latest().reject),
                                        CHANGE_WEIGHT_MODE_REJECT)
                self.__change_weights__(self.project_phrase_ngrams(flightrecorder.latest().accept),
                                        CHANGE_WEIGHT_MODE_ACCEPT)
                self.__change_weights__(flightrecorder.latest().implicit_reject, CHANGE_WEIGHT_MODE_REJECT)
        if oracle_type == ORACLE_TYPE_KEEPTRACK:
            if parse_type is None:
                self.__change_weights__(flightrecorder.latest().reject, CHANGE_WEIGHT_MODE_REJECT)
                if flightrecorder.latest().accept:
                    self.__change_weights__(flightrecorder.union().accept, CHANGE_WEIGHT_MODE_REJECT)
                else:
                    self.__change_weights__(flightrecorder.union().accept, CHANGE_WEIGHT_MODE_ACCEPT)
            if parse_type == PARSE_TYPE_PARSE:
                self.__change_weights__(self.project_phrase_ngrams(flightrecorder.latest().reject),
                                        CHANGE_WEIGHT_MODE_REJECT)
                self.__change_weights__(flightrecorder.latest().implicit_reject, CHANGE_WEIGHT_MODE_REJECT)
                if flightrecorder.latest().accept:
                    self.__change_weights__(self.project_phrase_ngrams(flightrecorder.latest().accept),
                                            CHANGE_WEIGHT_MODE_ACCEPT)
                else:
                    self.__change_weights__(self.project_phrase_ngrams(flightrecorder.union().accept),
                                            CHANGE_WEIGHT_MODE_ACCEPT)
        if oracle_type == ORACLE_TYPE_TOP_N:
            self.weights = self.initial_weights
            self.__change_weights__(flightrecorder.union().reject, CHANGE_WEIGHT_MODE_REJECT)
            self.__change_weights__(flightrecorder.union().accept, CHANGE_WEIGHT_MODE_ACCEPT)
            if flightrecorder.union().accept:
                sorted_weights = self.get_sorted_concepts()
                for key in self.weights:
                    if key not in sorted_weights[:400]:
                        self.weights[key] = 0

    def __change_weights__(self, concept_list, oracle_type):
        for key in concept_list:
            if oracle_type == CHANGE_WEIGHT_MODE_REJECT:
                self.weights[key] = 0.0
            if oracle_type == CHANGE_WEIGHT_MODE_ACCEPT:
                self.weights[key] = 1.0

    def get_weights(self):
        for (u, v) in self.weights.viewitems():
            assert(float(v) <= 1.0)
            yield (u, v)

    def get_sorted_concepts(self):
        '''
        Get sorted concepts
        '''
        sorted_concepts = sorted(self.weights,
                                 key=lambda x: self.weights[x],
                                 reverse=True)

        # iterates over the concept weights
        return sorted_concepts
