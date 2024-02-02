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
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
using namespace std;
#define DEFAULT_COMMAND_CHAR '~'
#define MAX_BUFFER_SIZE 256

int tcp_recv_whole(SOCKET s, char* buf, int len);
int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
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

void ServerCode(void)
{
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        printf("DEBUG// Socket function incorrect\n");
        return;
    }
    else
    {
        printf("DEBUG// I used the socket function\n");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    // Prompt user for TCP Port number
    printf("Enter TCP Port number: ");
    int port;
    std::cin >> port;
    serverAddr.sin_port = htons(port);

    int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    // Obtain server host IP using gethostname() and getaddrinfo()
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    printf("Server running on host: %s\n", hostname);

    struct addrinfo* info;
    getaddrinfo(hostname, nullptr, nullptr, &info);

    char ip[INET6_ADDRSTRLEN];
    for (auto addr = info; addr != nullptr; addr = addr->ai_next)
    {
        if (addr->ai_family == AF_INET)
        {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr->ai_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ip, sizeof(ip));
            printf("IPv4 Address: %s\n", ip);
        }
        else if (addr->ai_family == AF_INET6)
        {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr->ai_addr;
            inet_ntop(AF_INET6, &ipv6->sin6_addr, ip, sizeof(ip));
            printf("IPv6 Address: %s\n", ip);
        }
    }

    freeaddrinfo(info);

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(listenSocket, &readSet);

    printf("Waiting...\n\n");

    while (true)
    {
        fd_set tempSet = readSet;
        int selectResult = select(0, &tempSet, nullptr, nullptr, nullptr);

        if (selectResult == SOCKET_ERROR)
        {
            printf("DEBUG// Select function incorrect\n");
            break;
        }

        for (u_int i = 0; i < tempSet.fd_count; ++i)
        {
            if (tempSet.fd_array[i] == listenSocket)
            {
                // New connection
                SOCKET ComSocket = accept(listenSocket, nullptr, nullptr);
                if (ComSocket == INVALID_SOCKET)
                {
                    printf("DEBUG// Accept function incorrect\n");
                    break;
                }

                printf("DEBUG// New connection accepted\n");
                FD_SET(ComSocket, &readSet);
            }
            else
            {
                // Existing client data
                SOCKET currentSocket = tempSet.fd_array[i];
                uint8_t size = 0;

                result = tcp_recv_whole(currentSocket, (char*)&size, 1);
                if ((result == SOCKET_ERROR) || (result == 0))
                {
                    printf("DEBUG// recv is incorrect\n");
                    FD_CLR(currentSocket, &readSet);
                    closesocket(currentSocket);
                    break;
                }

                char* buffer = new char[size];

                result = tcp_recv_whole(currentSocket, buffer, size);
                if ((result == SOCKET_ERROR) || (result == 0))
                {
                    printf("DEBUG// recv is incorrect\n");
                    FD_CLR(currentSocket, &readSet);
                    closesocket(currentSocket);
                    delete[] buffer;
                    break;
                }

                printf("DEBUG// Received a message from a client\n");

                printf("\n\n");
                printf(buffer);
                printf("\n\n");

                delete[] buffer;
            }
        }
    }

    // close both sockets
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
}
