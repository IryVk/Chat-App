#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>
#include <ncurses.h>
#include <common/aes_ecb.h>
#include <common/dh_key.h>
#include <unistd.h>

class Client {
public:
    Client(std::string& server_ip, int port); // constructor
    ~Client(); // destructor

    bool connectToServer(); // connect to the server
    void disconnect(); // disconnect from the server
    void receiveMessages(); // receive messages from the server
    void sendMessage(const nlohmann::json& message); // send a message to the server (json)
    // atomic variable to control the status of the client
    std::atomic<bool> status{true}; // flag to indicate if the client is active (atomic for thread safety)
    
    int sock;
    std::string server_ip;
    int port;
    AESECB aes; // AES object to store the key and perform encryption/decryption
    CryptoPP::SecByteBlock priv_key;

    void printColoredMessage(const std::string& message, const std::string& color, WINDOW* outputWin); // print colored message (output window)
    void handleJsonMessage(const std::string& jsonStr, WINDOW* outputWin); // handle json message
    void keyExchangeInit(); // key exchange initialization
    void keyExchangeResponse(const std::string& jsonStr); // key exchange response
    void setKey(const std::string& jsonStr); // set the key for encryption (for the initiator)
};

#endif // SOCKET_CLIENT_H
