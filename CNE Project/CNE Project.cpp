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

SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
SOCKET ComSocket = accept(listenSocket, NULL, NULL);
unordered_map<string, string> userCredentials;
const int MAX_CLIENTS = 5;
WSADATA wsaData;
SOCKET udpSocket;
sockaddr_in broadcastAddr;
thread udpBroadcastThread;
bool udpBroadcastRunning;
fd_set masterSet;  //Set sockets
fd_set readySet;   //Sockets ready
int maxSocket;
void ServerCode(int port, int capacity, char commandChar, int udpPort);
int tcp_recv_whole(SOCKET s, char* buf, int len);
void readMessage(char* buffer, int32_t size);
void sendMessage(char* data, int32_t length);
string GetHelpMessage();
int RegisterUser(const string& username, const string& password);
int HandleCommand(const string& command);
//int ProcessLogin(const string& username, const string& password, SOCKET clientSocket);
//int BroadcastMessage(const string& message, SOCKET senderSocket);
//int SendClientList(SOCKET clientSocket);
//int SavePublicMessage(const string& message);
int initUDP(int udpPort);
void startUDPBroadcast();
void stopUDPBroadcast();
void stop();

int tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;
	do
	{
		int ret = recv(s, buf + total, len - total, 0);
		if (ret <= 0)
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

void sendMessage(char* data, int32_t length)
{
	//Communication
	uint8_t recvSize = static_cast<uint8_t>(length);

	int result = tcp_recv_whole(ComSocket, (char*)data, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnected");
	}
	char* buffer = new char[length];

	result = tcp_recv_whole(ComSocket, (char*)buffer, length);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnected");
	}
	delete[] buffer;
	printf("Success");
}
void readMessage(char* buffer, int32_t size)
{
	uint8_t recvSize = 0;
	SOCKET ComSocket = accept(listenSocket, NULL, NULL);
	int result = tcp_recv_whole(ComSocket, (char*)size, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnected");
	}
	buffer = new char[size];

	result = tcp_recv_whole(ComSocket, (char*)buffer, size);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("Disconnected");
	}
	delete[] buffer;
	printf("Success");
}


string GetHelpMessage()
{
	string helpMessage = "Available commands:\n";
	helpMessage += "~help - Display available commands\n";
	helpMessage += "~register - username password - Register a new user\n";
	helpMessage += "~login - username password - Login with user account\n";
	helpMessage += "~send - Display available commands\n";
	helpMessage += "~getlist - Display available commands\n";
	helpMessage += "~logout - Logs out of user account\n";

	return helpMessage;
}
int RegisterUser(const string& username, const string& password)
{
	if (userCredentials.size() >= MAX_CLIENTS)
	{
		string CAP_REACHED = "Max Client Capacity.";
		int CAPACITY_REACHED = stoi(CAP_REACHED);
		return CAPACITY_REACHED;
	}

	// Check if the username is already taken
	if (userCredentials.find(username) != userCredentials.end())
	{
		string USER_TAKEN = "Max Client Capacity.";
		int USERNAME_TAKEN = stoi(USER_TAKEN);
		return USERNAME_TAKEN;
	}

	// Register the user
	userCredentials[username] = password;
	printf("Success");
}
int ProcessLogin(const string& username, const string& password, SOCKET clientSocket)
{
	return 0;
}
int BroadcastMessage(const string& message, SOCKET senderSocket)
{
	return 0;
}
int SendClientList(SOCKET clientSocket)
{
	return 0;
}
int SavePublicMessage(const string& message)
{
	return 0;
}

int initUDP(int udpPort) {
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET) {
		cout << "UDP Socket creation failed." << endl;
		return 0;
	}

	//make broadcast address 
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));
	broadcastAddr.sin_family = AF_INET;
	broadcastAddr.sin_port = htons(udpPort);
	broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	return 1;
}
void startUDPBroadcast()
{

}

void ServerCode(int port, int capacity, char commandChar, int udpPort)
{
	//Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "WSAStartup failed." << endl; return;
	}
	//udp
	if (!initUDP(udpPort)) { cout << "UDP initialization failed." << endl; }

	//socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) { cout << "Socket not created." << endl; WSACleanup(); printf("Success"); }

	//bind
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(listening, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "Bind failed." << endl;
		closesocket(listening);
		WSACleanup();
		return;
	}

	//listen
	listen(listening, SOMAXCONN);
	cout << "Server listening: " << port << endl;
	FD_ZERO(&masterSet); 
	FD_SET(listenSocket, &masterSet);  
	FD_SET(udpSocket, &masterSet); 
	maxSocket = max(listenSocket, udpSocket); 

	//connection
	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Accept failed." << endl; 
		closesocket(listening); 
		WSACleanup(); 
		return;
	}
	char host[NI_MAXHOST];
	char service[NI_MAXHOST];
	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connect to port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connect to port " << ntohs(client.sin_port) << endl;
	}

	//close listen
	closesocket(listening);

	//loop
	char buff[4096];
	while (true)
	{
		ZeroMemory(buff, 4096);
		//client send message
		int byteRecv = recv(clientSocket, buff, 4096, 0);
		if (byteRecv == SOCKET_ERROR) { cout << "Recive Error" << endl; }
		if (byteRecv == 0) { cout << "Disconnected" << endl; break; }
		send(clientSocket, buff, byteRecv * 1, 0);
	}

	//close sock
	closesocket(clientSocket);
}

int HandleCommand(const string& command)
{
	if (command.empty()) return 0;
	char commandChar = command[0];
	string args = command.substr(1);
	switch (commandChar)
	{
	case '~':

		if (args.find("help") != string::npos) {
			string helpMessage = GetHelpMessage();
			const char* helpChar = helpMessage.c_str();
			delete[] helpChar;
		}
		if (args.find("register") == 0)
		{
			// ~register
			size_t spacePos = args.find(' ');
			if (spacePos != string::npos)
			{
				string username = args.substr(9, spacePos - 9);
				string password = args.substr(spacePos + 1);
			}
		}
		else if (args.find("login") == 0)
		{
			//~login
			size_t spacePos = args.find(' ');
			if (spacePos != string::npos)
			{
				string username = args.substr(6, spacePos - 6);
				string password = args.substr(spacePos + 1);
				int ProcessLogin(const string & username, const string & password, SOCKET clientSocket);
			}
		}
		else if (args.find("send") == 0)
		{
			size_t spacePos = args.find(' ');

			int SendClientList(SOCKET clientSocket);
		}
		else if (args.find("getlist") == 0)
		{
			size_t spacePos = args.find(' ');

			// ~getlist
		}
		else if (args.find("logout") == 0)
		{
			size_t spacePos = args.find(' ');

			// ~logout
		}
		else
		{
			printf("Please enter a command");
		}
		break;
	default:
		BroadcastMessage(command, ComSocket); break;
	}
}
void serverRun(int port, int capacity, char commandChar, int udpPort)
{
	char buffer[4096];
	bool serverActive=true;
	while (serverActive)
	{
		//masterSet to readySet
		readySet = masterSet;

		//find ready sockets
		int socketCount = select(0, &readySet, nullptr, nullptr, nullptr);
		if (socketCount == SOCKET_ERROR)
		{
			cerr << "Select failed." << endl;
			break;
		}
		for (int i = 0; i < maxSocket; ++i)
		{
			//socket is in readySet
			if (FD_ISSET(i, &readySet))
			{
				//Check if it's the listening socket
				if (i == listenSocket)
				{
					sockaddr_in clientAddr;
					int clientSize = sizeof(clientAddr);
					SOCKET newSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientSize);

					if (newSocket != INVALID_SOCKET)
					{
						// Add new connection to masterSet
						FD_SET(newSocket, &masterSet);

						// Update maxSocket
						if (newSocket > maxSocket)
						{
							maxSocket = newSocket;
						}

						cout << "New connection accepted." << endl;
					}
					else
					{
						cerr << "Accept failed." << endl;
					}
				}
				else
				{
					//Read from client socket
					char buffer[4096];
					ZeroMemory(buffer, sizeof(buffer));
					int bytesRead = recv(i, buffer, sizeof(buffer), 0);

					if (bytesRead <= 0)
					{
						if (bytesRead == 0)
						{
							cout << "Client disconnected." << endl;
						}
						else
						{
							cerr << "Recv error." << endl;
						}

						// Remove socket from masterSet
						FD_CLR(i, &masterSet);
						closesocket(i);
					}
					else
					{
						cout << "Received client data: " << buffer << endl;
					}
				}
			}
		}
	}
	//stop server
	serverActive = false;

	//Close sockets
	for (int i = 0; i <= maxSocket; ++i)
	{
		if (FD_ISSET(i, &masterSet))
		{
			if (i == listenSocket || i == udpSocket)
			{
				// Handle the listening and UDP sockets separately if needed
				continue;
			}

			int bytesRead = recv(i, buffer, sizeof(buffer), 0);
			if (bytesRead <= 0)
			{
				if (bytesRead == 0)
				{
					cout << "Client disconnected." << endl;
				}
				else
				{
					cerr << "Recv error." << endl;
				}

				// Remove socket from masterSet
				FD_CLR(i, &masterSet);
				closesocket(i);
			}
			else
			{
				cout << "Received client data: " << buffer << endl;
			}
		}
	}
}

void ClientCode(void)
{

	//socket
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) { cout << "Socket not created." << endl; return; }

	//bind
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(36000);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	//bind(listening, (sockaddr*)&hint, sizeof(hint));

	//listen
	//listen(listening, SOMAXCONN);

	//connection
	if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "Unable to connect to the server." << endl;
		closesocket(clientSocket);
		return;
	}
	cout << "Connect Server" << endl;

	//close listen
	//closesocket(listening);

	//loop
	char buff[4096];
	while (true)
	{
		ZeroMemory(buff, 4096);
		//client send message
		cout << "Enter a message: ";
		cin.getline(buff, sizeof(buff));
		int byteSent = send(clientSocket, buff, strlen(buff), 0);
		if (byteSent == SOCKET_ERROR) { cout << "Send error." << endl; break; }
		int byteRecv = recv(clientSocket, buff, sizeof(buff), 0);
		if (byteRecv == SOCKET_ERROR) { cout << "Recive Error" << endl; }
		if (byteRecv == 0) { cout << "Disconnected" << endl; break; }
		//send(clientSocket, buff, byteRecv * 1, 0);
		cout << "Server: " << buff << endl;
	}

	//close sock
	closesocket(clientSocket);
}

int main()
{
	//winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) { cout << "Winstock not initialize." << endl; return -1; }
	int choice;
	do
	{
		printf("Would you like to Create a Server or Client?\n");
		printf("1> Server\n");
		printf("2> Client\n");
		cin >> choice;
	} while (choice != 1 && choice != 2);

	//Server
	if (choice == 1)
	{
		int port, capacity;
		char commandChar;
		int udpPort{};
		cout << "Port: ";
		cin >> port;
		cout << "Capacity: ";
		cin >> capacity;
		cout << "Command character: ";
		cin >> commandChar;
		ServerCode(port, capacity, commandChar, udpPort);
	}

	//Client
	else if (choice == 2)
	{
		ClientCode();
	}

	//shutdown winsock
	WSACleanup();
	return 0;
}
void stopUDPBroadcast()
{
	udpBroadcastRunning = false;
	udpBroadcastThread.join();
	closesocket(udpSocket);
}
void stop()
{
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);
	shutdown(ComSocket, SD_BOTH);
	closesocket(ComSocket);
	WSACleanup();
}