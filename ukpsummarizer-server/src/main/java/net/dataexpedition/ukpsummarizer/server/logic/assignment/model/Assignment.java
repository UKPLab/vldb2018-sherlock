package net.dataexpedition.ukpsummarizer.server.logic.assignment.model;

import com.fasterxml.jackson.annotation.*;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.AssignmentTemplate;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.ConceptType;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Summarizer;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;

import javax.persistence.*;
import javax.validation.constraints.NotNull;
import java.util.*;

/**
 * A Assignment is the result of single iteration in the process of interactively personalizing a static set
 * of documents. The configuration that is used to calculate the summary is fixed in the first instance (which has no
 * predecessor).
 * <p>
 * Input is a topic plus the summarization configuration, and a list of weights, which is used to override the inherited
 * weights.
 * <p>
 * To get the resulting summary, one has to run the summarization that is defined in the root together and use the
 * weights to override certain aspects.
 */
@Entity
public class Assignment {

    @GeneratedValue
    @Id
    private long id;

    @ManyToOne(targetEntity = User.class)
    @JsonIgnore
    @NotNull
    private User user;

    @Column
    private Boolean isActive = false;

    @Column
    @OneToMany(cascade = CascadeType.ALL)
    private List<Iteration> iterations = new ArrayList<>();

    @ManyToOne(targetEntity = AssignmentTemplate.class)
    @JsonIdentityInfo(generator=ObjectIdGenerators.PropertyGenerator.class, property="id")
    @JsonIdentityReference(alwaysAsId=true)
    public AssignmentTemplate assignmentTemplate;

    public Assignment setInactive() {
        this.setActive(false);
        return this;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Assignment that = (Assignment) o;
        return getId() == that.getId() &&
                Objects.equals(getUser(), that.getUser());
    }

    @Override
    public int hashCode() {
        return Objects.hash(getId());
    }

    @JsonIgnore
    public List<String> getSummary() {
        return getLatestIteration().summary;
    }

    @JsonIgnore
    public Iteration getLatestIteration() {
        return getIterations().get(getIterations().size() - 1);
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public User getUser() {
        return user;
    }

    public void setUser(User user) {
        this.user = user;
    }

    public Boolean getActive() {
        return isActive;
    }

    public void setActive(Boolean active) {
        isActive = active;
    }

    public List<Iteration> getIterations() {
        return iterations;
    }

    public void setIterations(List<Iteration> iterations) {
        this.iterations = iterations;
    }

    public String getRun_id() {
        return assignmentTemplate.run_id;
    }

    public void setRun_id(String run_id) {
        assignmentTemplate.run_id = run_id;
    }

    public Summarizer getSummarizerType() {
        return assignmentTemplate.summarizerType;
    }

    public void setSummarizerType(Summarizer summarizerType) {
        assignmentTemplate.summarizerType = summarizerType;
    }

    public Integer getMaxIterationCount() {
        return assignmentTemplate.maxIterationCount;
    }

    public void setMaxIterationCount(Integer maxIterationCount) {
        assignmentTemplate.maxIterationCount = maxIterationCount;
    }

    public ConceptType getConceptType() {
        return assignmentTemplate.conceptType;
    }

    public void setConceptType(ConceptType conceptType) {
        assignmentTemplate.conceptType = conceptType;
    }

    public String getTopic() {
        return assignmentTemplate.topic;
    }

    public void setTopic(String topic) {
        assignmentTemplate.topic = topic;
    }

    public String getDataset() {
        return assignmentTemplate.dataset;
    }

    public void setDataset(String dataset) {
        assignmentTemplate.dataset = dataset;
    }

    public String getNarrative() {
        return assignmentTemplate.narrative;
    }

    public void setNarrative(String narrative) {
        assignmentTemplate.narrative = narrative;
    }

    public String getPath() {
        return assignmentTemplate.path;
    }

    public void setPath(String path) {
        assignmentTemplate.path = path;
    }

    public String getTitle() {
        return assignmentTemplate.title;
    }

    public void setTitle(String title) {
        assignmentTemplate.title = title;
    }

    public AssignmentTemplate getAssignmentTemplate() {
        return assignmentTemplate;
    }
}
