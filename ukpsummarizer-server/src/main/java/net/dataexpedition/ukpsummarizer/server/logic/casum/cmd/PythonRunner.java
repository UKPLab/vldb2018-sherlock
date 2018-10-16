package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd;

import java.io.File;
import java.io.IOException;

/**
 * Created by hatieke on 2017-06-28.
 */
public interface PythonRunner {

    int run(String s, StringBuffer output) throws IOException, InterruptedException;
}
