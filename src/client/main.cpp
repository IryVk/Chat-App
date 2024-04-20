#include <client/socket_client.h>
#include <common/aes_ecb.h>
#include <algorithm>
#include <cctype>
#include <locale>

using json = nlohmann::json;

int main() {
    // ip and port of the server
    std::cout << "Enter the server IP address: ";
    std::string ip;
    std::cin >> ip;
    std::cout << "Enter the server port: ";
    int port;
    std::cin >> port;
    Client client(ip, port);

    std::cin.ignore(); // ignore the newline character
    
    while (client.status){
        if (!client.connectToServer()) {
            return 1; // connection failed
        }

        std::thread recvThread(&Client::receiveMessages, &client);

        std::string line;
        while (getline(std::cin, line)) {
            if (!client.status) {
                // only !exit and recvMessage can set this to false
                client.status = true;
                std::cout << "Reconnecting to the server..." << std::endl;
                break;
            }
            if (line == "!exit") {
                client.status = false;
                break;
            } else if (line == "!disconnect") {
                client.disconnect();
                std::cout << "Disconnected from the chat, reconnecting..." << std::endl;
                break;
            }
                
            try {
                std::string encryptedMessage = AESECB::toHex(client.aes.Encrypt(line));
                client.sendMessage(json{{"type", "text"}, {"message", encryptedMessage}});
            } catch (const CryptoPP::InvalidKeyLength& e) {
            }
        }

        recvThread.join();
        client.disconnect();
    }

    return 0;
}