#include <server/socket_server.h>

using json = nlohmann::json;

// constructor
Server::Server(int port) : rsa(), isRunning(true) {
    // create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) { // check if the socket was created successfully
        std::cerr << "Failed to create socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    // server address
    sockaddr_in serverAddr{};
    // set the server address
    serverAddr.sin_family = AF_INET;
    // convert the port to network byte order
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {  // bind the socket to the server address
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

// destructor
Server::~Server() {
    // stop the server
    isRunning = false;
    // close all threads
    clientThreads.clear();
    close(serverSocket);
}

// run the server
void Server::run() {
    while (isRunning) {
        // accept clients
        int clientSocket1, clientSocket2;
        // wait for clients to connect
        waitForClients(clientSocket1);
        waitForClients(clientSocket2);
        // create a thread to handle the client pair and store it in the linked list
        std::thread newThread(&Server::handlePair, this, clientSocket1, clientSocket2);
        clientThreads.addThread(std::move(newThread));
    }
}

// get the current time in a formatted string
std::string Server::getFormattedCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_c);
    std::stringstream ss;
    ss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// wait for clients to connect
void Server::waitForClients(int& clientSocket) {
    sockaddr_in clientAddr{};
    // accept a client
    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error accepting client." << std::endl;
        return;
    }
    // successfully accepted a client
    std::cout << getFormattedCurrentTime() << ": Client connected from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
    // share the public key with the client
    std::string publicKey = RSAWrapper::sendPublicKey(rsa.getPublicKey());
    notifyClient(clientSocket, publicKey);
    // wait for the client to send their public key
    char buffer[1024] = {0};
    ssize_t len = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (len == 0) {
        std::cerr << "Error reading client's public key." << std::endl;
    }
    // successfully read the client's public key
    std::string pub(buffer, len);
    RSAWrapper::receivePublicKey(pub, rsa.publicKeyB);

    // send a welcome message to the client
    notifyClient(clientSocket, json{{"type", "success"},{"message", "Welcome!"}}.dump());
}

// handle client pair (aka a chat)
void Server::handlePair(int clientSocket1, int clientSocket2) {
    notifyClient(clientSocket1, json{{"type", "connected"},{"message", "You are now chatting with an anonymous stranger"}}.dump());
    // throttle to fix problem
    sleep(0.5);
    // create a thread to handle the chat
    fd_set readfds;
    // set of socket descriptors
    int max_sd;
    bool disconnected = false;

    while (isRunning) {
        if (disconnected) { // check if a client disconnected
            return;
        }
        // clear the socket set
        FD_ZERO(&readfds);

        // add client sockets to set
        FD_SET(clientSocket1, &readfds);
        FD_SET(clientSocket2, &readfds);
        // find the maximum socket descriptor
        max_sd = (clientSocket1 > clientSocket2) ? clientSocket1 : clientSocket2;

        // wait for activity on any of the sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // check for errors
        if ((activity < 0) && (errno != EINTR)) { // disconnect if there is an error
            disconnected = true;
        }

        processClientMessage(clientSocket1, clientSocket2, readfds);
        processClientMessage(clientSocket2, clientSocket1, readfds);
    }
}

// process client message
void Server::processClientMessage(int sourceSock, int targetSock, fd_set &readfds) {
    // check if the source socket is set
    if (FD_ISSET(sourceSock, &readfds)) {
        char buffer[1024] = {0};
        ssize_t bytesRead = read(sourceSock, buffer, sizeof(buffer) - 1);
        if (bytesRead == 0) { // check if the client disconnected
            // throttle to fix problem
            sleep(0.5);
            notifyClient(targetSock, json{{"type", "error"},{"status", "error"}, {"message", "The other user disconnected."}}.dump());
            close(sourceSock);
            close(targetSock);
            return;
        } else if (bytesRead > 0) { // check if the message is valid
            try {
                json msg = json::parse(std::string(buffer, bytesRead));
                send(targetSock, msg.dump().c_str(), msg.dump().size(), 0);
            } catch (const json::parse_error& e) {
                notifyClient(sourceSock, json{{"type", "error"},{"status", "error"}, {"message", "Invalid JSON format."}}.dump());
            }
        }
    }
}

// notify client (json)
void Server::notifyClient(int clientSocket, const std::string &jsonMessage) {
    send(clientSocket, jsonMessage.c_str(), jsonMessage.size(), 0);
}
