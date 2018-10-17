from __future__ import print_function, unicode_literals

import hashlib
import json
import logging
import os
import re
from os import path

import dill as pickle
import dill as pickle
import pandas as pd

from algorithms.flight_recorder import FlightRecorder, Record
from algorithms.oracle.human_oracle import HumanOracle
from algorithms.simulated_feedback import SimulatedFeedback
from algorithms.upper_bound_ilp import ExtractiveUpperbound
from baselines.sume_wrap import SumeWrap
from model.topic import Topic
from rouge.rouge import Rouge
from utils.data_helpers import load_w2v_embeddings
from utils.writer import write_to_file, write_details_file
from utils.load_clusters import get_clusters
import random
from performance_utils.mlogger import MeasurementLogger
from performance_utils.timer import IterationTimer, RunTimer
from performance_utils.mreader import MeasurementReader
import threading

def roundup(x):
    return int(math.ceil(x / 100.0)) * 100


def get_k_limit(data_set, topic):
    # dumsum = sume.ConceptBasedILPSummarizer(" ", language, True)
    # dumsum.sentences = SumeWrap(language).load_sume_sentences(docs, parser_type, parse_info)
    # dumsum.prune_sentences(remove_citations=True, remove_redundancy=True, imp_list=[])
    # topic_sentence_size = len(dumsum.sentences)
    from log_folder import log_folder
    mreader = MeasurementReader()
    mreader.info['dataset'] = data_set
    mreader.read_corpora_stats(log_folder)
    mreader.set_topic_rid(topic)
    topic_sentence_size = mreader.get_corpus_stat("Corpus Size after")
    return topic_sentence_size


def get_flightrecorder_from_file(weights_file=None):
    """
        Parses a json containing the feedbacks. And verifies its layout.

    :param weights_file:
    :return: dict of str -> double
    """
    flightrecorder = FlightRecorder()
    if weights_file is None:
        return flightrecorder
    elif weights_file.endswith(".json"):
        df = pd.read_json(path.normpath(weights_file))
    elif weights_file.endswith(".csv"):
        df = pd.read_csv(path.normpath(weights_file))

    record = Record()
    last_iteration = 0
    for row in df.iterrows():
        if row[1].iteration > last_iteration:
            flightrecorder.add_record(record)
            record = Record()
            last_iteration = row[1].iteration

        if row[1].value.lower() == "accept":
            record.accept.union_update([row[1].concept])
        elif row[1].value.lower() == "reject":
            record.reject.union_update([row[1].concept])
        elif row[1].value.lower() == "implicit_reject":
            record.implicit_reject.union_update([row[1].concept])

    flightrecorder.add_record(record)

    return flightrecorder


def load_ub_summary(language, docs, models, size, ngram_type=2,
                    base_dir=path.normpath(path.expanduser("~/.ukpsummarizer/cache/"))):
    import hashlib
    m = hashlib.sha256()
    shortened_docs = [path.split(f)[1] for (f, _) in docs]
    for doc in sorted(shortened_docs):
        m.update(doc)
    shortened_models = [path.split(f)[1] for (f, _) in models]
    for model in sorted(shortened_models):
        m.update(model)
    m.update(str(size))
    m.update(language)
    m.update(str(ngram_type))
    h = m.hexdigest()
    jsonloc = path.normpath(path.join(base_dir, h + ".json"))
    if path.isfile(jsonloc):
        try:
            ubs = json.load(open(jsonloc))
            upsum = ubs["summary"]
            return upsum
        except:
            pass
    upsum = ExtractiveUpperbound(language)
    ub_summary = upsum(docs, models, size, ngram_type)
    jdict = {"docs": sorted(shortened_docs), "summary": ub_summary, "models": sorted(shortened_models), "size": size,
             "language": language, "ngram_type": ngram_type}
    j = json.dumps(jdict)
    write_to_file(j, jsonloc)
    return ub_summary


def convert_to_json(sentences):
    """

    :param sentences: list(baselines.sume.base.Sentence)
    :return:
    """
    log = logging.getLogger()

    for s in sentences:
        t = {
            "untokenized_form": s.untokenized_form,
            "concepts": s.concepts,
            "untokenized_concepts": s.untokenized_concepts,
            "doc_id": s.doc_id,
            "sent_id": s.position,
            "length": s.length,
            "tokens": s.tokens,
            "phrases": s.phrases,
            "untokenized_phrases": s.raw_phrases
        }
        yield t

class SingleTopicRunner(object):
    tlog = logging.getLogger("timings")

    def __init__(self, iobasedir, rouge_dir, out=None, scores_dir=None, override_results_files=False,
                 pickle_store=None):
        self.iobasedir = path.normpath(path.expanduser(iobasedir))
        # resolved_rouge_dir = path.normpath(path.expanduser(rouge_dir))
        self.rouge = Rouge(rouge_dir)
        if out is None:
            self.out = None
        else:
            self.out = path.normpath(path.expanduser(out))
        if scores_dir is None:
            self.scores_storage_path = path.normpath(path.join(self.iobasedir, "scores_new"))
        else:
            self.scores_storage_path = path.normpath(path.join(self.iobasedir, scores_dir))

        if not path.exists(self.scores_storage_path):
            os.mkdir(self.scores_storage_path)

        self.override_results_switch = override_results_files

        if pickle_store is None:
            self.pickle_store = pickle_store
        else:
            p, f = path.split(path.join(self.iobasedir, pickle_store))
            if path.exists(p):
                self.pickle_store = path.join(self.iobasedir, pickle_store)
            else:
                p, f = path.split(pickle_store)
                if (path.exists(p)):
                    self.pickle_store = pickle_store
                else:
                    raise BaseException(
                        "Cannot resolve %s to a existing path for storing the serialized summarizer" % (pickle_store))

    def single_iteration(self, picklein, pickleout=None, feedbacks=None):
        log = logging.getLogger("SingleTopicRunner")
        log.info("unpickling input %s" % (picklein))

        sf = pickle.load(open(picklein, 'rb'))
        log.info("done unpick input")
        iteration = len(sf.flight_recorder.records) + 1
        labeled_data = feedbacks or []

        svm_flag = 0
        sf.oracle = HumanOracle(labeled_data)

        log.info("records before: %s", len(sf.flight_recorder.records))

        log.info("running iteration")

        samples = [i["concept"] for i in labeled_data]
        score, summary, summary_sentences, unlabeled_data, exploratory_sentences = sf.single_iteration(iteration,
                                                                                                       samples,
                                                                                                       svm_flag)

        sf.__print_iteration_info__(summary_sentences, iteration, summary, score,
                                    unlabeled_data, exploratory_sentences)
        log.info("records after run: %s", len(sf.flight_recorder.records))

        self.write_continue_output_result(sf,
                                          unlabeled_data,
                                          picklein,
                                          pickleout,
                                          summary,
                                          summary_sentences,
                                          exploratory_sentences)

        if pickleout is not None:
            self.pickle_write(sf, pickleout, log)

    def pickle_write(self, sf, pickleout, log):
        output = open(pickleout, 'wb')
        pickle.dump(sf, output)
        output.close()
        log.info("### wrote pickle output to %s" % (pickleout))

    def run(self, topic_path, size=None, summarizer="SUME", summary_idx=None, parser=None,
            oracle="accept", feedback_log=None, propagation=False, max_iteration_count=10, preload_embeddings=None,
            feedbackstore=None, override_results_files=False, num_clusters=8):
        log = logging.getLogger("SingleTopicRunner")

        sf = None  # just for the sake of being able to run without simulated feedback...
        self.tlog.debug("SingleTopicRunner started")
        # relativize the topic path!
        if type(topic_path) is Topic:
            topic = topic_path
        else:
            if topic_path.startswith("/"):
                relative_path = re.search('^(/)(.*)$', topic_path).group(2)
            else:
                relative_path = topic_path

            topic = Topic(path.join(self.iobasedir, path.normpath(relative_path)))
        language = topic.get_language()
        docs = topic.get_docs()
        summaries = topic.get_models()

        flightrecorder = get_flightrecorder_from_file(feedback_log)
        preceding_size = len(
            flightrecorder.records)  # the number of iterations that happened due to the provided feedback_log


        embeddings = None
        """
        if preload_embeddings:
            embeddings_path = path.normpath(path.join(self.iobasedir, "embeddings"))
            embeddings = load_w2v_embeddings(embeddings_path, language, 'active_learning')
        else:
            embeddings = preload_embeddings
        """

        if summary_idx is not None:
            summaries = [summaries[summary_idx]]

        if size is None:
            use_size = topic.get_summary_size()
        else:
            use_size = size

        clusters_path = path.join(self.iobasedir, 'clustering', '{}'.format(num_clusters))
        #print(clusters_path)
        #clusters = get_clusters(clusters_path, topic.docs_dir)

        if summarizer == "SUME":
            sw = SumeWrap(language)
            summary = sw(docs, use_size)
            outputfilecontents = {"summary": summary, "type": summarizer, "info_data": []}

            json_content = json.dumps(outputfilecontents)
            if self.out is not None:
                log.info("writing output to %s" % (self.out))
                write_to_file(json_content, self.out)
            write_to_file(json_content,
                          path.normpath(path.expanduser(path.join(self.iobasedir, "tmp", "tmp.json"))))
        elif summarizer == "UPPER_BOUND":
            ub_summary = load_ub_summary(language, docs, summaries, use_size, base_dir=self.iobasedir)
            summary = '\n'.join(ub_summary)

            outputfilecontents = {"summary": summary, "type": summarizer, "info_data": []}

            json_content = json.dumps(outputfilecontents)
            if self.out is not None:
                log.info("writing output to %s" % (self.out))
                write_to_file(json_content, self.out)
            write_to_file(json_content, path.normpath(path.expanduser(path.join(self.iobasedir, "tmp", "tmp.json"))))
        elif summarizer == "PROPAGATION":
            #UB considering all the summaries
            ub_summary = load_ub_summary(language, docs, summaries, use_size, base_dir=self.iobasedir)
            summary = '\n'.join(ub_summary)
            ub_scores = self.rouge(summary, summaries, use_size)

            log.debug("UB scores: R1:%s R2:%s SU4:%s" % (str(ub_scores[0]), str(ub_scores[1]), str(ub_scores[2])))

            ref_summ = random.choice(summaries)

            parse_info = []
            #parse_info = topic.get_parse_info(summaries.index(ref_summ))

            # initialize the Algorithm.
            run_config = dict()
            run_config['rank_subset'] = True
            run_config['relative_k'] = True
            run_config['dynamic_k'] = False
            for flag in ['adaptive_sampling', 'strategy']:
                run_config[flag] = False

            k = 0.1
            r = 0
            clusters = None

            #TODO: Added summaries instead of one single summary
            sf = SimulatedFeedback(language, self.rouge, embeddings=None,  #TODO: embeddings
                                   docs=docs, models=summaries,
                                   summary_length=use_size,
                                   oracle_type=oracle,
                                   ub_score=ub_scores, ub_summary=ub_summary,
                                   parser_type=parser, flightrecorder=flightrecorder,
                                   feedbackstore=feedbackstore, parse_info=parse_info,
                                   run_config=run_config, k=k, adaptive_window_size=r, clusters=clusters)

            if sf.embeddings is None or sf.embeddings == {}:
                embe_var = "none",
            else:
                if sf.embeddings.embedding_variant is None:
                    embe_var = "none"
                else:
                    embe_var = sf.embeddings.embedding_variant
            if feedbackstore is None:
                cfg = {"type": "Unconfigured default"}
            else:
                cfg = feedbackstore.get_config()

            rs = []
            for p, t in [ref_summ]:
                rs.append({
                    "name": os.path.split(p)[1],
                    "text": t})

            run_id_string = "%s-%s-%s-%s-%s-%s-%s-%s" % (
                oracle, summarizer, parser, embe_var, topic.get_dataset(), topic.get_name(),
                [item["name"] for item in rs], json.dumps(cfg))

            run_id = hashlib.sha224(run_id_string).hexdigest()
            filename = path.join(self.scores_storage_path, "result-%s.json" % (run_id))

            if (os.path.exists(filename)
                and self.out is None
                and self.override_results_switch is False):
                log.info("Skipping run_id '%s' because the result file does already exist. config: %s" % (
                    run_id, run_id_string))
                return
            else:
                log.info("Doing %s iterations for run_id '%s'\n %s" % (max_iteration_count, run_id, run_id_string))
                write_to_file("", filename)

            summary, confirmatory_summary, exploratory_summary = sf.run_full_simulation(
                max_iteration_count=max_iteration_count)

            recommendations, recom_sentences = sf.get_recommendations()

            derived_records = []
            # construct table-like array of feedbacks per iteration.
            for i, record in enumerate(sf.flight_recorder.records):
                for accept in record.accept:
                    derived_records.append({
                        "iteration": i,
                        "concept": accept,
                        "value": "accept"
                    })
                for reject in record.reject:
                    derived_records.append({
                        "iteration": i,
                        "concept": reject,
                        "value": "reject"
                    })
                for implicit_reject in record.implicit_reject:
                    derived_records.append({
                        "iteration": i,
                        "concept": implicit_reject,
                        "value": "implicit_reject"
                    })

            for item in recommendations:
                derived_records.append({
                    "iteration": -1,
                    "concept": item,
                    "value": "recommendation",
                    "weight": sf.summarizer.weights.get(item, 0.0),
                    "uncertainity": sf.svm_uncertainity.get(item, -1.0)
                })

            result = {
                "config_run_id": run_id,
                "config_oracle_type": oracle,
                "config_summarizer_type": summarizer,
                "config_parse_type": str(parser),
                #"config_wordembeddings": emb_var,
                "config_feedbackstore": sf.feedbackstore.get_config(),
                "config_feedback_interpretation": {},
                "config_concept_recommendation": {},
                "dataset": topic.get_dataset(),
                "topic": topic.get_name(),
                "models": rs,
                "model_rougescores": {
                    "iteration": -1,
                    "ROUGE-1 R score": ub_scores[0],
                    "ROUGE-2 R score": ub_scores[1],
                    "ROUGE-SU* R score": ub_scores[2],
                    "accepted": [],
                    "accept_count": 0,
                    "rejected": [],
                    "reject_count": 0,
                    "summary": ub_summary
                },
                "result_summary": summary,
                "result_rougescores": sf.log_sir_info_data,
                "log_feedbacks": derived_records
            }

            r2 = [{"iteration": i, "summary": sf.log_info_data[i]} for i in
                  range(len(sf.flight_recorder.records))]
            log.debug(
                "records: %s, infos %s, diff: %s" % (len(sf.flight_recorder.records), len(sf.log_info_data),
                                                     len(sf.flight_recorder.records) - len(sf.log_info_data)))

            write_to_file(json.dumps(result), filename)
            log.info("Writing results to %s" % (filename))

            df = pd.DataFrame(derived_records)
            filename = path.join(self.scores_storage_path, "flightrecorder-%s.csv" % (run_id))
            log.info("saving flightrecorder to %s with run_id %s" % (filename, run_id))
            df.to_csv(filename, encoding="UTF-8")

            write_to_file(json.dumps(sf.new_debug_weights_history),
                          path.join(self.scores_storage_path, "weightshistory-%s-%s-%s-%s.json" % (
                              topic.get_dataset(), topic.get_name(), summarizer, run_id)))
            log.info("Writing weights history to %s" % (filename))
            weights_hist = pd.DataFrame(sf.new_debug_weights_history)

            filename = path.join(self.scores_storage_path, "weightshistory-%s.csv" % (run_id))
            weights_hist.to_csv(filename, encoding="UTF-8")

            log.debug("----------------------------------------------")
            log.debug(summary)
            log.debug(sf.log_info_data[-1])
            log.debug("----------------------------------------------")
            if self.pickle_store is not None:
                # Pickle dictionary using protocol 0.
                print('Pickle in file %s' % self.pickle_store)
                self.pickle_write(sf, self.pickle_store, log)

            json_content = self.write_summarize_output_json(sf, confirmatory_summary, derived_records, log,
                                                            recom_sentences, result, run_id, summarizer,
                                                            summary, self.pickle_store)
            # write_to_file(json_content, path.normpath(path.expanduser(path.join(self.iobasedir, "tmp", "tmp.json"))))
        else:
            raise BaseException("You should tell which summarizer to use")

        if sf is not None:
            write_details_file([sf.log_info_data], path.join(self.iobasedir, "tmp", "tmp.csv"))
        self.tlog.debug("SingleTopicRunner finished")

    def write_continue_output_result(self,
                                     sf,
                                     unlabeled_data=None,
                                     picklein=None,
                                     pickleout=None,
                                     summary=None,
                                     summary_sentences=None,
                                     exploratory_sentences=None):
        log = logging.getLogger("SingleTopicRunner")
        if self.out is not None:

            derived_records = []
            # construct table-like array of feedbacks per iteration.
            for i, record in enumerate(sf.flight_recorder.records):
                for accept in record.accept:
                    derived_records.append({
                        "iteration": i,
                        "concept": accept,
                        "value": "accept"
                    })
                for reject in record.reject:
                    derived_records.append({
                        "iteration": i,
                        "concept": reject,
                        "value": "reject"
                    })
                for implicit_reject in record.implicit_reject:
                    derived_records.append({
                        "iteration": i,
                        "concept": implicit_reject,
                        "value": "implicit_reject"
                    })

            for item in unlabeled_data:
                if item not in [i.get("concept", "") for i in derived_records]:
                    derived_records.append({
                        "iteration": -1,
                        "concept": item,
                        "value": "recommendation",
                        "weight": sf.summarizer.weights.get(item, 0.0),
                        "uncertainity": sf.svm_uncertainity.get(item, -1.0)
                    })
                else:
                    log.info("recommendation included a already labeled instance, '%s'" % (item))

            outputfilecontents = {
                "picklein": picklein,
                "pickleout": pickleout,
                "summary": summary,
                "confirmatory_summary": list(summary_sentences),
                "exploratory_summary": list(exploratory_sentences),
                "weights": sf.summarizer.weights,
                "fbs_weights": dict(sf.feedbackstore.get_weights()),
                "sentence_ids": list(summary_sentences),
                "details": derived_records,
                "score": sf.log_sir_info_data
            }

            write_to_file(json.dumps(outputfilecontents), self.out)
            log.info("writing output to %s" % (self.out))
        log.info("done writing output")

    def write_summarize_output_json(self, sf, confirmatory_summary, derived_records, log, recom_sentences,
                                    result, run_id, summarizer, summary, pickle_store=None):
        # convert the sentences into a jsonizable structure:
        sents = convert_to_json(sf.summarizer.sentences)
        outputfilecontents = {
            "picklein": None,
            "pickleout": pickle_store,
            "summary": summary,
            "confirmatory_summary": list(confirmatory_summary),
            "exploratory_summary": list(recom_sentences),
            "type": summarizer,
            "run_id": run_id,
            "weights": sf.summarizer.weights,
            "fbs_weights": dict(sf.feedbackstore.get_weights()),
            "details": derived_records,
            "sentences": list(sents),
            "full": result,
            "score": sf.log_sir_info_data
        }
        json_content = json.dumps(outputfilecontents)
        if self.out is not None:
            log.info("writing output to %s" % (self.out))
            write_to_file(json_content, self.out)
        return json_content
