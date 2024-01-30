#include <iostream>
#include <thread>
#include <map>
#ifndef CNE Project
#define CNE Project
#include <vector>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
using namespace std;

map<string, SOCKET> activeClients;
map<string, string> userCredentials;
vector<string> publicMessages;

int tcp_recv_whole(SOCKET s, char* buf, int len);
int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
void ServerCode(int port, int capacity, char commandChar);
void HandleCommand(const string& command, SOCKET clientSocket);
void ProcessLogin(const string& username, const string& password, SOCKET clientSocket);
void BroadcastMessage(const string& message, SOCKET senderSocket);
void SendClientList(SOCKET clientSocket);
void SavePublicMessage(const string& message);

const int MAX_CLIENTS = 5;

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

void ServerCode(int port, int capacity, char commandChar)
{
	//socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) { cout << "Socket not created." << endl; return; }

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

	//connection
	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
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

void HandleCommand(const string& command, SOCKET clientSocket)
{
	if (command.empty()) return;

	char commandChar = command[0];
	string args = command.substr(1);
	switch (commandChar)
	{
	case '~':
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
				void ProcessLogin(const string & username, const string & password, SOCKET clientSocket);
			}
		}
		else if (args.find("send") == 0)
		{
			void SendClientList(SOCKET clientSocket);
		}
		else if (args.find("getlist") == 0)
		{
			// ~getlist .
		}
		else if (args.find("logout") == 0)
		{
			// ~logout
		}
		else
		{
			
		}
		break;
	default:
		BroadcastMessage(command, clientSocket); break;
	}
}

void ProcessLogin(const string& username, const string& password, SOCKET clientSocket)
{
}

void BroadcastMessage(const string& message, SOCKET senderSocket)
{
}

void SendClientList(SOCKET clientSocket)
{
}

void SavePublicMessage(const string& message)
{
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
	if (wsOk != 0) { cout << "Winstock not initialize." << endl; return; }
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
		cout << "Port: ";
		cin >> port;
		cout << "Capacity: ";
		cin >> capacity;
		cout << "Command character: ";
		cin >> commandChar;
		ServerCode(port, capacity, commandChar);
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
#endif CNE Project