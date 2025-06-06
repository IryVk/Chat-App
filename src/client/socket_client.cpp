/**
 * @file client/socket_client.cpp
 * @date 2024-04-22
 * @author Arwa Essam Abdelaziz
 * @brief This file contains the implementation of the Client class
 * 
 * This file contains the implementation of the Client class, which is used to manage the client side of the chat application. It provides methods to connect to the server, send and receive messages, and handle the key exchange process.
*/

#include <client/socket_client.h>
#include <server/socket_server.h>

using json = nlohmann::json;

/**
 * @brief Constructor for the Client class
 * 
 * This constructor initializes the Client object with the server IP address, port, username, password, and authentication type.
 * 
 * @param server_ip The IP address of the server
 * @param port The port number of the server
 * @param username The username of the client
 * @param password The password of the client
 * @param type The authentication type (1: create user, 2: verify user)
 * 
 * @return A Client object
*/
Client::Client(std::string& server_ip, int port, std::string username, std::string password, int type) : sock(-1), server_ip(server_ip), port(port), aes(), priv_key(), rsa(), username(username), password(password), authType(type) {}

/**
 * @brief Destructor for the Client class
 * 
 * This destructor disconnects the client from the server.
 * 
 * @return Void
*/
Client::~Client() {
    disconnect();
}

/**
 * @brief Connect to the server
 * 
 * This method creates a socket and connects to the server using the specified IP address and port number.
 * 
 * @return bool True if the connection was successful, false otherwise
*/
bool Client::connectToServer() {
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) { // Check if the socket was created successfully
        std::cerr << "Could not create socket." << std::endl;
        return false;
    }
    // Server address
    sockaddr_in server;
    // Set the server address
    server.sin_family = AF_INET;
    // Convert the server IP address to binary
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());
    // Convert the port to network byte order
    server.sin_port = htons(port);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) { // Check if the connection was successful
        std::cerr << "Connect failed." << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Disconnect from the server
 * 
 * This method closes the socket connection to the server.
 * 
 * @return Void
*/
void Client::disconnect() {
    // Close the socket
    if (sock != -1) {
        close(sock);
        sock = -1;
    }
}

/**
 * @brief Send messages to the server
 * 
 * This method sends a JSON message to the server using the established socket connection.
 * 
 * @param message The JSON message to send
 * 
 * @return Void
*/
void Client::sendMessage(const json& message) {
    std::string msg = message.dump();
    if (send(sock, msg.c_str(), msg.size(), 0) < 0) {
        std::cerr << "Send failed." << std::endl;
    }
}

// Define color codes as constants
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

/**
 * @brief Print colored message to the output window
 * 
 * This method prints a colored message to the output window using the specified color.
 * 
 * @param message The message to print
 * @param color The color of the message
 * @param outputWin The output window to print the message to
 * 
 * @return Void
*/
void Client::printColoredMessage(const std::string& message, const std::string& color, WINDOW* outputWin) {
    int color_pair = 7; // Default to white
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

    // Print the message in the chosen color
    wattron(outputWin, COLOR_PAIR(color_pair));  // Turn on the chosen color pair
    wprintw(outputWin, "%s\n", message.c_str());
    wattroff(outputWin, COLOR_PAIR(color_pair)); // Turn off the color pair
    wrefresh(outputWin);
}

/**
 * @brief Print colored message to the standard output
 * 
 * This method prints a colored message to the standard output using the specified color.
 * 
 * @param message The message to print
 * @param color The color of the message
 * 
 * @return Void
*/
void Client::printColoredMessage(const std::string& message, const std::string& color) {
    std::cout << color << message << RESET << std::endl;
}

/**
 * @brief Handle JSON messages received from the server
 * 
 * This method handles JSON messages received from the server. It parses the message, extracts the type and content, and takes appropriate actions based on the message type.
 * 
 * @param jsonStr The JSON message received from the server
 * @param outputWin The output window to print the message to
 * 
 * @return Void
*/
void Client::handleJsonMessage(const std::string& jsonStr, WINDOW* outputWin) {
    // Parse the json message
    json j;
    try{
        j = json::parse(jsonStr);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parsing error at byte " << e.byte << " with message: " << jsonStr << '\n';
        std::cerr << "HERE: " << e.what() << '\n';
        return;
    }
    // Get the type of the message
    std::string type = j.value("type", "other"); // Default to "other" if no type is specified
    // Get the message content if available
    std::string message = j.value("message", "");
    if (type == "text") {
        message = this->aes.Decrypt(AESECB::fromHex(message));
        std::string user = this->aes.Decrypt(AESECB::fromHex(j["user"]));
        printColoredMessage(user +": " + message, MAGENTA, outputWin); 
    } else if (type == "error") {
        printColoredMessage("Server: " + message, RED, outputWin); 
    } else if (type == "warning") {
        printColoredMessage(message, YELLOW, outputWin); 
    } else if (type == "info") {
        printColoredMessage("INFO: " + message, BLUE, outputWin); 
    } else if (type == "success") {
        printColoredMessage("Server: " + message, GREEN, outputWin); 
    } else if (type == "key_exchange") {
        printColoredMessage("dh_key_init: " + jsonStr, CYAN, outputWin); 
        this->keyExchangeResponse(jsonStr); // Respond to the key exchange
    } else if (type == "connected") {
        this->keyExchangeInit(); // Initiate the key exchange
        printColoredMessage("INFO: " + message, BLUE, outputWin);
    } else if (type == "key_exchange_response") {
        printColoredMessage("dh_key_response: " + jsonStr, CYAN, outputWin); 
        this->setKey(jsonStr); // Set the key for encryption (for the initiator)
    } else if (type == "public_key") {
        // Handle the received public key
        CryptoPP::RSA::PublicKey publicKey;
        std::string pub = jsonStr;
        if (RSAWrapper::receivePublicKey(pub, publicKey)) {
            this->rsa.publicKeyB = publicKey;
        }
        std::string response = RSAWrapper::sendPublicKey(this->rsa.getPublicKey());
        printColoredMessage("server_public_key: " + pub, CYAN, outputWin);
        printColoredMessage("client_public_key: " + response, CYAN, outputWin); 
        // Send client public key back to the server
        sendMessage(json::parse(response));
    } else if (type == "prompt"){
        std::string enc_username = rsa.encrypt(username, rsa.publicKeyB);
        std::string enc_password = rsa.encrypt(password, rsa.publicKeyB);
        if (authType == 1) {
            // Verify existing user
            json response = json{{"type", "verify"}, {"username", enc_username}, {"password", enc_password}};
            sendMessage(response);
        } else if (authType == 2) {
            // Create a new user
            json response = json{{"type", "create"}, {"username", enc_username}, {"password", enc_password}};
            sendMessage(response);
            authType = 1; // Next prompt should attempt verification
        }
    }
    else {
        printColoredMessage("Unknown message type: " + type, RED, outputWin); // print the unknown message type in red
    }
}

// ================ Utility functions =================

/**
 * @brief Convert a CryptoPP::Integer to a hex string
 * 
 * This function converts a CryptoPP::Integer to a hex string.
 * 
 * @param num The CryptoPP::Integer to convert
 * 
 * @return std::string The hex string representation of the CryptoPP::Integer
*/
std::string integerToHexString(const CryptoPP::Integer& num) {
    std::string hex;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hex));
    num.Encode(encoder, num.MinEncodedSize());
    encoder.MessageEnd();
    return hex;
}

/**
 * @brief Convert a byte array to a hex string
 * 
 * This function converts a byte array to a hex string.
 * 
 * @param data The byte array to convert
 * @param size The size of the byte array
 * 
 * @return std::string The hex string representation of the byte array
*/
std::string byteToHex(const byte* data, size_t size) {
    std::string output;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(output));
    encoder.Put(data, size);
    encoder.MessageEnd();
    return output;
}

/**
 * @brief The first step of the Deffie Hellman key exchange
 * 
 * This method initiates the first step of the Deffie Hellman key exchange by creating the domain parameters (p and g) and generating the asymmetric key pair.
 * 
 * @return Void
*/
void Client::keyExchangeInit(){
    // Create domain parameters (p and g)
    DHKeyExchange::createDomainParameters();
    // Create asymmetric key pair
    CryptoPP::SecByteBlock privKeyA, pubKeyA;
    DHKeyExchange::createAsymmetricKey(DHKeyExchange::dhA, privKeyA, pubKeyA);
    this->priv_key = privKeyA;
    // Turn modulus and generator into hex strings
    std::string modulusHex = integerToHexString(DHKeyExchange::dhA.GetGroupParameters().GetModulus());
    std::string generatorHex = integerToHexString(DHKeyExchange::dhA.GetGroupParameters().GetGenerator());

    json message;
    message["type"] = "key_exchange";
    message["pub_key"] = byteToHex(pubKeyA.BytePtr(), pubKeyA.SizeInBytes()); // Convert public key to hex
    message["p"] = modulusHex;  
    message["g"] = generatorHex;
    
    // Send public key and domain parameters 
    this->sendMessage(message);  
}

/**
 * @brief Respond to the key exchange
 * 
 * This method responds to the key exchange by generating the shared secret and deriving the encryption key.
 * 
 * @param jsonStr The JSON message containing the public key and domain parameters
 * 
 * @return Void
*/
void Client::keyExchangeResponse(const std::string& jsonStr) {
    auto j = json::parse(jsonStr);
    // Retrieve public key and domain parameters
    std::string pubKeyAHex = j["pub_key"];
    std::string pHex = j["p"];
    std::string gHex = j["g"];

    // Convert domain parameters to CryptoPP::Integer
    CryptoPP::Integer p(pHex.c_str()), g(gHex.c_str());

    // Set domain parameters
    DHKeyExchange::createDomainParameters();

    // Generate own asymmetric key pair
    CryptoPP::SecByteBlock privKeyB, pubKeyB, pubKeyA;
    DHKeyExchange::createAsymmetricKey(DHKeyExchange::dhA, privKeyB, pubKeyB);
    this->priv_key = privKeyB;

    // Decode the received public key from Hex
    pubKeyA = CryptoPP::SecByteBlock(pubKeyAHex.size() / 2);
    CryptoPP::StringSource(pubKeyAHex, true, new CryptoPP::HexDecoder(new CryptoPP::ArraySink(pubKeyA.BytePtr(), pubKeyA.SizeInBytes())));

    // Prepare shared secret
    CryptoPP::SecByteBlock sharedSecret(DHKeyExchange::dhA.AgreedValueLength());

    // Agree on shared secret
    if (!DHKeyExchange::dhA.Agree(sharedSecret, privKeyB, pubKeyA)) { 
        throw std::runtime_error("Failed to reach shared secret");
    }

    // Generate key from shared secret
    std::string key = aes.keyFromSharedSecret(sharedSecret);

    // Send public key back
    json message;
    message["type"] = "key_exchange_response";
    message["pub_key"] = byteToHex(pubKeyB.BytePtr(), pubKeyB.SizeInBytes()); // convert public key to hex

    this->sendMessage(message);  
}

/**
 * @brief Set the key for encryption
 * 
 * This method sets the key for encryption by generating the shared secret and deriving the encryption key.
 * 
 * @param jsonStr The JSON message containing the public key
 * 
 * @return Void
*/
void Client::setKey(const std::string& jsonStr){
    auto j = json::parse(jsonStr);
    std::string pubKeyBHex = j["pub_key"];

    DHKeyExchange::createDomainParameters();
    
    // Decode the received public key from Hex
    CryptoPP::SecByteBlock pubKeyB = CryptoPP::SecByteBlock(pubKeyBHex.size() / 2);
    CryptoPP::StringSource(pubKeyBHex, true, new CryptoPP::HexDecoder(new CryptoPP::ArraySink(pubKeyB.BytePtr(), pubKeyB.SizeInBytes())));

    // Prepare shared secret
    CryptoPP::SecByteBlock sharedSecret(DHKeyExchange::dhA.AgreedValueLength());
    
    if (!DHKeyExchange::dhA.Agree(sharedSecret, this->priv_key, pubKeyB)) {
        throw std::runtime_error("Failed to reach shared secret");
    }

    // Generate key from shared secret
    // Final step of the key exchange
    std::string key = aes.keyFromSharedSecret(sharedSecret);
}
