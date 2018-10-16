from algorithms.oracle.IOracle import IOracle


class HumanOracle(IOracle):
    def __init__(self, labels):
        self.accepts = [label["concept"] for label in labels if label["value"] == "accept"]
        self.rejects = [label["concept"] for label in labels if label["value"] == "reject"]

    def get_labels(self, samples):
        a = [sample for sample in samples if sample in self.accepts]
        r = [sample for sample in samples if sample in self.rejects]
        ir = []
        return a, r, ir
