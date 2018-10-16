package net.dataexpedition.ukpsummarizer.server.configuration;

import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.detachable.BaseRepository;
import net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.detachable.ExtendedJpaRepository;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.data.jpa.repository.config.EnableJpaRepositories;
import org.springframework.data.rest.core.config.RepositoryRestConfiguration;
import org.springframework.data.rest.webmvc.config.RepositoryRestConfigurer;
import org.springframework.data.rest.webmvc.config.RepositoryRestConfigurerAdapter;

@Configuration
@EnableJpaRepositories(value = "net.dataexpedition.ukpsummarizer.server.logic", repositoryBaseClass = ExtendedJpaRepository.class)
public class JpaConfiguration {


    @Bean
    public RepositoryRestConfigurer repositoryRestConfigurer() {

        return new RepositoryRestConfigurerAdapter() {

            @Override
            public void configureRepositoryRestConfiguration(RepositoryRestConfiguration config) {
                config.exposeIdsFor(User.class);
//                config.setDefaultMediaType(MediaType.APPLICATION_JSON);
            }
        };
    }
}
