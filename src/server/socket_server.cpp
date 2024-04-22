#include <server/socket_server.h>
#include <server/userhandler.h>

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
        while (!waitForClients(clientSocket1));
        while (!waitForClients(clientSocket2));
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
bool Server::waitForClients(int& clientSocket) {
    sockaddr_in clientAddr{};
    // accept a client
    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error accepting client." << std::endl;
        return false;
    }
    // successfully accepted a client
    std::cout << getFormattedCurrentTime() << ": Client connected from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
    // verify the client
    if (!verifyClient(clientSocket)) {
        return false;
    }
    // send a welcome message to the client
    notifyClient(clientSocket, json{{"type", "success"},{"message", "Welcome!"}}.dump());
    return true;
}

// verify the client
bool Server::verifyClient(int clientSocket){
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
    // prompt the client to send their username and password
    notifyClient(clientSocket, json{{"type", "prompt"}}.dump());
    // read user response
    char buffer2[1024] = {0};
    int len2 = read(clientSocket, buffer2, sizeof(buffer2) - 1);
    if (len2 == 0) {
        std::cerr << "Error reading client's credentials." << std::endl;
        return false;
    }
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


// handle client pair (aka a chat)
void Server::handlePair(int clientSocket1, int clientSocket2) {
    notifyClient(clientSocket1, json{{"type", "connected"},{"message", "You are now chatting with an anonymous stranger"}}.dump());
    notifyClient(clientSocket2, json{{"type", "info"},{"message", "You are now chatting with an anonymous stranger"}}.dump());
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
            notifyClient(targetSock, json{{"type", "error"},{"status", "error"}, {"message", "The other user disconnected."}}.dump());
            close(sourceSock);
            close(targetSock);
            return;
        } else if (bytesRead > 0) { // check if the message is valid
            try {
                json msg = json::parse(std::string(buffer, bytesRead));
                notifyClient(targetSock, msg.dump());
            } catch (const json::parse_error& e) {
                notifyClient(sourceSock, json{{"type", "error"},{"status", "error"}, {"message", "Invalid JSON format."}}.dump());
            }
        }
    }
}

// notify client (json)
void Server::notifyClient(int clientSocket, const std::string &jsonMessage) {
    std::string msg = jsonMessage + "\n";
    send(clientSocket, msg.c_str(), msg.size(), 0);
}

// creates a new user
bool Server::createUser(std::string& user) {
    // read user message
    auto j = json::parse(user);
    std::string username = j["username"];
    std::string password = j["password"];

    // decrypt the username and password
    username = this->rsa.decrypt(username);
    password = this->rsa.decrypt(password);

    // add the user to the users file using the user handler
    UserHandler userHandler("assets/users.txt");
    if (userHandler.AddUser(username, password)) {
        return true;
    }
    return false;
}

// verifies an existing user
bool Server::verifyUser(std::string& user) {

    auto j = json::parse(user);
    std::string username = j["username"];
    std::string password = j["password"];
    
    // verify the user using the user handler
    UserHandler userHandler("assets/users.txt");
    if (userHandler.VerifyUser(username, password)) {
        return true;
    } else {
        return false;
    }
}
