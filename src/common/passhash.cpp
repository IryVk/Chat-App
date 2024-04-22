/**
 * @file common/passhash.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the implementation of the PasswordHasher class
 * 
 * This file contains the implementation of the PasswordHasher class, which is used to generate and verify password hashes using a random salt and SHA-256.
*/

#include <common/passhash.h>

using namespace CryptoPP;

/**
 * @brief Generate a random salt of the specified length
 * 
 * @param length The length of the salt to generate
 * 
 * @return std::string The generated salt
*/
std::string PasswordHasher::GenerateRandomSalt(size_t length) {
    AutoSeededRandomPool rng;
    byte salt[length];
    rng.GenerateBlock(salt, sizeof(salt));
    return ToHexString(salt, sizeof(salt));
}

/**
 * @brief Hash the password using SHA-256 with the provided salt
 * 
 * @param password The password to hash
 * @param salt The salt to use for hashing
 * 
 * @return std::string The salted hash of the password
*/
std::string PasswordHasher::HashPassword(const std::string& password, const std::string& salt) {
    SHA256 hash;
    std::string digest;
    StringSource s(password + salt, true, new HashFilter(hash, new HexEncoder(new StringSink(digest), false)));
    return salt + digest; // {repend the salt to the hash for storage
}

/**
 * @brief Verify the password against the salted hash
 * 
 * @param password The password to verify
 * @param saltedHash The salted hash to verify against
 * 
 * @return bool True if the password matches the hash, false otherwise
*/
bool PasswordHasher::VerifyPassword(const std::string& password, const std::string& saltedHash) {
    size_t saltLength = 32; // Length of the salt in hex characters (16 bytes * 2 characters per byte)
    std::string salt = saltedHash.substr(0, saltLength);
    std::string expectedHash = saltedHash.substr(saltLength);

    std::string actualHash = HashPassword(password, salt).substr(saltLength); // Hash and strip the salt
    return actualHash == expectedHash;
}

/**
 * @brief Convert a byte array to a hex string
 * 
 * @param data The byte array to convert
 * @param length The length of the byte array
 * 
 * @return std::string The hex string
*/
std::string PasswordHasher::ToHexString(const unsigned char* data, size_t length) {
    std::string hex;
    HexEncoder encoder(new StringSink(hex));
    encoder.Put(data, length);
    encoder.MessageEnd();
    return hex;
}
