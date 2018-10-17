package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd;

import com.fasterxml.jackson.databind.ObjectMapper;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.ConceptType;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.PropagationType;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.CmdContinueResult;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.CmdSummarizeResult;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.RougeScore;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Summarizer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.cache.annotation.Cacheable;
import org.springframework.stereotype.Controller;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;

@Controller
public class CommandLineExecutionController {
    private static final Logger LOG = LoggerFactory.getLogger(CommandLineExecutionController.class);

    @Autowired
    ObjectMapper om;

    @Autowired
    CasumCmdService casumService;

    @Value("${ukpsummarizer.datadir}")
    File dataDir;

    public CmdContinueResult next(File interactions, File picklein, File pickleout) throws IOException, InterruptedException {
        StringBuffer output = new StringBuffer();
        Path outputTempFile = Files.createTempFile("output", ".json");
        String arguments = " -out \"" + outputTempFile.toFile().getCanonicalPath() + "\" " +
                "--iobasedir \"" + dataDir.getCanonicalPath() + "\" " +
                " continue " +
                " --picklein \"" + picklein.getCanonicalPath() + "\" " +
                " --pickleout \"" + pickleout.getCanonicalPath() + "\" " +
                " --oracle_labels \"" + interactions.getCanonicalPath() + "\"";

        int exitValue = casumService.runPythonCascadeSummarizer(arguments, output);
//        LOG.info(output.toString());
        if (exitValue == 0) {
            CmdContinueResult cmdResult = om.readValue(outputTempFile.toFile(), CmdContinueResult.class);
            return cmdResult;
        } else {
            throw new ProcessExcecutionException("Processes exited with exitValue <> 0", "", output.toString());
        }

    }

    private CmdSummarizeResult getFirstSummary(String topic) throws IOException, InterruptedException {
        Path outputTempFile = Files.createTempFile("output", ".json");
        Path inputTempFile = Files.createTempFile("input", ".json");

        StringBuffer output = new StringBuffer();
        String arguments = " -out \"" + outputTempFile.toFile().getCanonicalPath() + "\" " +
                " --iobasedir \"" + dataDir.getCanonicalPath() + "\" " +
                " summarize " +
                " " + topic + " " +
                " -s " + " PROPAGATION " +
                " --max_models 1 " +
                " --oracle ilp_feedback" +
                " --max_iteration_count 1" +
                "";


        int exitValue = casumService.runPythonCascadeSummarizer(arguments, output);
        LOG.info(output.toString());
        if (exitValue == 0) {
            CmdSummarizeResult cmdResult = om.readValue(outputTempFile.toFile(), CmdSummarizeResult.class);
            return cmdResult;
        } else {
            throw new ProcessExcecutionException("Processes exited with exitValue <> 0", "", output.toString());
        }
    }

    /**
     * Send command to python to create a new base summary for a specific user
     *
     * @param topic
     * @param conceptType
     * @param summarizerType
     * @param w
     * @param propagationType
     */
    public CmdSummarizeResult init(String topic, ConceptType conceptType, Summarizer summarizerType, File w, PropagationType propagationType) throws IOException, InterruptedException {
        StringBuffer output = new StringBuffer();
        Path outputTempFile = Files.createTempFile("output", ".json");
        String arguments = " -out \"" + outputTempFile.toString() + "\" " +
                " --iobasedir \"" + dataDir.getCanonicalPath() + "\" " +
                " summarize " + topic +
                " -s " + summarizerType.toString() +
                " --max_iteration_count 1" +
                " --pickleout \"" + w.getCanonicalPath() + "\" " +
                " --oracle ilp_feedback";

        if (conceptType == ConceptType.PARSE) {
            arguments = arguments + " --concept_type " + conceptType.toString().toLowerCase();
        }


        Double ma = null;
        Double mr = null;
        Integer ia = null;
        Integer ir = null;
        Double co = null;
        Double pat = null;
        switch (propagationType) {
            case WERWFG:
                ma = 1.0d;
                mr = -1.0d;
                ia = 1024;
                ir = 200;
                co = 0.6d;
                pat = 0.25d;
                arguments = arguments + " -rw  " + ma + " " + mr + " " + ia + " " + ir + " " + co + " " + pat + " ";
                break;
            case WEGFG:
                ma = 4.0d;
                mr = 0.0d;
                ia = 128;
                ir = 16;
                co = 0.6d;
                arguments = arguments + " -gb  " + ma + " " + mr + " " + ia + " " + ir + " " + co + " ";
                break;
            case BASELINE:
            default:
//                arguments = arguments + " -bl ";
                break;

        }
//        fim.add_argument("-bl", action="store_true", help="baseline, used if not configured")
//        fim.add_argument("-rw", nargs=6, type=str, help="random walk", metavar=("ma", "mr", "ia", "ir", "co", "pat"))
//        fim.add_argument("-gb", nargs=5, type=str, help="gaussian blur", metavar=("ma", "mr", "ia", "ir", "co"))
//        fim.add_argument("-cg", nargs=3, type=str, help="cooccurence graph", metavar=("ws", "fa", "fr"))


        int exitValue = casumService.runPythonCascadeSummarizer(arguments, output);
        if (exitValue == 0) {
            CmdSummarizeResult cmdResult = om.readValue(outputTempFile.toFile(), CmdSummarizeResult.class);
            return cmdResult;
        } else {
            throw new ProcessExcecutionException("Processes exited with exitValue <> 0", "", output.toString());
        }
    }
//    @Cacheable("rougeScores")
    public RougeScore rouge(String topic, String text) throws IOException, InterruptedException {
        Path inputTempFile = Files.createTempFile("rouge-input", ".json");
        om.writeValue(inputTempFile.toFile(), text);

        StringBuffer output = new StringBuffer();
        Path outputTempFile = Files.createTempFile("output", ".json");
        String arguments = " -out \"" + outputTempFile.toString() + "\" " +
                " --iobasedir \"" + dataDir.getCanonicalPath() + "\" " +
                " rouge " +
                topic +
                " " +
                inputTempFile.toAbsolutePath();

        int exitValue = casumService.runPythonCascadeSummarizer(arguments, output);

        if (exitValue == 0) {
            RougeScore rougeScore = om.readValue(outputTempFile.toFile(), RougeScore.class);
            return rougeScore;
        } else {
            throw new ProcessExcecutionException("Processes exited with exitValue <> 0", "", output.toString());
        }
    }
}

