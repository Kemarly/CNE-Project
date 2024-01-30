#include <iostream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
using namespace std;

int tcp_recv_whole(SOCKET s, char* buf, int len);
int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
void ServerCode(int port, int capacity, char commandChar);
void ClientCode();

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

void main()
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
}
void ServerCode(int port, int capacity, char commandChar)
{
	//socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) { cout << "Socket not created." << endl; return; }

	//bind
	/*sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(36000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(listening, (sockaddr*)&hint, sizeof(hint));*/
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(listening, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "Bind failed." << endl;
		closesocket(listening);
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