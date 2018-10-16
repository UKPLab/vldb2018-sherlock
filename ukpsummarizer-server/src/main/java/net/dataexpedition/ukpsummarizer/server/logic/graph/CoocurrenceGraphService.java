package net.dataexpedition.ukpsummarizer.server.logic.graph;

import de.tudarmstadt.ukp.dkpro.core.api.segmentation.type.StopWord;
import de.tudarmstadt.ukp.dkpro.core.dictionaryannotator.DictionaryAnnotator;
import de.tudarmstadt.ukp.dkpro.core.stanfordnlp.StanfordLemmatizer;
import de.tudarmstadt.ukp.dkpro.core.stanfordnlp.StanfordPosTagger;
import de.tudarmstadt.ukp.dkpro.core.stanfordnlp.StanfordSegmenter;
import net.dataexpedition.ukpsummarizer.server.logic.graph.model.CoocurrenceGraphConsumer;
import net.dataexpedition.ukpsummarizer.server.logic.graph.model.CoocurrenceVertex;
import net.dataexpedition.ukpsummarizer.server.logic.graph.model.FilterNgramsCoveringPunctuationCasAnnotator;
import net.dataexpedition.ukpsummarizer.server.logic.graph.model.Topic;
import net.dataexpedition.ukpsummarizer.server.logic.graph.model.CooccurenceEdge;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import static org.apache.uima.fit.factory.AnalysisEngineFactory.createEngine;
import static org.apache.uima.fit.factory.AnalysisEngineFactory.createEngineDescription;
import static org.apache.uima.fit.factory.CollectionReaderFactory.createReader;
import static org.apache.uima.fit.factory.CollectionReaderFactory.createReaderDescription;
import static org.apache.uima.fit.pipeline.SimplePipeline.runPipeline;

import de.tudarmstadt.ukp.dkpro.core.io.text.TextReader;
import org.apache.uima.UIMAException;
import de.tudarmstadt.ukp.dkpro.core.ngrams.NGramAnnotator;
import org.apache.uima.fit.factory.AnalysisEngineFactory;
import org.jgrapht.ext.*;
import org.jgrapht.graph.DefaultDirectedWeightedGraph;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

/**
 * Graph service based on ngram collocations
 */
@Service
public class CoocurrenceGraphService implements GraphService {
    public static final Logger LOG = LoggerFactory.getLogger(CoocurrenceGraphService.class);

    @Override
    public Map<String, Integer> getWeights(Topic t) {
        DefaultDirectedWeightedGraph<CoocurrenceVertex, CooccurenceEdge> graph = new DefaultDirectedWeightedGraph(CooccurenceEdge.class);
        try {
            Path tempFile = Files.createTempFile("cascade-graph-weights-", ".graphml");
            File temp = tempFile.toFile();
//            AnalysisEngineDescription desc = AnalysisEngineFactory.createPrimitiveDescription(
//                    Annotator.class,
//                    Annotator.FREQUENCY_COUNT_RESOURCE, ExternalResourceFactory.createExternalResourceDescription(
//                            Web1TFrequencyCountResource.class,
//                            Web1TFrequencyCountResource.PARAM_MIN_NGRAM_LEVEL, "1",
//                            Web1TFrequencyCountResource.PARAM_MAX_NGRAM_LEVEL, "3",
//                            Web1TFrequencyCountResource.PARAM_INDEX_PATH, indexPath
//                    )
//            );
            runPipeline(
                    createReaderDescription(TextReader.class,
                            TextReader.PARAM_SOURCE_LOCATION, t.path + "/docs/*",
                            TextReader.PARAM_LANGUAGE, "en"),
                    createEngineDescription(StanfordSegmenter.class),
                    createEngineDescription(StanfordPosTagger.class),
                    createEngineDescription(StanfordLemmatizer.class),
                    createEngineDescription(DictionaryAnnotator.class,
                            DictionaryAnnotator.PARAM_ANNOTATION_TYPE, StopWord.class,
                            DictionaryAnnotator.PARAM_MODEL_LOCATION, new File(getClass().getClassLoader().getResource("cl/stopwords/english").getFile())),
//                    createEngineDescription(PosFilter.class, PosFilter.PARAM_PUNC, true, PosFilter.PARAM_TYPE_TO_REMOVE, PUNC.class.getName()),
                    createEngineDescription(NGramAnnotator.class, NGramAnnotator.PARAM_N, 2),
                    AnalysisEngineFactory.createEngineDescription(FilterNgramsCoveringPunctuationCasAnnotator.class),
//                    createEngineDescription(StopWordRemover.class),
//                    createEngineDescription(FilterNGramsConsistingOfStopwordsOnly.class),
                    AnalysisEngineFactory.createEngineDescription(CoocurrenceGraphConsumer.class, CoocurrenceGraphConsumer.DUMP_FILE, temp.getCanonicalPath())
            );


            VertexProvider<CoocurrenceVertex> vertexProv = (label, attributes) -> {
                CoocurrenceVertex v = new CoocurrenceVertex();
                v.id = label;
                v.lemmatized = attributes.getOrDefault(CoocurrenceGraphConsumer.LABEL_ATTRIBUTE_NAME, "");
                v.occurences = Double.valueOf(attributes.getOrDefault(CoocurrenceGraphConsumer.COUNT_ATTRIBUTE_NAME, "1.0"));
                return v;
            };


            EdgeProvider<CoocurrenceVertex, CooccurenceEdge> edgeProv = (from, to, label, attributes) -> {
                CooccurenceEdge edge = new CooccurenceEdge();
                edge.id = from.id + " -> " + to.id;
                return edge;
            };
            GraphMLImporter<CoocurrenceVertex, CooccurenceEdge> importer = new GraphMLImporter<>(vertexProv, edgeProv);

            importer.setEdgeWeightAttributeName(CoocurrenceGraphConsumer.WEIGHT_ATTRIBUTE_NAME);
            importer.importGraph(graph, temp);

//                    createEngineDescription(LanguageToolLemmatizer.class),
//                    createEngineDescription(MaltParser.class),
//                    createEngineDescription(Conll2006Writer.class,
//                            Conll2006Writer.PARAM_TARGET_LOCATION, "."));
        } catch (UIMAException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ImportException e) {
            e.printStackTrace();

        }

        LOG.info(String.format("The spring graph contains %s nodes, and %s edges. ", graph.vertexSet().size(), graph.edgeSet().size()));

        Map<String, Integer> degreempa = new HashMap<>();
        Set<CoocurrenceVertex> vertices = graph.vertexSet();

        for(CoocurrenceVertex v : vertices) {
            degreempa.put(v.lemmatized, graph.inDegreeOf(v) + graph.outDegreeOf(v));
        }

//        vertices.stream().collect(Collectors.toMap(v -> v.lemmatized, v -> graph.inDegreeOf(v) + graph.outDegreeOf(v)));

        return degreempa;
    }

    @Override
    public Map<String, Double> getWeights() {
        return Collections.emptyMap();
    }
}
