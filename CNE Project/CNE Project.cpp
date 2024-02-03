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
void BroadcastMessage(const string& message)
{
    // broadcast to all clients
    std::lock_guard<std::mutex> lock(clientMutex);
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    for (u_int i = 0; i < masterset.fd_count; ++i)
    {
        SOCKET currentSocket = masterset.fd_array[i];
        if (currentSocket != listenSocket)
        {
            int result = tcp_send_whole(currentSocket, message.c_str(), message.length());
            if (result <= 0)
            {
                cerr << "Failed to send message to client." << endl;
            }
        }
    }
}

void HandleCommand(const string& command)
{
    if (command.empty()) return;
    char commandChar = command[0];
    string args = command.substr(1);
    switch (commandChar)
    {
    case '~':
        if (args.find("help") != string::npos) {
            // ~help command
        }
        else if (args.find("register") == 0) {
            // ~register command
        }
        else if (args.find("login") == 0) {
            // ~login command
        }
        else if (args.find("send") == 0) {
            // ~send command
        }
        else if (args.find("getlist") == 0) {
            //~getlist command
        }
        else if (args.find("logout") == 0) {
            // ~logout command
        }
        else {
            // Unknown command
            cout << "Command Unavailible: " << command << endl;
        }
        break;
    default:
        BroadcastMessage(command);
        break;
    }
}

void HandleClient(SOCKET clientSocket, fd_set& readSet, string& command)
{
    const char* welcomeMessage = "Welcome to the Server!\n Please enter your commands starting with (~): ";
    tcp_send_whole(clientSocket, welcomeMessage, strlen(welcomeMessage));
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
            printf("  Client stopped connection\n");
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

            // Set the command variable with the received message
            command = string(buffer); 
        }
        delete[] buffer;
    }
}

void ServerCode(void)
{
    //Create listening socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        cerr << "Failed to create socket." << endl;
        return;
    }

    //server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    cout << "Enter TCP Port number: ";
    int port;
    cin >> port;
    serverAddr.sin_port = htons(port);

    cout << "Enter chat capacity (maximum number of clients): ";
    int maxClients;
    cin >> maxClients;

    cout << "Enter command character (default is ~): ";
    char commandChar;
    cin >> commandChar;

    //bind
    int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        cerr << "Failed to bind socket." << endl;
        closesocket(listenSocket);
        return;
    }

    //listening for incoming connections
    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cerr << "Failed to listen on socket." << endl;
        closesocket(listenSocket);
        return;
    }

    //Obtain server host IP using gethostname() and getaddrinfo()
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

    //multiplexing
    FD_ZERO(&masterset);
    FD_SET(listenSocket, &masterset);
    cout << "Waiting for connections..." << endl;

    while (true)
    {
        // Copy the master set to the ready set
        readyset = masterset;

        // Use select to wait for activity on the sockets
        int temp = select(0, &readyset, NULL, NULL, NULL);
        if (temp == SOCKET_ERROR) { cerr << "Select function failed: " << WSAGetLastError() << endl;  break; }

        // Check for new connections on the listening socket
        if (FD_ISSET(listenSocket, &readyset))
        {
            // Accept the new connection
            SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) { cerr << "Failed to accept connection." << endl; }
            else if (masterset.fd_count < FD_SETSIZE)
            {
                cout << "New connection accepted." << endl;

                // Send the welcome message
                const char* welcomeMessage = "Welcome to the Server!\n Please enter your commands starting with (~): ";
                tcp_send_whole(clientSocket, welcomeMessage, strlen(welcomeMessage));

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
            if (currentSocket != listenSocket && FD_ISSET(currentSocket, &readyset))
            {
                //Handle client message
                string command;
                HandleClient(currentSocket, masterset, command);
                HandleCommand(command);
            }
        }
    }
    //Close listening socket
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
}

int main()
{
    WSADATA wsadata;
    if (WSAStartup(WINSOCK_VERSION, &wsadata) != 0)
    {
        cerr << "Failed to initialize Winsock." << endl;
        return EXIT_FAILURE;
    }
    ServerCode();
    if (WSACleanup() != 0)
    {
        cerr << "Failed to clean up Winsock." << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
#define UDP_BROADCAST_PORT 12345 // Choose a  port number
#define UDP_BROADCAST_INTERVAL 10 // Interval in seconds

string serverIP; // server IP
int serverPort;
void UDPBroadcast()
{
    // Create a UDP socket
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        cerr << "Failed to create UDP socket." << endl;
        return;
    }

    // Enable broadcast option
    int broadcastOption = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcastOption, sizeof(broadcastOption)) == SOCKET_ERROR)
    {
        cerr << "Failed to enable broadcast option." << endl;
        closesocket(udpSocket);
        return;
    }

    // Construct the broadcast address structure
    sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(UDP_BROADCAST_PORT);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    // Compose the broadcast message
    string broadcastMessage = "Server IP: " + serverIP + ", Port: " + to_string(serverPort);

    while (true)
    {
        // Send the broadcast message
        if (sendto(udpSocket, broadcastMessage.c_str(), broadcastMessage.length(), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) == SOCKET_ERROR)
        {
            cerr << "Failed to send broadcast message." << endl;
        }

        // Sleep for x seconds before sending the next broadcast
        this_thread::sleep_for(chrono::seconds(UDP_BROADCAST_INTERVAL));
    }

    // Close the UDP socket
    closesocket(udpSocket);
}
