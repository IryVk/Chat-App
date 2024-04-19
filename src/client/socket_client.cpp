#include "client/socket_client.h"
#include <common/aes_ecb.h>
#include <common/dh_key.h>
#include "cryptopp/cryptlib.h"
#include "cryptopp/hex.h"
#include "cryptopp/integer.h"
#include "cryptopp/filters.h"
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cassert>


using json = nlohmann::json;

Client::Client(const std::string& server_ip, int port) : sock(-1), server_ip(server_ip), port(port), aes(), priv_key() {}

Client::~Client() {
    disconnect();
}

bool Client::connectToServer() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Could not create socket." << std::endl;
        return false;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        std::cerr << "Connect failed." << std::endl;
        return false;
    }

    std::cout << "Connected to the server." << std::endl;
    return true;
}

void Client::disconnect() {
    if (sock != -1) {
        close(sock);
        sock = -1;
    }
}

void Client::receiveMessages() {
    char buffer[1024];
    while (true) {
        ssize_t len = recv(sock, buffer, sizeof(buffer), 0);
        if (len > 0) {
            std::string msg(buffer, len);
            json j = json::parse(msg);
            handleJsonMessage(j.dump());
        } else if (len == 0) {
            std::cout << "Server closed connection." << std::endl;
            break;
        } else {
            std::cerr << "Failed to receive data." << std::endl;
            break;
        }
    }
}

void Client::sendMessage(const json& message) {
    std::string msg = message.dump();
    if (send(sock, msg.c_str(), msg.size(), 0) < 0) {
        std::cerr << "Send failed." << std::endl;
    }
}

// define color codes as constants
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

void Client::printColoredMessage(const std::string& message, const std::string& color) {
    std::cout << color << message << RESET << std::endl;
}

void Client::handleJsonMessage(const std::string& jsonStr) {
    auto j = json::parse(jsonStr);
    std::string type = j.value("type", "info"); // default to "info" if no type is specified
    std::string message = j.value("message", "");
    if (type == "text") {
        message = this->aes.Decrypt(AESECB::fromHex(message));
    }

    if (type == "error") {
        printColoredMessage(message, RED);
    } else if (type == "warning") {
        printColoredMessage(message, YELLOW);
    } else if (type == "info") {
        printColoredMessage(message, BLUE);
    } else if (type == "success") {
        printColoredMessage(message, GREEN);
    } else if (type == "text") {
        printColoredMessage("USER: " + message, MAGENTA); 
    } else if (type == "key_exchange") {
        printColoredMessage(jsonStr, CYAN);
        this->keyExchangeResponse(jsonStr);
    } else if (type == "connected") {
        this->keyExchangeInit();
    } else if (type == "key_exchange_response") {
        printColoredMessage(jsonStr, CYAN);
        this->setKey(jsonStr);
    } 
}

std::string integerToHexString(const CryptoPP::Integer& num) {
    std::string hex;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hex));
    num.Encode(encoder, num.MinEncodedSize());
    encoder.MessageEnd();
    return hex;
}
std::string byteToHex(const byte* data, size_t size) {
    std::string output;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(output));
    encoder.Put(data, size);
    encoder.MessageEnd();
    return output;
}

void Client::keyExchangeInit(){
    DHKeyExchange::createDomainParameters();
    CryptoPP::SecByteBlock privKeyA, pubKeyA;
    DHKeyExchange::createAsymmetricKey(DHKeyExchange::dhA, privKeyA, pubKeyA);
    this->priv_key = privKeyA;
    // print public key, p and g as integers
    // std::cout << pubKeyA.BytePtr() << std::endl;
    std::string modulusHex = integerToHexString(DHKeyExchange::dhA.GetGroupParameters().GetModulus());
    std::string generatorHex = integerToHexString(DHKeyExchange::dhA.GetGroupParameters().GetGenerator());
    nlohmann::json message;
    message["type"] = "key_exchange";
    message["pub_key"] = byteToHex(pubKeyA.BytePtr(), pubKeyA.SizeInBytes());
    message["p"] = modulusHex;  
    message["g"] = generatorHex;
    
    

    this->sendMessage(message);  

}

void Client::keyExchangeResponse(const std::string& jsonStr) {
    auto j = json::parse(jsonStr);
    std::string pubKeyAHex = j["pub_key"];
    std::string pHex = j["p"];
    std::string gHex = j["g"];

    CryptoPP::Integer p(pHex.c_str()), g(gHex.c_str());


    DHKeyExchange::createDomainParameters();

    CryptoPP::SecByteBlock privKeyB, pubKeyB, pubKeyA;
    DHKeyExchange::createAsymmetricKey(DHKeyExchange::dhA, privKeyB, pubKeyB);
    this->priv_key = privKeyB;

    // Decode the received public key from Hex
    pubKeyA = CryptoPP::SecByteBlock(pubKeyAHex.size() / 2);
    CryptoPP::StringSource(pubKeyAHex, true, new CryptoPP::HexDecoder(new CryptoPP::ArraySink(pubKeyA.BytePtr(), pubKeyA.SizeInBytes())));

    // Prepare shared secret
    CryptoPP::SecByteBlock sharedSecret(DHKeyExchange::dhA.AgreedValueLength());

    if (!DHKeyExchange::dhA.Agree(sharedSecret, privKeyB, pubKeyA)) {
        throw std::runtime_error("Failed to reach shared secret");
    }

    std::string key = aes.keyFromSharedSecret(sharedSecret);
    // std::cout << "Shared Secret: " << key << std::endl;
    // send public key back
    nlohmann::json message;
    message["type"] = "key_exchange_response";
    message["pub_key"] = byteToHex(pubKeyB.BytePtr(), pubKeyB.SizeInBytes());

    this->sendMessage(message);  
}

void Client::setKey(const std::string& jsonStr){
    auto j = json::parse(jsonStr);
    std::string pubKeyBHex = j["pub_key"];

    DHKeyExchange::createDomainParameters();
    
    // Decode the received public key from Hex
    CryptoPP::SecByteBlock pubKeyB = CryptoPP::SecByteBlock(pubKeyBHex.size() / 2);
    CryptoPP::StringSource(pubKeyBHex, true, new CryptoPP::HexDecoder(new CryptoPP::ArraySink(pubKeyB.BytePtr(), pubKeyB.SizeInBytes())));

    // Prepare shared secret
    CryptoPP::SecByteBlock sharedSecret(DHKeyExchange::dhA.AgreedValueLength());
    
    if (!DHKeyExchange::dhA.Agree(sharedSecret, this->priv_key, pubKeyB)) {
        throw std::runtime_error("Failed to reach shared secret");
    }

    std::string key = aes.keyFromSharedSecret(sharedSecret);
    // std::cout << "Shared Secret: " << key << std::endl;
    // send public key back

}
