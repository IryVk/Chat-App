#include "client/socket_client.h"

using json = nlohmann::json;

Client::Client(const std::string& server_ip, int port) : sock(-1), server_ip(server_ip), port(port) {}

Client::~Client() {
    disconnect();
}

bool Client::connectToServer() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Could not create socket." << std::endl;
        return false;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        std::cerr << "Connect failed." << std::endl;
        return false;
    }

    std::cout << "Connected to the server." << std::endl;
    return true;
}

void Client::disconnect() {
    if (sock != -1) {
        close(sock);
        sock = -1;
    }
}

void Client::receiveMessages() {
    char buffer[1024];
    while (true) {
        ssize_t len = recv(sock, buffer, sizeof(buffer), 0);
        if (len > 0) {
            std::string msg(buffer, len);
            json j = json::parse(msg);
            handleJsonMessage(j.dump());
        } else if (len == 0) {
            std::cout << "Server closed connection." << std::endl;
            break;
        } else {
            std::cerr << "Failed to receive data." << std::endl;
            break;
        }
    }
}

void Client::sendMessage(const json& message) {
    std::string msg = message.dump();
    if (send(sock, msg.c_str(), msg.size(), 0) < 0) {
        std::cerr << "Send failed." << std::endl;
    }
}

// define color codes as constants
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

void Client::printColoredMessage(const std::string& message, const std::string& color) {
    std::cout << color << message << RESET << std::endl;
}

void Client::handleJsonMessage(const std::string& jsonStr) {
    auto j = json::parse(jsonStr);
    std::string type = j.value("type", "info"); // default to "info" if no type is specified
    std::string message = j.value("message", "");

    if (type == "error") {
        printColoredMessage(message, RED);
    } else if (type == "warning") {
        printColoredMessage(message, YELLOW);
    } else if (type == "info") {
        printColoredMessage(message, BLUE);
    } else if (type == "success") {
        printColoredMessage(message, GREEN);
    } else if (type == "text") {
        printColoredMessage("USER: " + message, MAGENTA); 
    } else {
        printColoredMessage(message, WHITE);
    }
}