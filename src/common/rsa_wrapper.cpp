#include "common/rsa_wrapper.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

RSAWrapper::RSAWrapper() {
    initializeKeys();
}

RSAWrapper::~RSAWrapper() {}

void RSAWrapper::initializeKeys() {
    // generate RSA keys
    CryptoPP::InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(rng, 2048);

    privateKey = CryptoPP::RSA::PrivateKey(params);
    publicKey = CryptoPP::RSA::PublicKey(params);
}

std::string RSAWrapper::encrypt(const std::string& plainText, const CryptoPP::RSA::PublicKey& publicKey) {
    std::string cipherText;
    CryptoPP::RSAES_OAEP_SHA_Encryptor e(publicKey);
    CryptoPP::StringSource(plainText, true,
        new CryptoPP::PK_EncryptorFilter(rng, e,
            new CryptoPP::StringSink(cipherText)
        )
    );
    return cipherText;
}

std::string RSAWrapper::decrypt(const std::string& cipherText) {
    std::string recoveredText;
    CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
    CryptoPP::StringSource(cipherText, true,
        new CryptoPP::PK_DecryptorFilter(rng, d,
            new CryptoPP::StringSink(recoveredText)
        )
    );
    return recoveredText;
}

void RSAWrapper::savePublicKey(const std::string& filename) {
    CryptoPP::Base64Encoder publicKeyEncoder;
    publicKey.Save(CryptoPP::FileSink(filename.c_str()).Ref());
}

void RSAWrapper::savePrivateKey(const std::string& filename) {
    CryptoPP::Base64Encoder privateKeyEncoder;
    privateKey.Save(CryptoPP::FileSink(filename.c_str()).Ref());
}

void RSAWrapper::loadPublicKey(const std::string& filename) {
    CryptoPP::FileSource file(filename.c_str(), true, new CryptoPP::Base64Decoder);
    publicKey.Load(file.Ref());
}

void RSAWrapper::loadPrivateKey(const std::string& filename) {
    CryptoPP::FileSource file(filename.c_str(), true, new CryptoPP::Base64Decoder);
    privateKey.Load(file.Ref());
}

// send public key
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

    // create JSON object
    json j;
    j["type"] = "public_key";
    j["modulus"] = nStr;
    j["exponent"] = eStr;

    // send JSON as a string
    std::string message = j.dump();
    return message;
}

// receive public key
bool RSAWrapper::receivePublicKey(std::string& jsonStr, CryptoPP::RSA::PublicKey& publicKey) {
     try {
        auto j = json::parse(jsonStr);

        // decode from base64
        CryptoPP::Base64Decoder decoder;
        std::string modulusDecoded, exponentDecoded;

        decoder.Attach(new CryptoPP::StringSink(modulusDecoded));
        decoder.Put((const byte*)j["modulus"].get<std::string>().data(), j["modulus"].get<std::string>().size());
        decoder.MessageEnd();

        decoder.Attach(new CryptoPP::StringSink(exponentDecoded));
        decoder.Put((const byte*)j["exponent"].get<std::string>().data(), j["exponent"].get<std::string>().size());
        decoder.MessageEnd();

        CryptoPP::Integer modulus((const byte*)modulusDecoded.data(), modulusDecoded.size());
        CryptoPP::Integer exponent((const byte*)exponentDecoded.data(), exponentDecoded.size());

        // set the public key
        publicKey.Initialize(modulus, exponent);
        return true;
    } catch (const json::exception& e) {
        std::cerr << "Error parsing JSON or setting RSA key: " << e.what() << std::endl;
        return false;
    }
}

