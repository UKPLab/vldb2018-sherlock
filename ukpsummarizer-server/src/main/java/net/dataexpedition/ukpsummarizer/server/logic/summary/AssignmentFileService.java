package net.dataexpedition.ukpsummarizer.server.logic.summary;

import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.InteractionType;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.FileRepository;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import javax.annotation.PostConstruct;
import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Created by hatieke on 2017-06-23.
 */
@Service
public class AssignmentFileService {

    private static final Logger LOG = LoggerFactory.getLogger(AssignmentFileService.class);

    @Autowired
    private FileRepository fileRepository;

    private static final String cacheDirName = "iocache";

    @PostConstruct
    private void postConstruct() {
        fileRepository.mkDir(cacheDirName);
    }

    public File getPickleFileFor(Assignment assignment) throws IOException {
        if(assignment.getIterations().size()>2) {
            return getFile(assignment, "pickleout.pkl", Optional.empty());
        } else {
            return getFile(String.format("base-%s-pickleout.pkl", assignment.assignmentTemplate.id), Optional.empty());
        }
    }

    public File getNextPickleFileFor(Assignment assignment) throws IOException {
        return getFile(assignment, "pickleout.pkl", Optional.empty(), true);
    }

    private File getFile(Assignment assignment, String suffix, Optional<Object> o) throws IOException {
        return getFile(assignment, suffix, o, false);
    }

    private File getFile(Assignment assignment, String suffix, Optional<Object> o, Boolean plusone) throws IOException {
        String filenamePrefix = String.format("%s-%s", assignment.getUser().getId(), assignment.getId());
        Integer iteration = plusone ? 1 : 0;
        if (assignment.getIterations().size() > 0) {
            iteration = plusone ? assignment.getLatestIteration().iteration : assignment.getLatestIteration().iteration-1;
        }
        String filename = filenamePrefix + "-" + iteration + "-" + suffix;

        return getFile(filename, o);
    }

    public File getFile(String fn, Optional<Object> o) throws IOException {
        String filename = cacheDirName + File.separator + fn;
        boolean exists = fileRepository.exists(filename);
        if (exists) {
            return fileRepository.findOne(filename);
        } else {
            if (o.isPresent()) {
                return fileRepository.storeJson(filename, o.get());
            } else {
                return fileRepository.findOneOrTouch(filename);
            }
        }
    }


    public File getLabelsFileFor(Assignment assignment) throws IOException {
        // the file should contain labeled interactions only (and only the current iteration)
        List<Interaction> i = assignment.getLatestIteration().interactions.stream()
                .filter(e -> e.value != InteractionType.RECOMMENDATION)
                .filter(e -> e.iteration >= assignment.getLatestIteration().iteration)
                .collect(Collectors.toList());


        return getFile(assignment, "labels.json", Optional.of(i));
    }
}
