package net.dataexpedition.ukpsummarizer.server.logic.summary;

import net.dataexpedition.ukpsummarizer.server.logic.assignment.AssignmentService;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;
import net.dataexpedition.ukpsummarizer.server.logic.casum.service.model.Summary;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;
import net.dataexpedition.ukpsummarizer.server.logic.user.UserService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;

import java.io.IOException;
import java.util.List;
import java.util.Set;

@Controller
@RequestMapping("summary")
public class SummaryController2 {

    private static final Logger LOG = LoggerFactory.getLogger(SummaryController2.class);

    @Autowired
    SummarizationService summarizationService;

    @Autowired
    AssignmentService assignmentService;

    @Autowired
    AssignmentFileService fileService;

    @Autowired
    UserService userService;

    @SuppressWarnings("unused")
    @RequestMapping("assignment/{assignmentId}")
    public ResponseEntity<Summary> getSummaryByAssignment(@PathVariable Long assignmentId, @RequestHeader String u) throws NotAuthorizedException, IOException, InterruptedException {
        User user = userService.getUser(u).orElseThrow(IllegalArgumentException::new);
        Assignment assignment = assignmentService.getAssignment(assignmentId);
        if (!user.equals(assignment.getUser())) {
            throw new NotAuthorizedException(user.getId(), "assignment " + assignment.getId());
        }

        Summary summary = Summary.newBuilder().setSummary(assignment.getSummary()).create();

        return ResponseEntity.ok(summary);
    }


    @SuppressWarnings("unused")
    @RequestMapping("recommendation/{assignmentId}")
    public ResponseEntity<Set<Interaction>> getRecommendationsByAssignment(
            @PathVariable Long assignmentId,
            @RequestHeader String u) throws NotAuthorizedException, IOException, InterruptedException {

        User user = userService.getUser(u).orElseThrow(IllegalArgumentException::new);
        final Assignment assignment = assignmentService.getAssignment(assignmentId);

        if (!user.equals(assignment.getUser()))
            throw new NotAuthorizedException(user.getId(), "assignment " + assignment.getId());

        return ResponseEntity.ok(assignment.getLatestIteration().interactions);
    }
}
