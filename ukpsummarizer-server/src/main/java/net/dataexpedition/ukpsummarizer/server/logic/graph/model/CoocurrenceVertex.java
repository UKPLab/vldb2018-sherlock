package net.dataexpedition.ukpsummarizer.server.logic.graph.model;

import de.tudarmstadt.ukp.dkpro.core.api.segmentation.type.Lemma;
import de.tudarmstadt.ukp.dkpro.core.api.segmentation.type.NGram;
import org.apache.uima.fit.util.JCasUtil;

import java.util.List;
import java.util.Objects;

public class CoocurrenceVertex {

    public String id;

    public String lemmatized;

    public Double occurences = 1.0;

    public CoocurrenceVertex() {
    }

    public CoocurrenceVertex(NGram ngram) {
        List<Lemma> lemmata = JCasUtil.selectCovered(Lemma.class, ngram);
        this.lemmatized = lemmata.stream().map(a -> a.getValue()).reduce("", (x, y) -> x.concat(" " + y)).trim();

        this.id = ngram.getText();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        CoocurrenceVertex that = (CoocurrenceVertex) o;
        return Objects.equals(id, that.id) &&
                Objects.equals(occurences, that.occurences);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }
}
