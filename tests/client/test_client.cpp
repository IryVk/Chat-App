#include "server/socket_server.h"
#include "client/socket_client.h"
#include <gtest/gtest.h>
#include <thread>

class IntegrationTest : public ::testing::Test {
protected:
    std::thread serverThread;
    SocketServer* server;
    SocketClient* client;

    static void StartServer(SocketServer* server) {
        server->start();
    }

    void SetUp() override {
        // Initialize server and start it in a separate thread
        server = new SocketServer(12346);  // Assume the server listens on port 12345
        serverThread = std::thread(StartServer, server);

        // Ensure the server has time to start up properly
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Initialize client
        client = new SocketClient("127.0.0.1", 12346);
        ASSERT_TRUE(client->connectToServer());  // Ensure client can connect
    }

    void TearDown() override {
        delete client;  // Cleanup client
        server->stop();  // Signal server to stop
        if (serverThread.joinable()) {
            serverThread.join();  // Wait for server thread to finish
        }
        delete server;  // Cleanup server
    }
};

TEST_F(IntegrationTest, TestSendMessage) {
    std::string testMessage = "Hello, Server!";
    ASSERT_TRUE(client->sendMessage(testMessage));
    std::string receivedMessage = client->receiveMessage();
    ASSERT_EQ(receivedMessage, "Echo: " + testMessage);
}

TEST_F(IntegrationTest, TestReceiveMessage) {
    std::string testMessage = "Hello, Server!";
    ASSERT_TRUE(client->sendMessage(testMessage));
    std::string receivedMessage = client->receiveMessage();
    ASSERT_EQ(receivedMessage, testMessage);  // assuming the server echoes messages
}
