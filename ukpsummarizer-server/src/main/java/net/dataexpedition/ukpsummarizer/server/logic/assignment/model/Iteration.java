package net.dataexpedition.ukpsummarizer.server.logic.assignment.model;

import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;

import javax.persistence.*;
import javax.persistence.Entity;
import java.util.*;
import java.util.stream.Collectors;

import static com.google.common.base.Preconditions.checkNotNull;

/**
 * Created by hatieke on 2017-06-25.
 */
@Entity
public class Iteration {


    @Id
    @GeneratedValue
    public Long id;

    @Column
    public Integer iteration;


    //    @Column
    @ElementCollection
    public Map<String, Double> weights = new HashMap<>();

    @ElementCollection(targetClass = Long.class)
    public List<Long> exploratory_summary;


    @ElementCollection(targetClass = Long.class)
    public List<Long> confirmatory_summary;

    @Column(length = 5000)
    @ElementCollection(targetClass = String.class)
    public List<String> summary = new ArrayList<>();

    @OneToMany(cascade = CascadeType.ALL)
    public Set<Interaction> interactions = new HashSet<>();


    @ElementCollection(targetClass = Integer.class)
    public List<Integer> sentence_ids = new ArrayList<>();


    public Iteration() {
        // needed for JPA
    }

    public Iteration(Assignment assignment) {
        this.iteration = assignment.getIterations().size();
    }

    Iteration(IterationBuilder iterationBuilder) {
        this(checkNotNull(iterationBuilder.assignment));
        this.weights = iterationBuilder.weights;
        this.summary = iterationBuilder.summary;
        this.interactions = iterationBuilder.interactions;
        this.exploratory_summary = iterationBuilder.exploratory_summary;
        this.confirmatory_summary = iterationBuilder.confirmatory_summary;
    }

    public static IterationBuilder newBuilder() {
        return new IterationBuilder();
    }

    public static class IterationBuilder {
        private Assignment assignment;
        private List<String> summary = new ArrayList<>();
        private Set<Interaction> interactions = new HashSet<>();
        private Map<String, Double> weights = new HashMap<>();
        private List<Long> confirmatory_summary = new ArrayList<>();
        private List<Long> exploratory_summary = new ArrayList<>();

        public IterationBuilder() {
        }

        public void assignment(Assignment assignment) {
            this.assignment = checkNotNull(assignment);
        }

        public IterationBuilder confirmatory(List<Long> confirmatory_summary) {
            this.confirmatory_summary = checkNotNull(confirmatory_summary);
            return this;
        }

        public IterationBuilder exploratory(List<Long> exploratory_summary) {
            this.exploratory_summary = checkNotNull(exploratory_summary);
            return this;
        }

        public void summary(List<String> summary) {
            this.summary = checkNotNull(summary);
        }

        public void interactions(Collection<Interaction> interactions) {
            Set<Interaction> converted = interactions.stream().map(e -> Interaction.newBuilder().setInteraction(e).create()).collect(Collectors.toSet());
            this.interactions = converted;
        }

        public void weights(Map<String, Double> weights) {
            this.weights = weights;
        }

        public Iteration create() {
            checkNotNull(assignment);
            return new Iteration(this);
        }
    }
}
