package net.dataexpedition.ukpsummarizer.server.logic.datasets.model;

import com.google.common.base.Preconditions;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.LongSummaryStatistics;
import java.util.stream.Stream;

/**
 * Created by hatieke on 2017-01-30.
 */
public class TopicDescription {

    public final LongSummaryStatistics documentsStatistics;
    public final TaskDescription task;
    public final LongSummaryStatistics modelStatistics;

    public TopicDescription(TopicDescriptionBuilder tdb) {
        this.documentsStatistics = tdb.documentStatistics;
        this.modelStatistics = tdb.modelStatistics;
        this.task = tdb.task;
    }

    public static TopicDescriptionBuilder newBuilder() {
        return new TopicDescriptionBuilder();
    }

    public static class TopicDescriptionBuilder {

        private LongSummaryStatistics modelStatistics = new LongSummaryStatistics();
        private long numberOfDocuments = 0;
        private long numberOfModels = 0;
        private TaskDescription task = new TaskDescription();
        private LongSummaryStatistics documentStatistics = new LongSummaryStatistics();

        private TopicDescriptionBuilder() {
        }

        public TopicDescription create() {
            return new TopicDescription(this);
        }

        public TopicDescriptionBuilder setTopicDetails(DatasetDescription d) {
            Preconditions.checkNotNull(d);
            Preconditions.checkArgument(d.numberOfDocuments > 0, "Requires more that %s documents", d.numberOfDocuments);
            Preconditions.checkArgument(d.numberOfModels > 0, "Requires more that %s models (reference summaries)", d.numberOfModels);
            Preconditions.checkNotNull(d.task);

            this.task = d.task;

            try (Stream<Path> modelStream = Files.list(d.getModelsPath())) {
                modelStatistics = modelStream.mapToLong(this::countLines).summaryStatistics();
            } catch (IOException e) {
                e.printStackTrace();
            }

            try (Stream<Path> docStream = Files.list(d.getDocumentsPath())) {

                documentStatistics = docStream.mapToLong(this::countLines).summaryStatistics();

            } catch (IOException e) {
                e.printStackTrace();
            }

            return this;

        }

        private long countLines(Path f) {
            try (Stream<String> lineStream = Files.lines(f)) {
                return lineStream.count();

            } catch (IOException e) {
                e.printStackTrace();
                return 0L;
            }
        }

    }
}
