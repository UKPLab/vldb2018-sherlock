package net.dataexpedition.ukpsummarizer.server.logic.datasets;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Repository;

import javax.annotation.PostConstruct;
import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Iterator;

@Repository
public class FileRepository
        implements org.springframework.data.repository.Repository<File, String> {
    private static final Logger log = LoggerFactory.getLogger(FileRepository.class);

    @Value("${ukpsummarizer.datadir}")
    private File dataDir;
//    = new File(
//            System.getProperty("user.home") + File.separator + ".ukpsummarizer");

    @SuppressWarnings("SpringJavaAutowiringInspection")
    @Autowired
    ObjectMapper om;

    @PostConstruct
    private void init() {
        dataDir.mkdirs();
        log.info(String.format("Using %s as dataDir", dataDir));
    }

    /**
     * Search for a file based on provided id exists
     *
     * @param id id of the file. will be directly used to construct a File instance.
     * @return the file at the dataDir location
     */
    public File findOne(String id) {
        if (exists(id)) {
            return new File(dataDir, id);
        } else {
            throw new IllegalArgumentException(
                    String.format("The requested file %s/%s doesn't exist", dataDir, id));
        }
    }

    public File storeJson(String id, Object content) throws IOException {
        File f = new File(dataDir, id);
        om.writeValue(f, content);
        return findOne(id);
    }

    public File findOneOrTouch(String id) throws IOException {
        if (exists(id)) {
            return findOne(id);
        } else {
            File f = new File(dataDir, id);
            f.createNewFile();
            return f;
        }
    }

    public File mkDir(String id) {
        if (exists(id)) {
            return findOne(id);
        } else {
            File f = new File(dataDir, id);
            f.mkdirs();
            return f;
        }
    }

    public boolean exists(String id) {
        return (new File(
                dataDir, id.toString())).exists();
    }

    /**
     * Returns all instances of the type.
     *
     * @return all entities
     */
    public Iterable<File> findAll() {
        File[] fl = dataDir.listFiles();
        Iterable<File> it = new Iterable<File>() {
            @Override
            public Iterator<File> iterator() {
                return new Iterator<File>() {
                    private int currentPos = 0;

                    @Override
                    public boolean hasNext() {
                        return currentPos < fl.length;
                    }

                    @Override
                    public File next() {
                        return fl[currentPos++];
                    }
                };
            }
        };
        return it;
    }

    /**
     * Returns the number of entities available.
     *
     * @return the number of entities
     */
    public long count() {
        return dataDir.listFiles().length;
    }

    public String convertToRelative(String pickle) {
        Path iocache = new File(dataDir, "iocache").toPath();
        Path p = Paths.get(pickle);
        Path relativize = iocache.relativize(p);

        return relativize.toString();
    }
}
