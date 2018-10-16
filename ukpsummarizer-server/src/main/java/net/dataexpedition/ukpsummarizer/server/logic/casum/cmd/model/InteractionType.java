package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.JsonMappingException;

/**
 * Created by hatieke on 2017-06-21.
 */
public enum InteractionType {

    @JsonProperty("recommendation")
    RECOMMENDATION("recommendation"),

    @JsonProperty("accept")
    ACCEPT("accept"),
    @JsonProperty("reject")
    REJECT("reject");

    private String value;

    InteractionType(String recommendation) {
        switch(recommendation) {
            case "recommendation":
            case "accept":
            case "reject":
                this.value=recommendation;
                break;
            default:
                throw new IllegalArgumentException(String.format("%s is not a valid InteractionType value.", recommendation));
        }
    }
//
//    @JsonCreator
//    public static InteractionType getContainerFromValue(String value) throws JsonMappingException {
//        return new InteractionType(value);
//    }
}
