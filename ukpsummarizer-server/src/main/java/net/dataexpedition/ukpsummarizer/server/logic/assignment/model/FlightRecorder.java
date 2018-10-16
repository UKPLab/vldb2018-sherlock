package net.dataexpedition.ukpsummarizer.server.logic.assignment.model;

import org.hibernate.annotations.Immutable;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;


@Entity
public class FlightRecorder {

    @Id
    @GeneratedValue
    public long id;



}
