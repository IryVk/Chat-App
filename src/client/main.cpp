/**
 * @file client/main.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the main function for the client side of the chat application
 * 
 * This file contains the main function for the client side of the chat application. It prompts the user for the server IP address and port, as well as the username and password.
 * The client then connects to the server and starts the chat interface using ncurses. The user can send messages to the server and receive messages from other clients.
 * 
 * The client uses the Client class to manage the connection to the server, send and receive messages, and handle the key exchange process.
 * The Client class uses the AESECB class to perform AES encryption and decryption, and the nlohmann json library to handle JSON messages.
 * 
 * @return 0 on success, 1 on failure
*/

#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include <client/socket_client.h>
#include <common/aes_ecb.h>

using json = nlohmann::json;

void clearCin();
int getInt(std::string& prompt);

int main() {
    // Get ip and port of the server
    std::cout << "Enter the server IP address: ";
    std::string ip;
    std::cin >> ip;
    std::cout << "Enter the server port: ";
    int port;
    std::cin >> port;
    // Ask the user if they want to log in
    std::string username, password;
    int type;
    while (true) {
        std::string prompt = "Do you want to log in or create a new account? (1: Log in, 2: Create account): ";
        int choice = getInt(prompt);
        if (choice == 1) {
            // Log in
            std::cout << "Enter your username: ";
            std::cin >> username;
            std::cout << "Enter your password: ";
            std::cin >> password;
            type = 1;
            break;
        } else if (choice == 2) {
            // Create account
            std::cout << "Enter your username: ";
            std::cin >> username;
            std::cout << "Enter your password: ";
            std::cin >> password;
            type = 2;
            break;
        } else {
            std::cout << "Invalid choice.\n";
        }
    }
    // Start the client
    Client client(ip, port, username, password, type);

    std::cin.ignore(); // Ignore the newline character

    // Initialize ncurses (the shell for the chat program)
    initscr(); // Start curses mode

    // Start the color functionality
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);     
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  
    init_pair(4, COLOR_BLUE, COLOR_BLACK);    
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK); 
    init_pair(6, COLOR_CYAN, COLOR_BLACK);    
    init_pair(7, COLOR_WHITE, COLOR_BLACK);

    cbreak(); // Line buffering disabled
    echo(); // Echo input

    // Get the size of the window
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create a window for input and another for output
    WINDOW* inputWin = newwin(1, maxX, maxY - 1, 0); // Last line for input
    WINDOW* outputWin = newwin(maxY - 1, maxX, 0, 0); // Rest for output

    scrollok(outputWin, TRUE); // Allow the output window to scroll

    // User text input window
    mvwprintw(inputWin, 0, 0, "You: ");
    wrefresh(inputWin);
    
    // Main loop, keep trying to connect to the server if the client is still active
    while (client.status){
        if (!client.connectToServer()) {
            return 1; // Connection failed
        }

         // Start the receive thread 
         //(lambda function to receive messages from the server and send the window object to other functions)
        std::thread recvThread([&]() {
            std::string buffer; // persistent buffer to store incomplete data
            char tempBuffer[1024];

            // Setting a timeout period for recv (to avoid blocking indefinitely)
            struct timeval tv;
            tv.tv_sec = 1;  // Timeout after 1 second
            tv.tv_usec = 0;
            setsockopt(client.sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

            // Keep receiving messages from the server while the client is active
            while (client.status) {
                // Receive the message into the buffer
                ssize_t len = recv(client.sock, tempBuffer, sizeof(buffer), 0); // ssize_t is a signed integer type used to represent the sizes of objects
                // If the message is not empty, parse it as json
                if (len > 0) {
                    buffer.append(tempBuffer, len); // Append new data to the buffer
                    size_t pos;
                    // Added this loop to handle multiple messages in the buffer
                    while ((pos = buffer.find('\n')) != std::string::npos) {
                        std::string msg = buffer.substr(0, pos);
                        buffer.erase(0, pos + 1); // Remove the processed message from buffer
                        try {
                            auto j = json::parse(msg);
                            client.handleJsonMessage(j.dump(), outputWin);
                        } catch (const json::parse_error& e) {
                            std::cerr << "JSON parsing error: " << e.what() << std::endl;
                        }
                    }
                } else if (len == 0) { // If the message is empty, the server has closed the connection
                    wprintw(outputWin, "Server closed connection.\n");
                    wrefresh(outputWin);
                    client.status = false;
                    wprintw(outputWin, "Press ENTER to reconnect...\n");
                    wrefresh(outputWin);
                } else if (len == -1) { // If the message is -1, there was an error or timeout
                    // Check if it was just a timeout or a real error
                    if (errno == EWOULDBLOCK || errno == EAGAIN) {
                        continue;
                    }
                    std::cerr << "Failed to receive data: " << strerror(errno) << std::endl;
                    break;
                }
            }
        });

        // main loop for user input
        char str[1024];
        while (true) {
            if (!client.status) {
                // Only !exit and recvMessage can set this to false
                // !exit will exit the program, if recvMessage closed it that means the server closed the connection
                // So we should break out of the loop and try to reconnect
                client.status = true;
                // Use wprintw instead of std::cout to print to the output window
                wprintw(outputWin, "Disconnected from the chat, reconnecting...\n");
                // Refresh the output window to show the message
                wrefresh(outputWin);
                break;
            }
            // Use wgetnstr instead of std::cin to get input from the user
            wgetnstr(inputWin, str, sizeof(str) - 2);
            // Exit the program if the user types !exit
            if (strcmp(str, "!exit") == 0) {
                client.status = false;
                break;
            } else if (strcmp(str, "!disconnect") == 0) { // Reset connection if user types !disconnect
                client.disconnect();
                wprintw(outputWin, "Disconnected from the chat, reconnecting...\n");
                wrefresh(outputWin);
                break;
            }
                
            try {
                // Attempt to encrypt the message using the AES key
                std::string encryptedMessage = AESECB::toHex(client.aes.Encrypt(str));
                std::string encryptedUsername = AESECB::toHex(client.aes.Encrypt(client.username));
                client.sendMessage(json{{"type", "text"}, {"message", encryptedMessage}, {"user", encryptedUsername}});
            } catch (const CryptoPP::InvalidKeyLength& e) {
                // Do nothing, the key is not set yet
            }
            wmove(inputWin, 0, 5); // Move cursor back to after "You: "
            wclrtoeol(inputWin); // Clear the line after "You: "
            wrefresh(inputWin);
            wprintw(outputWin, "You: %s\n", str);
            wrefresh(outputWin);
        }

        // Wait for the receive thread to finish
        recvThread.join();
        // Close the socket
        client.disconnect();
        endwin(); // End curses mode
    }

    return 0;
}

/**
 * @brief Utility function to clear the input buffer
 * 
 * This function clears the input buffer by clearing the error flag on cin and ignoring everything in the buffer up to and including the next newline.
 * This is useful when the user enters a non-numeric value and we need to clear the buffer before asking for input again.
 * 
 * @return void
*/
void clearCin() {
    // Clear the error flag on cin (like if user entered a non-numeric value)
    std::cin.clear();
    // Ignore everything in the buffer up to and including the next newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/**
 * @brief Utility function to get an integer from the user
 * 
 * This function prompts the user with a message and reads an integer from the input.
 * If the user enters a non-numeric value, it clears the input buffer and asks for input again.
 * 
 * @param prompt The message to prompt the user with
 * @return The integer entered by the user
*/
int getInt(std::string& prompt) {
    int choice;
    while (true) {
        std::cout << prompt;
        if (std::cin >> choice) {
            // If we successfully read a number, break out of the loop
            break;
        } else {
            std::cout << "Please enter a valid number.\n";
            clearCin(); // Clear the error state and the input buffer
        }
    }
    return choice;
}