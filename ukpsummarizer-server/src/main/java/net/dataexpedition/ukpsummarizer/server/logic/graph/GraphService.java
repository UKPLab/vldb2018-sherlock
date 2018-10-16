package net.dataexpedition.ukpsummarizer.server.logic.graph;

import net.dataexpedition.ukpsummarizer.server.logic.graph.model.Topic;
import org.springframework.stereotype.Service;

import java.util.Map;

@Service
public interface GraphService {


    public Map<String, Integer> getWeights(Topic t);

    Map<String,Double> getWeights();

//    public List<Concept> recommendConcepts();
//
//    public void interpretFeedback(Feedback f);
}
