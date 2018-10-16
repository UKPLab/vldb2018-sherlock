from abc import abstractmethod


class IOracle():
    def __init__(self):
        pass

    @abstractmethod
    def get_labels(self, samples):
        pass
