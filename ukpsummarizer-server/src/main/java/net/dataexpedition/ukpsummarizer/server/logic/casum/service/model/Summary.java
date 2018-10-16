package net.dataexpedition.ukpsummarizer.server.logic.casum.service.model;

import com.google.common.base.Preconditions;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by hatieke on 2017-02-05.
 */
public class Summary {
    private final Integer iteration;
    private final String subject;
    private final List<String> summary;

    private Summary(SummaryBuilder summaryBuilder) {
        this.summary = summaryBuilder.getSummary();
        this.subject = summaryBuilder.getSubject();
        this.iteration = summaryBuilder.getIteration();
    }

    public static SummaryBuilder newBuilder() {
        return new SummaryBuilder();
    }

    public List<String> getSummary() {
        return summary;
    }

    public String getSubject() {
        return subject;
    }

    public Integer getIteration() {
        return iteration;
    }

    public static class SummaryBuilder {
        private Integer iteration = 0;
        private String subject = "Subject";
        private List<String> summary = new ArrayList<>();

        private SummaryBuilder() {
        }

        public SummaryBuilder setIteration(Integer iteration) {
            this.iteration = Preconditions.checkNotNull(iteration);
            return this;
        }

        public SummaryBuilder setSubject(String subject) {
            this.subject = Preconditions.checkNotNull(subject);
            return this;
        }

        public SummaryBuilder setSummary(String summary) {
            this.summary = new ArrayList<>();
            this.summary.add(Preconditions.checkNotNull(summary));
            return this;
        }

        public Summary create() {
            return new Summary(this);
        }

        public Integer getIteration() {
            return iteration;
        }

        public String getSubject() {
            return subject;
        }

        public List<String> getSummary() {
            return summary;
        }

        public SummaryBuilder setSummary(List<String> summary) {
            this.summary = Preconditions.checkNotNull(summary);
            return this;
        }
    }
}
