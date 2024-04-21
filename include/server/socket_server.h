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
#include <common/rsa_wrapper.h>
#include <cryptopp/base64.h>
#include <sstream>

class Server {
public:
    Server(int port); // constructor
    ~Server(); // destructor
    void run(); // start the server

    static std::string getFormattedCurrentTime(); // get the current time in a formatted string
    
private:
    int serverSocket; // server socket
    RSAWrapper rsa; // RSA wrapper
public:
    std::atomic<bool> isRunning; // flag to indicate if the server is running (atomic for thread safety)
private:
    ThreadList clientThreads;  // stores threads for client pairs

    void handlePair(int clientSocket1, int clientSocket2); // handle client pair
    bool waitForClients(int& clientSocket); // wait for clients to connect
    void notifyClient(int clientSocket, const std::string &message); // notify clients (send json)
    void processClientMessage(int sourceSock, int targetSock, fd_set &readfds); // process client message (make sure message is json)
    bool createUser(std::string& user); // create a user
    bool verifyUser(std::string& user); // verify a user
    bool verifyClient(int clientSocket); // verify a client
};

#endif // SOCKET_SERVER_H
