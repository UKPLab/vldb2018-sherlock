package net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate;

import com.google.common.collect.ImmutableList;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.CmdSummarizeResult;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.FileRepository;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.DatasetDescription;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.TopicDescription;
import net.dataexpedition.ukpsummarizer.server.logic.interaction.InteractionRepository;
import net.dataexpedition.ukpsummarizer.server.logic.summary.SummarizationService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class AssignmentTemplateService {

    @Autowired
    AssignmentTemplateRepository assignmentTemplateRepository;

    @Autowired
    InteractionRepository interactionRepository;

    @Autowired
    private SummarizationService summarizationService;

    @Autowired
    FileRepository datasetFileRepository;

    public List<AssignmentTemplate> findAllByTopic(String topic) {
        return assignmentTemplateRepository.findAllByTopic(topic);
    }

    public List<AssignmentTemplate> createAllTemplatesForTopic(String topic) throws IOException, InterruptedException {
        List<AssignmentTemplate> result=new ArrayList<>();
  //      for (PropagationType p : ImmutableList.of(PropagationType.BASELINE, PropagationType.WEGFG, PropagationType.WERWFG)) {
        for (PropagationType p : ImmutableList.of(PropagationType.BASELINE)) {
            for (ConceptType t : ImmutableList.of(ConceptType.NGRAMS)) {

//            for (ConceptType t : ImmutableList.of(ConceptType.PARSE, ConceptType.NGRAMS)) {
                AssignmentTemplateCreationDto dto = new AssignmentTemplateCreationDto();
                dto.topic=topic;
                dto.propagationType=p;
                dto.conceptType=t;
                result.add(createTemplateFromDto(dto));
           }
        }
        return result;
    }

    public AssignmentTemplate findOne(Long id) {
        return assignmentTemplateRepository.findOne(id);
    }

    public List<AssignmentTemplate> findAll() {
        final List<AssignmentTemplate> all = assignmentTemplateRepository.findAll();
        return all;
    }

    public AssignmentTemplate createTemplateFromDto(AssignmentTemplateCreationDto dto) throws IOException, InterruptedException {
        AssignmentTemplate at = new AssignmentTemplate();
        TopicDescription td = getTopicDescription(dto.topic);
        at = assignmentTemplateRepository.save(at);
        at.topic = dto.topic;
        at.narrative = td.task.narrative;
        at.title = td.task.title;
        at.conceptType = dto.conceptType;
        at.propagationType =dto.propagationType;
        at.count = td.documentsStatistics.getCount();

        CmdSummarizeResult cmdResult = summarizationService.initiateNewAssignmentTemplate(at);

        final AssignmentTemplate finalAt = at;
        at.sentences = cmdResult.sentences.stream().map(e -> {
            e.assignmentTemplate = finalAt;
            return e;
        }).collect(Collectors.toList());
        at.sentences.stream()
                .forEach(e -> {
                    e.sentenceSubsetIndex = finalAt.sentences.indexOf(e);
                });
        at.confirmatory_summary = cmdResult.confirmatory_summary;
        at.exploratory_summary = cmdResult.exploratory_summary;
        at.weights = cmdResult.fbs_weights;



        at.interactions = cmdResult.details.stream().collect(Collectors.toSet());
        at.pickleFile = datasetFileRepository.convertToRelative(cmdResult.pickleout);
//        at.pickleFile = cmdResult.pickleout;
        at.runid = cmdResult.run_id;
        at.summary = cmdResult.summary;

        //interactionRepository.save(at.interactions);
        return assignmentTemplateRepository.save(at);

    }

    private TopicDescription getTopicDescription(String path) {
        Path baseDir = datasetFileRepository.findOne(path).toPath();
        TopicDescription.TopicDescriptionBuilder tdb = TopicDescription.newBuilder();
        DatasetDescription datasetDescription = new DatasetDescription(baseDir);
        tdb.setTopicDetails(datasetDescription);
        return tdb.create();

    }

}
