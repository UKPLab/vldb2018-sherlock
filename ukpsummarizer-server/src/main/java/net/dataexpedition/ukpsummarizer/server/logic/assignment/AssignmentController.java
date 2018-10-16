package net.dataexpedition.ukpsummarizer.server.logic.assignment;

import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;
import net.dataexpedition.ukpsummarizer.server.logic.user.UserService;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * Created by hatieke on 2017-06-16.
 */
@RestController
@RequestMapping("assignments")
public class AssignmentController {

    private static final Logger LOG = LoggerFactory.getLogger(AssignmentController.class);

    @Autowired
    private AssignmentService assignmentService;

    @Autowired
    private UserService userService;

    /**
     * create a assignment for a user. If th
     *
     * @param u
     * @param topic
     * @return
     */
    @RequestMapping(value = "create", method = RequestMethod.POST, consumes = MediaType.APPLICATION_JSON_VALUE)
    public ResponseEntity<Assignment> createAssignment(@RequestHeader String u, @RequestHeader Optional<String> topic) throws IOException, InterruptedException {
        Optional<User> optUser = userService.getUser(u);
        User user = optUser.orElseThrow(IllegalArgumentException::new);

        Assignment assignment = assignmentService.getOrCreateAssignment(user, topic.get());
        assignmentService.setActiveAssignment(assignment);
        return ResponseEntity.ok(assignment);
    }

    @RequestMapping(value = "latest", method = RequestMethod.GET)
    public Assignment getLatestAssignment(@RequestHeader String u) {
        Optional<User> optUser = userService.getUser(u);
        User user = optUser.orElseThrow(IllegalArgumentException::new);

        Optional<Assignment> activeAssignment = assignmentService.getActiveAssignment(user);
        return activeAssignment.get();
    }

    @RequestMapping(value = "{assignmentId}", method = RequestMethod.PATCH)
    public Assignment recordFeedback(
            @PathVariable Long assignmentId,
            @RequestHeader String u,
            @RequestBody InteractionFeedback body) throws IOException, InterruptedException {
        Optional<User> optUser = userService.getUser(u);
        User user = optUser.orElseThrow(IllegalArgumentException::new);

        Optional<Assignment> optAssignment = assignmentService.getAssignment(assignmentId, user);
        if (!optAssignment.isPresent()) {
            new FileNotFoundException("Assignment " + assignmentId + " is not available");
        }

        Assignment assignment = optAssignment.get();
        return assignmentService.recordFeedback(assignment, body);
    }

    @RequestMapping(value="all", method = RequestMethod.GET)
    public Set<Assignment> getAllAssignmentsByUser(@RequestHeader String u) {
        Optional<User> optUser = userService.getUser(u);
        User user = optUser.orElseThrow(IllegalArgumentException::new);

        Set<Assignment> assignments = assignmentService.getAssignments(user);

        return assignments;

    }

    @RequestMapping(value="{id}/activate", method = RequestMethod.GET)
    public Assignment activate(@PathVariable Long id, @RequestHeader String u) {
        Optional<User> optUser = userService.getUser(u);
        User user = optUser.orElseThrow(IllegalArgumentException::new);

        Optional<Assignment> optAssignment = assignmentService.getAssignment(id, user);
        if (!optAssignment.isPresent()) {
            new FileNotFoundException("Assignment " + id + " is not available");
        }
        assignmentService.setActiveAssignment(optAssignment.get());

        return assignmentService.getActiveAssignment(user).get();


    }
}
