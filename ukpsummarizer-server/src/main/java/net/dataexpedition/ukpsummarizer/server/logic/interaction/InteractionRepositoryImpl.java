package net.dataexpedition.ukpsummarizer.server.logic.interaction;

//import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.InteractionRepositoryCustom;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;

import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

public class InteractionRepositoryImpl implements InteractionRepositoryCustom {

    @PersistenceContext
    private EntityManager em;
//    EntityManager em;

    /**
     * Configure the entity manager to be used.
     *
     * @param em the {@link EntityManager} to set.
     */
    public void setEntityManager(EntityManager em) {
        this.em = em;
    }

    public Interaction detach(Interaction clone) {
        em.detach(clone);
        clone.id=null;
//        em.persist(clone);
        return clone;
    }
}
