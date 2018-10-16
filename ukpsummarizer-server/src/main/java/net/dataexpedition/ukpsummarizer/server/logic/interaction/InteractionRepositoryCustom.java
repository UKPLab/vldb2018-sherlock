package net.dataexpedition.ukpsummarizer.server.logic.interaction;

import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.Interaction;


public interface InteractionRepositoryCustom {
    Interaction detach(Interaction clone);
}

