package net.dataexpedition.ukpsummarizer.server.logic.analysis;

import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.model.RougeScore;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;

public class UserResults {

    public RougeScore score;
    public String user;
    public String text;
    public Integer lastIteration;
}
