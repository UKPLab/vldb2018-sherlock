from __future__ import print_function, unicode_literals
import argparse

import datetime

from os import path

from web.single_iteration_runner import SingleTopicRunner
from web.gridsearch import GridSearch
import logging
import logging.config


def add_args(parser):
    # -- summary_len: 100, 250, 400
    parser.add_argument('-s', '--summary_size', type=str, help='Summary Length, going to be optional', required=False)

    # --oracle_type:  reject_all, accept_all, weighted, top_n, keep_track
    parser.add_argument('-o', '--oracle_type', type=str, help='Oracle Type: reject_all, accept_all, weighted, top_n',
                        required=False, default="")

    # --data_set: DUC2001, DUC2002, DUC2004
    parser.add_argument('-tp', '--topic_path', type=str, help='Topic path. Has to be a subdirectory of iobasedir', required=True)

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

    parser.add_argument('-out', '--output_filename', type=str, help="IO file for java/python information exchange", default=None, required=False)

    parser.add_argument('-weights', "--initial_weights_file", type=str, help="IO file location of file providing weights", required=False)

    parser.add_argument('-sid', '--summary_index', type=int, help='Which summary index to process, process all, if not used', required=False, default=None)

    parser.add_argument('-iterations', '--iterations', type=int,
                        help='How many iterations to run on the summarizer', required=False, default=25)

    parser.add_argument('-grid', '--gridsearch', type=bool, help="If True, a grid search is performed on the given topic.", required=False, default=False)


if __name__ == '__main__':
    logging.config.fileConfig('logging.conf')

    parser = argparse.ArgumentParser(description='Individual Topic summarizer')

    add_args(parser)

    args = parser.parse_args()
    summary_size = args.summary_size
    oracle_type = args.oracle_type
    topic_path = args.topic_path
    summarizer_type = args.summarizer_type
    parser_type = args.parser_type
    language = args.language
    rouge_dir = args.rouge
    iobasedir = args.iobasedir
    out = args.output_filename
    iterations = args.iterations
    initial_weights_file = args.initial_weights_file
    do_grid_search = args.gridsearch
    summary_index = args.summary_index

    logging.info("+---------------- Running single iteration ------------------")
    logging.info("| configuration settings:")
    logging.info("| grid search is: %s" % (do_grid_search))
    logging.info("| summary_size    %s" % (summary_size))
    logging.info("| topic_path      %s" % (topic_path))
    logging.info("| oracle_type     %s" % (oracle_type))
    logging.info("| summarizer_type %s" % (summarizer_type))
    logging.info("| parser_type     %s" % (parser_type))
    logging.info("| iterations      %s" % (iterations))
    logging.info("| summary_index   %s" % (summary_index))
    logging.info("| lang            %s" % (language))
    logging.info("| rouge_dir       %s" % (rouge_dir))
    logging.info("| iobasedir       %s" % (iobasedir))
    logging.info("| out             %s" % (out))
    logging.info("| weights_file    %s" % (initial_weights_file))
    logging.info("+--------------------------------------------------------")

    start = datetime.datetime.now()
    logging.info("starting on      %s" % (str(start)))
    logging.info("+--------------------------------------------------------")

    if summarizer_type == "PROPAGATION":
        prop = True
    else:
        prop = False

    if do_grid_search:
        runner = GridSearch(iobasedir, rouge_dir, out)
        runner.run(topic_path,  max_iteration_count=5, parser=parser_type)
    else:
        runner = SingleTopicRunner(iobasedir, rouge_dir, out)
        runner.run(topic_path=topic_path, summarizer=summarizer_type, parser=parser_type,
                   oracle=oracle_type, size=summary_size, feedback_log=initial_weights_file, max_iteration_count=iterations,
                   summary_idx=summary_index)

    logging.info("+--------------------------------------------------------")
    end = datetime.datetime.now()
    logging.info("| stopped on %s" %(str(end)))
    logging.info("| total time: %s" %(str(end - start)))
    logging.info("+--------------------------------------------------------")
