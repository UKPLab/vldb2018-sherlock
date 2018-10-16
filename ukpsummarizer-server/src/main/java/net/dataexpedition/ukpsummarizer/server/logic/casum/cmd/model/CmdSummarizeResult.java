package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class CmdSummarizeResult {

    @JsonProperty
    public String pickleout = "";

    @JsonProperty
    public Summarizer type = Summarizer.NONE;

    @JsonProperty
    public List<String> summary = new ArrayList<>();

    @JsonProperty
    public String run_id = "";

    @JsonProperty
    public List<Long> confirmatory_summary = new ArrayList<>();

    @JsonProperty
    public List<Long> exploratory_summary = new ArrayList();

    @JsonProperty
    public Map<String, Double> weights = new HashMap<>();

    @JsonProperty
    public Map<String, Double> fbs_weights = new HashMap<>();

    @JsonProperty
    public List<Interaction> details = new ArrayList<>();

    @JsonProperty
    public FullResult full = new FullResult();

    @JsonProperty
    public List<PySentence> sentences = new ArrayList<>();

}
