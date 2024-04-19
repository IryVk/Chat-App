#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <string>

class SocketClient {
public:
    SocketClient(const std::string &serverIP, int serverPort);
    ~SocketClient();

    bool connectToServer();
    bool sendMessage(const std::string &message);
    std::string receiveMessage();

private:
    int sockfd;  // socket file descriptor
    std::string serverIP;
    int serverPort;
};

#endif // SOCKET_CLIENT_H
