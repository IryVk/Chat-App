#include "server/socket_server.h" 
#include <gtest/gtest.h>


// test fixture for socket server
class SocketServerTest : public ::testing::Test {
protected:
    SocketServer *server;

    SocketServerTest() {
        // initialize with a test port
        server = new SocketServer(12345);
    }

    ~SocketServerTest() override {
        delete server;
    }
};


// Test Case: constructor should properly initialize
TEST_F(SocketServerTest, ConstructorInitializesProperly) {
    ASSERT_NE(server, nullptr);  // Server object should not be nullptr
}

// Test Case: server Starts and Stops Properly
TEST_F(SocketServerTest, ServerStartsAndStops) {
    ASSERT_FALSE(server->isRunning);  // should initially not be running
    ASSERT_TRUE(server->start());       // start should succeed
    ASSERT_TRUE(server->isRunning);   // should now be running
    server->stop();                     // stop the server
    ASSERT_FALSE(server->isRunning);  // should no longer be running
}
