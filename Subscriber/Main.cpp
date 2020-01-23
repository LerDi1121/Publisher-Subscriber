#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512

#define DEFAULT_PORT_FOR_PUB_SUB_ENG_SEND 27017

bool InitializeWindowsSockets();
bool Connect(SOCKET subscribeSocket);
bool Subscribe(SOCKET subscribeSocket);



int  main(int argc, char** argv)
{
	
	SOCKET connectSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	if (argc != 2)
	{
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	if (InitializeWindowsSockets() == false)
	{

		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddress.sin_port = htons(DEFAULT_PORT_FOR_PUB_SUB_ENG_SEND);

	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)& serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}
	if (Connect(connectSocket))
		Subscribe(connectSocket);
		while (1) {

			Sleep(1000);
		}
	// cleanup

	closesocket(connectSocket);
	WSACleanup();

	return 0;
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}
bool Connect(SOCKET subscribeSocket)
{
	int por = 1;

	int iResult = send(subscribeSocket, (char *)(&por), 4,0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(subscribeSocket);
		return false;
	}
	return true;
}
bool Subscribe (SOCKET subscribeSocket)
{

	int por = 2;

	int iResult = send(subscribeSocket, (char *)(&por), 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(subscribeSocket);
		return false;
	}
	return true;
}




