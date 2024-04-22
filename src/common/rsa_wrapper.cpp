/**
 * @file common/rsa_wrapper.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the implementation of the RSAWrapper class
 * 
 * This file contains the implementation of the RSAWrapper class, which is used to manage RSA encryption and decryption. It provides methods to encrypt and decrypt data, save and load keys to and from files, and send and receive public keys.
*/

#include <common/rsa_wrapper.h>

using json = nlohmann::json;

/**
 * @brief Construct a new RSAWrapper object
 * 
 * Initialize the RSA keys for encryption and decryption.
 * 
 * @return RSAWrapper object
*/
RSAWrapper::RSAWrapper() {
    initializeKeys();
}

/**
 * @brief Destroy the RSAWrapper object
 * 
 * Destroy the RSAWrapper object and free any allocated resources.
 * 
 * @return void
*/
RSAWrapper::~RSAWrapper() {}


/**
 * @brief Initialize the RSA keys for encryption and decryption
 * 
 * Generate the RSA keys for encryption and decryption.
 * 
 * @return void
*/
void RSAWrapper::initializeKeys() {
    // Generate RSA keys
    CryptoPP::InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(rng, 2048);

    // Set the public and private keys
    privateKey = CryptoPP::RSA::PrivateKey(params);
    publicKey = CryptoPP::RSA::PublicKey(params);
}

/**
 * @brief Encrypt data using an external public key
 * 
 * Encrypt the provided plaintext using the specified public key.
 * 
 * @param plainText The plaintext to encrypt
 * @param publicKey The public key to use for encryption
 * 
 * @return std::string The encrypted ciphertext
*/
std::string RSAWrapper::encrypt(const std::string& plainText, const CryptoPP::RSA::PublicKey& publicKey) {
    std::string cipherText;
    CryptoPP::RSAES_OAEP_SHA_Encryptor e(publicKey);
    CryptoPP::StringSource(plainText, true,
        new CryptoPP::PK_EncryptorFilter(rng, e,
            new CryptoPP::Base64Encoder(
                new CryptoPP::StringSink(cipherText), false // 'false' means do not append a newline
            )
        )
    );
    return cipherText;
}

/**
 * @brief Decrypt data using the private key
 * 
 * Decrypt the provided ciphertext using the private key.
 * 
 * @param cipherText The ciphertext to decrypt
 * 
 * @return std::string The decrypted plaintext
*/
std::string RSAWrapper::decrypt(const std::string& cipherText) {
    std::string recoveredText, binaryText;
    // first, decode the Base64 to binary
    CryptoPP::StringSource(cipherText, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(binaryText)
        )
    );
    // now, decrypt the binary text
    CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
    CryptoPP::StringSource(binaryText, true,
        new CryptoPP::PK_DecryptorFilter(rng, d,
            new CryptoPP::StringSink(recoveredText)
        )
    );
    return recoveredText;
}

/**
 * @brief Save the public key to a file
 * 
 * Save the public key to the specified file.
 * 
 * @param filename The name of the file to save the public key to
 * 
 * @return void
*/
void RSAWrapper::savePublicKey(const std::string& filename) {
    CryptoPP::Base64Encoder publicKeyEncoder;
    publicKey.Save(CryptoPP::FileSink(filename.c_str()).Ref());
}

/**
 * @brief Save the private key to a file
 * 
 * Save the private key to the specified file.
 * 
 * @param filename The name of the file to save the private key to
 * 
 * @return void
*/
void RSAWrapper::savePrivateKey(const std::string& filename) {
    CryptoPP::Base64Encoder privateKeyEncoder;
    privateKey.Save(CryptoPP::FileSink(filename.c_str()).Ref());
}

/**
 * @brief Load the public key from a file
 * 
 * Load the public key from the specified file.
 * 
 * @param filename The name of the file to load the public key from
 * 
 * @return void
*/
void RSAWrapper::loadPublicKey(const std::string& filename) {
    CryptoPP::FileSource file(filename.c_str(), true, new CryptoPP::Base64Decoder);
    publicKey.Load(file.Ref());
}

/**
 * @brief Load the private key from a file
 * 
 * Load the private key from the specified file.
 * 
 * @param filename The name of the file to load the private key from
 * 
 * @return void
*/
void RSAWrapper::loadPrivateKey(const std::string& filename) {
    CryptoPP::FileSource file(filename.c_str(), true, new CryptoPP::Base64Decoder);
    privateKey.Load(file.Ref());
}

/**
 * @brief Send the public key as a JSON string
 * 
 * Send the public key as a JSON string to be transmitted over the network.
 * 
 * @param publicKey The public key to send
 * 
 * @return std::string The JSON string containing the public key
*/
std::string RSAWrapper::sendPublicKey(const CryptoPP::RSA::PublicKey& publicKey) {
    CryptoPP::Integer n = publicKey.GetModulus();
    CryptoPP::Integer e = publicKey.GetPublicExponent();

    // Convert the integers to a Base64 string
    std::string nStr, eStr;
    CryptoPP::Base64Encoder nEncoder(new CryptoPP::StringSink(nStr));
    n.Encode(nEncoder, n.ByteCount());
    nEncoder.MessageEnd();

    CryptoPP::Base64Encoder eEncoder(new CryptoPP::StringSink(eStr));
    e.Encode(eEncoder, e.ByteCount());
    eEncoder.MessageEnd();

    // Create JSON object
    json j;
    j["type"] = "public_key";
    j["modulus"] = nStr;
    j["exponent"] = eStr;

    // Send JSON as a string
    std::string message = j.dump();
    return message;
}

/**
 * @brief Receive the public key from a JSON string
 * 
 * Receive the public key from a JSON string transmitted over the network.
 * 
 * @param jsonStr The JSON string containing the public key
 * @param publicKey The public key to receive
 * 
 * @return bool True if the public key was received successfully, false otherwise
*/
bool RSAWrapper::receivePublicKey(std::string& jsonStr, CryptoPP::RSA::PublicKey& publicKey) {
     try {
        auto j = json::parse(jsonStr);

        // Decode from base64
        CryptoPP::Base64Decoder decoder;
        std::string modulusDecoded, exponentDecoded;
        // Decode the modulus and exponent
        decoder.Attach(new CryptoPP::StringSink(modulusDecoded));
        decoder.Put(reinterpret_cast<const byte*>(j["modulus"].get<std::string>().data()), j["modulus"].get<std::string>().size());
        decoder.MessageEnd();

        decoder.Attach(new CryptoPP::StringSink(exponentDecoded));
        decoder.Put(reinterpret_cast<const byte*>(j["exponent"].get<std::string>().data()), j["exponent"].get<std::string>().size());
        decoder.MessageEnd();

        // Convert to CryptoPP::Integer
        CryptoPP::Integer modulus(reinterpret_cast<const byte*>(modulusDecoded.data()), modulusDecoded.size());
        CryptoPP::Integer exponent(reinterpret_cast<const byte*>(exponentDecoded.data()), exponentDecoded.size());

        // Set the public key
        publicKey.Initialize(modulus, exponent);
        return true;
    } catch (const json::exception& e) {
        std::cerr << "Error parsing JSON or setting RSA key: " << e.what() << std::endl;
        return false;
    }
}

