package net.dataexpedition.ukpsummarizer.server.logic.analysis;

import net.dataexpedition.ukpsummarizer.server.logic.assignment.AssignmentService;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.CommandLineExecutionController;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.RougeScore;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.cache.annotation.Cacheable;
import org.springframework.web.bind.annotation.*;

import java.io.IOException;
import java.util.*;
import java.util.stream.Collectors;

@RestController
@RequestMapping(value = "analysis", method = RequestMethod.GET)
public class AnalysisController {

    private static final Logger LOG = LoggerFactory.getLogger(AnalysisController.class);

    @Autowired
    AssignmentService assignmentService;

    @Autowired
    CommandLineExecutionController casum;

    @RequestMapping(value = "summaries", method = RequestMethod.GET)
    public List<UserResults> getRougeSummaries() throws IOException, InterruptedException {
        List<Assignment> assignments = assignmentService.getAssignments();
        List<String> userids = Arrays.asList("8253a7e8-5e0f-194e-815e-18b07d45000b", "8253a7e8-5e0f-194e-815e-2844b0170015", "8253a7e8-5e0e-11a0-815e-0f1d3fa20003", "8253a7e8-5e0f-194e-815e-147551a30005", "8253a7e8-5e0f-194e-815e-2d6145f00021", "8253a7e8-5e0f-194e-815e-2d5564b00020", "8253a7e8-5e0f-194e-815e-29cecb1a001e", "8253a7e8-5e0f-194e-815e-29bc1c59001d", "8253a7e8-5e0f-194e-815e-18c255bd000c", "8253a7e8-5e0e-11a0-815e-0f6e473a0004", "8253a7e8-5e0f-1a54-815e-0f97ada40000", "8253a7e8-5e0f-194e-815e-1a224038000f", "8253a7e8-5e0f-194e-815e-28e419840017", "8253a7e8-5e2f-1a9a-815e-2fb071c60000");

        List<Assignment> assignmentStream = assignments.stream()
                .filter(a -> userids.contains(a.getUser().getId()))
                .filter(a -> "datasets/usability/d31043t".equalsIgnoreCase(a.getTopic()))
                .collect(Collectors.toList());

        List<UserResults> results = new ArrayList<>();

        for(Assignment a : assignmentStream) {
            // call rouge:
            List<String> summary = a.getSummary();
            String topic = a.getTopic();
            String joined = String.join(" ", summary).trim();

            RougeScore score = getRouge(topic, joined);
            UserResults ur = new UserResults();
            ur.score=score;
            ur.user=a.getUser().getId();
            ur.text=joined;
            ur.lastIteration = a.getLatestIteration().iteration;
            results.add(ur);

        }

        return results;
    }


    @RequestMapping(value = "rouge", method = RequestMethod.POST)
    public RougeScore getRougeScore(@RequestBody String text, @RequestHeader(defaultValue = "datasets/usability/d31043t") String topic) throws IOException, InterruptedException {
        return getRouge(topic, text);
    }

    private RougeScore getRouge(String topic, String text) throws IOException, InterruptedException {
        return casum.rouge(topic, text);
    }

}
