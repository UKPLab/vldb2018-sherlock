package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by hatieke on 2017-06-26.
 */
@JsonIgnoreProperties(ignoreUnknown = true)
public class FullResult {

    public String dataset = "";
    public String topic = "";
    public List<?> models = new ArrayList<>();

}
