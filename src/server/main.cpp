/**
 * @file server/main.cpp
 * @date 2024-04-22
 * @brief This file contains the main function for the server
 * 
 * This file contains the main function for the server, which creates a Server object and starts the server on the specified port.
*/

#include <server/socket_server.h>

int main() {
    // Get port from user
    std::cout << "Enter the port number: ";
    int port;
    std::cin >> port;
    // Start the server
    Server myServer(port);
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Press Ctrl+C to stop the server." << std::endl;
    myServer.run();
    return 0;
}