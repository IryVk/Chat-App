#ifndef USERHANDLER_H
#define USERHANDLER_H

#include <string>
#include <optional>
#include <common/passhash.h>
#include <fstream>
#include <sstream>

class UserHandler {
public:
    // constructor that specifies the file used for storing user data
    UserHandler(const std::string& filename);

    // adds a new user with the given username and password
    bool AddUser(const std::string& username, const std::string& password);

    // verifies if the provided username and password are correct
    bool VerifyUser(const std::string& username, const std::string& password);

private:
    std::string filename;

    // utility function to find a user's salted hash by username
    // use std::optional to indicate that the user may not exist instead of returning empty string
    std::optional<std::string> FindUserSaltedHash(const std::string& username);
};

#endif // USERHANDLER_H
