#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "..\common\AllEnums.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();
bool Connect(SOCKET conSoc );
bool Publish(void *topic, void * type, const char * message, SOCKET conSoc);

int __cdecl main(int argc, char **argv)
{

	// socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;

	const char *messageToSend = "this is a test";

	// Validate the parameters
	if (argc != 2)
	{
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
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
	serverAddress.sin_port = htons(DEFAULT_PORT);

	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}
//if (Connect(connectSocket))
		while (1) {
			Topic t =Status;
			TypeTopic tst = MER;

			
			Publish((void *) t, (void *) tst, messageToSend, connectSocket);

			Sleep(1000);
		}
//else
		printf(" Greska prilikom konektovanja");
	// cleanup
	closesocket(connectSocket);
	WSACleanup();
	getchar();

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
bool Publish(void *topic, void * type, const char * message, SOCKET conSoc)
{
	int TopicSize = sizeof(Topic);
	int TypeSize = sizeof(TypeTopic);

	int duzinaUkupnePoruke = strlen(message) + TopicSize+ TypeSize;
	int duzinaTexta = strlen(message);

	char *poruka = (char *)malloc(duzinaUkupnePoruke);
	memcpy(poruka, &duzinaUkupnePoruke, 4);
	memcpy(poruka +4, &topic, TopicSize);
	memcpy(poruka + 4+ TopicSize, &type, TypeSize);
	memcpy(poruka + 4+ TopicSize+ TypeSize, message, duzinaTexta);
//	memcpy(poruka + 4, message, duzina);
	int iResult = send(conSoc, poruka, duzinaUkupnePoruke+4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(conSoc);
		return false;
	}

	printf("Bytes Sent: %ld\n", iResult);
	return true;

}
bool Connect(SOCKET conSoc)
{
	// Send an prepared message with null terminator included'
	const char *messageToSend = "Connect Publisher";
	int duzina = strlen(messageToSend);
	char *poruka = (char *)malloc(duzina + 4);
	memcpy(poruka, &duzina, 4);
	memcpy(poruka + 4, messageToSend, duzina);
	int iResult = send(conSoc, poruka, duzina + 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(conSoc);
		return false;
	}
	return true;

}

