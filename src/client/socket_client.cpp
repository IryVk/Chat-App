#include <client/socket_client.h>
#include <server/socket_server.h>

using json = nlohmann::json;

// constructor
Client::Client(std::string& server_ip, int port, std::string username, std::string password, int type) : sock(-1), server_ip(server_ip), port(port), aes(), priv_key(), rsa(), username(username), password(password), authType(type) {}

// destructor
Client::~Client() {
    disconnect();
}

// connect to the server
bool Client::connectToServer() {
    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) { // check if the socket was created successfully
        std::cerr << "Could not create socket." << std::endl;
        return false;
    }
    // server address
    sockaddr_in server;
    // set the server address
    server.sin_family = AF_INET;
    // convert the server IP address to binary
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());
    // convert the port to network byte order
    server.sin_port = htons(port);

    // connect to the server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) { // check if the connection was successful
        std::cerr << "Connect failed." << std::endl;
        return false;
    }

    return true;
}

// disconnect from the server
void Client::disconnect() {
    // close the socket
    if (sock != -1) {
        close(sock);
        sock = -1;
    }
}

// receive messages from the server (unused)
void Client::receiveMessages() {
    std::string buffer; // persistent buffer to store incomplete data
    char tempBuffer[1024];

    // setting a timeout period for recv
    struct timeval tv;
    tv.tv_sec = 1;  // timeout after 1 second
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    while (this->status) {
        ssize_t len = recv(sock, tempBuffer, sizeof(tempBuffer), 0);
        if (len > 0) {
            buffer.append(tempBuffer, len); // append new data to the buffer
            size_t pos;
            // process all complete messages in the buffer
            while ((pos = buffer.find('\n')) != std::string::npos) {
                std::string msg = buffer.substr(0, pos);
                buffer.erase(0, pos + 1); // remove the processed message from buffer
                try {
                    auto j = json::parse(msg);
                    //handleJsonMessage(j.dump(), outputWin);
                } catch (const json::parse_error& e) {
                    std::cerr << "JSON parsing error: " << e.what() << std::endl;
                }
            }

            try {
                //auto j = json::parse(msg);
                //handleJsonMessage(j.dump(), outputWin);
            } catch (const json::parse_error& e) {
                this->status = false;
            }
        } else if (len == 0) {
            std::cout << "Server closed connection.\n";
            this->status = false;
        } else if (len == -1) {
            // check if it was just a timeout or a real error
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                continue;
            }
            std::cerr << "Failed to receive data: " << strerror(errno) << std::endl;
            break;
        }
    }
}

// send a message to the server (json)
void Client::sendMessage(const json& message) {
    std::string msg = message.dump();
    if (send(sock, msg.c_str(), msg.size(), 0) < 0) {
        std::cerr << "Send failed." << std::endl;
    }
}

// define color codes as constants
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

// print colored message (output window)
void Client::printColoredMessage(const std::string& message, const std::string& color, WINDOW* outputWin) {
    int color_pair = 7; // default to white
    if (color == RED) {
        color_pair = 1;
    } else if (color == GREEN) {
        color_pair = 2;
    } else if (color == YELLOW) {
        color_pair = 3;
    } else if (color == BLUE) {
        color_pair = 4;
    } else if (color == MAGENTA) {
        color_pair = 5;
    } else if (color == CYAN) {
        color_pair = 6;
    } else if (color == WHITE) {
        color_pair = 7;
    }

    // print the message in the chosen color
    wattron(outputWin, COLOR_PAIR(color_pair));  // turn on the chosen color pair
    wprintw(outputWin, "%s\n", message.c_str());
    wattroff(outputWin, COLOR_PAIR(color_pair)); // turn off the color pair
    wrefresh(outputWin);
}

//print colored message (standard output)
void Client::printColoredMessage(const std::string& message, const std::string& color) {
    std::cout << color << message << RESET << std::endl;
}

// handle json message
void Client::handleJsonMessage(const std::string& jsonStr, WINDOW* outputWin) {
    // parse the json message
    json j;
    try{
        j = json::parse(jsonStr);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parsing error at byte " << e.byte << " with message: " << jsonStr << '\n';
        std::cerr << "HERE: " << e.what() << '\n';
        return;
    }
    // get the type of the message
    std::string type = j.value("type", "info"); // default to "info" if no type is specified
    // get the message content if available
    std::string message = j.value("message", "");
    if (type == "text") {
        message = this->aes.Decrypt(AESECB::fromHex(message));
        std::string user = this->aes.Decrypt(AESECB::fromHex(j["user"]));
        printColoredMessage(user +": " + message, MAGENTA, outputWin); // print the message in magenta
    } else if (type == "error") {
        printColoredMessage("Server: " + message, RED, outputWin); // print the error in red
    } else if (type == "warning") {
        printColoredMessage(message, YELLOW, outputWin); // print the warning in yellow
    } else if (type == "info") {
        printColoredMessage("INFO: " + message, BLUE, outputWin); // print the info in blue
    } else if (type == "success") {
        printColoredMessage("Server: " + message, GREEN, outputWin); // print the success message in green
    } else if (type == "key_exchange") {
        printColoredMessage("dh_key_init: " + jsonStr, CYAN, outputWin); // print the key exchange message in cyan
        this->keyExchangeResponse(jsonStr); // respond to the key exchange
    } else if (type == "connected") {
        this->keyExchangeInit(); // initiate the key exchange
        printColoredMessage("INFO: " + message, BLUE, outputWin);
    } else if (type == "key_exchange_response") {
        printColoredMessage("dh_key_response: " + jsonStr, CYAN, outputWin); // print the key exchange response in cyan
        this->setKey(jsonStr); // set the key for encryption (for the initiator)
    } else if (type == "public_key") {
        // handle public key
        CryptoPP::RSA::PublicKey publicKey;
        std::string pub = jsonStr;
        if (RSAWrapper::receivePublicKey(pub, publicKey)) {
            this->rsa.publicKeyB = publicKey;
        }
        std::string response = RSAWrapper::sendPublicKey(this->rsa.getPublicKey());
        printColoredMessage("server_public_key: " + pub, CYAN, outputWin); // print the public key in cyan
        printColoredMessage("client_public_key: " + response, CYAN, outputWin); // print the public key in cyan
        sendMessage(json::parse(response));
    } else if (type == "prompt"){

        std::string enc_username = rsa.encrypt(username, rsa.publicKeyB);
        std::string enc_password = rsa.encrypt(password, rsa.publicKeyB);
        if (authType == 1) {
            // create user
            json response = json{{"type", "verify"}, {"username", username}, {"password", password}};
            sendMessage(response);
        } else if (authType == 2) {
            // verify user
            json response = json{{"type", "create"}, {"username", enc_username}, {"password", enc_password}};
            sendMessage(response);
            authType = 1;
        }
    }
    else {
        printColoredMessage("Unknown message type: " + type, RED, outputWin); // print the unknown message type in red
    }
}

// utility functions
// convert integer to hex string
std::string integerToHexString(const CryptoPP::Integer& num) {
    std::string hex;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hex));
    num.Encode(encoder, num.MinEncodedSize());
    encoder.MessageEnd();
    return hex;
}
// convert byte array to hex string
std::string byteToHex(const byte* data, size_t size) {
    std::string output;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(output));
    encoder.Put(data, size);
    encoder.MessageEnd();
    return output;
}

// first step of the key exchange
void Client::keyExchangeInit(){
    // create domain parameters (p and g)
    DHKeyExchange::createDomainParameters();
    // create asymmetric key pair
    CryptoPP::SecByteBlock privKeyA, pubKeyA;
    DHKeyExchange::createAsymmetricKey(DHKeyExchange::dhA, privKeyA, pubKeyA);
    this->priv_key = privKeyA;
    // turn modulus and generator into hex strings
    std::string modulusHex = integerToHexString(DHKeyExchange::dhA.GetGroupParameters().GetModulus());
    std::string generatorHex = integerToHexString(DHKeyExchange::dhA.GetGroupParameters().GetGenerator());

    nlohmann::json message;
    message["type"] = "key_exchange";
    message["pub_key"] = byteToHex(pubKeyA.BytePtr(), pubKeyA.SizeInBytes()); // convert public key to hex
    message["p"] = modulusHex;  
    message["g"] = generatorHex;
    
    // send public key and domain parameters 
    this->sendMessage(message);  
}

// second step of the key exchange
void Client::keyExchangeResponse(const std::string& jsonStr) {
    auto j = json::parse(jsonStr);
    // retrieve public key and domain parameters
    std::string pubKeyAHex = j["pub_key"];
    std::string pHex = j["p"];
    std::string gHex = j["g"];

    // convert domain parameters to CryptoPP::Integer
    CryptoPP::Integer p(pHex.c_str()), g(gHex.c_str());

    // set domain parameters
    DHKeyExchange::createDomainParameters();

    // generate own asymmetric key pair
    CryptoPP::SecByteBlock privKeyB, pubKeyB, pubKeyA;
    DHKeyExchange::createAsymmetricKey(DHKeyExchange::dhA, privKeyB, pubKeyB);
    this->priv_key = privKeyB;

    // decode the received public key from Hex
    pubKeyA = CryptoPP::SecByteBlock(pubKeyAHex.size() / 2);
    CryptoPP::StringSource(pubKeyAHex, true, new CryptoPP::HexDecoder(new CryptoPP::ArraySink(pubKeyA.BytePtr(), pubKeyA.SizeInBytes())));

    // prepare shared secret
    CryptoPP::SecByteBlock sharedSecret(DHKeyExchange::dhA.AgreedValueLength());

    // agree on shared secret
    if (!DHKeyExchange::dhA.Agree(sharedSecret, privKeyB, pubKeyA)) { 
        throw std::runtime_error("Failed to reach shared secret");
    }

    // generate key from shared secret
    std::string key = aes.keyFromSharedSecret(sharedSecret);

    // send public key back
    nlohmann::json message;
    message["type"] = "key_exchange_response";
    message["pub_key"] = byteToHex(pubKeyB.BytePtr(), pubKeyB.SizeInBytes()); // convert public key to hex

    this->sendMessage(message);  
}

// final step of the key exchange
void Client::setKey(const std::string& jsonStr){
    auto j = json::parse(jsonStr);
    std::string pubKeyBHex = j["pub_key"];

    DHKeyExchange::createDomainParameters();
    
    // decode the received public key from Hex
    CryptoPP::SecByteBlock pubKeyB = CryptoPP::SecByteBlock(pubKeyBHex.size() / 2);
    CryptoPP::StringSource(pubKeyBHex, true, new CryptoPP::HexDecoder(new CryptoPP::ArraySink(pubKeyB.BytePtr(), pubKeyB.SizeInBytes())));

    // prepare shared secret
    CryptoPP::SecByteBlock sharedSecret(DHKeyExchange::dhA.AgreedValueLength());
    
    if (!DHKeyExchange::dhA.Agree(sharedSecret, this->priv_key, pubKeyB)) {
        throw std::runtime_error("Failed to reach shared secret");
    }

    // generate key from shared secret
    // final step of the key exchange
    std::string key = aes.keyFromSharedSecret(sharedSecret);
}
