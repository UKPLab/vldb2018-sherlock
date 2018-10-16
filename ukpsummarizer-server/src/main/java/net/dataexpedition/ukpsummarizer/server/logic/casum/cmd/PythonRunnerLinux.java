package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd;

import com.google.common.base.Preconditions;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Created by hatieke on 2017-06-28.
 */
public class PythonRunnerLinux implements PythonRunner {
    private static final Logger LOG = LoggerFactory.getLogger(PythonRunnerLinux.class);
    private final File workdir;

    public PythonRunnerLinux(File workdir) throws IOException, InterruptedException {
        this.workdir = workdir;
        File venv_dir = new File(this.workdir, ".venv");
        if (!Files.exists(venv_dir.toPath())) {
            LOG.info(String.format("Setting up a venv for python in %s", this.workdir.toString()));
            ProcessBuilder pb = new ProcessBuilder("/bin/sh", "prepare.sh");
            pb.directory(this.workdir);
            LOG.info(String.format("using workdir %s", pb.directory()));

            pb.redirectErrorStream(true);
            Process process = pb.start();
            BufferedReader mergedProcessOuputReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            LOG.info(String.format("started process"));
            String mergedLine = null;
            try {
                while ((mergedLine = mergedProcessOuputReader.readLine()) != null) {
                    LOG.info(mergedLine);
                }
            } catch (IOException e) {
                LOG.error(e.getMessage(), e);
            }
            int i = process.waitFor();
            LOG.info(String.format("Finished process with exitCode %s", i));
        } else {
            LOG.info(String.format("venv already exists in %s", venv_dir.toPath()));
        }
    }

    @Override
    public int run(String arguments, StringBuffer output) throws IOException, InterruptedException {
        Preconditions.checkNotNull(arguments);

        ArrayList<String> newArgs = new ArrayList<>();
        newArgs.add("/bin/sh");
        newArgs.add("cascade.sh");
        String[] split = arguments.trim().replaceAll("\\s+", " ").split("\\s");
        newArgs.addAll(Arrays.asList(split));


        ProcessBuilder pb = new ProcessBuilder(newArgs);
        pb.directory(this.workdir);
        LOG.info(String.format("Going to run the following command: %s", pb.command()));
        LOG.info(String.format("using workdir %s", pb.directory()));
        pb.redirectErrorStream(true);
        Process process = pb.start();
        LOG.info("command: " + process.toString());

        BufferedReader mergedProcessOuputReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
        LOG.info(String.format("started process"));
        String line = null;
        try {
            while ((line = mergedProcessOuputReader.readLine()) != null) {
                LOG.info(line);
                output.append("\n" + line);
            }
        } catch (IOException e) {
            LOG.info(e.getMessage(), e);
        }
        int i = process.waitFor();
        LOG.info(String.format("Finished process with exitCode %s", i));

        return i;

    }

}
