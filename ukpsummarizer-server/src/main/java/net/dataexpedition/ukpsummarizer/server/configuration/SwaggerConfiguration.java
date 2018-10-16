package net.dataexpedition.ukpsummarizer.server.configuration;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import springfox.documentation.builders.ApiInfoBuilder;
import springfox.documentation.service.ApiInfo;
import springfox.documentation.service.Contact;
import springfox.documentation.spi.DocumentationType;
import springfox.documentation.spring.web.plugins.Docket;
import springfox.documentation.swagger2.annotations.EnableSwagger2;

/**
 * Created by hatieke on 2017-02-02.
 */
@Configuration
@EnableSwagger2
public class SwaggerConfiguration {

    @Bean //Don't forget the @Bean annotation
    public Docket customImplementation(){
        return new Docket(DocumentationType.SWAGGER_2)
                .apiInfo(apiInfo())
                .forCodeGeneration(true);
//                .select()
//                .paths(x -> true)



    }

    private ApiInfo apiInfo() {
        return new ApiInfoBuilder()
                .contact(new Contact("Avinesh P.V.S.", "https://www.informatik.tu-darmstadt.de/ukp/", "avinesh@ukp.informatik.tu-darmstadt.de"))
                .description("RESTful endpoints for Sherlock")
                .title("cascade-api")
                .license("license-tbd")
                .build();
    };

}
