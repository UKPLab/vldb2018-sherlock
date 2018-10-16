from __future__ import print_function, unicode_literals
import argparse

import datetime

from os import path

from web.dataset_runner import DatasetRunner

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='DatasetRunner summarizer')
    # -- summary_len: 100, 250, 400
    parser.add_argument('-s', '--summary_size', type=str, help='Summary Length, going to be optional', required=False)

    # --oracle_type:  reject_all, accept_all, weighted, top_n, keep_track
    parser.add_argument('-o', '--oracle_type', type=str, help='Oracle Type: reject_all, accept_all, weighted, top_n',
                        required=False, default="")

    # --data_set: DUC2001, DUC2002, DUC2004
    parser.add_argument('-data', '--data_path', type=str, help='data. Has to be a relative path of iobasedir, conating topics as dubdirectories', required=True)

    # --summarizer_type: upper_bound, feedback
    parser.add_argument('-t', '--summarizer_type', type=str, help='Summarizing Algorithm: upper_bound, sume, feedback',
                        required=True)

    # --parser_type: parse, ner
    parser.add_argument('-p', '--parser_type', type=str, help='Parser info type: None, parse, ner', required=False)

    # --language: english, german
    parser.add_argument('-l', '--language', type=str, help='Language: english, german; going to be optional', required=True) # TODO

    # --rouge: "~"
    parser.add_argument('-r', '--rouge', type=str, help='Rouge: ROUGE directory', required=False,
                        default="rouge/ROUGE-1.5.5")

    # --rouge: "~"
    parser.add_argument('-io', '--iobasedir', type=str, help='IO base directory', required=False,
                        default=path.join(path.dirname(path.abspath(__file__)), "data"))

    parser.add_argument('-out', '--output_filename', type=str, help="IO file for java/python information exchange", required=False)

    parser.add_argument('-weights', "--initial_weights_file", type=str, help="IO file location of file providing weights", required=False)

    parser.add_argument('-sid', '--summary_index', type=int,
                        help='Which summary index to process, process all, if not used', required=False, default=None)

    args = parser.parse_args()
    summary_size = args.summary_size
    oracle_type = args.oracle_type
    data_path = args.data_path
    summarizer_type = args.summarizer_type
    parser_type = args.parser_type
    language = args.language
    rouge_dir = args.rouge
    iobasedir = args.iobasedir
    out = args.output_filename
    initial_weights_file = args.initial_weights_file
    summary_idx = args.summary_index

    print("+---------------- Running DatasetRunner iteration ------------------")
    print("| configuration settings:")
    print("| summary_size", summary_size)
    print("| data_path", data_path)
    print("| oracle_type", oracle_type)
    print("| summarizer_type", summarizer_type)
    print("| parser_type", parser_type)
    print("| lang ", language)
    print("| rouge_dir ", rouge_dir)
    print("| iobasedir", iobasedir)
    print("| out", out)
    print("| weights_file", initial_weights_file)
    print("| summaries_idx", summary_idx)
    print("+--------------------------------------------------------")

    start = datetime.datetime.now()
    print("starting DatasetRunner on ", str(start))
    print("+--------------------------------------------------------")

    if summarizer_type == "PROPAGATION":
        prop = True
    else:
        prop = False

    runner = DatasetRunner(iobasedir, rouge_dir, out)
    runner.run(dataset_path=data_path, summarizer=summarizer_type, parser=parser_type,
               oracle=oracle_type, size=int(summary_size), weights_file=initial_weights_file, propagation=prop,
               max_iteration_count=25, summary_idx=summary_idx)

    print("+--------------------------------------------------------")
    end = datetime.datetime.now()
    print("| stopped on ", str(end))
    print("| total time: ", str(end - start))
    print("+--------------------------------------------------------")
