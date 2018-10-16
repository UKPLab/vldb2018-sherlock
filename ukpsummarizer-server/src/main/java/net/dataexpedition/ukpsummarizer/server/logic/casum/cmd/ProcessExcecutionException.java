package net.dataexpedition.ukpsummarizer.server.logic.casum.cmd;

import org.springframework.core.NestedRuntimeException;

public class ProcessExcecutionException extends NestedRuntimeException {
    private final String errorLog;
    private final String outLog;

    public ProcessExcecutionException(String msg) {
        this(msg, "", "");
    }

    public ProcessExcecutionException(String msg, Throwable cause) {
        this(msg, cause, "", "");
    }

    public ProcessExcecutionException(String msg, Throwable cause, String errorLog, String outLog) {
        super(msg, cause);
        this.errorLog = errorLog;
        this.outLog = outLog;
    }

    public ProcessExcecutionException(String msg, String errorLog, String outLog) {
        super(msg);
        this.errorLog = errorLog;
        this.outLog = outLog;
    }

    public String getErrorLog() {
        return this.errorLog;
    }

    public String getOutLog() {
        return outLog;
    }
}
