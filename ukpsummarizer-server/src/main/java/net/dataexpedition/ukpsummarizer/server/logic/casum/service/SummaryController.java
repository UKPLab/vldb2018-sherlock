package net.dataexpedition.ukpsummarizer.server.logic.casum.service;

import com.google.common.base.Preconditions;
import net.dataexpedition.ukpsummarizer.server.logic.casum.service.model.Summary;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import springfox.documentation.annotations.Incubating;

@Incubating
@RestController
@RequestMapping(value = "casum/service", method = RequestMethod.GET)
public class SummaryController {

    @Autowired
    CasumAsAService casum;

    @RequestMapping("summary/{subjectId}/{iteration}")
    public ResponseEntity<Summary> getSummaryForSubjectIteration(
            @PathVariable("subjectId") String subjectId,
            @PathVariable("iteration") Integer iteration,
            @RequestParam(value = "topic", defaultValue = "/processed/TEST/d30051t") String topic) {
        Preconditions.checkNotNull(subjectId);
        Preconditions.checkNotNull(iteration);

        Summary s = casum.getSummary(subjectId, iteration, topic);

        return ResponseEntity.ok(s);
    }
}
