from sets import Set

from algorithms.oracle.IOracle import IOracle
from utils.data_helpers import prune_ngrams


class OldOracle(IOracle):
    def __init__(self, input_parse_type, summarizer, flight_recorder, ref_phrases, ref_ngrams, stoplist=None,
                 ngrams_size=2, oracle=None):
        self.input_parse_type = input_parse_type
        self.summarizer = summarizer
        self.stoplist = stoplist or Set()
        self.ngrams_size = ngrams_size
        self.flight_recorder = flight_recorder
        self.ref_phrases = ref_phrases
        self.ref_ngrams = ref_ngrams
        self.__hiddenOracle__ = Oracle()

    def get_labels(self, samples, recommender=None):
        """
            Generate feedback for the subset sentences by peeking into the reference summary.

        """

        new_rejects = Set()
        new_accepts = Set()
        new_implicit_rejects = Set()  # currently not used (all writing occurences are commented out)

        if recommender is None:
            use_samples = samples
        else:
            use_samples = recommender.get_samples(samples)
        # elif recommender == RECOMMENDER_METHOD_SAMPLING:
        #     use_samples = random.sample(samples, allowed_number_of_feedback_per_iteration)
        # elif recommender == RECOMMENDER_METHOD_HIGHEST_WEIGHT:
        #     use_samples = self.recommend_highest_weight(samples, allowed_number_of_feedback_per_iteration);

        if self.input_parse_type == "parse":
            new_rejects = list(
                self.__hiddenOracle__.reject_concepts(use_samples, self.ref_phrases) - self.flight_recorder.union().reject)
            new_accepts = list(
                self.__hiddenOracle__.accept_concepts(use_samples, self.ref_phrases) - self.flight_recorder.union().accept)
        else:
            new_rejects = list(
                self.__hiddenOracle__.reject_concepts(use_samples, self.ref_ngrams) - self.flight_recorder.union().reject)
            new_accepts = list(
                self.__hiddenOracle__.accept_concepts(use_samples, self.ref_ngrams) - self.flight_recorder.union().accept)

        new_rejects = prune_ngrams(new_rejects, self.stoplist, self.ngrams_size)
        new_accepts = prune_ngrams(new_accepts, self.stoplist, self.ngrams_size)

        '''
        if self.parse_type == 'parse':
            self.recorder.total_accept_keys += self.project_phrase_ngrams(self.recorder.accepted_concepts)
            self.recorder.total_reject_keys += self.project_phrase_ngrams(self.recorder.rejected_concepts)
            
            x = list(Set(self.recorder.total_accept + self.recorder.union.reject))
            new_implicit_rejects = list(self.get_implicit_feedback(summ_ngrams, x) - Set(self.recorder.total_implicit_reject))
            # self.recorder.total_implicit_reject += self.recorder.latest().implicit_reject
        '''

        # self.recorder.total_accept += self.recorder.accepted_concepts
        # self.recorder.total_reject += self.recorder.rejected_concepts
        # self.recorder.total_implicit_reject += self.recorder.latest().implicit_reject
        return (new_accepts, new_rejects, new_implicit_rejects)


class Oracle():
    def reject_concepts(self, summ_concepts, ref_concepts):
        '''
        Reject Ngrams

        Keyword arguments:
        ref_ngrams: list of reference n-gram tuples
                    ['1 2', '2 3', '3 4']
        summ_ngrams: list of summary n-gram tuples
                    ['1 2', '2 4']

        return:
        Return N-grams not present in reference
                    ['2 4']
        '''
        return Set(summ_concepts) - ref_concepts

    def accept_concepts(self, summ_concepts, ref_concepts):
        '''
        Accept Ngrams

        Keyword arguments:
        ref_ngrams: list of reference n-gram tuples
                    ['1 2', '2 3', '3 4']
        summ_ngrams: list of summary n-gram tuples
                    ['1 2', '2 4']

        return: Overlap of N-grams
                    ['1 2']

        '''
        return Set(summ_concepts) & ref_concepts
