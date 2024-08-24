#include <iostream>
#include <sys/socket.h>    // Defines socket-related functions and data structures.
#include <netinet/in.h>    // Contains definitions for the Internet Protocol family, such as structures for handling addresses.
#include <arpa/inet.h>     // For inet_pton and inet_ntop functions  Provides functions for manipulating IP addresses.
#include <unistd.h>        // Provides access to the POSIX operating system API, including functions like close().
#include <cstring>         // For memset
#include <fcntl.h>         // For fcntl() For fcntl() used for setting socket blocing or non blocking      
#include <queue>
#include <map>

#define PORT 9909
using namespace std;

struct sockaddr_in srv;
fd_set fr, fw, fe;
int nMaxFd;
int nSocket;

queue<int> availableClients; // Queue to manage available clients
map<int, int> clientPairs;   // Map to manage client pairs

void ProcessNewMessage(int nClientSocket) {
    cout << "\nProcessing the new message for client socket: " << nClientSocket << endl;
    char buff[256 + 1] = {0,};
    int nRet = recv(nClientSocket, buff, 256, 0);

    if (nRet < 0) {
        cout << "\nError occurred, closing connection for client: " << nClientSocket << endl;
        close(nClientSocket);
        
        int pairedClient = clientPairs[nClientSocket];
        clientPairs.erase(nClientSocket);
        clientPairs.erase(pairedClient);

        if (pairedClient != 0) {
            send(pairedClient, "Your chat partner disconnected. Searching for a new partner...", 61, 0);
            availableClients.push(pairedClient);
        }

    } else {
        int pairedClient = clientPairs[nClientSocket];
        if (pairedClient != 0) {
            send(pairedClient, buff, sizeof(buff), 0);
        }
        cout << "\n******************************************************************\n";
    }
}

void ProcessTheNewRequest() {
    if (FD_ISSET(nSocket, &fr)) {
        struct sockaddr_in clientAddr;
        socklen_t nLen = sizeof(clientAddr);
        int nClientSocket = accept(nSocket, (struct sockaddr *)&clientAddr, &nLen);

        if (nClientSocket >= 0) {
            if (availableClients.empty()) {
                availableClients.push(nClientSocket);
                send(nClientSocket, "Waiting for a partner...", 26, 0);
            } else {
                int pairedClient = availableClients.front();
                availableClients.pop();

                clientPairs[nClientSocket] = pairedClient;
                clientPairs[pairedClient] = nClientSocket;

                send(nClientSocket, "You are now connected to a random client", 41, 0);
                send(pairedClient, "You are now connected to a random client", 41, 0);
            }

            if (nClientSocket > nMaxFd) {
                nMaxFd = nClientSocket;
            }
        } else {
            cout << "\nError accepting connection\n";
        }
    } else {
        for (auto it = clientPairs.begin(); it != clientPairs.end(); ++it) {
            int clientSocket = it->first;
            if (FD_ISSET(clientSocket, &fr)) {
                ProcessNewMessage(clientSocket);
            }
        }
    }
}

int main() {
    int nRet = 0;
    // The socket() function is a system call that creates a new socket and returns a file descriptor (an integer) that can be used to refer to that socket.
    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // AF_INET is use to resolve the addreses provided by the UDP or TCP
   //AF_INET: This specifies the address family for the socket. AF_INET stands for "Address Family - Internet," which indicates that the socket will be used for IPv4 addresses.
   //SOCK_STREAM indicates that the socket is a stream socket. Stream sockets provide a reliable, connection-oriented communication channel, meaning that data is transmitted in a continuous stream and guarantees delivery (like a phone call).This is typically used for TCP (Transmission Control Protocol) connections.
   //IPPROTO_TCP indicate

    if (nSocket < 0) {
        cout << "\nSocket could not be opened\n";
        exit(EXIT_FAILURE);
    } else {
        cout << "\nSocket opened successfully\n";
    }

    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    int optVal = 0;
    int optLen = sizeof(optVal);
    nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optVal, optLen);

    if (nRet == 0) {
        cout << "\nSetsockopt call successful\n";
    } else {
        cout << "\nFailed\n";
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(nSocket, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    // nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));
    nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));

    if (nRet < 0) {
        cout << "\nFailed to bind to the local port\n";
        exit(EXIT_FAILURE);
    } else {
        cout << "\nSuccessfully bound to local port\n";
    }

    nRet = listen(nSocket, 5);
    if (nRet < 0) {
        cout << "\nFailed to listen to the local port\n";
        exit(EXIT_FAILURE);
    } else {
        cout << "\nStarted listening to the local port\n";
    }

    nMaxFd = nSocket;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while (true) {
        FD_ZERO(&fr);
        FD_ZERO(&fw);
        FD_ZERO(&fe);

        FD_SET(nSocket, &fr);
        FD_SET(nSocket, &fe);

        for (auto it = clientPairs.begin(); it != clientPairs.end(); ++it) {
            int clientSocket = it->first;
            if (clientSocket != 0) {
                FD_SET(clientSocket, &fr);
                FD_SET(clientSocket, &fe);
            }
        }

        nRet = select(nMaxFd + 1, &fr, &fw, &fe, &tv);
        if (nRet > 0) {
            ProcessTheNewRequest();
        } else if (nRet == 0) {
            // No connection or any other request
        } else {
            cout << "Nothing in PORT: " << PORT << endl;
        }
    }
}
