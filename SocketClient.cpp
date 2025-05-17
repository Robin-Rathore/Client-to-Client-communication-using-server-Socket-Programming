#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include <atomic>

#define PORT 9909
using namespace std;
atomic<bool> running(true);

int nClientSocket;
struct sockaddr_in srv;

void takeInput() {

    while (running) {
       char buff[256] = {0};
    //    cout << "Enter message: ";
        if (fgets(buff, sizeof(buff), stdin) == nullptr) {
            cerr << "Failed to read input.\n";
            break;
        }

        // Remove newline character from fgets
        buff[strcspn(buff, "\n")] = '\0';

        if (strcmp(buff, "exit") == 0) {
            cout << "Exiting...\n";
            break;
        }

        int nRet = send(nClientSocket, buff, strlen(buff), 0);
        if (nRet < 0) {
            cerr << "Failed to send message to server.\n";
            break;
        }
    }
}

void showOutput() {
    while (running) {
        char buff[256] = {0};
        int nRet = recv(nClientSocket, buff, sizeof(buff) - 1, 0);
        if (nRet > 0) {
            if (strlen(buff) > 0) 
            cout << "Message From your friend : " << buff << "\n";
        } else if (nRet == 0) {
            cout << "Server closed the connection.\n";
            break;
        } else if (nRet < 0) {
            cerr << "No response from server (timed out).\n";
        }
    }
}

int main() {
    int nRet = 0;
    nClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (nClientSocket < 0) {
        cerr << "Socket creation failed.\n";
        exit(EXIT_FAILURE);
    } else {
        cout << "Socket created successfully.\n";
    }

    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr("0.0.0.0");
    memset(&srv.sin_zero, 0, 8);

    nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));

    if (nRet < 0) {
        cerr << "Connection to server failed.\n";
        close(nClientSocket);
        exit(EXIT_FAILURE);
    } else {
        cout << "Connected to server successfully.\n";
        
        char buff[256] = {0};
        nRet = recv(nClientSocket, buff, sizeof(buff) - 1, 0);
        if (nRet > 0) {
            cout << "Message from server: " << buff << "\n";
        } else if (nRet == 0) {
            cout << "Server closed the connection.\n";
            close(nClientSocket);
            exit(EXIT_SUCCESS);
        } else {
            cerr << "Failed to receive initial message from server.\n";
            close(nClientSocket);
            exit(EXIT_FAILURE);
        }

        cout << "You can start sending messages to the server. Type 'exit' to quit.\n";


        thread inputThread(takeInput);
        thread outputThread(showOutput);

        inputThread.join();
        outputThread.join();

    }

    close(nClientSocket);
    return 0;
}
