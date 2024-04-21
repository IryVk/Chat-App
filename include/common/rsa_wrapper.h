#ifndef RSAWRAPPER_H
#define RSAWRAPPER_H

#include <string>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h> // for file operations
#include <cryptopp/base64.h> // for Base64 encoding and decoding

class RSAWrapper {
public:
    RSAWrapper();  // constructor to initialize keys
    ~RSAWrapper(); // destructor

    std::string encrypt(const std::string& plainText, const CryptoPP::RSA::PublicKey& publicKey); // encrypt data using external public key
    std::string decrypt(const std::string& cipherText); // decrypt data

    void savePublicKey(const std::string& filename);  // save public key to a file
    void savePrivateKey(const std::string& filename); // save private key to a file
    void loadPublicKey(const std::string& filename);  // load public key from a file
    void loadPrivateKey(const std::string& filename); // load private key from a file
    static std::string sendPublicKey(const CryptoPP::RSA::PublicKey& publicKey); // send public key
    static bool receivePublicKey(std::string& jsonStr, CryptoPP::RSA::PublicKey& publicKey); // receive public key

    CryptoPP::RSA::PublicKey getPublicKey() const { return publicKey; } // getter for the public key
    CryptoPP::RSA::PrivateKey getPrivateKey() const { return privateKey; } // getter for the private key

    CryptoPP::RSA::PrivateKey privateKey;
    CryptoPP::RSA::PublicKey publicKey;
    CryptoPP::RSA::PublicKey publicKeyB; // other user's public key

private:
    CryptoPP::AutoSeededRandomPool rng; // random number generator

    void initializeKeys(); // Helper function to initialize keys
};

#endif // RSAWRAPPER_H