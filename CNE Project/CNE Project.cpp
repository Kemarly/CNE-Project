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
int main()
{
    WSADATA wsadata;
    WSAStartup(WINSOCK_VERSION, &wsadata);
    ServerCode();
    return WSACleanup();
}
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
            printf("  recv is incorrect\n");
            FD_CLR(clientSocket, &readSet);
            closesocket(clientSocket);
            break;
        }

        char* buffer = new char[size];
        result = tcp_recv_whole(clientSocket, buffer, size);
        if ((result == SOCKET_ERROR) || (result == 0))
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            printf("  recv is incorrect\n");
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
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        printf("  Socket function incorrect\n");
        return;
    }
    else
    {
        printf("  I used the socket function\n");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    // Prompt user for TCP Port number
    printf("Enter TCP Port number: ");
    int port;
    std::cin >> port;
    serverAddr.sin_port = htons(port);

    // Prompt user for chat capacity (maximum number of clients)
    printf("Enter chat capacity (maximum number of clients): ");
    int maxClients;
    std::cin >> maxClients;

    // Prompt user for the command character
    printf("Enter command character (default is ~): ");
    char commandChar;
    std::cin >> commandChar;

    int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    if (result == SOCKET_ERROR) {
        printf("  Bind function incorrect\n");
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        printf("  Listen function incorrect\n");
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    // Obtain server host IP using gethostname() and getaddrinfo()
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    printf("Server running on host: %s\n", hostname);

    struct addrinfo* info;
    getaddrinfo(hostname, nullptr, nullptr, &info);

    char ip[INET6_ADDRSTRLEN];
    for (auto addr = info; addr != nullptr; addr = addr->ai_next) {
        if (addr->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr->ai_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ip, sizeof(ip));
            printf("IPv4 Address: %s\n", ip);
        }
        else if (addr->ai_family == AF_INET6) {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr->ai_addr;
            inet_ntop(AF_INET6, &ipv6->sin6_addr, ip, sizeof(ip));
            printf("IPv6 Address: %s\n", ip);
        }
    }
    freeaddrinfo(info);
    printf("Listening on port: %d\n", port); 

    fd_set readyset, masterset;
    FD_ZERO(&masterset);
    FD_SET(listenSocket, &masterset);
    FD_ZERO(&readyset);

    printf("Waiting...\n\n");

    while (true)
    {
        readyset = masterset;

        int temp = select(0, &readyset, NULL, NULL, NULL);

        if (temp == SOCKET_ERROR)
        {
            printf("  Select function incorrect\n");
            break;
        }

        if (FD_ISSET(listenSocket, &readyset))
        {
            // New connection
            SOCKET ComSocket = accept(listenSocket, nullptr, nullptr);
            if (ComSocket == INVALID_SOCKET)
            {
                printf("  Accept function incorrect\n");
            }
            else if (FD_SETSIZE > masterset.fd_count)
            {
                printf("  New connection accepted\n");
                FD_SET(ComSocket, &masterset);
                thread(HandleClient, ComSocket, std::ref(masterset)).detach();
            }
            else
            {
                printf("  Maximum number of clients reached. Connection rejected.\n");
                closesocket(ComSocket);
            }
        }

        for (u_int i = 0; i < masterset.fd_count; ++i)
        {
            SOCKET currentSocket = masterset.fd_array[i];

            if (FD_ISSET(currentSocket, &readyset))
            {
                // Existing client data
                uint8_t size = 0;

                result = tcp_recv_whole(currentSocket, (char*)&size, 1);
                if ((result == SOCKET_ERROR) || (result == 0))
                {
                    std::lock_guard<std::mutex> lock(clientMutex);
                    printf("  recv is incorrect\n");
                    FD_CLR(currentSocket, &masterset);
                    closesocket(currentSocket);
                }
                else
                {
                    char* buffer = new char[size];
                    result = tcp_recv_whole(currentSocket, buffer, size);
                    if ((result == SOCKET_ERROR) || (result == 0))
                    {
                        std::lock_guard<std::mutex> lock(clientMutex);
                        printf("  recv is incorrect\n");
                        FD_CLR(currentSocket, &masterset);
                        closesocket(currentSocket);
                        delete[] buffer;
                    }
                    else
                    {
                        std::lock_guard<std::mutex> lock(clientMutex);
                        printf("  Received a message from a client\n");
                        printf("\n\n");
                        printf(buffer);
                        printf("\n\n");
                        delete[] buffer;
                    }
                }
            }
        }
    }

    // close both sockets
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
}