import json
import re

from flask import Flask, request
from os import path

from baselines.sume_wrap import SumeWrap
from model.topic import Topic
from summarizer.pipeline import pipeline

app = Flask(__name__)


@app.route("/")
@app.route("/hello/")
def hello():
    return "Hello World!"


@app.route("/cascade/<path>")
def run_pipeline(path=""):
    pipeinfo = pipeline(100, "TEST", "feedback", None, "english", "rouge/RELEASE-1.5.5/", "accept_reject")
    j = json.dumps(pipeinfo)
    return j



def get_summary(topic_path, summary_size=100, oracle="accept_reject", summarizer="sume", parser=None,
                language="english",
                rouge_dir="rouge/RELEASE-1.5.5/"):

    # relativize the topic path!!!!
    if topic_path.startswith("/"):
        relative_path = re.search('^(/)(.*)$', topic_path).group(2)
    else:
        relative_path = topic_path

    resolved_topic_path = path.normpath(path.join(path.expanduser("~"), ".ukpsummarizer", path.normpath(relative_path)))
    topic = Topic(resolved_topic_path)
    docs = topic.get_docs()
    models = topic.get_models()

    if summarizer == "sume":
        sw = SumeWrap(language)
        summary = sw(docs, summary_size)
        return summary
    elif summarizer == "custom_weights":
        sw = SumeWrap(language)


    return "no summary for summarizer type %s" % summarizer


@app.route("/summary", methods=['POST', 'GET'])
def summary_resource():
    # pipeinfo = pipeline(100, "accept_reject", "TEST", "feedback",  None, "english", "rouge/RELEASE-1.5.5/")
    # body2 = request.get_json(True)
    topic = request.args.get('topic', None)
    summary_size = request.args.get('summary_size', 100)
    oracle = request.args.get('oracle', 'accept_reject')
    summarizer = request.args.get('summarizer', 'feedback')
    parser = request.args.get('parser', None)
    language = request.args.get('language', 'english')
    rouge_dir = "rouge/RELEASE-1.5.5/"

    print "producing summary! ", topic, summarizer, oracle, summary_size, parser, language

    sumamry = get_summary(topic, summary_size, oracle, summarizer, parser, language, rouge_dir)

    # pipeline(summary_size, oracle, topic, summarizer, parser, language, rouge_dir)

    retval = json.dumps({"oracle": oracle, "topic": topic, "summary": sumamry})
    # make_response()

    return retval, 200, {"Content-Type": "application/json"};


if __name__ == "__main__":
    app.run(debug=True)
