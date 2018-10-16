from ..baselines import sume
from ..baselines.sume_wrap import SumeWrap
from decimal import Decimal


def run_once(f):
    def wrapper(*args, **kwargs):
        if not wrapper.has_run:
            wrapper.has_run = True
            return f(*args, **kwargs)
    wrapper.has_run = False
    return wrapper


class CorpusAnalyzer(object):

    def __init__(self, data_set, topic, docs, language='english', parser_type=None, parse_info=[]):
        self.topic = topic
        self.log_file = "performance_utils/corpora stats/" + data_set

        self.init_summarizer(docs, language, parser_type, parse_info)

        # Params to be determined
        self.stat_keys = [
            "Corpus Size",
            "Concept Size",
            "Avg words per sentence",
            "Lexical Diversity",
            "LD without stopwords",
            "Number of documents",
        ]
        self.pruning_point = ["before", "after"]
        self.blueprint = {self.stat_keys[i]: 0.0 for i in range(0, len(self.stat_keys))}
        self.stats = {when: dict(self.blueprint) for when in self.pruning_point}
        self.write_header()

    def init_summarizer(self, docs, language, parser_type, parse_info):
        self.docs = docs
        self.language = language
        self.parser_type = parser_type
        self.parse_info = parse_info
        self.summarizer = sume.ConceptBasedILPSummarizer(" ", self.language, True)
        self.SumeWrap = SumeWrap(self.language)
        self.summarizer.sentences = self.SumeWrap.load_sume_sentences(
            self.docs, self.parser_type, self.parse_info)
        self.set_summarizer_attributes()

    def set_summarizer_attributes(self):
        self.summarizer.extract_ngrams2(concept_type='ngrams')
        # compute document frequency as concept weights
        self.summarizer.compute_document_frequency()
        # compute word_frequency
        self.summarizer.compute_word_frequency()

    def prune_data(self):
        self.summarizer.weights = {}
        self.summarizer.sentences = self.SumeWrap.load_sume_sentences(
            self.docs, self.parser_type, self.parse_info)
        self.summarizer.prune_sentences(remove_citations=True, remove_redundancy=True, imp_list=[])
        self.set_summarizer_attributes()

    def compute_params(self):
        corpus_size = len(self.summarizer.sentences)
        concept_size = len(self.summarizer.weights.keys())

        # [item for sublist in l for item in sublist]
        vocab = [token.lower()
                 for sentence in self.summarizer.sentences for token in sentence.tokens]

        avg_words_per_sent = len(vocab) / corpus_size
        lexical_diversity = len(set(vocab)) / len(vocab)

        filtered_vocab = [token for token in vocab if token not in self.summarizer.stoplist]
        ld_without_stopwords = len(set(filtered_vocab)) / len(filtered_vocab)

        return corpus_size, concept_size, avg_words_per_sent, lexical_diversity, ld_without_stopwords, len(self.docs)

    def determine_params(self):
        for when in self.pruning_point:
            params = self.compute_params()
            for key, param in zip(self.stat_keys, params):
                self.stats[when][key] = param
            self.prune_data()
        self.log_values()

    @run_once
    def write_header(self):
        log_header = "Topic"
        for when in self.pruning_point:
            log_header += "|"
            for stat in self.stat_keys:
                log_header += stat
                if when == "after":
                    log_header += " {}".format(when)
                log_header += "|"
        with open(self.log_file, 'w+') as f:
            f.write(log_header + "\n")

    def log_values(self):
        line = self.topic
        for when in self.pruning_point:
            line += "|"
            for key in self.stat_keys:
                val = self.stats[when][key]
                if isinstance(val, float):
                    val = round(Decimal(val), 4)
                line += str(val) + "|"
            # log line to self.log_file
        with open(self.log_file, 'a+') as f:
            f.write(line + "\n")
