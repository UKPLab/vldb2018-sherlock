package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd;

/**
 * Created by hatieke on 2017-06-28.
 */
public class ConfigurationError extends RuntimeException {
    public ConfigurationError() {
    }

    public ConfigurationError(String message) {
        super(message);
    }

    public ConfigurationError(String message, Throwable cause) {
        super(message, cause);
    }

    public ConfigurationError(Throwable cause) {
        super(cause);
    }

    public ConfigurationError(String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
        super(message, cause, enableSuppression, writableStackTrace);
    }
}
