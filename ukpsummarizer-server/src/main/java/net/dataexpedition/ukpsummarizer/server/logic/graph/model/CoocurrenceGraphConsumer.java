package net.dataexpedition.ukpsummarizer.server.logic.graph.model;

import de.tudarmstadt.ukp.dkpro.core.api.segmentation.type.NGram;
import de.tudarmstadt.ukp.dkpro.core.api.segmentation.type.Sentence;
import org.apache.uima.analysis_engine.AnalysisEngineProcessException;
import org.apache.uima.fit.component.JCasAnnotator_ImplBase;

import static org.apache.uima.fit.util.JCasUtil.*;

import org.apache.uima.fit.descriptor.ConfigurationParameter;
import org.apache.uima.jcas.JCas;
import org.jetbrains.annotations.NotNull;
import org.jgrapht.ext.ExportException;
import org.jgrapht.ext.GraphMLExporter;
import org.jgrapht.graph.DefaultDirectedWeightedGraph;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.util.*;


public class CoocurrenceGraphConsumer extends JCasAnnotator_ImplBase {
    private static final Logger LOG = LoggerFactory.getLogger(CoocurrenceGraphConsumer.class);

    public static final String DUMP_FILE = "DUMP_FILE";

    public static final String LABEL_ATTRIBUTE_NAME = "label";
    public static final String COUNT_ATTRIBUTE_NAME = "count";
    public static final String WEIGHT_ATTRIBUTE_NAME = "weight";

    @ConfigurationParameter(name = DUMP_FILE)
    File dumpFile;

    DefaultDirectedWeightedGraph<CoocurrenceVertex, CooccurenceEdge> graph;

    public CoocurrenceGraphConsumer() {
        this.graph = new DefaultDirectedWeightedGraph<>(CooccurenceEdge.class);
    }


    @Override
    public void process(JCas aJCas) throws AnalysisEngineProcessException {
        for (Sentence s : select(aJCas, Sentence.class)) {
            List<NGram> nGrams = selectCovered(NGram.class, s);
            for (NGram ngram : nGrams) {
                final CoocurrenceVertex sourceVertex = lookupVertex(ngram);
                List<NGram> followingNgrams = selectFollowing(NGram.class, ngram, 3);
                for (NGram follower : followingNgrams) {
                    final CoocurrenceVertex targetVertex = lookupVertex(follower);
                    addEdge(sourceVertex, targetVertex);
                }
            }
        }
    }

    private void addEdge(CoocurrenceVertex lastVertex, CoocurrenceVertex currentVertex) {
        CooccurenceEdge edge = graph.getEdge(lastVertex, currentVertex);
        if (edge == null) {
            edge = new CooccurenceEdge();
            edge.id = lastVertex.id + " -> " + currentVertex.id;
            graph.addEdge(lastVertex, currentVertex, edge);
            graph.setEdgeWeight(edge, 1.0);
        } else {
            graph.setEdgeWeight(edge, graph.getEdgeWeight(edge) + 1);
        }
    }

    @NotNull
    private CoocurrenceVertex lookupVertex(NGram ngram) {
        Optional<CoocurrenceVertex> vertex = graph.vertexSet().stream()
                .filter(v -> v.id.trim().equalsIgnoreCase(ngram.getText().trim())).findFirst();
        if (!vertex.isPresent()) {
            CoocurrenceVertex v = new CoocurrenceVertex(ngram);
            graph.addVertex(v);
            vertex = Optional.of(v);
        } else {
            vertex.get().occurences += 1;
        }

        return vertex.get();
    }

    @Override
    public void collectionProcessComplete() throws AnalysisEngineProcessException {
        super.collectionProcessComplete();

        GraphMLExporter<CoocurrenceVertex, CooccurenceEdge> exporter = new GraphMLExporter<>();
        exporter.setVertexLabelAttributeName(LABEL_ATTRIBUTE_NAME);
        exporter.setVertexLabelProvider(v -> v.id);
        exporter.registerAttribute(COUNT_ATTRIBUTE_NAME, GraphMLExporter.AttributeCategory.NODE, GraphMLExporter.AttributeType.DOUBLE);

        exporter.setVertexAttributeProvider(component -> {
            HashMap<String, String> m = new HashMap<>();
            m.put(COUNT_ATTRIBUTE_NAME, component.occurences.toString());
            return m;
        });

        exporter.setEdgeLabelProvider(e -> e.id);
        exporter.setEdgeLabelAttributeName(LABEL_ATTRIBUTE_NAME);

        exporter.setExportEdgeWeights(true);
        exporter.setEdgeWeightAttributeName(WEIGHT_ATTRIBUTE_NAME);
        try {
            exporter.exportGraph(graph, dumpFile);
        } catch (ExportException e) {
            e.printStackTrace();
            throw new AnalysisEngineProcessException(e);
        }
    }
}
