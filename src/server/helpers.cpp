#include "server/helpers.h"
#include <server/socket_server.h>

// creates a new user
bool createUser(std::string& key, std::string& iv, std::string& user) {
    // read user input
    std::string username;
    std::string password;

    // verify username and password length
    std::cout << "\nCreating New User...\n";
    while (true) {
        std::string prompt = "Enter your username: ";
        std::cout << prompt;
        std::cin >> username;
        if (username.length() < 4) {
            std::cout << "Username must be at least 4 characters long.\n";
            continue;
        } else if (username.length() > 16) {
            std::cout << "Username must be at most 16 characters long.\n";
            continue;
        }
        break;
    }
    while (true) {
        std::string prompt = "Enter your password: ";
        std::cout << prompt;
        std::cin >> password;
        if (password.length() < 8) {
            std::cout << "Password must be at least 8 characters long.\n";
            continue;
        } else if (password.length() > 25) {
            std::cout << "Password must be at most 16 characters long.\n";
            continue;
        }
        break;
    }
    // add the user to the users file using the user handler
    UserHandler userHandler("assets/users.txt");
    if (userHandler.AddUser(username, password)) {
        return true;
    }
    std::cout << "Failed to create user.\n";
    return false;
}

// verifies an existing user
bool verifyUser(std::string& key, std::string& iv, std::string& user) {
    std::cout << "\nUser Verification Process...\n";
    // read user input
    std::string username;
    std::string password;
    std::string prompt = "Enter your username: ";
    std::cout << prompt;
    std::cin >> username;
    prompt = "Enter your password: ";
    std::cout << prompt;
    std::cin >> password;
    
    // verify the user using the user handler
    UserHandler userHandler("assets/users.txt");
    if (userHandler.VerifyUser(username, password)) {
        std::cout << "User verified successfully.\n";
        std::cout << "--------------------------------\n";
        return true;
    } else {
        std::cout << "Failed to verify user.\n";
        std::cout << "--------------------------------\n";
        return false;
    }
}