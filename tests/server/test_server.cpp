#include <server/socket_server.h>
#include <gtest/gtest.h>
#include <server/userhandler.h>
#include <common/passhash.h>
#include <fstream>

// test fixture for socket server
class SocketServerTest : public ::testing::Test {
protected:
    Server *server;

    SocketServerTest() {
        // initialize with a test port
        server = new Server(4444);
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

// ==================== User Handler Tests ====================

class UserHandlerTest : public ::testing::Test {
protected:
    std::string testFilename = "test_users.txt";
    UserHandler* userHandler;

    void SetUp() override {
        // ensure the test file is clean before each test
        std::ofstream outFile(testFilename, std::ios::trunc);
        userHandler = new UserHandler(testFilename);
    }

    void TearDown() override {
        delete userHandler;
        std::remove(testFilename.c_str());
    }

    // helper function to populate the file with initial data
    void AddUserDirectly(const std::string& username, const std::string& password) {
        std::ofstream file(testFilename, std::ios::app);
        std::string salt = PasswordHasher::GenerateRandomSalt(16);
        std::string saltedHash = PasswordHasher::HashPassword(password, salt);
        file << username << " " << saltedHash << std::endl;
    }
};

TEST_F(UserHandlerTest, AddUser_NewUser_ReturnsTrue) {
    ASSERT_TRUE(userHandler->AddUser("newuser", "password123"));
}

TEST_F(UserHandlerTest, AddUser_ExistingUser_ReturnsFalse) {
    AddUserDirectly("existinguser", "password123");
    ASSERT_FALSE(userHandler->AddUser("existinguser", "newpassword"));
}

TEST_F(UserHandlerTest, VerifyUser_CorrectCredentials_ReturnsTrue) {
    AddUserDirectly("user", "correctpassword");
    ASSERT_TRUE(userHandler->VerifyUser("user", "correctpassword"));
}

TEST_F(UserHandlerTest, VerifyUser_IncorrectCredentials_ReturnsFalse) {
    AddUserDirectly("user", "correctpassword");
    ASSERT_FALSE(userHandler->VerifyUser("user", "wrongpassword"));
}

TEST_F(UserHandlerTest, VerifyUser_NonexistentUser_ReturnsFalse) {
    ASSERT_FALSE(userHandler->VerifyUser("nonexistent", "password"));
}
