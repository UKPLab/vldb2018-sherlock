package net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonView;
import net.dataexpedition.ukpsummarizer.server.configuration.jackson.Views;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.PySentence;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Summarizer;

import javax.persistence.*;
import java.util.*;

@Entity
public class AssignmentTemplate {

    @JsonView(Views.Minimal.class)
    @GeneratedValue
    @Id
    public long id;

    @JsonView(Views.Internal.class)
//    @JsonIgnore
    @Column
    @OneToMany(cascade = CascadeType.ALL)
    public Set<Assignment> assignments = new HashSet<>();

    @JsonView(Views.Public.class)
    @Column
    public String runid = "";

    @JsonView(Views.Public.class)
    @Column
    public String run_id = "";

    @JsonView(Views.Internal.class)
//    @Column
    @OneToMany(cascade = CascadeType.ALL)
    public List<PySentence> sentences = new ArrayList<>();

    @JsonIgnore
    @Column(length = 5000)
    @ElementCollection(targetClass = String.class)
    public List<String> summary = new ArrayList<>();


    @JsonView(Views.Public.class)
    @Column
    @Enumerated
    public Summarizer summarizerType = Summarizer.PROPAGATION;

    @JsonView(Views.Public.class)
    @Column
    public Integer maxIterationCount = 1;

    @JsonView(Views.Public.class)
    @Column
    @Enumerated
    public ConceptType conceptType = ConceptType.NGRAMS;

    @JsonView(Views.Public.class)
    @Column
    @Enumerated
    public PropagationType propagationType = PropagationType.BASELINE;

    @JsonView(Views.Minimal.class)
    @Column
    //public String topic = "datasets/processed/DUC2004TEST/1doc1sum";
    public String topic = "datasets/processed/DUC2006TEST/D0601A";

    @JsonView(Views.Public.class)
    @Column
    public String dataset ="";

    @JsonView(Views.Public.class)
    @Column
    public String narrative = "";

    @JsonView(Views.Public.class)
    @Column
    public String path = "";

    @JsonView(Views.Minimal.class)
    @Column
    public String title = "";

    @ElementCollection
    public Map<String, Double> weights = new HashMap<>();

    @OneToMany(cascade = CascadeType.ALL)
    public Set<Interaction> interactions = new HashSet<>();

    @Column(length = 512)
    public String pickleFile = "";

    @ElementCollection(targetClass = Long.class)
    public List<Long> confirmatory_summary;

    @ElementCollection(targetClass = Long.class)
    public List<Long> exploratory_summary;

    @Column
    public Long count;
}
