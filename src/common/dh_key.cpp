/**
 * @file common/dh_key.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the implementation of the DHKeyExchange class
 * 
 * This file contains the implementation of the DHKeyExchange class, which is used to perform Diffie-Hellman key exchange.
*/

#include "common/dh_key.h"

CryptoPP::AutoSeededRandomPool DHKeyExchange::rnd;
CryptoPP::DH DHKeyExchange::dhA;
CryptoPP::SecByteBlock DHKeyExchange::privKeyA, DHKeyExchange::pubKeyA, DHKeyExchange::pubKeyB;

/**
 * @brief Construct the CryptoPP:: object in the DHKeyExchange class
 * 
 * Initialize the CryptoPP:: object in the DHKeyExchange class using the NIST 2048-bit MODP Group (RFC 3526 Group 14) parameters.
*/
void DHKeyExchange::createDomainParameters() {
    // NIST 2048-bit MODP Group (RFC 3526 Group 14)
    const CryptoPP::Integer p("0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1"
                              "29024E088A67CC74020BBEA63B139B22514A08798E3404DD"
                              "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
                              "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
                              "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D"
                              "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"
                              "83655D23DCA3AD961C62F356208552BB9ED529077096966D"
                              "670C354E4ABC9804F1746C08CA237327FFFFFFFFFFFFFFFF");

    const CryptoPP::Integer g("0x2");
    CryptoPP::Integer q = (p - 1) / 2;
    dhA = CryptoPP::DH(p, q, g);
}

/**
 * @brief Create the public and private keys for the DH key exchange
 * 
 * Create the public and private keys for the DH key exchange using the CryptoPP::DH object.
 * 
 * @param dh The CryptoPP::DH object to use for key exchange
 * @param privKey The private key to generate
 * @param pubKey The public key to generate
 * 
 * @return void
*/
void DHKeyExchange::createAsymmetricKey(const CryptoPP::DH &dh, CryptoPP::SecByteBlock &privKey, CryptoPP::SecByteBlock &pubKey) {
    privKey = CryptoPP::SecByteBlock(dh.PrivateKeyLength());
    pubKey = CryptoPP::SecByteBlock(dh.PublicKeyLength());
    dh.GenerateKeyPair(rnd, privKey, pubKey);

    CryptoPP::Integer a, b;
    a.Decode(privKey.BytePtr(), privKey.SizeInBytes());
    b.Decode(pubKey.BytePtr(), pubKey.SizeInBytes());
}

/**
 * @brief Create the symmetric key from the public and private keys
 * 
 * Create the symmetric key from the public and private keys using the CryptoPP::DH object.
 * 
 * @param dh The CryptoPP::DH object to use for key exchange
 * @param privKey The private key to use
 * @param pubKey The public key to use
 * 
 * @return void
*/
void DHKeyExchange::createSymmetricKey(const CryptoPP::DH &dh,const CryptoPP::SecByteBlock &privKey, const CryptoPP::SecByteBlock &pubKey) {
    CryptoPP::SecByteBlock shared(dh.AgreedValueLength());
    if (!dh.Agree(shared, privKey, pubKey))
        throw std::runtime_error("Failed to reach shared secret");

    CryptoPP::Integer x;
    x.Decode(shared.BytePtr(), shared.SizeInBytes());
}
