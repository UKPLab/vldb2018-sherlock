package net.dataexpedition.ukpsummarizer.server.configuration.jackson;

/**
 * Created by hatieke on 2017-06-23.
 */
public class Views {


    public interface Minimal {
    }

    public interface Public extends Minimal {

    }

    public interface Internal extends Public {


    }
}
