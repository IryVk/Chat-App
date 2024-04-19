#ifndef DH_KEY_EXCHANGE_H
#define DH_KEY_EXCHANGE_H

#include <cryptopp/dh.h>
#include <cryptopp/osrng.h>
#include <cryptopp/secblock.h>

class DHKeyExchange {
public:
    static void createDomainParameters();
    static void createAsymmetricKey(const CryptoPP::DH &dh, CryptoPP::SecByteBlock &privKey,  CryptoPP::SecByteBlock &pubKey);
    static void createSymmetricKey(const CryptoPP::DH &dh,const CryptoPP::SecByteBlock &privKey, const CryptoPP::SecByteBlock &pubKey);

    static CryptoPP::AutoSeededRandomPool rnd;
    static CryptoPP::DH dhA;
    static CryptoPP::SecByteBlock privKeyA, pubKeyA, pubKeyB;
    
};

#endif // DH_KEY_EXCHANGE_H
