#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include "common/aes_ecb.h"
#include <atomic>

class Client {
public:
    Client(std::string& server_ip, int port);
    ~Client();

    bool connectToServer();
    void disconnect();
    void receiveMessages();
    void sendMessage(const nlohmann::json& message);
    // atomic variable to control the status of the client
    std::atomic<bool> status{true};;
    

private:
    int sock;
    std::string server_ip;
    int port;
public:
    AESECB aes;
private:
    CryptoPP::SecByteBlock priv_key;
    void printColoredMessage(const std::string& message, const std::string& color);
    void handleJsonMessage(const std::string& jsonStr);
    void keyExchangeInit();
    void keyExchangeResponse(const std::string& jsonStr);
    void setKey(const std::string& jsonStr);
};

#endif // SOCKET_CLIENT_H
