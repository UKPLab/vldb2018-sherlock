package net.dataexpedition.ukpsummarizer.server.logic.user;

import com.google.common.base.Preconditions;
import net.dataexpedition.ukpsummarizer.server.logic.user.User;
import net.dataexpedition.ukpsummarizer.server.logic.user.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Optional;

/**
 * Created by hatieke on 2017-06-16.
 */
@Service
public class UserService {
    @Autowired
    private UserRepository users;


    public Optional<User> getUser(String u) {
        Preconditions.checkNotNull(u);

        User one = users.findOne(u);


        return Optional.ofNullable(one);
    }
}
