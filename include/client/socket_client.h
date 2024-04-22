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
#include <common/rsa_wrapper.h>

/**
 * @brief A class to manage the client side of the chat application
 * 
 * This class is used to manage the client side of the chat application. It provides methods to connect to the server, send and receive messages, and handle the key exchange process.
 * 
 * The class uses the Crypto++ library for encryption and decryption, and the nlohmann json library for handling JSON messages.
*/
class Client {
public:
    Client(std::string& server_ip, int port, std::string username, std::string password, int type); // Constructor
    ~Client(); // Destructor

    bool connectToServer(); // Connect to the server
    void disconnect(); // Disconnect from the server
    void receiveMessages(); // Receive messages from the server
    void sendMessage(const nlohmann::json& message); // Send a message to the server (json)

    // Atomic variable to control the status of the client (can be accessed by multiple threads)
    std::atomic<bool> status{true}; // Flag to indicate if the client is active (atomic for thread safety)
    int sock;
    std::string server_ip;
    int port;
    AESECB aes; // AES object to store the key and perform encryption/decryption
    CryptoPP::SecByteBlock priv_key;
    RSAWrapper rsa; // RSA wrapper
    std::string username;
    std::string password;
    int authType;

    void printColoredMessage(const std::string& message, const std::string& color, WINDOW* outputWin); // Print colored message (output window)
    void printColoredMessage(const std::string& message, const std::string& color); // Print colored message (standard output)
    void handleJsonMessage(const std::string& jsonStr, WINDOW* outputWin); // Handle json message
    void keyExchangeInit(); // Key exchange initialization
    void keyExchangeResponse(const std::string& jsonStr); // Key exchange response
    void setKey(const std::string& jsonStr); // Set the key for encryption (for the initiator)
};

#endif // SOCKET_CLIENT_H
