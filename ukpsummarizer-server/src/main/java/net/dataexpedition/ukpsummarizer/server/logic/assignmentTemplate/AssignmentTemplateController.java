package net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate;

/**
 * Created by hatieke on 2017-07-05.
 */

import com.fasterxml.jackson.annotation.JsonView;
import io.swagger.annotations.AuthorizationScope;
import net.dataexpedition.ukpsummarizer.server.configuration.jackson.Views;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.AssignmentService;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.CmdSummarizeResult;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.FileRepository;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.DatasetDescription;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.TopicDescription;
import net.dataexpedition.ukpsummarizer.server.logic.interaction.InteractionRepository;
import net.dataexpedition.ukpsummarizer.server.logic.summary.SummarizationService;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;
import net.dataexpedition.ukpsummarizer.server.logic.user.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.expression.spel.ast.Assign;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

import java.io.IOException;
import java.nio.file.Path;
import java.util.*;
import java.util.function.Function;
import java.util.stream.Collectors;

@RestController()
@RequestMapping(value = "assignmentTemplates")
public class AssignmentTemplateController {


    @Autowired
    AssignmentTemplateService assignmentTemplateService;

    @Autowired
    private AssignmentService assignmentService;

    @Autowired
    private UserService userService;

    @RequestMapping(method = RequestMethod.GET, produces = MediaType.APPLICATION_JSON_UTF8_VALUE, value = "{id}")
    public AssignmentTemplate get(@PathVariable Long id) {
        return assignmentTemplateService.findOne(id);
    }

    @RequestMapping(method = RequestMethod.GET, produces = MediaType.APPLICATION_JSON_UTF8_VALUE)
    @JsonView(Views.Minimal.class)
    public List<AssignmentTemplate> getAll() {
        return assignmentTemplateService.findAll();
    }

    @RequestMapping(method = RequestMethod.POST)
    public AssignmentTemplate postTemplate(@RequestBody AssignmentTemplateCreationDto dto) throws IOException, InterruptedException {
        return assignmentTemplateService.createTemplateFromDto(dto);
    }

    @RequestMapping(method = RequestMethod.GET, produces = MediaType.APPLICATION_JSON_UTF8_VALUE, value = "forUser")
    @JsonView(Views.Minimal.class)
    public List<AssignmentTemplate> getRecommendedTemplates(@RequestHeader String u) {
        Optional<User> optUser = userService.getUser(u);
        User user = optUser.orElseThrow(IllegalArgumentException::new);

        Set<Assignment> assignments = assignmentService.getAssignments(user);
        List<String> assignedTopics = assignments.stream().map(x -> x.getTopic()).collect(Collectors.toList());
        Map<String, Optional<AssignmentTemplate>> collect = assignmentTemplateService.findAll()
                .stream()
                .filter(x -> !assignedTopics.contains(x.topic))
                .collect(Collectors.groupingBy(o -> o.topic, Collectors.reducing((o, o2) -> o)));

        List<AssignmentTemplate> collect1 = collect.entrySet().stream().filter(x -> x.getValue().isPresent()).map(x -> x.getValue().get()).sorted((a, b) -> b.assignments.size() - a.assignments.size()).collect(Collectors.toList());
        return collect1;
    }
}
