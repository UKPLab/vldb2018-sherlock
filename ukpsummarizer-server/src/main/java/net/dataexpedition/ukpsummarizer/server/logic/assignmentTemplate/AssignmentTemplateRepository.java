package net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.rest.core.annotation.RepositoryRestResource;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Set;


@Repository
//@RepositoryRestResource
public interface AssignmentTemplateRepository extends JpaRepository<AssignmentTemplate, Long> {


    List<AssignmentTemplate> findAllByTopic(String topic);

}



