package net.dataexpedition.ukpsummarizer.server.logic.casum.service.model;

/**
 * Created by hatieke on 2017-02-05.
 */
public class CasumSummaryPayload {
    public final Integer summaryLength;
    public final String language;

    public CasumSummaryPayload(int i, String english, int i1) {
        this.summaryLength = i;
        this.language = english;
    }
}
