#include "client/socket_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

SocketClient::SocketClient(const std::string &serverIP, int serverPort)
    : sockfd(-1), serverIP(serverIP), serverPort(serverPort) {}

SocketClient::~SocketClient() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

bool SocketClient::connectToServer() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Cannot open socket." << std::endl;
        return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIP.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported." << std::endl;
        close(sockfd);
        sockfd = -1;
        return false;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed." << std::endl;
        close(sockfd);
        sockfd = -1;
        return false;
    }
    return true;
}

bool SocketClient::sendMessage(const std::string &message) {
    if (send(sockfd, message.c_str(), message.length(), 0) < 0) {
        std::cerr << "Send failed." << std::endl;
        return false;
    }
    return true;
}

std::string SocketClient::receiveMessage() {
    char buffer[1024] = {0};
    std::string result;
    while (true) {
        ssize_t bytesReceived = recv(sockfd, buffer, 1024, 0);
        if (bytesReceived < 0) {
            std::cerr << "Error in receiving data." << std::endl;
            break;
        }
        if (bytesReceived == 0) {  // connection closed
            break;
        }
        result.append(buffer, bytesReceived);
        if (bytesReceived < 1024) {  // likely end of message
            break;
        }
    }
    return result;
}

