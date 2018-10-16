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

/**
 * Created by hatieke on 2017-06-28.
 */
public class PythonRunnerWindows implements PythonRunner {
    private static final Logger LOG = LoggerFactory.getLogger(PythonRunnerWindows.class);
    private final File workdir;

    public PythonRunnerWindows(File workdir) throws IOException, InterruptedException {
        this.workdir = Preconditions.checkNotNull(workdir);
        File venv_dir = new File(workdir, ".venv");
        if (!Files.exists(venv_dir.toPath())) {
            ProcessBuilder pb = new ProcessBuilder("cmd", "/S","/C","prepare.bat");
            pb.directory(workdir);

            Process process = pb.start();
            BufferedReader input = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line = null;
            try {
                while ((line = input.readLine()) != null) {
                    LOG.info(line);
                }
            } catch (IOException e) {
                LOG.error(e.getMessage(), e);
            }
            process.waitFor();
        }

    }

    @Override
    public int run(String arguments, StringBuffer output) throws IOException, InterruptedException {
        Preconditions.checkNotNull(arguments);
        ArrayList<String> newArgs = new ArrayList<>();
        newArgs.add("cmd");
        newArgs.add("/S");
        newArgs.add("/C");
        newArgs.add("cascade.cmd");
        String[] split = arguments.trim().replaceAll("\\s+", " ").split("\\s");
        newArgs.addAll(Arrays.asList(split));
        ProcessBuilder pb = new ProcessBuilder(newArgs);

        pb.directory(workdir);

//        File directory = pb.directory();
//        LOG.info("Working dir for process: " + directory);
//        pb.environment().put("PYTHONUNBUFFERED","1");
        pb.environment().put("PYTHONPATH", ".");
        pb.redirectErrorStream(true);
        Process process = pb.start();
        LOG.info("Started process in: " + workdir);
        LOG.info("command: " + pb.command());

        BufferedReader input = new BufferedReader(new InputStreamReader(process.getInputStream()));
        String line = null;
        try {
            while ((line = input.readLine()) != null) {
                LOG.info(line);
                output.append("\n" + line);
            }
        } catch (IOException e) {
            LOG.info(e.getMessage(), e);
        }

        return process.waitFor();

    }
}
