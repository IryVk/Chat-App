#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>

/**
 * @brief A class to hash passwords using SHA-256
 * 
 * This class is used to hash passwords using SHA-256 and a random salt. It provides methods to generate a random salt, hash a password using the salt, and verify if a password matches the expected hash.
*/
class PasswordHasher {
public:
    static std::string GenerateRandomSalt(size_t length); // Generate a random salt of a specified length
    static std::string HashPassword(const std::string& password, const std::string& salt); // Hash the password using SHA-256 and a given salt
    static bool VerifyPassword(const std::string& password, const std::string& saltedHash); // Verify if the provided password matches the expected hash when using the same salt

private:
    static std::string ToHexString(const unsigned char* data, size_t length); // Utility function to convert binary data to a hex string
};

#endif // PASSWORDHASHER_H
