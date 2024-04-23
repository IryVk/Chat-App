/**
 * @file server/socket_server.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the implementation of the Server class
 * 
 * This file contains the implementation of the Server class, which is used to create a socket server that listens for incoming connections and handles chat sessions between clients.
*/

#include <server/socket_server.h>
#include <server/userhandler.h>

using json = nlohmann::json;

/**
 * @brief Construct a new Server object
 * 
 * Initialize the server object with the specified port and RSA keys.
 * 
 * @param port The port number to listen on
 * 
 * @return Server object
*/
Server::Server(int port) : rsa(), isRunning(true) {
    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) { // Check if the socket was created successfully
        std::cerr << "Failed to create socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Server address
    sockaddr_in serverAddr{};
    // Set the server address
    serverAddr.sin_family = AF_INET;
    // Convert the port to network byte order
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {  // Bind the socket to the server address
        std::cerr << "Binding failed." << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    if (listen(serverSocket, 10) < 0) {  // Listen for more connections
        std::cerr << "Listening failed." << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Destroy the Server object
 * 
 * Destroy the Server object and free any allocated resources.
 * 
 * @return void
*/
Server::~Server() {
    // Stop the server
    isRunning = false;
    // Close all threads
    clientThreads.clear();
    close(serverSocket);
}

/**
 * @brief Run the server
 * 
 * Start the server and listen for incoming connections.
 * 
 * @return void
*/
void Server::run() {
    while (isRunning) {
        // Accept clients
        int clientSocket1, clientSocket2;
        // Wait for clients to connect
        while (!waitForClients(clientSocket1));
        while (!waitForClients(clientSocket2));
        // Create a thread to handle the client pair and store it in the linked list
        std::thread newThread(&Server::handlePair, this, clientSocket1, clientSocket2);
        clientThreads.addThread(std::move(newThread));
    }
}

/**
 * @brief Get the current time in a formatted string
 * 
 * Get the current time in a formatted string (YYYY-MM-DD HH:MM:SS).
 * 
 * @return std::string The formatted current time
*/
std::string Server::getFormattedCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_c);
    std::stringstream ss;
    ss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

/**
 * @brief Wait for clients to connect
 * 
 * Wait for clients to connect to the server and verify their identity.
 * 
 * @param clientSocket The socket to store the client connection
 * 
 * @return bool True if the client was successfully accepted and verified, false otherwise
*/
bool Server::waitForClients(int& clientSocket) {
    sockaddr_in clientAddr{};
    // Accept a client
    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error accepting client." << std::endl;
        return false;
    }
    // Successfully accepted a client
    std::cout << getFormattedCurrentTime() << ": Client connected from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
    // Verify the client
    if (!verifyClient(clientSocket)) {
        return false;
    }
    // Send a welcome message to the client
    notifyClient(clientSocket, json{{"type", "success"},{"message", "Welcome!"}}.dump());
    return true;
}

/**
 * @brief Verify the client's identity
 * 
 * Verify the client's identity by exchanging RSA public keys and prompting the client to send their username and password.
 * 
 * @param clientSocket The client socket to verify
 * 
 * @return bool True if the client was successfully verified, false otherwise
*/
bool Server::verifyClient(int clientSocket){
    // Share the public key with the client
    std::string publicKey = RSAWrapper::sendPublicKey(rsa.getPublicKey());
    notifyClient(clientSocket, publicKey);
    // Wait for the client to send their public key
    char buffer[1024] = {0};
    ssize_t len = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (len == 0) {
        std::cerr << "Error reading client's public key." << std::endl;
    }
    // Successfully read the client's public key
    std::string pub(buffer, len);
    RSAWrapper::receivePublicKey(pub, rsa.publicKeyB);
    // Prompt the client to send their username and password
    notifyClient(clientSocket, json{{"type", "prompt"}}.dump());
    // Read user response
    char buffer2[1024] = {0};
    int len2 = read(clientSocket, buffer2, sizeof(buffer2) - 1);
    if (len2 == 0) {
        std::cerr << "Error reading client's credentials." << std::endl;
        return false;
    }

    // Attempt to create or verify the user
    std::string user(buffer2, len2);
    auto j = json::parse(user);
    if (j["type"] == "create") {
        if (createUser(user)) {
            notifyClient(clientSocket, json{{"type", "success"},{"message", "User created successfully."}}.dump());
        } else {
            notifyClient(clientSocket, json{{"type", "error"},{"status", "error"}, {"message", "User already exists."}}.dump());
            close(clientSocket);
            return false;
        }
    } else if (j["type"] == "verify") {
        if (verifyUser(user)) {
            notifyClient(clientSocket, json{{"type", "success"},{"message", "User verified successfully."}}.dump());
        } else {
            notifyClient(clientSocket, json{{"type", "error"},{"status", "error"}, {"message", "Invalid credentials."}}.dump());
            close(clientSocket);
            return false;
        }
    } else {
        notifyClient(clientSocket, json{{"type", "error"},{"status", "error"}, {"message", "Invalid request."}}.dump());
        close(clientSocket);
        return false;
    }
    return true;
}


/**
 * @brief Handle a pair of clients
 * 
 * Handle a pair of clients by creating a chat session between them.
 * 
 * @param clientSocket1 The first client socket
 * @param clientSocket2 The second client socket
 * 
 * @return void
*/
void Server::handlePair(int clientSocket1, int clientSocket2) {
    notifyClient(clientSocket1, json{{"type", "connected"},{"message", "You are now chatting!"}}.dump());
    notifyClient(clientSocket2, json{{"type", "info"},{"message", "You are now chatting!"}}.dump());
    // Create a thread to handle the chat
    fd_set readfds;
    // Set of socket descriptors
    int max_sd;
    bool disconnected = false;

    while (isRunning) {
        if (disconnected) { // Check if a client disconnected
            return;
        }
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add client sockets to set
        FD_SET(clientSocket1, &readfds);
        FD_SET(clientSocket2, &readfds);
        // Find the maximum socket descriptor
        max_sd = (clientSocket1 > clientSocket2) ? clientSocket1 : clientSocket2;

        // Wait for activity on any of the sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // Check for errors
        if ((activity < 0) && (errno != EINTR)) { // Disconnect if there is an error
            disconnected = true;
        }

        processClientMessage(clientSocket1, clientSocket2, readfds);
        processClientMessage(clientSocket2, clientSocket1, readfds);
    }
}

/**
 * @brief Process a message from a client
 * 
 * Process a message from a client by reading the message and sending it to the target client.
 * 
 * @param sourceSock The source client socket
 * @param targetSock The target client socket
 * @param readfds The set of socket descriptors
 * 
 * @return void
*/
void Server::processClientMessage(int sourceSock, int targetSock, fd_set &readfds) {
    // Check if the source socket is set
    if (FD_ISSET(sourceSock, &readfds)) {
        char buffer[1024] = {0};
        ssize_t bytesRead = read(sourceSock, buffer, sizeof(buffer) - 1);
        if (bytesRead == 0) { // Check if the client disconnected
            notifyClient(targetSock, json{{"type", "error"},{"status", "error"}, {"message", "The other user disconnected."}}.dump());
            close(sourceSock);
            close(targetSock);
            return;
        } else if (bytesRead > 0) { // Check if the message is valid
            try {
                json msg = json::parse(std::string(buffer, bytesRead));
                notifyClient(targetSock, msg.dump());
            } catch (const json::parse_error& e) {
                notifyClient(sourceSock, json{{"type", "error"},{"status", "error"}, {"message", "Invalid JSON format."}}.dump());
            }
        }
    }
}

/**
 * @brief Notify a client
 * 
 * Notify a client of any information by sending a JSON message.
 * 
 * @param clientSocket The client socket to notify
 * @param jsonMessage The JSON message to send
 * 
 * @return void
*/
void Server::notifyClient(int clientSocket, const std::string &jsonMessage) {
    std::string msg = jsonMessage + "\n";
    send(clientSocket, msg.c_str(), msg.size(), 0);
}

/**
 * @brief Create a new user
 * 
 * Create a new user by reading the username and password from a JSON message and adding the user to the users file.
 * 
 * @param user The JSON message containing the username and password
 * 
 * @return bool True if the user was successfully created, false otherwise
*/
bool Server::createUser(std::string& user) {
    // Read user message
    auto j = json::parse(user);
    std::string username = j["username"];
    std::string password = j["password"];

    // Decrypt the username and password
    username = this->rsa.decrypt(username);
    password = this->rsa.decrypt(password);

    // Add the user to the users file using the user handler
    UserHandler userHandler("assets/users.txt");
    if (userHandler.AddUser(username, password)) {
        return true;
    }
    return false;
}

/**
 * @brief Verify a user
 * 
 * Verify a user by reading the username and password from a JSON message and checking if the user exists in the users file.
 * 
 * @param user The JSON message containing the username and password
 * 
 * @return bool True if the user was successfully verified, false otherwise
*/
bool Server::verifyUser(std::string& user) {
    auto j = json::parse(user);
    std::string username = j["username"];
    std::string password = j["password"];
    
    // Verify the user using the user handler
    UserHandler userHandler("assets/users.txt");
    if (userHandler.VerifyUser(username, password)) {
        return true;
    } else {
        return false;
    }
}
