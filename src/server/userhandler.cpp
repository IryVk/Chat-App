#include <server/userhandler.h>


// constructor that specifies the file used for storing user data
UserHandler::UserHandler(const std::string& filename) : filename(filename) {}

// adds a new user with the given username and password
bool UserHandler::AddUser(const std::string& username, const std::string& password) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        return false;
    }
    // check if the username already exists
    if (FindUserSaltedHash(username).has_value()) {
        return false;
    }
    std::string saltedHash = PasswordHasher::HashPassword(password, PasswordHasher::GenerateRandomSalt(16));
    file << username << " " << saltedHash << std::endl;
    return true;
}

// verifies if the provided username and password are correct
bool UserHandler::VerifyUser(const std::string& username, const std::string& password) {
    auto saltedHashOpt = FindUserSaltedHash(username);
    if (!saltedHashOpt.has_value()) {
        return false;
    }
    return PasswordHasher::VerifyPassword(password, saltedHashOpt.value());
}

// utility function to find a user's salted hash by username
std::optional<std::string> UserHandler::FindUserSaltedHash(const std::string& username) {
    std::ifstream file(filename);
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
