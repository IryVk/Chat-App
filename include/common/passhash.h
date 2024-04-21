#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>

class PasswordHasher {
public:
    // generate a random salt of a specified length
    static std::string GenerateRandomSalt(size_t length);

    // hash the password using SHA-256 and a given salt
    static std::string HashPassword(const std::string& password, const std::string& salt);

    // Verify if the provided password matches the expected hash when using the same salt
    static bool VerifyPassword(const std::string& password, const std::string& saltedHash);


private:
    // utility function to convert binary data to a hex string
    static std::string ToHexString(const unsigned char* data, size_t length);
};

#endif // PASSWORDHASHER_H
