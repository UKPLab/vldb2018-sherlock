from abc import ABCMeta, abstractmethod


class FeedbackStore(object):
    __metaclass__ = ABCMeta

    @abstractmethod
    def incorporate_feedback(self, flightrecorder):
        """
            incorporates the feedback object into the graph, incl. propagation among neighbors
        :param flightrecorder: FlightRecorder
        @type flightrecorder: FlightRecorder
        """
        pass

    @abstractmethod
    def get_weights(self):
        """
            returns a generator which provides "concept -> weight" tuples, and weight should be a float in the range
            of [0..1]. i.e. the resulting weights that can be plumbed into the ILP-Solver
        """
        pass

    @abstractmethod
    def add_sentences(self, sentences=None, weights=None, max_weight=None):
        """
            incorporates the given parameters into the internal model which in turn calculates the weights on demand
            :param sentences: 
            :return: 
        """
        pass

    @abstractmethod
    def get_config(self):
        """
            returns the current configuration as a dict ready for putting into a json
            
        :return: 
        """
        pass