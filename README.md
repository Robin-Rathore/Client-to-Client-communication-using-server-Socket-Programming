Client-to-Client Communication using Server Socket Programming
Overview

This project is a simple client-to-client communication application built using C++ and socket programming. It allows users to connect and chat anonymously with strangers. The chat is not stored anywhere, making it a secure and ephemeral conversation tool. Once a user exits, the chat is lost permanently.
Getting Started
Prerequisites

    C++11 or later
    Compatible with macOS and Linux systems. For Windows, some modifications are required.

Installation

Clone the repository:

bash

git clone <repository-url>

Running the Application

    Compile the Server:

    bash

g++ -o server server.cpp

Run the Server:

bash

./server

Compile the Client:

bash

g++ -std=c++11 -pthread -o socketclient socketclient.cpp

Run the Client:

bash

    ./socketclient

    You can open multiple instances of the client (up to 5 by default, configurable as needed) to simulate multiple users.

Features

    Socket Programming: Utilizes TCP for reliable communication between clients.
    Scalability: Easily extendable to support more clients.
    Multithreading: Enables simultaneous sending and receiving of messages.

Technical Requirements

    Languages: C++
    Concepts: Socket programming, TCP/UDP, multithreading, networking.
    Knowledge Areas: Networking protocols, data structures, C++ programming.

License

This project is licensed under the MIT License - see the LICENSE file for details.

Contact:
Robin Rathore
robinsingh248142@gmail.com
