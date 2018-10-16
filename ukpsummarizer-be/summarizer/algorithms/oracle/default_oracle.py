from algorithms.oracle.IOracle import IOracle


class DefaultOracle(IOracle):
    def __init__(self, ref_concepts):
        self.ref_concepts = ref_concepts or []

    def get_labels(self, samples):
        pass
