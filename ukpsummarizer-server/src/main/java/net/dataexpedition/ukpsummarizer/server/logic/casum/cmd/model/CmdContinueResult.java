package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.jaxrs.json.annotation.JSONP;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Summarizer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class CmdContinueResult {

    @JsonProperty
    public String pickle = "";

    @JsonProperty
    public Summarizer type = Summarizer.NONE;

    @JsonProperty
    public List<String> summary = new ArrayList<>();

    @JsonProperty
    public List<Integer> sentence_ids=new ArrayList<>();

    @JsonProperty
    public String run_id = "";

    @JsonProperty
    public Map<String, Double> weights = new HashMap<>();

    @JsonProperty
    public Map<String, Double> fbs_weights = new HashMap<>();

    @JsonProperty
    public List<Interaction> details = new ArrayList<>();

    @JsonProperty
    public FullResult full = new FullResult();

    @JsonProperty
    public String picklein = "";

    @JsonProperty
    public String pickleout = "";

    @JsonProperty
    public List<Long> confirmatory_summary = new ArrayList<>();

    @JsonProperty
    public List<Long> exploratory_summary = new ArrayList();
}
