package net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate;

/**
 * Created by hatieke on 2017-07-05.
 */
public class AssignmentTemplateCreationDto {
    public String topic;
    //public ConceptType conceptType = ConceptType.PARSE;
    public ConceptType conceptType = ConceptType.NGRAMS;
    public PropagationType propagationType = PropagationType.BASELINE;

}
