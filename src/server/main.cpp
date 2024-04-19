#include <server/socket_server.h>


int main() {
    Server myServer(12346);
    myServer.run();
    return 0;
}