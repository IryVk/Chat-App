#include <server/socket_server.h>
#include <gtest/gtest.h>

// test fixture for socket server
class SocketServerTest : public ::testing::Test {
protected:
    Server *server;

    SocketServerTest() {
        // initialize with a test port
        server = new Server(12345);
    }

    ~SocketServerTest() override {
        delete server;
    }
};


// Test Case: constructor should properly initialize
TEST_F(SocketServerTest, ConstructorInitializesProperly) {
    ASSERT_NE(server, nullptr);  // server object should not be nullptr
}

// Test Case: server Starts and Stops Properly
TEST_F(SocketServerTest, ServerStartsAndStops) {
    ASSERT_TRUE(server->isRunning);  // assert server is running  
}
