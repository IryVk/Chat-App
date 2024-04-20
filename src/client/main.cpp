#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include "client/socket_client.h"
#include "common/aes_ecb.h"

using json = nlohmann::json;

int main() {
    // get ip and port of the server
    std::cout << "Enter the server IP address: ";
    std::string ip;
    std::cin >> ip;
    std::cout << "Enter the server port: ";
    int port;
    std::cin >> port;
    // start the client
    Client client(ip, port);

    std::cin.ignore(); // ignore the newline character

    // initialize ncurses (the shell for the chat program)
    initscr(); // start curses mode

    // start the color functionality
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);     
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  
    init_pair(4, COLOR_BLUE, COLOR_BLACK);    
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK); 
    init_pair(6, COLOR_CYAN, COLOR_BLACK);    
    init_pair(7, COLOR_WHITE, COLOR_BLACK);

    cbreak(); // line buffering disabled
    echo(); // echo input

    // get the size of the window
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // create a window for input and another for output
    WINDOW* inputWin = newwin(1, maxX, maxY - 1, 0); // last line for input
    WINDOW* outputWin = newwin(maxY - 1, maxX, 0, 0); // rest for output

    scrollok(outputWin, TRUE); // allow the output window to scroll

    // initial user instructions
    mvwprintw(inputWin, 0, 0, "You: ");
    wrefresh(inputWin);
    
    // main loop, keep trying to connect to the server if the client is still active
    while (client.status){
        if (!client.connectToServer()) {
            return 1; // connection failed
        }

         // start the receive thread 
         //(lambda function to receive messages from the server and send the window object to other functions)
        std::thread recvThread([&]() {
            char buffer[1024];

            // setting a timeout period for recv (to avoid blocking indefinitely)
            struct timeval tv;
            tv.tv_sec = 1;  // timeout after 1 second
            tv.tv_usec = 0;
            setsockopt(client.sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

            // keep receiving messages from the server while the client is active
            while (client.status) {
                // receive the message into the buffer
                // ssize_t is a signed integer type used to represent the sizes of objects
                ssize_t len = recv(client.sock, buffer, sizeof(buffer), 0);
                // if the message is not empty, parse it as json
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
                } else if (len == 0) { // if the message is empty, the server has closed the connection
                    wprintw(outputWin, "Server closed connection.\n");
                    wrefresh(outputWin);
                    client.status = false;
                } else if (len == -1) { // if the message is -1, there was an error or timeout
                    // check if it was just a timeout or a real error
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
                // only !exit and recvMessage can set this to false
                // !exit will exit the program, if recvMessage closed it that means the server closed the connection
                // so we should break out of the loop and try to reconnect
                client.status = true;
                // use wprintw instead of std::cout to print to the output window
                wprintw(outputWin, "Disconnected from the chat, reconnecting...\n");
                // refresh the output window to show the message
                wrefresh(outputWin);
                break;
            }
            // use wgetnstr instead of std::cin to get input from the user
            wgetnstr(inputWin, str, sizeof(str) - 2);
            // exit the program if the user types !exit
            if (strcmp(str, "!exit") == 0) {
                client.status = false;
                break;
            } else if (strcmp(str, "!disconnect") == 0) { // reset connection if user types !disconnect
                client.disconnect();
                wprintw(outputWin, "Disconnected from the chat, reconnecting...\n");
                wrefresh(outputWin);
                break;
            }
                
            try {
                std::string encryptedMessage = AESECB::toHex(client.aes.Encrypt(str));
                client.sendMessage(json{{"type", "text"}, {"message", encryptedMessage}});

            } catch (const CryptoPP::InvalidKeyLength& e) {
                // std::cerr << "Invalid key length: " << e.what() << std::endl;
            }
            wmove(inputWin, 0, 5); // move cursor back to after "You: "
            wclrtoeol(inputWin); // clear the line after "You: "
            wrefresh(inputWin);
            wprintw(outputWin, "You: %s\n", str);
            wrefresh(outputWin);
        }

        // wait for the receive thread to finish
        recvThread.join();
        // close the socket
        client.disconnect();
        endwin(); // end curses mode
    }

    return 0;
}