package net.dataexpedition.ukpsummarizer.server.logic.user;

import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import org.hibernate.annotations.GenericGenerator;
import org.hibernate.annotations.Parameter;
import org.hibernate.id.UUIDGenerator;
import org.springframework.data.jpa.convert.threeten.Jsr310JpaConverters;

import javax.persistence.*;
import java.time.Instant;
import java.util.*;


@Entity
public class User {

    @Id
    @GeneratedValue(generator = "uuid")
    @GenericGenerator(
            name = "uuid",
            strategy = "org.hibernate.id.UUIDGenerator",
            parameters = {
                    @Parameter(name = UUIDGenerator.UUID_GEN_STRATEGY_CLASS,
                            value = "org.hibernate.id.uuid.CustomVersionOneStrategy"
                    )
            }
    )
    private String id;

    public String getId() {
        return id;
    }

    //    @CreationTimestamp
    @Convert(converter = Jsr310JpaConverters.InstantConverter.class)
    private Instant created = Instant.now();

    public Instant getCreated() {
        return created;
    }

    @Column
    @OneToMany
    public List<Assignment> assignments = new ArrayList<>();
}
