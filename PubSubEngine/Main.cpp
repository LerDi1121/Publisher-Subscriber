#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "EnumsEngine.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT_FOR_SUB "27017"
#define DEFAULT_PORT_FOR_PUB "27016"
#define DEFAULT_PORT_SUBSCRIBER 27018

bool InitializeWindowsSockets();

int  main(int argc, char **argv)
{
	

	SOCKET listenSocketForPub = INVALID_SOCKET;

	SOCKET acceptedSocket = INVALID_SOCKET;

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
	addrinfo *resultingAddress2 = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT_FOR_PUB, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	listenSocketForPub = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (listenSocketForPub == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(listenSocketForPub, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocketForPub);
		WSACleanup();
		return 1;
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(listenSocketForPub, FIONBIO, &nonBlockingMode);// ******omoguciti  ne blokirajuci rezim

	// Since we don't need resultingAddress any more, free it
	//freeaddrinfo(resultingAddress);

	// Set listenSocket in listening mode
	iResult = listen(listenSocketForPub, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocketForPub);
		WSACleanup();
		return 1;
	}
	iResult = ioctlsocket(acceptedSocket, FIONBIO, (u_long *)1);

	printf("Server initialized, waiting for clients.\n");

	//////////************napraviti fd setove
	FD_SET set;
	FD_SET setSub;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	/* primanje poruka*/
	do
	{
		// konektovanje 
		FD_ZERO(&set);
		FD_ZERO(&setSub);
		FD_SET(listenSocketForPub, &set);

		
		iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
		}
		else if(iResult!=0) {
			if (FD_ISSET(listenSocketForPub, &set)) {
				acceptedSocket = accept(listenSocketForPub, NULL, NULL);
				/* kada dobijemo zahtev onda pravimo accepted socket*/
				if (acceptedSocket == INVALID_SOCKET)
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
					closesocket(listenSocketForPub);
					WSACleanup();
					return 1;
				}

				nonBlockingMode = 1;
				iResult = ioctlsocket(acceptedSocket, FIONBIO, &nonBlockingMode);
			//	break;
			}
			
		}
		///


		FD_SET(acceptedSocket, &set);

		iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);

	
		if (iResult == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
		}
		else if(iResult!=0) {
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
							Topic t = (Topic ) * ((int *) Poruka) ;
							
							TypeTopic tt = (TypeTopic) *((int *) (Poruka +4));
							
							
							int MessSize = *velicinaPor - (sizeof(Topic) + sizeof(TypeTopic));
							char * Message = (char *)malloc(MessSize);
							Poruka = Poruka + sizeof(Topic) + sizeof(TypeTopic);

							memcpy(Message, (void*)Poruka ,MessSize );
				
							Message[MessSize] = '\0';

							printf("klinet je poslao  : %s.\n", Message);
							printf("Topic : %d", t);
							printf("Topic Type : %d", tt);
							

							/*
							////slanje poruke na sub
							SOCKET connectSocket = INVALID_SOCKET;
							connectSocket = socket(AF_INET,
								SOCK_STREAM,
								IPPROTO_TCP);

							if (connectSocket == INVALID_SOCKET)
							{
								printf("socket failed with error: %ld\n", WSAGetLastError());
								//WSACleanup();
								
							}

							// create and initialize address structure
							sockaddr_in serverAddress;
							serverAddress.sin_family = AF_INET;
							serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
							serverAddress.sin_port = htons(DEFAULT_PORT_SUBSCRIBER);
							// connect to server specified in serverAddress and socket connectSocket
							if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
							{
								
								printf("Unable to connect to server.\n");
								closesocket(connectSocket);
								//WSACleanup();
							}
					
							printf("USlanje poruke na sub.\n");
								int duzina = strlen(Poruka);
								char *poruka = (char *)malloc(duzina + 4);
								memcpy(poruka, &duzina, 4);
								memcpy(poruka + 4, Poruka, duzina);
								iResult = send(connectSocket, poruka, duzina + 4, 0);

								if (iResult == SOCKET_ERROR)
								{
									printf("send failed with error: %d\n", WSAGetLastError());
									closesocket(connectSocket);
									WSACleanup();
									return 1;
								}

								printf("Bytes Sent: %ld\n", iResult);
								Sleep(1000);
							
							// cleanup
							//closesocket(connectSocket);
							//WSACleanup();

								*/



							//// end slanje poruke na sub
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
	closesocket(listenSocketForPub);
	closesocket(acceptedSocket);
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
