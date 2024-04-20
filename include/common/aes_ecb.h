#ifndef AESECB_H
#define AESECB_H

#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include "cryptopp/sha.h"

using namespace CryptoPP;

class AESECB {
public:
    // constructor that takes the encryption key
    AESECB(std::string& key);
    // constructor that needs no key
    AESECB() {}

    // encrypts the given plaintext using AES-ECB and applies PKCS#7 padding
    std::string Encrypt(const std::string& plaintext);

    // decrypts the given ciphertext using AES-ECB and removes PKCS#7 padding
    std::string Decrypt(const std::string& ciphertext);

    // utility to cast to hex
    static std::string toHex(const std::string& input);

    // utility to cast from hex
    static std::string fromHex(const std::string& input);

    // generate key from DH shared secret
    std::string keyFromSharedSecret(const SecByteBlock& sharedSecret);

private:
    std::string key;
};

#endif // AESECB_H