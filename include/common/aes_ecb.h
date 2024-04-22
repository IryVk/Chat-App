#ifndef AESECB_H
#define AESECB_H

#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include "cryptopp/sha.h"

using namespace CryptoPP;

/**
 * @brief A class to encrypt and decrypt data using AES-ECB
 * 
 * This class is used to encrypt and decrypt data using AES-ECB. It provides methods to encrypt and decrypt data, as well as utility functions to convert data to and from hex.
*/
class AESECB {
public:
    AESECB(std::string& key); // Constructor that takes the encryption key
    AESECB() {} // Constructor that needs no key

    std::string Encrypt(const std::string& plaintext); // Encrypts the given plaintext using AES-ECB and applies PKCS#7 padding
    std::string Decrypt(const std::string& ciphertext); // Decrypts the given ciphertext using AES-ECB and removes PKCS#7 padding

    
    static std::string toHex(const std::string& input); // Utility to cast to hex
    static std::string fromHex(const std::string& input); // Utility to cast from hex

    std::string keyFromSharedSecret(const SecByteBlock& sharedSecret); // Generate key from DH shared secret

private:
    std::string key;
};

#endif // AESECB_H