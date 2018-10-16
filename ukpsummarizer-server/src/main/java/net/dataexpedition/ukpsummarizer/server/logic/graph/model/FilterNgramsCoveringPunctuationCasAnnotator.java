package net.dataexpedition.ukpsummarizer.server.logic.graph.model;

import de.tudarmstadt.ukp.dkpro.core.api.lexmorph.type.pos.PUNC;
import de.tudarmstadt.ukp.dkpro.core.api.segmentation.type.NGram;
import de.tudarmstadt.ukp.dkpro.core.api.segmentation.type.StopWord;
import de.tudarmstadt.ukp.dkpro.core.api.segmentation.type.Token;
import org.apache.uima.analysis_engine.AnalysisEngineProcessException;
import org.apache.uima.fit.component.JCasAnnotator_ImplBase;
import org.apache.uima.fit.util.JCasUtil;
import org.apache.uima.jcas.JCas;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Collection;
import java.util.Set;
import java.util.stream.Collectors;

import static org.apache.uima.fit.util.JCasUtil.selectCovered;

/**
 * Created by hatieke on 18.02.2017.
 */
public class FilterNgramsCoveringPunctuationCasAnnotator extends JCasAnnotator_ImplBase {
    private static final Logger LOG = LoggerFactory.getLogger(FilterNgramsCoveringPunctuationCasAnnotator.class);

    @Override
    public void process(JCas aJCas) throws AnalysisEngineProcessException {
        Collection<NGram> select = JCasUtil.select(aJCas, NGram.class);
        LOG.info(String.format("Cas now contains %s ngram annotations", select.size()));

//        List<NGram> ngramsThatShallBeRemoved = new ArrayList<>();
//        for(NGram ngram : select) {
//            List<Token> coveredTokens = JCasUtil.selectCovered(aJCas, Token.class, ngram);
//            List<PUNC> puncList = JCasUtil.selectCovered(aJCas, PUNC.class, ngram);
//            Boolean ngramContainsPunc = !puncList.isEmpty();
//
//            boolean wrongTokenCount = coveredTokens.size() != 2;
//            if(ngramContainsPunc || wrongTokenCount) {
//                ngramsThatShallBeRemoved.add(ngram);
//            }
//        }
        Set<NGram> ngramsThatShallBeRKept = select.stream()
                .filter(ngram -> (selectCovered(aJCas, Token.class, ngram).size() == 2))
                .filter(ngram -> (selectCovered(aJCas, PUNC.class, ngram).isEmpty()))
                .filter(ngram -> selectCovered(StopWord.class, ngram).size() < 2)
                .collect(Collectors.toSet());

//        ngramsThatShallBeRKept.stream();
        select.stream().filter(ngram -> !ngramsThatShallBeRKept.contains(ngram)).forEach(aJCas.getCas()::removeFsFromIndexes);
//        ngramsThatShallBeRemoved.forEach(aJCas.getCas()::removeFsFromIndexes);
        LOG.info(String.format("Cas now contains %s ngram annotations", JCasUtil.select(aJCas, NGram.class).size()));
    }

    @Override
    public void collectionProcessComplete() throws AnalysisEngineProcessException {
        super.collectionProcessComplete();
        LOG.info("Processing complete");
    }
}
