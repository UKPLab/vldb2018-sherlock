package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model;

import com.fasterxml.jackson.annotation.*;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.fasterxml.jackson.jaxrs.json.annotation.JSONP;
import io.swagger.models.auth.In;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Iteration;
import org.hibernate.internal.util.Cloneable;

import javax.persistence.*;
import java.io.Serializable;
import java.util.Objects;


@Entity
@Embeddable
public class Interaction {


    @Id
    @GeneratedValue
    public Long id;

//    @EmbeddedId
//    @JsonUnwrapped
//    public PK pk;

    @Column
    public Integer iteration = -2;

    @Column
    public InteractionType value = InteractionType.RECOMMENDATION;

    @Column
    public Double weight = 0.0;

    @Column
    @JsonProperty("uncertainity")
    public Double uncertainty;

//
//    public Interaction() {
//    }
//
//    public static class PK implements Serializable {

    @Column
    public String concept = "";

    protected Interaction(InteractionBuilder interactionBuilder) {
        Interaction template = interactionBuilder.interaction;

        this.iteration = template.iteration;
        this.concept = template.concept;
        this.value = template.value;
        this.weight = template.weight;
        this.uncertainty = template.uncertainty;
        this.id = null;
    }

    public Interaction() {

    }

    public static InteractionBuilder newBuilder() {
        return new InteractionBuilder();
    }

    public static class InteractionBuilder {

        private Interaction interaction;

        public InteractionBuilder setInteraction(Interaction interaction) {
            this.interaction = interaction;
            if (interaction.iteration < 0) {
                this.interaction.iteration = 0;
            }
            return this;
        }

        public Interaction create() {
            return new Interaction(this);
        }
    }


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Interaction that = (Interaction) o;
        return Objects.equals(id, that.id) &&
                Objects.equals(concept, that.concept) &&
                Objects.equals(iteration, that.iteration);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id, concept, iteration);
    }
}
