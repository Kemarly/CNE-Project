#pragma once
#include <iostream>
#include <thread>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
using namespace std;
#define DEFAULT_COMMAND_CHAR '~'
#define MAX_BUFFER_SIZE 256

mutex clientMutex;
int tcp_recv_whole(SOCKET s, char* buf, int len);
int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
void HandleClient(SOCKET clientSocket, fd_set& readSet);
fd_set readyset, masterset;
void ServerCode(void);

int tcp_recv_whole(SOCKET s, char* buf, int len)
{
    int total = 0;
    int ret = recv(s, buf, 1, 0);
    if (ret <= 0)
        return ret;

    uint8_t size = static_cast<uint8_t>(*buf);

    do
    {
        int ret = recv(s, buf + total, len - total, 0);
        if (ret < 1)
            return ret;
        else
            total += ret;

    } while (total < len);

    return total;
}

int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
    int result;
    result = send(skSocket, reinterpret_cast<const char*>(&length), 1, 0);
    if (result <= 0)
        return result;

    int bytesSent = 0;

    while (bytesSent < length)
    {
        result = send(skSocket, data + bytesSent, length - bytesSent, 0);

        if (result <= 0)
            return result;

        bytesSent += result;
    }

    return bytesSent;
}

void HandleClient(SOCKET clientSocket, fd_set& readSet)
{
    while (true)
    {
        uint8_t size = 0;

        int result = tcp_recv_whole(clientSocket, reinterpret_cast<char*>(&size), 1);
        if ((result == SOCKET_ERROR) || (result == 0))
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            printf("  recv disconnected client\n");
            FD_CLR(clientSocket, &readSet);
            closesocket(clientSocket);
            break;
        }

        char* buffer = new char[size];
        result = tcp_recv_whole(clientSocket, buffer, size);
        if ((result == SOCKET_ERROR) || (result == 0))
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            printf("  recv is incorrect in handle 1\n");
            FD_CLR(clientSocket, &readSet);
            closesocket(clientSocket);
            delete[] buffer;
            break;
        }

        {
            std::lock_guard<std::mutex> lock(clientMutex);
            printf(" Received a message from a client\n");
            printf("\n\n");
            printf("%s", buffer);
            printf("\n\n");
        }

        delete[] buffer;
    }
}

void ServerCode(void)
{
    // Create the listening socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        cerr << "Failed to create socket." << endl;
        return;
    }

    // Set up the server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    // Prompt the user for the TCP port number
    cout << "Enter TCP Port number: ";
    int port;
    cin >> port;
    serverAddr.sin_port = htons(port);

    // Prompt the user for the chat capacity
    cout << "Enter chat capacity (maximum number of clients): ";
    int maxClients;
    cin >> maxClients;

    // Prompt the user for the command character
    cout << "Enter command character (default is ~): ";
    char commandChar;
    cin >> commandChar;

    // Bind the listening socket to the server address
    int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        cerr << "Failed to bind socket." << endl;
        closesocket(listenSocket);
        return;
    }

    // Start listening for incoming connections
    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cerr << "Failed to listen on socket." << endl;
        closesocket(listenSocket);
        return;
    }

    // Obtain server host IP using gethostname() and getaddrinfo()
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    cout << "Server running on host: " << hostname << endl;

    struct addrinfo* info;
    getaddrinfo(hostname, nullptr, nullptr, &info);

    char ip[INET6_ADDRSTRLEN];
    for (auto addr = info; addr != nullptr; addr = addr->ai_next) {
        if (addr->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr->ai_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ip, sizeof(ip));
            cout << "IPv4 Address: " << ip << endl;
        }
        else if (addr->ai_family == AF_INET6) {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr->ai_addr;
            inet_ntop(AF_INET6, &ipv6->sin6_addr, ip, sizeof(ip));
            cout << "IPv6 Address: " << ip << endl;
        }
    }
    freeaddrinfo(info);
    cout << "Listening on port: " << port << endl;

    // Set up the file descriptor sets for multiplexing
    fd_set readyset, masterset;
    FD_ZERO(&masterset);
    FD_SET(listenSocket, &masterset);
    FD_ZERO(&readyset);

    cout << "Waiting for connections..." << endl;

    while (true)
    {
        // Copy the master set to the ready set
        readyset = masterset;

        // Use select to wait for activity on the sockets
        int temp = select(0, &readyset, NULL, NULL, NULL);
        if (temp == SOCKET_ERROR)
        {
            cerr << "Select function failed: " << WSAGetLastError() << endl;
            break;
        }

        // Check for new connections on the listening socket
        if (FD_ISSET(listenSocket, &readyset))
        {
            // Accept the new connection
            SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET)
            {
                cerr << "Failed to accept connection." << endl;
            }
            else if (masterset.fd_count < FD_SETSIZE)
            {
                cout << "New connection accepted." << endl;
                FD_SET(clientSocket, &masterset);
            }
            else
            {
                cout << "Maximum number of clients reached. Connection rejected." << endl;
                closesocket(clientSocket);
            }
        }

        // Check for activity on the connected sockets
        for (u_int i = 0; i < masterset.fd_count; ++i)
        {
            SOCKET currentSocket = masterset.fd_array[i];

            if (FD_ISSET(currentSocket, &readyset))
            {
                // Handle the client's message
                HandleClient(currentSocket, masterset);
            }
        }
    }

    // Close the listening socket
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
}


int main()
{
    // Initialize Winsock
    WSADATA wsadata;
    if (WSAStartup(WINSOCK_VERSION, &wsadata) != 0)
    {
        cerr << "Failed to initialize Winsock." << endl;
        return EXIT_FAILURE;
    }
    ServerCode();

    // Clean up Winsock
    if (WSACleanup() != 0)
    {
        cerr << "Failed to clean up Winsock." << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
