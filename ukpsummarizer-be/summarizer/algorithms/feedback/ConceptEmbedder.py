import numpy as np


class ConceptEmbedder():
    def __init__(self, word_embeddings):
        self.word_embeddings = word_embeddings
        self.cache = {}
        self.errorcount = 0

    def __call__(self, words):
        """

        :param concept: str representing the concept
        :return: vectorial representation of the concept
        @type concept: list[str]
        """

        w2v = self.word_embeddings
        vector = np.zeros(w2v.embedding_size)
        if not self.cache.has_key(" ".join(words)):
            for word in words:
                if w2v.isKnown(word):
                    lookup = w2v.word2embedd(word.lower())
                    vector += lookup
                else:
                    # print("unknown word:", word)
                    self.errorcount += 1
            self.cache[" ".join(words)] = vector
        return self.cache[" ".join(words)]