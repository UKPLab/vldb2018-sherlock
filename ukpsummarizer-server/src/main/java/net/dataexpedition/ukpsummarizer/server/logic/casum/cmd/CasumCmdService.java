package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd;

import com.google.common.base.Preconditions;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import javax.annotation.PostConstruct;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;

/**
 * Created by hatieke on 2017-06-18.
 */
@Service
public class CasumCmdService {
    private static final Logger LOG = LoggerFactory.getLogger(CasumCmdService.class);


    @Value("${ukpsummarizer.casum.os}")
    private String os = "unknown_os";

    @Value("${ukpsummarizer.casum.workdir}")
    private File workdir;

    private PythonRunner pythonRunner;

    /**
     * Runs the setup script for python, which initiates a venv and installs required packages
     * <p>
     * TODO use configurable pathes for source, venv and working dir.
     *
     * @throws IOException
     */
    @PostConstruct
    private void postConstruct() throws IOException, InterruptedException {
        if (os.toLowerCase().contains("windows")) {
            pythonRunner = new PythonRunnerWindows(workdir);
        } else if (os.toLowerCase().contains("linux")) {
            pythonRunner = new PythonRunnerLinux(workdir);
        } else if (os.toLowerCase().contains("mac")) {
            pythonRunner = new PythonRunnerLinux(workdir);
        }
        else {
            throw new ConfigurationError(String.format("The OS '%s' is not supported", os));
        }
    }

    int runPythonCascadeSummarizer(String arguments, StringBuffer output) throws IOException, InterruptedException {
        return pythonRunner.run(Preconditions.checkNotNull(arguments), output);
    }


}
