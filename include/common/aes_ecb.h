#ifndef AESECB_H
#define AESECB_H

#include <string>

class AESECB {
public:
    // constructor that takes the encryption key
    AESECB(const std::string& key);

    // encrypts the given plaintext using AES-ECB and applies PKCS#7 padding
    std::string Encrypt(const std::string& plaintext);

    // decrypts the given ciphertext using AES-ECB and removes PKCS#7 padding
    std::string Decrypt(const std::string& ciphertext);

private:
    std::string key;
    // utility to cast to hex
    std::string toHex(const std::string& input);
    // utility to cast from hex
    std::string fromHex(const std::string& input);
};

#endif // AESECB_H