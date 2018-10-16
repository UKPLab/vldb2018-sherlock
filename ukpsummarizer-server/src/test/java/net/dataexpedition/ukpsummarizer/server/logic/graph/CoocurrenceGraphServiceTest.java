package net.dataexpedition.ukpsummarizer.server.logic.graph;

import net.dataexpedition.ukpsummarizer.server.logic.graph.model.Topic;
import org.junit.Assert;
import org.junit.Test;

import java.io.File;
import java.util.Map;

/**
 * Created by hatieke on 18.02.2017.
 */
public class CoocurrenceGraphServiceTest {

    CoocurrenceGraphService gs = new CoocurrenceGraphService();

    @Test
    public void getWeights() throws Exception {
        File f = new File(getClass().getClassLoader().getResource("datasets/processed/DUC2004TEST/1doc1sum").getFile());
        Topic t = new Topic(f.toString());

        Map<String, Integer> weights = gs.getWeights(t);
        Assert.assertTrue(weights.keySet().size() > 0);
    }
}
