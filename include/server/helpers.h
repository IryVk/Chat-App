#ifndef HELPERS_H
#define HELPERS_H

#include <common/passhash.h>
#include <server/userhandler.h>
#include <fstream>
#include <limits> 
#include <cstdlib>  
#include <csignal>  
#include <exception>
#include <sstream>
#include <string>
#include <cctype>

using namespace CryptoPP;

// helper functions

bool createUser(std::string& key, std::string& iv, std::string& user); // create a new user
bool verifyUser(std::string& key, std::string& iv, std::string& user); // verify an existing user

#endif // HELPERS_H