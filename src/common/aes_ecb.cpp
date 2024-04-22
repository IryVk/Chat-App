/**
 * @file common/aes_ecb.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the implementation of the AESECB class
 * 
 * This file contains the implementation of the AESECB class, which is used to perform AES encryption and decryption in ECB mode.
*/

#include <common/aes_ecb.h>

/**
 * @brief Construct a new AESECB object
 * 
 * @param key The AES key to use for encryption and decryption
 * 
 * @return AESECB object
*/
AESECB::AESECB(std::string& key) : key(key) {}

/**
 * @brief Encrypt plaintext using AES in ECB mode
 * 
 * @param plaintext The plaintext to encrypt
 * 
 * @return std::string The encrypted ciphertext
*/
std::string AESECB::Encrypt(const std::string& plaintext) {
    std::string ciphertext;
    // Setup AES key and ECB mode. No IV is needed for ECB.
    ECB_Mode<AES>::Encryption ecbEncryption((byte*)key.data(), key.size());
    
    // Use StringSource and StreamTransformationFilter for encryption with PKCS #7 padding
    StringSource ss(plaintext, true,
        new StreamTransformationFilter(ecbEncryption,
            new StringSink(ciphertext),
            BlockPaddingSchemeDef::PKCS_PADDING
        )
    );

    // Turn into hex
    ciphertext = toHex(ciphertext);
    return ciphertext;
}

/**
 * @brief Decrypt ciphertext using AES in ECB mode
 * 
 * @param ciphertext The ciphertext to decrypt
 * 
 * @return std::string The decrypted plaintext
*/
std::string AESECB::Decrypt(const std::string& ciphertext) {
    // convert from hex
    std::string ct = fromHex(ciphertext);
    std::string decryptedText;
    // setup AES key and ECB mode. No IV is needed for ECB.
    ECB_Mode<AES>::Decryption ecbDecryption((byte*)key.data(), key.size());

    // use StringSource and StreamTransformationFilter for decryption with PKCS #7 padding
    StringSource ss(ct, true,
        new StreamTransformationFilter(ecbDecryption,
            new StringSink(decryptedText),
            BlockPaddingSchemeDef::PKCS_PADDING
        )
    );

    return decryptedText;
}

/**
 * @brief Derive an AES key from a shared secret
 * 
 * @param sharedSecret The shared secret to derive the key from
 * 
 * @return std::string The derived AES key
*/
std::string AESECB::keyFromSharedSecret(const SecByteBlock& sharedSecret) {
    // use SHA256 to hash the shared secret to get a 32-byte key
    std::string key;
    SHA256 sha256;
    // since sharedSecret is a SecByteBlock, we can directly use it in StringSource
    StringSource ss(sharedSecret.data(), sharedSecret.size(), true,
        new HashFilter(sha256,
            new StringSink(key)
        )
    );
    this->key = key;
    return key;
}

/**
 * @brief Utility function to convert string to hex
 * 
 * @param input The string to convert to hex
 * 
 * @return std::string The hex representation of the input string
*/
std::string AESECB::toHex(const std::string& input) {
    std::string hex;
    StringSource ss(input, true,
        new HexEncoder(
            new StringSink(hex)
        )
    );
    return hex;
}

/**
 * @brief Utility function to convert hex to string
 * 
 * @param input The hex string to convert to a normal string
 * 
 * @return std::string The normal string representation of the input hex string
*/
std::string AESECB::fromHex(const std::string& input) {
    std::string decoded;
    StringSource ss(input, true,
        new HexDecoder(
            new StringSink(decoded)
        )
    );
    return decoded;
}