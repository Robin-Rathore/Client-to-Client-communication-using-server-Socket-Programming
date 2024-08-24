#include <iostream>
#include <sys/socket.h>    // Defines socket-related functions and data structures.
#include <netinet/in.h>    // Contains definitions for the Internet Protocol family, such as structures for handling addresses.
#include <arpa/inet.h>     // For inet_pton and inet_ntop functions  Provides functions for manipulating IP addresses.
#include <unistd.h>        // Provides access to the POSIX operating system API, including functions like close().
#include <cstring>         // For memset
#include <fcntl.h>         // For fcntl() For fcntl() used for setting socket blocing or non blocking      

#define PORT 9909
using namespace std;

struct sockaddr_in srv;
fd_set fr, fw, fe; // Read , Write and Exception
int nMaxFd;
int nSocket;
int nArrClient[5];

void ProcessNewMessage(int nClientSocket){
    cout<<"\nProcessing the new message for client socket :"<<nClientSocket<<endl;
    char buff[256+1] = {0,};
    int nRet = recv(nClientSocket, buff, 256, 0);
    if(nRet < 0){
        cout<<"\nSomething wrong happened, Closing the connection for client : "<<nClientSocket<<endl;
        close(nClientSocket);
        for(int nIndex =0; nIndex < 5; nIndex++){
            if(nArrClient[nIndex] == nClientSocket){
                nArrClient[nIndex] = 0;
                break;
            }
        }
    }else{
        // cout<<"\nThe Message received from the client is : "<<buff;
        // send(nClientSocket, "Sent Sucessfully", 17, 0);
        //sending message to another clients
        for(int index =0; index < 5; index++){
            if(nArrClient[index] != nClientSocket && nArrClient[index] != 0){
                send(nArrClient[index], buff, sizeof(buff), 0);
            }
        }
        cout<<"\n******************************************************************\n";
    }
}

void ProcessTheNewRequest() {
    if (FD_ISSET(nSocket, &fr)) {
        struct sockaddr_in clientAddr;
        socklen_t nLen = sizeof(clientAddr); // Corrected to socklen_t
        int nClientSocket = accept(nSocket, (struct sockaddr *)&clientAddr, &nLen);

        if (nClientSocket >= 0) { // Accepting connection, check against >= 0
            int index;
            for (index = 0; index < 5; ++index) {
                if (nArrClient[index] == 0) {
                    nArrClient[index] = nClientSocket;
                    send(nClientSocket, "Got the connection done successfully", 37, 0);

                    // Update nMaxFd if needed
                    if (nClientSocket > nMaxFd) {
                        nMaxFd = nClientSocket;
                    }
                    break;
                }
            }

            if (index == 5) {
                cout << "\nNo Space found for a new client\n";
                close(nClientSocket); // Close the socket if no space
            }
        } else {
            cout << "\nError accepting connection\n";
        }
    } else {
        for (int index = 0; index < 5; index++) {
            if (FD_ISSET(nArrClient[index], &fr)) {
                // Got the new message from the client
                ProcessNewMessage(nArrClient[index]);
            }
        }
    }
}


int main(){
    int nRet =0;
    // INITIALISE THE SOCKET 

    //The socket() function is a system call that creates a new socket and returns a file descriptor (an integer) that can be used to refer to that socket.
    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // AF_INET is use to resolve the addreses provided by the UDP or TCP
   //AF_INET: This specifies the address family for the socket. AF_INET stands for "Address Family - Internet," which indicates that the socket will be used for IPv4 addresses.
   //SOCK_STREAM indicates that the socket is a stream socket. Stream sockets provide a reliable, connection-oriented communication channel, meaning that data is transmitted in a continuous stream and guarantees delivery (like a phone call).This is typically used for TCP (Transmission Control Protocol) connections.
   //IPPROTO_TCP indicates that the Transmission Control Protocol (TCP) will be used.

    if(nSocket < 0){
        cout<<"\nThe Socket is not Opened\n";
        exit(EXIT_FAILURE);
    }else{
        cout<<"\n The socket is Opened Sucessfully\n";
    }

    //INITIALISE THE ENVIRONMENT FOR SOCKADDR STRUCTURE
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    //setcoketopt
    int optVal =0;
    int optLen = sizeof(optVal);
    nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optVal, optLen);

    if(nRet == 0){
        cout<<"\n The Setsockopt call sucessful\n";
    }else{
        cout<<"\nFailed\n";
        exit(EXIT_FAILURE);
    }
    //ABOUT THE BLOCKING AND NON-BLOCKING OF SOCKET
    // Equivalent of ioctlsocket on macOS/Linux
    int flags = fcntl(nSocket, F_GETFL, 0);//fcntl: This is a system call in Unix-like operating systems used to manipulate file descriptors. A file descriptor is an integer that uniquely identifies an open file (or socket, in this case) within a process.
    //F_GETFL: This is a command that tells fcntl to retrieve the current file status flags of the file descriptor nSocket. These flags determine how the file descriptor behaves (e.g., whether it's blocking or non-blocking, whether it's append-only, etc.).
    if (flags == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    //BY DEFAULT EVERY SOCKET IS A BLOCKING SOCKET, MULTITHRADING IS USED IN THIS
    //flags |= O_NONBLOCK;  // Set non-blocking mode
    // flags &= ~O_NONBLOCK; // In our server we will use the blocking sockets
    // if (fcntl(nSocket, F_SETFL, flags) == -1) {
    //     perror("fcntl");
    //     exit(EXIT_FAILURE);
    // } else {
    //     cout << "\nSocket is now set to blocking mode\n" << endl;
    // }

    //BIND THE SOCET TO THE LOCAL PORT
    nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));
    if(nRet < 0){
        cout<<"\n Failed to bind to the local port\n";
        exit(EXIT_FAILURE);
    }else{
        cout<<"\n Sucessfully bind to local port\n";
    }

    // LISTEN THE REQUEST FROM CLIENT (QUEUES THE REQUEST)
    nRet = listen(nSocket, 5); // Here nSocket is the Socket to listent to and 5 is the number of active queue request
    if(nRet < 0){
        cout<<"\n Failed to listen to the local port\n";
        exit(EXIT_FAILURE);
    }else{
        cout<<"\n Started listening to the local port\n";
    }

    nMaxFd = nSocket;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0; // 0 Micro seconds

    while(true){
        FD_ZERO(&fr);
        FD_ZERO(&fw);
        FD_ZERO(&fe);

        FD_SET(nSocket, &fr);
        FD_SET(nSocket, &fe);

        for(int index =0; index < 5; ++index){
            if(nArrClient[index] != 0){
                FD_SET(nArrClient[index], &fr);
                FD_SET(nArrClient[index], &fe);
            }
        }
        // cout<<"\n Before Select Call : "<<fr.fds_bits;
        //KEEP WAITING FOR THE NEW REQUESTS AND PROCEED AS PER THE REQUEST 
        nRet = select(nMaxFd +1, &fr, &fw, &fe, &tv);
        if(nRet > 0){
            // When someone is connects or communicates with a message over a dedicated connection
            cout<<"\nConnected Sucessfully\n";

                // if(FD_ISSET(nSocket, &fe)){
                //     cout<<"\nThere is an exception\n";
                // }
                // if(FD_ISSET(nSocket, &fw)){
                //     cout<<"\nReady to write something\n";
                // }
                // if(FD_ISSET(nSocket, &fr)){
                //     cout<<"\nReady to read, somethiong new came up at the port\n";
                // }

            ProcessTheNewRequest();
            // break;// Breack as we are not serving any request now, so until the request is serve the request is maintainde in queue so to avoid that we have break here, when we write some code to serve those request we will remove this break statement
        }else if(nRet == 0){
            // No connection or any other request is made or you cn say that non of the socket descriptor is ready
        }else{
            // It failed and your application should show some usefull message
            cout<<"Nothing in PORT : "<<PORT<<endl;
        }
        // cout<<"\n after Select Call : "<<fr.fds_bits;
    }
}