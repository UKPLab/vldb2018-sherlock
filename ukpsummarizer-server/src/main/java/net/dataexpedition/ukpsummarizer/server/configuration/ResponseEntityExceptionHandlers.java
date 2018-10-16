package net.dataexpedition.ukpsummarizer.server.configuration;

import net.dataexpedition.ukpsummarizer.server.logic.casum.cmd.ProcessExcecutionException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.context.request.WebRequest;
import org.springframework.web.servlet.mvc.method.annotation.ResponseEntityExceptionHandler;

@ControllerAdvice
public class ResponseEntityExceptionHandlers extends ResponseEntityExceptionHandler {
    private static final Logger LOG = LoggerFactory.getLogger(ResponseEntityExceptionHandlers.class);

    @ExceptionHandler(value = {ProcessExcecutionException.class})
    protected ResponseEntity<RestExceptionResource> handleProcessExcecutionException(ProcessExcecutionException ex) {
        LOG.error(ex.getMessage(), ex);


        RestExceptionResource body = new RestExceptionResource(ex.getMessage(), ex.getOutLog(), ex.getErrorLog());
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(body);
    }
}
