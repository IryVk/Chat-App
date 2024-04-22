/**
 * @file server/userhandler.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the implementation of the UserHandler class
 * 
 * This file contains the implementation of the UserHandler class, which is used to manage user information. It provides methods to add new users, verify user credentials, and find user salted hashes. 
*/

#include <server/userhandler.h>
#include <iostream>

/**
 * @brief Construct a new UserHandler object
 * 
 * Initialize a new UserHandler object with the specified filename.
 * 
 * @param filename The filename to use for storing user information
 * 
 * @return UserHandler object
*/
UserHandler::UserHandler(const std::string& filename) : filename(filename) {}

/**
 * @brief Add a new user to the user database
 * 
 * Add a new user with the specified username and password to the user database.
 * 
 * @param username The username of the new user
 * @param password The password of the new user
 * 
 * @return bool True if the user was added successfully, false otherwise
*/
bool UserHandler::AddUser(const std::string& username, const std::string& password) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        return false;
    }
    // Check if the username already exists
    if (FindUserSaltedHash(username).has_value()) {
        return false;
    }
    std::string saltedHash = PasswordHasher::HashPassword(password, PasswordHasher::GenerateRandomSalt(16));
    file << username << " " << saltedHash << std::endl;
    return true;
}

/**
 * @brief Verify a user's credentials
 * 
 * Verify the specified username and password against the user database.
 * 
 * @param username The username to verify
 * @param password The password to verify
 * 
 * @return bool True if the user's credentials are valid, false otherwise
*/
bool UserHandler::VerifyUser(const std::string& username, const std::string& password) {
    auto saltedHashOpt = FindUserSaltedHash(username);
    if (!saltedHashOpt.has_value()) {
        return false;
    }
    return PasswordHasher::VerifyPassword(password, saltedHashOpt.value());
}

/**
 * @brief Find a user's salted hash
 * 
 * Find the salted hash for the specified username in the user database.
 * 
 * @param username The username to find
 * 
 * @return std::optional<std::string> The salted hash of the user, or std::nullopt if the user was not found
*/
std::optional<std::string> UserHandler::FindUserSaltedHash(const std::string& username) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return std::nullopt;
    }
    std::string line;
    while (getline(file, line)) {
        std::istringstream iss(line);
        std::string storedUsername, storedSaltedHash;
        if (!(iss >> storedUsername >> storedSaltedHash)) { break; } // error

        if (storedUsername == username) {
            return storedSaltedHash;
        }
    }
    return std::nullopt;
}
