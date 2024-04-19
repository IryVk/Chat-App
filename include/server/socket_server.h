#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "nlohmann/json.hpp" 

class Server {
public:
    Server(int port);
    ~Server();
    void run();

private:
    int serverSocket;
    std::atomic<bool> isRunning;
    std::vector<std::thread> clientThreads;  // stores threads for client pairs

    void acceptClients();
    void handlePair(int clientSocket1, int clientSocket2);
    void waitForClients(int& clientSocket);
    void notifyClient(int clientSocket, const std::string &message);
    void processClientMessage(int sourceSock, int targetSock, fd_set &readfds);
};

#endif // SOCKET_SERVER_H
