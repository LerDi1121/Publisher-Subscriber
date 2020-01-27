#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include  "..\common\AllEnums.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

#define DEFAULT_PORT_FOR_PUB_SUB_ENG_SEND 27017

void PrintMessage(char* msg, int totalSize);
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


	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	FD_ZERO(&set);

	char messageBufer[DEFAULT_BUFLEN];

	while (true)//subscribe
	{
		FD_SET(connectSocket, &set);
		char* message = NULL;
		int iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			printf("SOCKET_ERROR");
			continue;
		}
		else if (iResult != 0) {
			if (FD_ISSET(connectSocket, &set)) {
				char * someBuff = (char *)malloc(4);
				int* bufflen ;
				int iResult = recv(connectSocket, someBuff, 4, 0);
				if (iResult > 0)
				{
					bufflen = (int*)someBuff;
					int temp = *bufflen;
					message = (char*)malloc(*bufflen);
					char* mess = message;
				
					while (*bufflen > 0) {
						iResult = recv(connectSocket, messageBufer, DEFAULT_BUFLEN, 0);
						*bufflen -= iResult;
						memcpy(mess, messageBufer, iResult);
						mess += iResult;
					}
					PrintMessage(message, temp);
					free((void *) message);
				}
				else if (iResult == 0)
				{
					printf("Connection with client closed.\n");
					break;
				}
				else
				{
					printf("recv failed with error: %d\n", WSAGetLastError());
					break;
				}
			}
		}
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

	int iResult = send(subscribeSocket, (char*)(&por), 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(subscribeSocket);
		return false;
	}
	return true;
}
bool Subscribe(SOCKET subscribeSocket)
{
	int por = 1;

	int iResult = send(subscribeSocket, (char*)(&por), 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(subscribeSocket);
		return false;
	}
	return true;
}


void PrintMessage(char* msg, int totalSize) {
	char* message =msg;
	int* msgSize = (int*)msg;
	char* msge = NULL;
	
	while (totalSize >0) {
		msgSize = (int*)message;
		message += 4;
		Topic t = (Topic) * ((int*)(message ));
		TypeTopic tt = (TypeTopic) * ((int*)(message + 4));
		msge = (char*)malloc(*msgSize - 8);
		memcpy(msge, message+8, *msgSize - 8);
		msge[*msgSize - 8] = '\0';
		printf(" %s\n", msge);
		message += (*msgSize);
		totalSize -= (*msgSize + 4);
		//free(&msge);
		
	}
	printf("\n************ \n");
	
}