package net.dataexpedition.ukpsummarizer.server.logic.assignment;

import net.dataexpedition.ukpsummarizer.server.logic.assignment.model.Assignment;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.AssignmentTemplate;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.rest.core.annotation.RepositoryRestResource;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Optional;
import java.util.Set;

@Repository
public interface AssignmentRepository extends JpaRepository<Assignment, Long> {

    Set<Assignment> findByUser(User user);

//    @Query(value = "SELECT a FROM Assignment a JOIN AssignmentTemplate at ON (a.assignmentTemplate = at.id) WHERE a.user=?1 AND at.topic=?2")
    Set<Assignment> findFirstByUserAndAssignmentTemplateTopic(User u, String topic);


    Optional<Assignment> findFirstByUserAndId(User u, Long id);


}
