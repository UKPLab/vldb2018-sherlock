package net.dataexpedition.ukpsummarizer.server;

import net.dataexpedition.ukpsummarizer.server.configuration.JacksonConfiguration;
import net.dataexpedition.ukpsummarizer.server.configuration.JpaConfiguration;
import net.dataexpedition.ukpsummarizer.server.configuration.SinglePageAppConfiguration;
import net.dataexpedition.ukpsummarizer.server.configuration.SwaggerConfiguration;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.domain.EntityScan;
import org.springframework.cache.annotation.EnableCaching;
import org.springframework.data.jpa.convert.threeten.Jsr310JpaConverters;
import springfox.documentation.swagger2.annotations.EnableSwagger2;

@SpringBootApplication
@EnableCaching
@EntityScan(basePackageClasses = {
        UkpSummarizerApplication.class,
        Jsr310JpaConverters.class,
        JacksonConfiguration.class,
        JpaConfiguration.class,
        SwaggerConfiguration.class,
        SinglePageAppConfiguration.class})
public class UkpSummarizerApplication {

    public static void main(String[] args) {
        SpringApplication.run(UkpSummarizerApplication.class, args);
    }
}
