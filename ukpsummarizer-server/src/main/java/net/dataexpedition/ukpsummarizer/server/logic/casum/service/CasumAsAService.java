package net.dataexpedition.ukpsummarizer.server.logic.casum.service;

import com.fasterxml.jackson.jaxrs.json.JacksonJsonProvider;
import com.google.common.base.Preconditions;
import net.dataexpedition.ukpsummarizer.server.logic.casum.service.model.CasumSummaryResult;
import net.dataexpedition.ukpsummarizer.server.logic.casum.service.model.Summary;
import net.dataexpedition.ukpsummarizer.server.logic.user.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import javax.ws.rs.client.Client;
import javax.ws.rs.client.ClientBuilder;
import javax.ws.rs.client.WebTarget;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

/**
 * Created by hatieke on 2017-02-05.
 */
@Service
public class CasumAsAService {

    @Autowired
    UserRepository userRepository;

    public Summary getSummary(String subjectId, Integer iteration, String topic) {
        CasumSummaryResult callCasum = callCasum(topic);
        return Summary.newBuilder()
                .setIteration(iteration + 1)
                .setSubject(subjectId)
                .setSummary(callCasum.summary)
                .create();
    }

    private CasumSummaryResult callCasum(String topic) {
        Client client = ClientBuilder.newClient().register(JacksonJsonProvider.class);

        WebTarget target = client.target("http://localhost:5000")
                .path("summary")
                .queryParam("topic", Preconditions.checkNotNull(topic))
                .queryParam("summarizer", "sume")
                .queryParam("oracle", "accept_reject");

//        CasumSummaryPayload payload = new CasumSummaryPayload(100, "english", 0);
        Response response =
                target.request(MediaType.APPLICATION_JSON_TYPE)
                        .get(Response.class);
//        response.bufferEntity();
//        String resp = response.readEntity(String.class);
        CasumSummaryResult s = response.readEntity(CasumSummaryResult.class);
        return s;
    }
}
