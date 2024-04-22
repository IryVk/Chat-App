#ifndef RSAWRAPPER_H
#define RSAWRAPPER_H

#include <string>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h> 
#include <cryptopp/base64.h> 
#include <nlohmann/json.hpp>

/**
 * @brief A class to manage RSA encryption and decryption
 * 
 * This class is used to manage RSA encryption and decryption. It provides methods to encrypt and decrypt data, save and load keys to and from files, and send and receive public keys.
 * 
 * The class uses Crypto++ library for RSA encryption and decryption.
*/
class RSAWrapper {
public:
    RSAWrapper();  // Constructor to initialize keys
    ~RSAWrapper(); // Destructor

    std::string encrypt(const std::string& plainText, const CryptoPP::RSA::PublicKey& publicKey); // encrypt data using external public key
    std::string decrypt(const std::string& cipherText); // Decrypt data

    void savePublicKey(const std::string& filename);  // Save public key to a file
    void savePrivateKey(const std::string& filename); // Save private key to a file
    void loadPublicKey(const std::string& filename);  // Load public key from a file
    void loadPrivateKey(const std::string& filename); // Load private key from a file
    static std::string sendPublicKey(const CryptoPP::RSA::PublicKey& publicKey); // Send public key
    static bool receivePublicKey(std::string& jsonStr, CryptoPP::RSA::PublicKey& publicKey); // Receive public key

    CryptoPP::RSA::PublicKey getPublicKey() const { return publicKey; } // Getter for the public key
    CryptoPP::RSA::PrivateKey getPrivateKey() const { return privateKey; } // Getter for the private key

    CryptoPP::RSA::PrivateKey privateKey;
    CryptoPP::RSA::PublicKey publicKey;
    CryptoPP::RSA::PublicKey publicKeyB; // Other user's public key

private:
    CryptoPP::AutoSeededRandomPool rng; // Random number generator

    void initializeKeys(); // Helper function to initialize keys
};

#endif // RSAWRAPPER_H