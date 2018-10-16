package net.dataexpedition.ukpsummarizer.server.logic.summary;

import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.AssignmentTemplate;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.CommandLineExecutionController;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.CmdContinueResult;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.CmdSummarizeResult;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Summarizer;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.io.File;
import java.io.IOException;
import java.util.Optional;

/**
 * Created by hatieke on 2017-06-18.
 */
@Service
public class SummarizationService {

    @Autowired
    CommandLineExecutionController cmd;

    @Autowired
    AssignmentFileService fileService;

    public CmdContinueResult getNext(Assignment assignment) throws IOException, InterruptedException {
        File l = fileService.getLabelsFileFor(assignment);
        File w = fileService.getPickleFileFor(assignment);
        File o = fileService.getNextPickleFileFor(assignment);

        return cmd.next(l, w, o);
    }

    public CmdSummarizeResult initiateNewAssignmentTemplate(AssignmentTemplate t) throws IOException, InterruptedException {
        File w = fileService.getFile(String.format("base-%s-pickleout.pkl", t.id), Optional.empty());

        return cmd.init(t.topic, t.conceptType, Summarizer.PROPAGATION, w, t.propagationType);

    }
}
