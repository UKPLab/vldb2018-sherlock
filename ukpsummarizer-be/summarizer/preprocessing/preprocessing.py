from __future__ import print_function, unicode_literals

import argparse
import itertools
import re

import pandas as pd
from os import path

from algorithms.simulated_feedback import SimulatedFeedback
from baselines.sume_wrap import SumeWrap
from model.dataset import DataSet
from rouge.rouge import Rouge
from utils.data_helpers import load_w2v_by_name
from utils.reader import resolve_against_iobase


def do_preprocess(rouge, datasets, topics, operation):
    print(operation)
    if "vocab" == operation:
        analyse_vocab(rouge, datasets, topics)
    elif "make" == operation:
        print("make")
    else:
        print("wurg")


def analyse_vocab(rouge, datasets=None, topics=None):
    if datasets is None:
        return

    concept_type = ("parse", "ngrams")
    embedding_variants = ("google.neg.300d", "glove.6B.300d", "tudarmstadt_german")
    # concept_type = None
    topic_details = []
    concept_details = []
    # else:
    #     embeddings = load_w2v_embeddings(embeddings_path, language, oracle)
    token_details = []
    embeddings_path = path.normpath(path.join(args.iobasedir, "embeddings"))

    for dataset, concept_type, embedding_variant in itertools.product(datasets, concept_type, embedding_variants):
        print("running analysis for ", dataset, concept_type, embedding_variant,
              "--------------------------------------")
        i = 0
        ds = resolve_against_iobase(dataset, args.iobasedir)
        d = DataSet(ds)
        language = d.get_language()
        embeddings = load_w2v_by_name(embeddings_path, variant=embedding_variant)
        for topic in d.get_topics():
            # if i > 2:
            #     continue
            sumewrap = SumeWrap(language=language)
            i += 1
            docs = topic.get_docs()
            summaries = topic.get_models()

            parse_info = topic.get_parse_info(0)

            sf = SimulatedFeedback(language, rouge, embeddings=embeddings, docs=docs, models=summaries,
                               summary_length=100, oracle_type="active_learning", ub_score=(1, 1, 1),
                               ub_summary=" ", parser_type=concept_type)
            # sf.run_full_simulation(max_iteration_count=0)

            doc_sentences = sf.summarizer.sentences

            summaries_parse_info = [list(topic.get_models(parsed=True)), list(topic.get_models(parsed=True))]
            if concept_type is "parse":
                sumewrap.s.sentences = sumewrap.load_sume_sentences(summaries, parse_type=concept_type,
                                                                    parse_info=list(summaries_parse_info))
                sumewrap.s.extract_ngrams2(concept_type="phrase")
            else:
                sumewrap.s.sentences = sumewrap.load_sume_sentences(summaries)
                sumewrap.s.extract_ngrams2()
            sumewrap.s.compute_document_frequency()
            model_sentences = sumewrap.s.sentences

            #
            #  token_details
            #
            for s in doc_sentences:
                sentence_pos = s.position
                doc_id = s.doc_id
                token_from_summary = False
                token_from_document = True
                for concept in s.concepts:
                    ngrams = concept.split(' ')
                    for token in ngrams:
                        pos = "UNK"
                        try:
                            word, pos = s.tokens_pos[token].split('::')
                        except:
                            token = re.sub('[-\.](\s|$)', '\\1', token)
                            try:
                                word, pos = s.tokens_pos[concept].split('::')
                            except:
                                word, pos = token, 'NN'
                        token_details.append({
                            "sentence_pos": sentence_pos,
                            "doc_id": doc_id,
                            "topic": topic.get_name(),
                            "dataset": d.get_name(),
                            "language": d.get_language(),
                            "token": token,
                            "word": word,
                            "pos_tag": pos,
                            "from_summary": token_from_summary,
                            "from_document": token_from_document,
                            "concept_type": concept_type,
                            "embedding_variant": embedding_variant,
                            "token_has_embedding": embeddings.isKnown(token),
                            "word_has_embedding": embeddings.isKnown(word)
                        })
            for s in model_sentences:
                sentence_pos = s.position
                doc_id = s.doc_id
                token_from_summary = True
                token_from_document = False
                for concept in s.concepts:
                    ngrams = concept.split(' ')
                    for token in ngrams:
                        pos = "UNK"
                        try:
                            word, pos = s.tokens_pos[token].split('::')
                        except:
                            token = re.sub('[-\.](\s|$)', '\\1', token)
                            try:
                                word, pos = s.tokens_pos[concept].split('::')
                            except:
                                word, pos = token, 'NN'

                        token_details.append({
                            "sentence_pos": sentence_pos,
                            "doc_id": doc_id,
                            "topic": topic.get_name(),
                            "dataset": d.get_name(),
                            "language": d.get_language(),
                            "token": token,
                            "word": word,
                            "pos_tag": pos,
                            "from_summary": token_from_summary,
                            "from_document": token_from_document,
                            "concept_type": concept_type,
                            "embedding_variant": embedding_variant,
                            "token_has_embedding": embeddings.isKnown(token),
                            "word_has_embedding": embeddings.isKnown(word)
                        })

    # post-process token details
    token_df = pd.DataFrame(token_details)
    # token_df.groupby("dataset")
    # print(token_df.head())
    filename = "C:\\Users\\hatieke\\.ukpsummarizer\\tmp\\tokens_new.csv"
    print("saving token_df to ", filename)
    token_df.to_csv(filename, encoding="UTF-8")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Preprocessing dataset analysis (word embeddings etc)')

    parser.add_argument('-io', '--iobasedir',
                        type=str,
                        help='IO base directory',
                        required=False,
                        default=path.normpath(path.join(path.expanduser("~"), ".ukpsummarizer")))
    parser.add_argument('-r', '--rouge',
                        type=str,
                        help='Rouge: ROUGE directory',
                        required=False,
                        default="rouge/ROUGE-1.5.5")

    subparsers = parser.add_subparsers(help="choose between different modes of operation")
    preprocess = subparsers.add_parser("preprocess",
                                       help="preprocessing related commands")
    preprocess.add_argument("--op",
                            help="operation",
                            type=str,
                            choices=["make", "vocab"], required=True)
    exclusive = preprocess.add_mutually_exclusive_group(required=True)
    exclusive.add_argument("--dataset",
                           help="the dataset to process, should be subdir of iobasedir",
                           action="append",
                           default=[])
    exclusive.add_argument("--topic",
                           help="the topic to process, should be subdir of iobasedir, and contain a processed topic",
                           action="append",
                           default=[])

    # preprocess_data = preprocess.add_subparsers(help="preprocessing commands")
    # preprocess_dataset = preprocess_data.add_parser("dataset",
    #                                                 help="data preprocessing tool. prepare raw dataset for summarization")
    # preprocess_dataset.add_argument("")

    # postprocess = subparsers.add_parser("postprocess",
    #                                     help="Postprocessing of results. Convering of raw results into pretty pictures and reports")


    args = parser.parse_args()

    do_preprocess(rouge=Rouge(args.rouge), datasets=args.dataset, topics=args.topic, operation=args.op)
