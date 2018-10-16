package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.AssignmentTemplate;

import javax.persistence.*;
import java.util.ArrayList;
import java.util.List;


@Entity
public class PySentence {

    @Id
    @GeneratedValue
    public Long id;

    @JsonIgnore
    @ManyToOne
    public AssignmentTemplate assignmentTemplate;

    @Column
    @ElementCollection(targetClass = String.class)
    public List<String> concepts = new ArrayList<>();

    @Column
    public Integer doc_id = -1;

    @Column
    public Integer length = 0;

    @Column
    @ElementCollection(targetClass = String.class)
    public List<String> phrases = new ArrayList<>();

    @Column
    public Integer sent_id = -1;

    @Column
    @ElementCollection(targetClass = String.class)
    public List<String> tokens = new ArrayList<>();

    @Column
    @ElementCollection(targetClass = String.class)
    public List<String> untokenized_concepts = new ArrayList<>();

    @Column(length = 16384)
    public String untokenized_form = "";

    @Column
    @ElementCollection(targetClass = String.class)
    public List<String> untokenized_phrases = new ArrayList<>();

    @Column
    public Integer sentenceSubsetIndex=-1;

}
