package net.dataexpedition.ukpsummarizer.server.logic.assignmentTemplate.detachable;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.repository.CrudRepository;
import org.springframework.data.repository.NoRepositoryBean;

import java.io.Serializable;

@NoRepositoryBean
public interface BaseRepository<T, ID extends Serializable> extends CrudRepository<T,ID> {
    T detach(T clone);

}
