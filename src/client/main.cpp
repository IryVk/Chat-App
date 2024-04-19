#include <client/socket_client.h>

int main() {
    SocketClient* client = new SocketClient("127.0.0.1", 12346);
    client->connectToServer();
    std::string testMessage = "Hello, Server!";
    client->sendMessage(testMessage);
    return 0;
}
