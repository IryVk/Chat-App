# Anonymous Chat Application
A chat application built for coursework 2 of KH5062CEM Programming and Algorithms 2.
<a name="readme-top"></a>

## Description
This project is a chat application developed in C++ as part of the coursework for KH5062CEM Programming and Algorithms. It is designed to securely send encrypted messages between two clients using Diffie Helmann Key Exchange and AES encryption using the derived shared secret.


<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#installation">Installation</a></li>
    <ul>
        <li><a href="#dependencies">Dependencies</a></li>
        <li><a href="#building">Building</a></li>
      </ul>
    <li>
      <a href="#usage">Usage</a>
    </li>
    <li>
      <a href="#usage">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
        <li><a href="#file-structure">File Structure</a></li>
        <li><a href="#features">Features</a></li>
      </ul>
    </li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Installation

### Dependencies
RUN ON LINUX
Before installing the Secure Password Manager, you must install Crypto++ library on your Linux system.

1. Update your packaged list:
   ```bash
   sudo apt-get update
   ```
2. Install the Crypto++ library:
   ```bash
   sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils
   ```
3. Install OpenSSL
   ```bash
   sudo apt-get install libssl-dev
   ```
4. Install ncurses
   ```bash
   sudo apt-get install libncurses5-dev libncursesw5-dev
   ```

### Building 

1. Clone the repository
   ```bash
   git clone https://github.com/IryVk/chat-app.git
   ```
2. Navigate to project directory
   ```bash
   cd chat-app
   ```
3. Build the project
   ```bash
   make
   ```
4. Build the unit tests
   ```bash
   cd tests && make && cd ..
   ```
5. Run the compiled binaries.
   ```bash
   # the server
   ./chat-server.exe
   # the client
   ./chat-client.exe
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Usage



DEMO



<p align="right">(<a href="#readme-top">back to top</a>)</p>

## About The Project

### Built With

+ <img src="https://img.shields.io/badge/-C++-blue?logo=cplusplus">

### File Structure

```bash
Secure-Password-Manager/  # project root
├── assets/ 
├── bin/ 
├── include/  # header files
│   └── client/  # client header files
│   └── common/  # common header files
│   └── server/  # server header files
├── lib/  # compiled static libraries
├── obj/  # compiled object files
├── src/  # source cpp files
│   └── client/  # client source files
│   └── common/  # common source files
│   └── server/  # server source files
├── test/  # unittests
│   └── client/  # client source files
│        └── Makefile  # make file for client unit tests
│   └── common/  # common source files
│       └── Makefile  # make file for common unit tests
│   └── server/  # server source files
│       └── Makefile  # make file for server unit tests
│   └── Makefile  # make file for all unit tests
├── .gitignore
├── LICENSE
├── Makefile  # make file for program
└── README.md
```

### Features

+ 
+ 

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Contact

Arwa Essam Abdelaziz

aa2101585@tkh.edu.eg - arwa.abdelaziz.03@gmail.com

<p align="right">(<a href="#readme-top">back to top</a>)</p>

