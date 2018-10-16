package net.dataexpedition.ukpsummarizer.server.logic.graph.model;

import com.google.common.base.Preconditions;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.TaskDescription;

import java.io.File;
import java.nio.file.Files;
import java.util.List;

/**
 * Created by hatieke on 17.02.2017.
 */
public class Topic {

    public String path;
    public List<Document> docs;
    public List<Document> summaries;
    public TaskDescription task;

    public Topic(String s) {
        Preconditions.checkArgument(Files.exists(new File(s).toPath()), "Topic path has to exist!");
        path = s;

    }
}
