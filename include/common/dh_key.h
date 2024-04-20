#ifndef DH_KEY_EXCHANGE_H
#define DH_KEY_EXCHANGE_H

#include <cryptopp/dh.h>
#include <cryptopp/osrng.h>

class DHKeyExchange {
public:
    static void createDomainParameters(); // generate domain parameters, static so that it can be called without creating an object
    static void createAsymmetricKey(const CryptoPP::DH &dh, CryptoPP::SecByteBlock &privKey,  CryptoPP::SecByteBlock &pubKey); // generate asymmetric key pair (aka private and public key)
    static void createSymmetricKey(const CryptoPP::DH &dh,const CryptoPP::SecByteBlock &privKey, const CryptoPP::SecByteBlock &pubKey); // generate symmetric key

    static CryptoPP::AutoSeededRandomPool rnd; // random number generator
    static CryptoPP::DH dhA; // domain parameters for A
    static CryptoPP::SecByteBlock privKeyA, pubKeyA, pubKeyB; // private key of A, public key of A and B
    
};

#endif // DH_KEY_EXCHANGE_H
