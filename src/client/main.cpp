#include <client/socket_client.h>

int main() {
    // ip and port of the server
    std::cout << "Enter the server IP address: ";
    std::string ip;
    std::cin >> ip;
    std::cout << "Enter the server port: ";
    int port;
    std::cin >> port;
    Client client(ip, port);
    if (!client.connectToServer()) {
        return 1; // connection failed
    }

    std::thread recvThread(&Client::receiveMessages, &client);

    std::cin.ignore(); // ignore the newline character

    std::string line;
    while (getline(std::cin, line)) {
        if (line == "exit") {
            break;
        }
        client.sendMessage(nlohmann::json{{"type", "text"},{"message", line}});
    }

    recvThread.join();
    client.disconnect();
    return 0;
}