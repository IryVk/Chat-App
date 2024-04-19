#include "server/socket_server.h"

using json = nlohmann::json;


Server::Server(int port) : isRunning(true) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Binding failed." << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) < 0) {  // listen for more connections
        std::cerr << "Listening failed." << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
}

Server::~Server() {
    isRunning = false;
    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    close(serverSocket);
}

void Server::run() {
    while (isRunning) {
        int clientSocket1, clientSocket2;
        waitForClients(clientSocket1);
        waitForClients(clientSocket2);
        clientThreads.emplace_back(&Server::handlePair, this, clientSocket1, clientSocket2);
    }
}

void Server::waitForClients(int& clientSocket) {
    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error accepting client." << std::endl;
        return;
    }
    // zuccessfully accepted a client
    std::cout << "Client connected from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

    // send a welcome message to the client
    notifyClient(clientSocket, json{{"type", "success"},{"message", "Welcome!"}}.dump());
}

void Server::handlePair(int clientSocket1, int clientSocket2) {
    notifyClient(clientSocket1, json{{"type", "info"},{"message", "You are now chatting with an anonymous stranger"}}.dump());
    notifyClient(clientSocket2, json{{"type", "info"},{"message", "You are now chatting with an anonymous stranger"}}.dump());
    fd_set readfds;
    int max_sd;
    bool disconnected = false;

    while (isRunning) {
        if (disconnected) {
            return;
        }
        FD_ZERO(&readfds);

        FD_SET(clientSocket1, &readfds);
        FD_SET(clientSocket2, &readfds);
        max_sd = (clientSocket1 > clientSocket2) ? clientSocket1 : clientSocket2;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            // std::cerr << "select error" << std::endl;
            disconnected = true;
        }

        processClientMessage(clientSocket1, clientSocket2, readfds);
        processClientMessage(clientSocket2, clientSocket1, readfds);
    }
}

void Server::processClientMessage(int sourceSock, int targetSock, fd_set &readfds) {
    if (FD_ISSET(sourceSock, &readfds)) {
        char buffer[1024] = {0};
        ssize_t bytesRead = read(sourceSock, buffer, sizeof(buffer) - 1);
        if (bytesRead == 0) {
            notifyClient(targetSock, json{{"type", "error"},{"status", "error"}, {"message", "The other user disconnected."}}.dump());
            close(sourceSock);
            close(targetSock);
            return;
        } else if (bytesRead > 0) {
            try {
                json msg = json::parse(std::string(buffer, bytesRead));
                send(targetSock, msg.dump().c_str(), msg.dump().size(), 0);
            } catch (const json::parse_error& e) {
                // std::cerr << "Failed to parse JSON message: " << e.what() << std::endl;
                notifyClient(sourceSock, json{{"type", "error"},{"status", "error"}, {"message", "Invalid JSON format."}}.dump());
            }
        }
    }
}

void Server::notifyClient(int clientSocket, const std::string &jsonMessage) {
    send(clientSocket, jsonMessage.c_str(), jsonMessage.size(), 0);
}