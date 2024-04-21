#include <common/aes_ecb.h>
#include <common/dh_key.h>
#include <cryptopp/nbtheory.h>
#include <gtest/gtest.h>
#include <cryptopp/secblock.h>
#include <cryptopp/osrng.h>
#include <thread>
#include <chrono>
#include <common/thread_list.h>
#include <common/rsa_wrapper.h>

// ==================== AESECB Tests ====================
// fixture class for AESEncryption tests
class AESEncryptionTest : public ::testing::Test {
protected:
};

// Test Case: test encryption and decryption for a basic string
TEST_F(AESEncryptionTest, EncryptDecryptBasicString) {
    std::string key = "16bytesecretkey!"; 
    AESECB aes(key);

    std::string plaintext = "Hello, World!";
    std::string ciphertext = aes.Encrypt(plaintext);
    std::string decryptedText = aes.Decrypt(ciphertext);

    EXPECT_EQ(plaintext, decryptedText);
}

// Test Case: test encryption and decryption for an empty string
TEST_F(AESEncryptionTest, EncryptDecryptEmptyString) {
    std::string key = "16bytesecretkey!";
    AESECB aes(key);

    std::string plaintext = "";
    std::string ciphertext = aes.Encrypt(plaintext);
    std::string decryptedText = aes.Decrypt(ciphertext);

    EXPECT_EQ(plaintext, decryptedText);
}

// Test Case: test encryption and decryption for a string that is exactly one block size
TEST_F(AESEncryptionTest, EncryptDecryptOneBlockSize) {
    std::string key = "16bytesecretkey!";
    AESECB aes(key);

    // AES block size is 16 bytes, so we use a 16-byte plaintext
    std::string plaintext = "1234567890ABCDEF";
    std::string ciphertext = aes.Encrypt(plaintext);
    std::string decryptedText = aes.Decrypt(ciphertext);

    EXPECT_EQ(plaintext, decryptedText);
}

// Test Case: test encryption and decryption for a string that is 1 byte less than a block size
TEST_F(AESEncryptionTest, EncryptDecryptOneByteLessThanBlockSize) {
    std::string key = "16bytesecretkey!";
    AESECB aes(key);

    // AES block size is 16 bytes, so we use a 15-byte plaintext
    std::string plaintext = "1234567890ABCDE";
    std::string ciphertext = aes.Encrypt(plaintext);
    std::string decryptedText = aes.Decrypt(ciphertext);

    EXPECT_EQ(plaintext, decryptedText);
}

// ==================== DHKeyExchange Tests ====================
// Test Case: test fixture for DHKeyExchange
class DHKeyExchangeTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        // set up the domain parameters once for all tests
        DHKeyExchange::createDomainParameters();
    }

    void SetUp() override {
        // each test will have fresh key pairs
        DHKeyExchange::createAsymmetricKey(DHKeyExchange::dhA, privKeyA, pubKeyA);
        DHKeyExchange::createAsymmetricKey(DHKeyExchange::dhA, privKeyB, pubKeyB);
    }

    CryptoPP::SecByteBlock privKeyA, pubKeyA;
    CryptoPP::SecByteBlock privKeyB, pubKeyB;
};

// Test Case: test for parameter initialization
TEST_F(DHKeyExchangeTest, DomainParametersAreValid) {
    ASSERT_NE(DHKeyExchange::dhA.GetGroupParameters().GetModulus(), 0);
    ASSERT_NE(DHKeyExchange::dhA.GetGroupParameters().GetGenerator(), 0);
}

// Test Case: test for asymmetric key generation
TEST_F(DHKeyExchangeTest, AsymmetricKeyGeneration) {
    ASSERT_GT(privKeyA.size(), 0);
    ASSERT_GT(pubKeyA.size(), 0);
    ASSERT_GT(privKeyB.size(), 0);
    ASSERT_GT(pubKeyB.size(), 0);
}

// Test Case: test for symmetric key agreement
TEST_F(DHKeyExchangeTest, SymmetricKeyAgreement) {
    try {
        CryptoPP::SecByteBlock sharedA = CryptoPP::SecByteBlock(DHKeyExchange::dhA.AgreedValueLength());
        ASSERT_TRUE(DHKeyExchange::dhA.Agree(sharedA, privKeyA, pubKeyB));

        CryptoPP::SecByteBlock sharedB = CryptoPP::SecByteBlock(DHKeyExchange::dhA.AgreedValueLength());
        ASSERT_TRUE(DHKeyExchange::dhA.Agree(sharedB, privKeyB, pubKeyA));

        // check if both shared secrets are equal
        ASSERT_EQ(memcmp(sharedA.BytePtr(), sharedB.BytePtr(), sharedA.size()), 0);
    } catch (const std::exception& e) {
        FAIL() << "Exception during symmetric key agreement: " << e.what();
    }
}
// ==================== Shared Secret Tests ====================
class AESKeyFromSecretTest : public ::testing::Test {
protected:
    CryptoPP::AutoSeededRandomPool rng;
    CryptoPP::SecByteBlock sharedSecret;

    // set up a known shared secret for testing
    void SetUp() override {
        sharedSecret = CryptoPP::SecByteBlock(32); // assuming a 32-byte shared secret
        rng.GenerateBlock(sharedSecret, sharedSecret.size());
    }
};

// Test Case: test for key generation from shared secret
TEST_F(AESKeyFromSecretTest, GeneratesCorrectKeyLength) {
    AESECB aes;
    std::string key = aes.keyFromSharedSecret(sharedSecret);
    // check that the key is exactly 32 bytes long, which is the output size of SHA256
    EXPECT_EQ(key.size(), 32);
}

// Test Case: test for consistent output for the same input
TEST_F(AESKeyFromSecretTest, ConsistentOutputForSameInput) {
    AESECB aes;
    std::string key1 = aes.keyFromSharedSecret(sharedSecret);
    std::string key2 = aes.keyFromSharedSecret(sharedSecret);
    // check that the function gives the same output for the same input
    EXPECT_EQ(key1, key2);
}

// Test Case: test for different output for different input
TEST_F(AESKeyFromSecretTest, DifferentOutputForDifferentInput) {
    AESECB aes;
    CryptoPP::SecByteBlock differentSecret(32);
    rng.GenerateBlock(differentSecret, differentSecret.size());

    std::string key1 = aes.keyFromSharedSecret(sharedSecret);
    std::string key2 = aes.keyFromSharedSecret(differentSecret);
    // check that different inputs produce different outputs
    EXPECT_NE(key1, key2);
}

// ==================== Thread Linked List Tests ====================

class ThreadListTests : public ::testing::Test {
protected:
    ThreadList threadList;

    void TearDown() override {
        threadList.clear(); 
    }
};

// Test Case: test that the list is initially empty
TEST_F(ThreadListTests, InitiallyEmpty) {
    EXPECT_TRUE(threadList.isEmpty());
}

// Test Case: test adding a single thread
TEST_F(ThreadListTests, AddSingleThread) {
    threadList.addThread(std::thread([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // simulate work
    }));
    EXPECT_FALSE(threadList.isEmpty());
}


// Test Case: test adding multiple threads
TEST_F(ThreadListTests, ClearList) {
    threadList.addThread(std::thread([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }));
    threadList.clear();
    EXPECT_TRUE(threadList.isEmpty());
}

// ==================== RSAWrapper Tests ====================
// test fixture class for RSAWrapper tests
class RSAWrapperTests : public ::testing::Test {
protected:
    RSAWrapper rsaWrapper; // RSAWrapper instance for testing

    // test data
    std::string plainText = "Hello, world! This is a test string for RSA encryption.";
};


// Test Case: test encryption and decryption with its own keys
TEST_F(RSAWrapperTests, EncryptDecrypt) {
    std::string encryptedText = rsaWrapper.encrypt(plainText, rsaWrapper.getPublicKey());
    std::string decryptedText = rsaWrapper.decrypt(encryptedText);
    ASSERT_EQ(plainText, decryptedText);
}

// Test Case: test encryption with one RSAWrapper's public key and decryption with another's private key
TEST_F(RSAWrapperTests, PublicKeyEncryption) {
    // generating another pair of keys for encryption
    RSAWrapper anotherRsaWrapper;
    std::string encryptedText = rsaWrapper.encrypt(plainText, anotherRsaWrapper.getPublicKey());
    ASSERT_NE(plainText, encryptedText); // The ciphertext should not be the same as plaintext

    // decrypt using the second RSAWrapper's private key
    std::string decryptedText = anotherRsaWrapper.decrypt(encryptedText);
    ASSERT_EQ(plainText, decryptedText);
}

// Test Case: test serialization and deserialization of RSA keys
TEST(RSASerialization, SerializeAndDeserialize) {
    CryptoPP::AutoSeededRandomPool rng;

    // Generate RSA keys
    CryptoPP::RSA::PrivateKey privateKey;
    CryptoPP::RSA::PublicKey publicKey;
    privateKey.GenerateRandomWithKeySize(rng, 2048);
    publicKey.AssignFrom(privateKey);

    // Serialize
    std::string serialized = RSAWrapper::sendPublicKey(publicKey);

    // Deserialize
    CryptoPP::RSA::PublicKey deserializedKey;
    ASSERT_TRUE(RSAWrapper::receivePublicKey(serialized, deserializedKey));

    // Check if the deserialized key is correct
    ASSERT_EQ(publicKey.GetModulus(), deserializedKey.GetModulus());
    ASSERT_EQ(publicKey.GetPublicExponent(), deserializedKey.GetPublicExponent());
}
