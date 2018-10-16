package net.dataexpedition.ukpsummarizer.server.logic.assignment;

import com.google.common.base.Preconditions;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Iteration;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.AssignmentTemplateService;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.AssignmentTemplate;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.AssignmentTemplateRepository;
import net.dataexpedition.ukpsummarizer.server.logic.interaction.InteractionRepository;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.CmdContinueResult;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.FileRepository;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.DatasetDescription;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.TopicDescription;
import net.dataexpedition.ukpsummarizer.server.logic.summary.SummarizationService;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;
import net.dataexpedition.ukpsummarizer.server.logic.user.UserRepository;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Path;
import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by hatieke on 2017-06-15.
 */
@Service
public class AssignmentService {
    private static final Logger LOG = LoggerFactory.getLogger(AssignmentService.class);

    @Autowired
    AssignmentTemplateService assignmentTemplateService;

    @Autowired
    AssignmentRepository assignmentRepository;

    @Autowired
    UserRepository userRepository;

    @Autowired
    InteractionRepository interactionRepository;

    @Autowired
    SummarizationService summarizationService;

    @Autowired
    FileRepository datasetFileRepository;

    public Assignment createNewTaskFor(User user, String topic) throws IOException, InterruptedException {
        Preconditions.checkArgument(userRepository.exists(user.getId()), String.format("User %s doesnt exist", user.getId()));

        Set<Assignment> assignments = assignmentRepository.findByUser(user);

        assignments.stream().map(e -> {
            e.setInactive();
            return e;
        }).forEach(assignmentRepository::save);


        TopicDescription td = getTopicDescription(topic);
        List<AssignmentTemplate> templates = assignmentTemplateService.findAllByTopic(topic).stream().filter(e -> e.topic.equals(topic)).collect(Collectors.toList());
        if(templates.isEmpty()) {
            templates = assignmentTemplateService.createAllTemplatesForTopic(topic);
        }

        Double rand = Math.floor(Math.random() * templates.size());
        AssignmentTemplate assignmentTemplate = templates.get(rand.intValue());


        Assignment t = createAssignmentFromTemplate(assignmentTemplate, user);

        t = assignmentRepository.save(t);

        return t;
    }

    private Assignment createAssignmentFromTemplate(AssignmentTemplate assignmentTemplate, User user) {

        Assignment a = new Assignment();
        a.assignmentTemplate = assignmentTemplate;
        assignmentTemplate.assignments.add(a);
        a.setUser(user);

        Iteration.IterationBuilder builder = Iteration.newBuilder();
        builder.assignment(a);
        builder.summary(new ArrayList<>(assignmentTemplate.summary));
        builder.exploratory(new ArrayList<>(assignmentTemplate.exploratory_summary));
        builder.confirmatory(new ArrayList<>(assignmentTemplate.confirmatory_summary));

        List<Interaction> clonedInteractions = assignmentTemplate.interactions
                .stream()
                .map(e -> interactionRepository.detach(e))
                .collect(Collectors.toList());

        builder.interactions(clonedInteractions);
        builder.weights(new HashMap<>(assignmentTemplate.weights));

        Iteration iteration = builder.create();
        a.getIterations().add(iteration);
        return a;
    }

    private TopicDescription getTopicDescription(String path) {
        Path baseDir = datasetFileRepository.findOne(path).toPath();
        TopicDescription.TopicDescriptionBuilder tdb = TopicDescription.newBuilder();
        DatasetDescription datasetDescription = new DatasetDescription(baseDir);
        tdb.setTopicDetails(datasetDescription);
        return tdb.create();

    }

    public Optional<Assignment> getActiveAssignment(User user) {
        Set<Assignment> userAssignments = assignmentRepository.findByUser(user);
        Optional<Assignment> first = userAssignments.stream().filter(e -> e.getActive()).findFirst();
        return first;
    }

    public Assignment save(Assignment assignment) {
//        Stream<Iteration> iterationStream = assignment.getIterations().stream();
//        Stream<Interaction> interactionStream = iterationStream.flatMap(iteration -> iteration.interactions.stream()).filter(it -> it.id == null);
//        List<Interaction> listed = interactionStream.collect(Collectors.toList());
//        interactionRepository.save(listed);


        try {
            return assignmentRepository.save(assignment);
        } catch (Exception | Error e) {
            LOG.info("EVERYTHING IS DOOF.", e);
        }
        return assignmentRepository.save(assignment);
    }

    public Assignment getAssignment(Long assignmentId) {
        return Preconditions.checkNotNull(assignmentRepository.findOne(assignmentId));
    }

    public Assignment getOrCreateAssignment(User user, String s) throws IOException, InterruptedException {
        Set<Assignment> assignmentByUserAndAndTopic = assignmentRepository.findFirstByUserAndAssignmentTemplateTopic(Preconditions.checkNotNull(user), Preconditions.checkNotNull(s));
        if (!assignmentByUserAndAndTopic.isEmpty()) {
            Optional<Assignment> assignmentOptional = assignmentByUserAndAndTopic.stream().filter(e -> e.getActive()).findFirst();
            if (assignmentOptional.isPresent()) {
                return assignmentOptional.get();
            }

            assignmentOptional = assignmentByUserAndAndTopic.stream().findFirst();
            if (assignmentOptional.isPresent()) {
                Assignment assignment = assignmentOptional.get();
                setActiveAssignment(assignment);
                return assignment;
            }
        }
        return createNewTaskFor(user, s);
    }

    public Optional<Assignment> getAssignment(Long assignmentId, User user) {
        return assignmentRepository.findFirstByUserAndId(user, assignmentId);
    }


    /**
     * given feedback is added to the "old" iteration. then a summary is run with those feedbacks. And the results are
     * stored in the new iteration
     *
     * @param assignment
     * @param body
     * @return
     * @throws IOException
     * @throws InterruptedException
     */
    public Assignment recordFeedback(Assignment assignment, InteractionFeedback body) throws IOException, InterruptedException {


        // the given feedback is attached to the current iteration (i.e. some labels are changed from "RECOMMENDATION" to "ACCEPT" or "REJECT".
        Iteration currentIteration = assignment.getLatestIteration();


        // change the stored interactiontype for feedback that has been given
        for (Interaction receivedFeedbackLabels : body.items) {
            Optional<Interaction> optionalInteraction = currentIteration.interactions.stream().filter(e -> e.equals(receivedFeedbackLabels)).findAny();
            if (optionalInteraction.isPresent()) {
                Interaction interaction = optionalInteraction.get();
                interaction.value = receivedFeedbackLabels.value;
                interaction.uncertainty = 0.0;
            } else {
                /**
                 * the interaction has been received via inline feedback, which may contain interactions other than the ones proposed.
                 *
                 * This means, we have to save this interaction
                 */
                receivedFeedbackLabels.uncertainty = 0.0;
                currentIteration.interactions.add(receivedFeedbackLabels);
//                LOG.warn("Ignoring a empty optional which should never be empty...");
            }
//            // Due to the Set spec, we have to remove prior adding otherwise "add" would not work.
//            currentIteration.interactions.remove(receivedFeedbackLabels);
//            currentIteration.interactions.add(receivedFeedbackLabels);
        }
        assignment = assignmentRepository.save(assignment);

        // Calculate new summary + recommendations using the current assignment.
        CmdContinueResult next = summarizationService.getNext(assignment);

        Iteration nextIteration = new Iteration(assignment);
        nextIteration.iteration = currentIteration.iteration + 1;
        assignment.getIterations().add(nextIteration);
        List<Interaction> recommendations = next.details
                .stream()
                .filter(e -> e.iteration < 0)
//                .filter(e -> !"".equals(e.concept))
                .map(e -> {
                    e.iteration = nextIteration.iteration;
                    return e;
                })
                .collect(Collectors.toList());

        Iteration finalNextIteration = nextIteration;
        assert (next.details
                .stream()
                .filter(e -> e.iteration.equals(finalNextIteration.iteration))
                .count() == 0);


        nextIteration.interactions.addAll(recommendations);
//        nextIteration.interactions = nextIteration.interactions.stream().map(e -> {
//            interactionRepository.detach(e);
//            e.id = null;
//            e = interactionRepository.save(e);
//            return e;
//        }).collect(Collectors.toList());

        nextIteration.summary = next.summary;
        nextIteration.sentence_ids = next.sentence_ids;
        nextIteration.weights = next.fbs_weights;
        nextIteration.confirmatory_summary = next.confirmatory_summary;
        nextIteration.exploratory_summary = next.exploratory_summary;

        return save(assignment);


    }

    public Set<Assignment> getAssignments(User user) {
        return assignmentRepository.findByUser(user);
    }

    public void setActiveAssignment(Assignment activeAssignment) {
        assignmentRepository.findByUser(activeAssignment.getUser()).forEach(e -> {
            e.setInactive();
            assignmentRepository.save(e);
        });
        activeAssignment.setActive(true);
        assignmentRepository.save(activeAssignment);


    }

    public List<Assignment> getAssignments() {
        return assignmentRepository.findAll();
    }
}
