#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <nlohmann/json.hpp>
#include <common/aes_ecb.h>
#include <common/thread_list.h>

class Server {
public:
    Server(int port); // constructor
    ~Server(); // destructor
    void run(); // start the server
    
private:
    int serverSocket; // server socket
public:
    std::atomic<bool> isRunning; // flag to indicate if the server is running (atomic for thread safety)
private:
    ThreadList clientThreads;  // stores threads for client pairs

    void acceptClients(); // accept clients
    void handlePair(int clientSocket1, int clientSocket2); // handle client pair
    void waitForClients(int& clientSocket); // wait for clients to connect
    void notifyClient(int clientSocket, const std::string &message); // notify clients (send json)
    void processClientMessage(int sourceSock, int targetSock, fd_set &readfds); // process client message (make sure message is json)
};

#endif // SOCKET_SERVER_H
