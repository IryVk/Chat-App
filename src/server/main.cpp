#include <server/socket_server.h>


int main() {
    Server myServer(12345);
    myServer.run();
    return 0;
}