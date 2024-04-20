#include <server/socket_server.h>


int main() {
    // get port from user
    std::cout << "Enter the port number: ";
    int port;
    std::cin >> port;
    // start the server
    Server myServer(port);
    myServer.run();
    return 0;
}