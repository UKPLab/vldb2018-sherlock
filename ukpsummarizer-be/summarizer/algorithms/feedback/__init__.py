import copy
import datetime
import json
import tempfile
import time

import logging
import networkx as nx
import numpy as np

from utils.writer import write_to_file


def print_graph_stats(G, class_type=None):
    stats = {}
    g = stats["graph"] = {}
    if class_type is not None:
        g["_type"] = class_type
    g["name"] = G.name
    type_name = [type(G).__name__]
    g["type_name"] = ",".join(type_name)
    g["node_count"] = G.number_of_nodes()
    g["edge_count"] = G.number_of_edges()
    if len(G) > 0:
        if G.is_directed():
            g["type"] = "directed"
            g["indegree_average"] = (sum(G.in_degree().values()) / float(g["node_count"]))
            g["outdegree_average"] = (sum(G.out_degree().values()) / float(g["node_count"]))
        else:
            g["type"] = "undirected"
            g["degree_average"] = (sum(G.degree().values())) / float(g["node_count"])
            g["degree_min"] = min(G.degree().values())
            g["degree_max"] = max(G.degree().values())

    g["density"] = nx.density(G)
    if not nx.is_directed(G):
        g["components"] = []

        cc_nodes = []
        cc_edges = []
        for CC in nx.connected_component_subgraphs(G):
            cc = {"nodes": len(CC.nodes()), "edges": len(CC.edges())}

            cc_nodes.append(len(CC.nodes()))
            cc_edges.append(len(CC.edges()))

            g["components"].append(cc)
        (ccn_arr, ccn_bins) = np.histogram(cc_nodes, bins="sturges")
        (cce_arr, cce_bins) = np.histogram(cc_edges, bins="sturges")
        g["components_overview"] = {
            "count": nx.number_connected_components(G),
            "nodes_hist": {
                "bins": ccn_bins.tolist(),
                "data": ccn_arr.tolist()
            },
            "edges_hist": {
                "bins": cce_bins.tolist(),
                "data": cce_arr.tolist()
            }
        }

    (dha, dhb) = np.histogram(G.degree().values(), bins="sturges")
    g["degree_hist"] = {
        "bins": dhb.tolist(),
        "data": dha.tolist()
    }

    feedbacks = []
    dfs = []
    for (n, d) in G.nodes(data=True):
        dfs.append(d["df"])
        feedbacks.append(d["feedback"][-1:][0])

    (fha, fhb) = np.histogram(feedbacks, bins="sturges")
    g["feedbacks_histogram"] = {
        "bins": fhb.tolist(),
        "data": fha.tolist()
    }
    (cha, chb) = np.histogram(dfs, bins="sturges")
    g["concepts_histogram"] = {
        "bins": chb.tolist(),
        "data": cha.tolist()
    }

    t = datetime.datetime.now()
    ts = int(time.mktime(t.timetuple()))
    temp = tempfile.mktemp(prefix=str(ts), suffix=".json")
    logging.getLogger("io").info("feedback-graph stats dumped to ", temp)
    write_to_file(json.dumps(stats), temp)
    # dump_details(G)


def dump_details(G):
    t = datetime.datetime.now()
    ts = int(time.mktime(t.timetuple()))
    temp = tempfile.mktemp(prefix=str(ts), suffix=".gexf")
    # g = nx.copy(G)
    # g = nx.relabel_gexf_graph(G)

    CG = copy.deepcopy(G)
    g = nx.convert_node_labels_to_integers(CG)

    for (n, v) in g.nodes(data=True):
        for (k, i) in v.items():
            if isinstance(i, np.float64):
                v[k] = float(i)
            elif k is "feedback":
                v[k] = float(v[k][-1:][0])

                # if isinstance(i, list):
                #     for lv in i:
                #         if isinstance(lv, np.float64):
                #             lv=float(lv)

    for (u, v, d) in g.edges(data=True):
        for (k, i) in d.items():
            if isinstance(i, np.float64):
                g[u][v][k] = float(i)

    print("dumping G = (V: %s, E: %s) to disk %s"
          % (len(g.nodes()), len(g.edges()), temp))
    # for n in g.nodes():
    #     n["vector"]=None
    # nx.set_node_attribute(g, "vector", None)
    nx.write_gexf(g, temp)
    print("dumped G = (V: %s, E: %s) to disk %s"
          % (len(g.nodes()), len(g.edges()), temp))


def rescale_thing(arr):
    return int(float(max(arr) - min(arr)) / float(min(5, max(arr)))) + 1