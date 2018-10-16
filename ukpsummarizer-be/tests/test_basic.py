from algorithms.oracle.OldOracle import Oracle

import unittest

class OracleTest(unittest.TestCase):
    def __init__(self):
        self.summ = [('1','2'), ('2', '3'), ('3','4')]
        self.ref = [('1','2'), ('2', '4')]
        self.oracle = Oracle()
    
    def test_functions(self):
        self.oracle.reject_ngrams(self.ref, self.summ)
