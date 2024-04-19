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

class Client {
public:
    Client(const std::string& server_ip, int port);
    ~Client();

    bool connectToServer();
    void disconnect();
    void receiveMessages();
    void sendMessage(const nlohmann::json& message);

private:
    int sock;
    std::string server_ip;
    int port;
    void printColoredMessage(const std::string& message, const std::string& color);
    void handleJsonMessage(const std::string& jsonStr);
};

#endif // SOCKET_CLIENT_H
