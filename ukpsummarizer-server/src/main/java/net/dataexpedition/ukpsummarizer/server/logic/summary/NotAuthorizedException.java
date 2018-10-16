package net.dataexpedition.ukpsummarizer.server.logic.summary;


import org.springframework.web.bind.annotation.ExceptionHandler;

public class NotAuthorizedException extends Exception {
    public NotAuthorizedException(String user, String target) {
        super(String.format("%s is not allowed to access %s", user, target));
    }
}
