#include <server/socket_server.h>


int main() {
    // get port from user
    std::cout << "Enter the port number: ";
    int port;
    std::cin >> port;
    // start the server
    Server myServer(port);
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Press Ctrl+C to stop the server." << std::endl;
    myServer.run();
    return 0;
}