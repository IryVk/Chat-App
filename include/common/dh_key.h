#ifndef DH_KEY_EXCHANGE_H
#define DH_KEY_EXCHANGE_H

#include <cryptopp/dh.h>
#include <cryptopp/osrng.h>

/**
 * @brief A class to manage Diffie-Hellman key exchange
 * 
 * This class is used to manage Diffie-Hellman key exchange. It provides methods to generate domain parameters, create asymmetric key pairs, and create symmetric keys.
 * 
 * The class uses Crypto++ library for Diffie-Hellman key exchange.
*/
class DHKeyExchange {
public:
    static void createDomainParameters(); // Generate domain parameters, static so that it can be called without creating an object
    static void createAsymmetricKey(const CryptoPP::DH &dh, CryptoPP::SecByteBlock &privKey,  CryptoPP::SecByteBlock &pubKey); // Generate asymmetric key pair (aka private and public key)
    static void createSymmetricKey(const CryptoPP::DH &dh,const CryptoPP::SecByteBlock &privKey, const CryptoPP::SecByteBlock &pubKey); // Generate symmetric key

    static CryptoPP::AutoSeededRandomPool rnd; // Random number generator
    static CryptoPP::DH dhA; // Domain parameters for A
    static CryptoPP::SecByteBlock privKeyA, pubKeyA, pubKeyB; // Private key of A, public key of A and B
    
};

#endif // DH_KEY_EXCHANGE_H
