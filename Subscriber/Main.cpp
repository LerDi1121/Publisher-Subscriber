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
#define DEFAULT_PORT_FOR_PUB_SUB_ENG "27018"

bool InitializeWindowsSockets();

int  main(void)
{
	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;
;
	// Socket used for communication with client
	SOCKET acceptedSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	// Buffer used for storing incoming data
	char recvbuf[DEFAULT_BUFLEN];

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Prepare address information structures
	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT_FOR_PUB_SUB_ENG, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &nonBlockingMode);// ******omoguciti  ne blokirajuci rezim

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	iResult = ioctlsocket(acceptedSocket, FIONBIO, (u_long *)1);

	printf("Server initialized, waiting for clients.\n");

	//////////************napraviti fd setove
	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	do
	{
		// konektovanje 
		FD_ZERO(&set);

		FD_SET(listenSocket, &set);
		iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);

		if (iResult == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
		}
		else if (iResult != 0) {
			if (FD_ISSET(listenSocket, &set)) {
				acceptedSocket = accept(listenSocket, NULL, NULL);
				/* kada dobijemo zahtev onda pravimo accepted socket*/
				if (acceptedSocket == INVALID_SOCKET)
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
					closesocket(listenSocket);
					WSACleanup();
					return 1;
				}

				nonBlockingMode = 1;
				iResult = ioctlsocket(acceptedSocket, FIONBIO, &nonBlockingMode);
				//	break;
			}
		}
		///

		///prijem poruke

		FD_SET(acceptedSocket, &set);

		iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);

		if (iResult == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
		}
		else if (iResult != 0) {
			if (FD_ISSET(acceptedSocket, &set)) {
				char someBuff[4];
				iResult = recv(acceptedSocket, someBuff, 4, 0);
				if (iResult > 0)
				{
					int* velicinaPor = (int*)someBuff;

					char* Poruka = (char*)malloc(*velicinaPor);
					bool temp = true;
					printf("klinet zeli da posalje : %d.\n", *velicinaPor);
					iResult = recv(acceptedSocket, Poruka, *velicinaPor, 0);
					if (iResult > 0)
					{
						Poruka[iResult] = '\0';
						printf("pubSub engine je pposlao : %s.\n", Poruka);
					}
					else if (iResult == 0)
					{
						printf("Connection with client closed.\n");
						closesocket(acceptedSocket);
					}
					else
					{
						printf("1 recv failed with error: %d\n", WSAGetLastError());

						closesocket(acceptedSocket);
					}
					//Sleep(1022);



				}
				else if (iResult == 0)
				{
					// connection was closed gracefully
					printf("Connection with client closed.\n");
					closesocket(acceptedSocket);
				}
				else
				{
					// there was an error during recv
					printf(" 2recv failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket);
				}
			}
		}

	} while (1);


	// shutdown the connection since we're done
	iResult = shutdown(acceptedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(listenSocket);
	closesocket(acceptedSocket);
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
