package net.dataexpedition.ukpsummarizer.server.logic.datasets.model;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.base.Preconditions;

import java.awt.image.RescaleOp;
import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Optional;
import java.util.stream.Stream;

/**
 * Created by hatieke on 2017-01-29.
 */
public class DatasetDescription {


    public static final String DOCS_DIRECTORY = "docs";
    public static final String SUMMARIES_DIRECTORY = "summaries";
    public static final String TASK_FILENAME = "task.json";

    public DatasetDescription(Path path) {
        this.path = Preconditions.checkNotNull(path);

        this.name = path.getFileName().toString();

        if (Files.isDirectory(path.toAbsolutePath())) {
            this.type = ObjectType.DIRECTORY;

        } else {
            this.type = ObjectType.FILE;
        }


        long temporaryNumberOfDocuments = 0L;
        long tempNumberOfModels = 0L;
        TaskDescription taskDescription = null;
        if (this.type == ObjectType.DIRECTORY) {
            try {
                try (Stream<Path> pathstream = Files.list(path)) {
                    Optional<Path> docsPath = pathstream.filter(x -> DOCS_DIRECTORY.equalsIgnoreCase(x.getFileName().toString())).findAny();
                    if (docsPath.isPresent()) {
                        if (Files.isDirectory(docsPath.get().toAbsolutePath())) {
                            try(Stream<Path> filestream = Files.list(docsPath.get())) {
                                temporaryNumberOfDocuments  = filestream.count();
                            }

                        }
                    }
                }
                try (Stream<Path> modelStream = Files.list(path)) {
                    Optional<Path> modelsPath = modelStream.filter(x -> SUMMARIES_DIRECTORY.equalsIgnoreCase(x.getFileName().toString())).findAny();
                    if (modelsPath.isPresent()) {
                        if (Files.isDirectory(modelsPath.get().toAbsolutePath())) {
                            try(Stream<Path> filestream = Files.list(modelsPath.get())) {
                                tempNumberOfModels = filestream.count();
                            }
                        }
                    }
                }

                // check if its the taskdescription.
                try (Stream<Path> subdirStream = Files.list(path)) {
                    Optional<Path> subdir = subdirStream.filter(x -> TASK_FILENAME.equalsIgnoreCase(x.getFileName().toString())).findAny();
                    if (subdir.isPresent()) {
                        ObjectMapper om = new ObjectMapper();
                        taskDescription = om.readValue(subdir.get().toFile(), TaskDescription.class);

                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {


        }
        this.task = taskDescription;
        this.numberOfDocuments = temporaryNumberOfDocuments;
        this.numberOfModels = tempNumberOfModels;

    }

    public Path getPath() {
        return path;
    }

    public Path getModelsPath() {
        return path.resolve("summaries");
    }

    public Path getDocumentsPath() {
        return path.resolve(DOCS_DIRECTORY);
    }

    public enum ObjectType {
        DIRECTORY, FILE
    }

    @JsonIgnore
    private final Path path;

    public final String name;
    public final ObjectType type;
    public final long numberOfDocuments;
    public final long numberOfModels;
    public final TaskDescription task;


}
