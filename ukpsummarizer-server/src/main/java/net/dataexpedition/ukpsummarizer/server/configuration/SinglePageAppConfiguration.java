/**
 * Hendrik Lücke-Tieke
 * dataexpedition.net
 *
 * Copyright (c) 2017 Hendrik Lücke-Tieke. All rights reserved.
 * 
 * Do not use without prior consent by the copyright holder.
 *
 **/

package net.dataexpedition.ukpsummarizer.server.configuration;

import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Import;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpHeaders;
import org.springframework.util.StringUtils;
import org.springframework.web.servlet.config.annotation.DefaultServletHandlerConfigurer;
import org.springframework.web.servlet.config.annotation.ResourceHandlerRegistry;
import org.springframework.web.servlet.config.annotation.ViewControllerRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurerAdapter;
import org.springframework.web.servlet.resource.ResourceResolver;
import org.springframework.web.servlet.resource.ResourceResolverChain;

import javax.servlet.http.HttpServletRequest;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;

@Configuration
@Import({springfox.documentation.spring.data.rest.configuration.SpringDataRestConfiguration.class})
public class SinglePageAppConfiguration extends WebMvcConfigurerAdapter {

  @Override
  public void addResourceHandlers(ResourceHandlerRegistry registry) {
    super.addResourceHandlers(registry);

    if (!registry.hasMappingForPattern("/webjars/**")) {
      registry
          .addResourceHandler("/webjars/**")
          .addResourceLocations("classpath:/META-INF/resources/webjars/");
    }

    registry
        .addResourceHandler("/**")
        .addResourceLocations("classpath:/META-INF/resources/webjars/ukpsummarizer-ui/0.0.1-SNAPSHOT/")
        .addResourceLocations("classpath:/META-INF/resources/")
        .resourceChain(false)
        .addResolver(new PushStateResourceResolver());
  }

  @Override
  public void addViewControllers(ViewControllerRegistry registry) {
    registry.addViewController("/").setViewName("forward:index.html");
  }

  private static class PushStateResourceResolver implements ResourceResolver {
    private final Resource index =
        new ClassPathResource("/META-INF/resources/webjars/ukpsummarizer-ui/0.0.1-SNAPSHOT/index.html");

    private final List<String> handledExtensions =
        Arrays.asList("html");

    @Override
    public Resource resolveResource(HttpServletRequest request,
                                    String requestPath,
                                    List<? extends Resource> locations,
                                    ResourceResolverChain chain) {
      if (!request.getHeader(HttpHeaders.ACCEPT).startsWith("text/html")) {
        return null;
      }
      return resolve(requestPath, locations);
    }

    @Override
    public String resolveUrlPath(String resourcePath,
                                 List<? extends Resource> locations,
                                 ResourceResolverChain chain) {
      Resource resolvedResource = resolve(resourcePath, locations);
      if (resolvedResource == null) {
        return null;
      }
      try {
        return resolvedResource.getURL().toString();
      } catch (IOException e) {
        return resolvedResource.getFilename();
      }
    }

    private Resource resolve(String requestPath, List<? extends Resource> locations) {

      if (isHandled(requestPath)) {
        return locations.stream()
            .map(loc -> createRelative(loc, requestPath))
            .filter(resource -> resource != null && resource.exists())
            .findFirst()
            .orElse(index);
      }
      return index;
    }

    private Resource createRelative(Resource resource, String relativePath) {
      try {
        return resource.createRelative(relativePath);
      } catch (IOException e) {
        return null;
      }
    }

    private boolean isHandled(String path) {
      String extension = StringUtils.getFilenameExtension(path);
      return handledExtensions.stream().anyMatch(ext -> ext.equals(extension));
    }
  }
}
