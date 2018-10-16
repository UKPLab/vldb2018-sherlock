package net.dataexpedition.ukpsummarizer.server.logic.interaction;

import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.detachable.BaseRepository;
import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface InteractionRepository extends JpaRepository<Interaction, Long>, InteractionRepositoryCustom {}
