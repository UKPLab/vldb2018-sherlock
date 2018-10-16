package net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.detachable;

import org.springframework.data.jpa.repository.support.JpaEntityInformation;
import org.springframework.data.jpa.repository.support.SimpleJpaRepository;

import javax.persistence.EntityManager;
import java.io.Serializable;

/**
 * Created by hatieke on 2017-07-06.
 */
public class ExtendedJpaRepository<T, ID extends Serializable> extends SimpleJpaRepository<T, ID> implements BaseRepository<T, ID> {

    private final EntityManager em;

    public ExtendedJpaRepository(JpaEntityInformation<T, ?> entityInformation, EntityManager entityManager) {
        super(entityInformation, entityManager);
        this.em=entityManager;
    }

    public T detach(T clone) {
        em.detach(clone);
        return clone;
    }

}
