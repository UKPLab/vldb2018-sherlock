package net.dataexpedition.ukpsummarizer.server.logic.datasets;

import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.DatasetDescription;
import net.dataexpedition.ukpsummarizer.server.logic.datasets.model.TopicDescription;
import org.jboss.logging.Logger;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;
import springfox.documentation.annotations.Incubating;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Created by hatieke on 2017-01-29.
 */
@RequestMapping(value = "dataset", produces = MediaType.APPLICATION_JSON_VALUE)
@RestController
@Incubating
public class DatasetController {

    @Autowired
    FileRepository repo;
    private Logger log = Logger.getLogger(DatasetController.class);

    /**
     * Return a directory list if a valid path has been specified.
     *
     * @param path
     * @return
     * @throws IOException
     */
    @RequestMapping(method = RequestMethod.GET)
    public List<DatasetDescription> getDatasets(@RequestParam(value = "path", defaultValue = "/") String path) throws IOException {
        Path baseDir = repo.findOne(path).toPath();

        try (Stream<Path> dirstream = Files.list(baseDir)) {
            List<DatasetDescription> list = dirstream
                    .map(x -> new DatasetDescription(x))
                    .collect(Collectors.toList());
            return list;
        }
    }

    /**
     * Provides summative details about a topic
     *
     * @param path
     * @return
     */
    @RequestMapping(path = "meta", method = RequestMethod.GET)
    public TopicDescription getTopicDescription(@RequestParam("path") String path) {
        Path baseDir = repo.findOne(path).toPath();
        TopicDescription.TopicDescriptionBuilder tdb = TopicDescription.newBuilder();
        DatasetDescription datasetDescription = new DatasetDescription(baseDir);
        tdb.setTopicDetails(datasetDescription);
        return tdb.create();

    }

}
