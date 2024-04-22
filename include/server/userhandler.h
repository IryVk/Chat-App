#ifndef USERHANDLER_H
#define USERHANDLER_H

#include <string>
#include <optional>
#include <common/passhash.h>
#include <fstream>
#include <sstream>

/**
 * @brief A class to manage user data
 * 
 * This class is used to manage user data. It provides methods to add a new user, verify a user's credentials, and find a user's salted hash.
 */
class UserHandler {
public:
    UserHandler(const std::string& filename); // Constructor that specifies the file used for storing user data

    bool AddUser(const std::string& username, const std::string& password); // Adds a new user with the given username and password
    bool VerifyUser(const std::string& username, const std::string& password); // Verifies if the provided username and password are correct

private:
    std::string filename;

    // use std::optional to indicate that the user may not exist instead of returning empty string
    std::optional<std::string> FindUserSaltedHash(const std::string& username); // Utility function to find a user's salted hash by username
};

#endif // USERHANDLER_H
