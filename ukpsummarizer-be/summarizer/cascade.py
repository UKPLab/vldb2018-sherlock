from __future__ import print_function, unicode_literals

import argparse
import codecs

import itertools
import json
import random

from nltk import SnowballStemmer
from os import path

import single_iteration_pipes
import utils.reader
from algorithms.feedback import BaselineFeedbackStore
from algorithms.feedback.ConceptEmbedder import ConceptEmbedder
from algorithms.feedback.SimpleNgramFeedbackGraph import SimpleNgramFeedbackGraph
from algorithms.feedback.WordEmbeddingGaussianFeedbackGraph import WordEmbeddingGaussianFeedbackGraph
from algorithms.feedback.WordEmbeddingRandomWalkDiffusionFeedbackGraph import \
    WordEmbeddingRandomWalkDiffusionFeedbackGraph

from model.dataset import DataSet
from model.topic import Topic
from utils.data_helpers import load_w2v_embeddings
from utils.writer import write_to_file
from web.single_iteration_runner import SingleTopicRunner
from rouge.rouge import Rouge


def get_fbs_args(args):
    # feedbackstore configuration
    if args.rw:
        fbs_type = "rw"
        feedback_store_kwargs = {
            "mass_accept": float(args.rw[0]),
            "mass_reject": float(args.rw[1]),
            "iterations_accept": int(args.rw[2]),
            "iterations_reject": int(args.rw[3]),
            "cut_off_threshold": float(args.rw[4]),
            "propagation_abort_threshold": float(args.rw[5])
        }
    elif args.gb:
        fbs_type = "gb"
        feedback_store_kwargs = {
            "mass_accept": float(args.gb[0]),
            "mass_reject": float(args.gb[1]),
            "iterations_accept": int(args.gb[2]),
            "iterations_reject": int(args.gb[3]),
            "cut_off_threshold": float(args.gb[4])
        }
    elif args.cg:
        fbs_type = "cg"
        feedback_store_kwargs = {
            "N": float(args.cg[0]),
            "factor_accept": int(args.cg[1]),
            "factor_reject": int(args.cg[2])
        }
    else:
        fbs_type = "bl"
        feedback_store_kwargs = {}

    return fbs_type, feedback_store_kwargs


def get_fbs(fbclass, fbkwargs, embedder=None, language=None, stemmer=None):
    if fbclass == "rw":
        return WordEmbeddingRandomWalkDiffusionFeedbackGraph(embedder, **fbkwargs)
    elif fbclass == "gb":
        return WordEmbeddingGaussianFeedbackGraph(embedder, **fbkwargs)
    elif fbclass == "cg":
        return SimpleNgramFeedbackGraph(stemmer, language, **fbkwargs)
    else:
        return None


class Summary(object):
    def __init__(self, summary_file):
        p, f = path.split(summary_file)
        self.topic = Topic(path.normpath(path.join(p, "..")))

        self.idx = None

        for i, (fn, t) in enumerate(self.topic.get_models()):
            if fn.startswith(summary_file):
                self.idx = i

        print(f, self.idx)

    def get_index(self):
        return self.idx

    def get_topic(self):
        return self.topic

if __name__ == '__main__':

    def resolve_filename(filename, base="~/.ukpsummarizer"):
        p, f = path.split(path.join(base, filename))
        if path.exists(p):
            resolved_name = path.join(base, filename)
        else:
            p, f = path.split(filename)
            if path.exists(p):
                resolved_name = filename
            else:
                raise BaseException("Cannot resolve %s to a existing path" % (filename))

        return resolved_name


    import logging
    import logging.config

    logging.config.fileConfig('logging.conf')
    log = logging.info

    parser = argparse.ArgumentParser(
        description='CASCADE - Computer Assisted Summarization Combatting Accelerated Decline in Electronic Journalism')

    # I/O-based arguments
    io = parser.add_argument_group("I/O")
    io.add_argument('-r', '--rouge', type=str, help='Rouge: ROUGE directory', required=False,
                    default="rouge/RELEASE-1.5.5/")
    io.add_argument('-io', '--iobasedir', type=str, help="IO base directory. default is '~/.ukpsummarizer'",
                    required=False,
                    default=path.join(path.expanduser("~"), ".ukpsummarizer"))

    io.add_argument('-out', '--output_filename', type=str, help="output file for JSON", default='tmp/output', required=False)
    # io.add_argument('-fb', '--feedback', type=str, help="List of feedbacks to incorporate prior running any summarization.", default=None, required=False)
    io.add_argument('--scores_dir', type=str, help="scores dumping directory", default="scores_cascade", required=False)
    io.add_argument('--override_results', action='store_true',
                    help="Set to true if the system should override existing results files. Use with care.",
                    required=False)

    subparsers = parser.add_subparsers(help="different modes of operation are available", dest='command')

    # recommender_parser = subparsers.add_parser("recommend", help="Get recommendations for an existing base summary. Will be created if not already existing")
    # recommender_parser.add_argument("topic", type=str)

    summarizer_parser = subparsers.add_parser("summarize", help="Summarize a topic, a dataset, or whatever")
    summarizer_parser.add_argument("file", help="dataset, topic or modelsummary relative to the iobasedir", type=str)

    #    dataset_parser = subparsers.add_parser("dataset", help="Run on full dataset")
    #  single_iteration_pipes.add_args(dataset_parser)

    fi = summarizer_parser.add_argument_group("feedback interpretation")

    fim = fi.add_mutually_exclusive_group()
    fim.add_argument("-bl", action="store_true", help="baseline, used if not configured")
    fim.add_argument("-rw", nargs=6, type=str, help="random walk", metavar=("ma", "mr", "ia", "ir", "co", "pat"))
    fim.add_argument("-gb", nargs=5, type=str, help="gaussian blur", metavar=("ma", "mr", "ia", "ir", "co"))
    fim.add_argument("-cg", nargs=3, type=str, help="cooccurence graph", metavar=("ws", "fa", "fr"))

    sc = summarizer_parser.add_argument_group("Summarizer configuration")
    sc.add_argument("-s", "--summarizer", choices=["SUME", "UPPER_BOUND", "PROPAGATION"],
                    help='The summarizer that should be used. TODO: add the baseline-summarizers, too')
    sc.add_argument("--max_iteration_count", type=int, default=10)
    sc.add_argument("--oracle",
                    choices=['active_learning', 'active_learning2', "ilp_feedback", 'accept_reject', 'accept', 'reject',
                             'accept_all', 'reject_all', 'CUSTOM_WEIGHT', 'top_n', 'keeptrack', 'user'],
                    type=str,
                    default="accept",
                    help="Oracle defines (a) how content is selected for feedback and (b) how feedback is interpreted, "
                         "when using BaselineFeedbackStore (not for the others)")
    sc.add_argument("--max_models", type=int,
                    help='If you want to limit the number of models to run on, this is your way to restrict it.',
                    default=0, required=False)
    sc.add_argument("--concept_type", choices=["parse", "ngrams"], default=None)
    # sc.add_argument("--weights", type=str, help='Json File which contains the weights for the sumarizer.')

    summarizer_parser.add_argument("--pickleout", type=str,
                                   help="Use to pickle summarizer (only the first instance. => early abort is a good thing)")



    continuation_parser = subparsers.add_parser("continue",
                                                help="Runs a single iteration on an existing (pickled) instance of the summarizer")
    continuation_parser.add_argument("--picklein", help="location of the pickled summarizer", type=str)
    continuation_parser.add_argument("--feedback", help="location of the new feedback", type=str)
    continuation_parser.add_argument("--pickleout", help="location where the result summarizer should be stored to",
                                     type=str)
    continuation_parser.add_argument("--oracle_labels", type=str,
                                     help='Json File which contains labeled data that has been provided by a human user')

    #### dataset and other data-related settings
    dc = summarizer_parser.add_argument_group("Data configuration")

    dc.add_argument("--summary_size", type=int,
                    help="override the target size for the summary. Normally this defaults to the size used in the dataset")


    #### just rouge
    rouge_parser = subparsers.add_parser("rouge", help="Calculate the rouge scores given a file with plain text and a topic")
    rouge_parser.add_argument("reference", help="dataset, topic or modelsummary relative to the iobasedir", type=str)
    rouge_parser.add_argument("input", help="the file which contains the text to calc rouge for",type=str)

    args = parser.parse_args()

    iobasedir = path.expanduser(path.normpath(args.iobasedir.replace("\"","")))

    #args.output_filename= path.join(iobasedir, args.output_filename)
    log("Output file: %s" % (args.output_filename))
    if args.command == 'continue':
        log("continue !")

        runner = SingleTopicRunner(iobasedir,
                                   args.rouge,
                                   scores_dir=args.scores_dir.replace("\"",""),
                                   out=args.output_filename.replace("\"",""),
                                   override_results_files=args.override_results)

        if args.oracle_labels is not None:

            js = json.load(open(path.normpath(args.oracle_labels.replace("\"",""))))
        else:
            js = []

        picklein = resolve_filename(args.picklein.replace("\"",""), base=iobasedir)

        if args.pickleout is None:
            pickleout=None
        else:
            pickleout = resolve_filename(args.pickleout.replace("\"",""), base=iobasedir)

        runner.single_iteration(picklein=picklein, pickleout=pickleout,
                                feedbacks=js)

    elif args.command == 'summarize':

        # check if the path refers to a dataset, a topic or a sole model:
        queue = []
        f = utils.reader.resolve_against_iobase(args.file, iobasedir)
        if path.exists(path.join(f, "index.json")):
            # is_dataset
            d = DataSet(f)
            # unroll to get topics
            for t in d.get_topics():
                for (mf, mt) in t.get_models():
                    mf = path.normpath(mf)
                    pref = path.commonprefix([mf, iobasedir])
                    tn = mf[len(pref) + 1:]
                    print("shortened:", tn)
                    queue.append(mf)

                    # topics.append([t.get_name for t in d.get_topics()])

        elif path.exists(path.join(f, "task.json")):
            # is topic
            t = Topic(f)
            for (mf, mt) in t.get_models():
                mf = path.normpath(mf)
                pref = path.commonprefix([mf, iobasedir])
                tn = mf[len(pref) + 1:]
                print("shortened:", tn)
                queue.append(mf)
        elif path.exists(path.join(f, "..", "..", "task.json")) \
                and path.exists(f):
            # should be model
            queue.append(f)
        else:
            raise BaseException("Invalid file given.", f, " is neither a dataset nor a topic nor a model.")

        if args.max_models:
            queue = queue[:args.max_models]

        if args.pickleout:
            queue = queue[:1]


        queue = [Summary(i) for i in queue]

        embeddings_path = path.normpath(path.join(iobasedir, "embeddings"))
        embeddings = {
            "english": None,
            "german": None
        }

        for m in queue:
            t = m.get_topic()
            i = m.get_index()

            log("%s - %s" % (t.get_name(), t.get_language()))

            summary_size = args.summary_size or t.get_summary_size()

            # parse the feedbackstore arguments
            fbclass, fbkwargs = get_fbs_args(args)
            if embeddings[t.get_language()] is None:
                embeddings[t.get_language()] = load_w2v_embeddings(embeddings_path, t.get_language(), "active_learning")
            e = embeddings[t.get_language()]

            fbs = get_fbs(fbclass, fbkwargs, ConceptEmbedder(e), language=t.get_language(),
                          stemmer=SnowballStemmer(t.get_language()))

            if args.pickleout is not None:
                pickleout= resolve_filename(args.pickleout.replace("\"",""))
            else:
                pickleout=None

            runner = SingleTopicRunner(iobasedir,
                                       args.rouge,
                                       scores_dir=args.scores_dir.replace("\"",""),
                                       out=args.output_filename.replace("\"",""),
                                       override_results_files=args.override_results,
                                       pickle_store=pickleout)

            runner.run(t,
                       size=summary_size,
                       summarizer=args.summarizer,
                       summary_idx=i,
                       parser=args.concept_type,
                       oracle=args.oracle,
                       # feedback_log=args.feedback,
                       # propagation=False,
                       max_iteration_count=args.max_iteration_count,
                       preload_embeddings=e,
                       feedbackstore=fbs)

        log("finished SingleTopicRunner")
    elif args.command == 'rouge':
        log("Doing rouge")
        rouge_dir = args.rouge
        outfile = path.normpath(args.output_filename)
        input_file = path.normpath(args.input)
        with codecs.open(input_file, 'r', 'utf-8') as fp:
            text = fp.read().splitlines()
        summary = text or ""

        f = utils.reader.resolve_against_iobase(args.reference, iobasedir)
        if path.exists(path.join(f, "task.json")):
            # is topic
            t = Topic(f)

            # run rouge on topic
            lang = t.get_language()
            max_size = t.get_summary_size()

            # resolved_rouge_dir = path.normpath(path.expanduser(rouge_dir))
            rouge = Rouge(rouge_dir)
            # reference_summaries = [mt for _, mt in t.get_models()]

            r1,r2,r4 = rouge(summary, t.get_models(), max_size)
            outputfilecontents = {
                "R1": r1,
                "R2": r2,
                "R4": r4
            }
            write_to_file(json.dumps(outputfilecontents), outfile)
        else:
            raise BaseException("Invalid file given.", f, " is neither a topic nor a model.")

        log("Done with rouge")
    log("Done")
