#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include "client/socket_client.h"
#include "common/aes_ecb.h"

using json = nlohmann::json;

int main() {
    // ip and port of the server

    std::cout << "Enter the server IP address: ";
    std::string ip;
    std::cin >> ip;
    std::cout << "Enter the server port: ";
    int port;
    std::cin >> port;
    Client client(ip, port);

    std::cin.ignore(); // ignore the newline character

    // Initialize ncurses
    initscr(); // Start curses mode
    // Start the color functionality
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);     // Red
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Green
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Yellow
    init_pair(4, COLOR_BLUE, COLOR_BLACK);    // Blue
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Magenta
    init_pair(6, COLOR_CYAN, COLOR_BLACK);    // Cyan
    init_pair(7, COLOR_WHITE, COLOR_BLACK);   // White
    cbreak(); // Line buffering disabled
    echo(); // Echo input

    // Get the size of the window
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Create a window for input and another for output
    WINDOW* inputWin = newwin(1, maxX, maxY - 1, 0); // Last line for input
    WINDOW* outputWin = newwin(maxY - 1, maxX, 0, 0); // Rest for output

    scrollok(outputWin, TRUE); // Allow the output window to scroll

    // Initial user instructions
    mvwprintw(inputWin, 0, 0, "You: ");
    wrefresh(inputWin);
    
    while (client.status){
        if (!client.connectToServer()) {
            return 1; // connection failed
        }

         // Start the receive thread
    std::thread recvThread([&]() {
        char buffer[1024];

        // setting a timeout period for recv
        struct timeval tv;
        tv.tv_sec = 1;  // timeout after 1 second
        tv.tv_usec = 0;
        setsockopt(client.sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

        while (client.status) {
            ssize_t len = recv(client.sock, buffer, sizeof(buffer), 0);
            if (len > 0) {
                std::string msg(buffer, len);
                try {
                    auto j = json::parse(msg);
                    client.handleJsonMessage(j.dump(), outputWin);
                } catch (const json::parse_error& e) {
                    std::cerr << "JSON parsing error at byte " << e.byte << " with message: " << msg << '\n';
                    std::cerr << "Exception message: " << e.what() << '\n';
                    continue;
                }
            } else if (len == 0) {
                wprintw(outputWin, "Server closed connection.\n");
                wrefresh(outputWin);
                client.status = false;
            } else if (len == -1) {
                // check if it was just a timeout or a real error
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    continue;
                }
                std::cerr << "Failed to receive data: " << strerror(errno) << std::endl;
                break;
            }
        }
    });

        // Main loop for user input
        char str[1024];
        while (true) {
            if (!client.status) {
                // only !exit and recvMessage can set this to false
                client.status = true;
                wprintw(outputWin, "Disconnected from the chat, reconnecting...\n");
                wrefresh(outputWin);
                break;
            }
            wgetnstr(inputWin, str, sizeof(str) - 2);
            if (strcmp(str, "!exit") == 0) {
                client.status = false;
                break;
            } else if (strcmp(str, "!disconnect") == 0) {
                client.disconnect();
                wprintw(outputWin, "Disconnected from the chat, reconnecting...\n");
                wrefresh(outputWin);
                break;
            }
                
            try {
                std::string encryptedMessage = AESECB::toHex(client.aes.Encrypt(str));
                client.sendMessage(json{{"type", "text"}, {"message", encryptedMessage}});

            } catch (const CryptoPP::InvalidKeyLength& e) {
                std::cerr << "Invalid key length: " << e.what() << std::endl;
            }
            wmove(inputWin, 0, 5); // Move cursor back to after "You: "
            wclrtoeol(inputWin); // Clear the line after "You: "
            wrefresh(inputWin);
            wprintw(outputWin, "You: %s\n", str);
            wrefresh(outputWin);
        }

        recvThread.join();
        client.disconnect();
        endwin(); // End curses mode
    }

    return 0;
}