package net.dataexpedition.ukpsummarizer.server.configuration;

/**
 * Created by hatieke on 17.02.2017.
 */
public class RestExceptionResource {


    private final String message;
    private final String errorlog;
    private final String outLog;

    public RestExceptionResource(String message, String outLog, String errorlog) {
        this.message = message;
        this.errorlog = errorlog;
        this.outLog = outLog;
    }

    public String getMessage() {
        return message;
    }

    public String getErrorlog() {
        return errorlog;
    }

    public String getOutLog() {
        return outLog;
    }
}
