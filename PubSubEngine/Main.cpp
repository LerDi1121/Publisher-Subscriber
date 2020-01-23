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
char * msg_queue;

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT_FOR_SUB "27017"
#define DEFAULT_PORT_FOR_PUB "27016"
#define DEFAULT_PORT_SUBSCRIBER 27018

typedef struct Subscriber {
	SOCKET Socket;
	char** queue;
	Topic Topics[2];
} Subscriber;


typedef struct message_queue {
	char* message;
	struct message_queue* next;
} message_queue_t;

typedef struct node {
	HANDLE value;
	struct node* next;
} node_t;

typedef struct node_t_socket {
	SOCKET value;
	struct node_t_socket* next;
} node_t_socket;

typedef struct data_for_thread {
	SOCKET socket;
	char ** msgQueue;

}data_for_thread;

bool InitializeWindowsSockets();
DWORD WINAPI RcvMessage(LPVOID param);
void AddToList(node_t** head, HANDLE value);
void AddSocketToList(node_t_socket** head, SOCKET value);
SOCKET* CreateAceptSocket(SOCKET Listen);
void Enqueue(char** queue, char* msg, int msg_size);
void CreateQueue();

int  main(int argc, char** argv)
{
	node_t* listThread = NULL;
	node_t_socket* listSockets = NULL;

	SOCKET listenSocketForPub = INVALID_SOCKET;

	SOCKET acceptedSocket = INVALID_SOCKET;

    //char* msg_queue = NULL;

	//message_queue_t* msg_queue = NULL;
	CreateQueue();

	int iResult;
	// Buffer used for storing incoming data
	//char recvbuf[DEFAULT_BUFLEN];

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Prepare address information structures
	addrinfo* resultingAddress = NULL;
	addrinfo* resultingAddress2 = NULL;
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
	iResult = ioctlsocket(acceptedSocket, FIONBIO, (u_long*)1);

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
		else if (iResult != 0) {
			if (FD_ISSET(listenSocketForPub, &set)) {
				acceptedSocket = *CreateAceptSocket(listenSocketForPub);
				//AddSocketToList(&listSockets, acceptedSocket);
				DWORD print1ID;
				HANDLE Thread;
				data_for_thread  temp=*( (data_for_thread*) malloc(sizeof(data_for_thread)));
				temp.socket = acceptedSocket;
				temp.msgQueue = &msg_queue;
				
				printf("Pravljenje treda\n");
			
			//	Thread = CreateThread(NULL, 0, &RcvMessage, &acceptedSocket, 0, &print1ID);
				Thread = CreateThread(NULL, 0, &RcvMessage, &temp, 0, &print1ID);
				AddToList(&listThread, Thread);

				//	break;
			}
		}
		///
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

DWORD WINAPI RcvMessage(LPVOID param)
{
	//SOCKET acceptedSocket = *((SOCKET *)param);
	data_for_thread temp = *((data_for_thread *)param);
	SOCKET acceptedSocket = temp.socket;
	char * msgQueue= *(temp.msgQueue);

	FD_SET set;
//	FD_SET setSub;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	FD_ZERO(&set);
	while (true)
	{
		FD_SET(acceptedSocket, &set);

		int iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);

		if (iResult == SOCKET_ERROR) {
			printf("SOCKET_ERROR");
			continue;
		}
		else if (iResult != 0) {
			if (FD_ISSET(acceptedSocket, &set)) {
			

				char someBuff[4];
				int iResult = recv(acceptedSocket, someBuff, 4, 0);
				if (iResult > 0)
				{
					int* velicinaPor = (int*)someBuff;

					char* Poruka = (char*)malloc(*velicinaPor);
					bool temp = true;
				//	printf("klinet zeli da posalje : %d.\n", *velicinaPor);
					iResult = recv(acceptedSocket, Poruka, *velicinaPor, 0);
					if (iResult > 0)
					{
						char * start = Poruka;
						Topic t = (Topic) * ((int*)Poruka);

						TypeTopic tt = (TypeTopic) * ((int*)(Poruka + 4));

						int MessSize = *velicinaPor - (sizeof(Topic) + sizeof(TypeTopic));
						char* Message = (char*)malloc(MessSize);
						Poruka = Poruka + sizeof(Topic) + sizeof(TypeTopic);

						memcpy(Message, (void*)Poruka, MessSize);

						Message[MessSize] = '\0';

						Enqueue(&msgQueue, start, *velicinaPor);

					/*	printf("klinet je poslao  : %s.\n", Message);
						printf("Topic : %d \n", t);
						printf("Topic Type : %d\n ", tt);*/
					}
					else if (iResult == 0)
					{
						printf("Connection with client closed.\n");
						closesocket(acceptedSocket);
						return false;
					}
					else
					{
						printf("1 recv failed with error: %d\n", WSAGetLastError());

						closesocket(acceptedSocket);
						return false;
					}
				}
				else if (iResult == 0)
				{
					// connection was closed gracefully
					printf("Connection with client closed.\n");
					closesocket(acceptedSocket);
					return false;
				}
				else
				{
					// there was an error during recv
					printf(" 2recv failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket);
					return false;
				}
			}
		}
	}
	//return true;
}
SOCKET* CreateAceptSocket(SOCKET  listenSocket)
{
	SOCKET* acceptedSocket = (SOCKET*)malloc(sizeof(SOCKET));
	*acceptedSocket = accept(listenSocket, NULL, NULL);
	/* kada dobijemo zahtev onda pravimo accepted socket*/
	if (*acceptedSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
	}

	unsigned long int nonBlockingMode = 1;
	int iResult = ioctlsocket(*acceptedSocket, FIONBIO, &nonBlockingMode);
	return acceptedSocket;
}

void AddSocketToList(node_t_socket** head, SOCKET value)
{
	if ((*head) == NULL)
	{
		(*head) = (node_t_socket*)malloc(sizeof(node_t_socket));
		(*head)->value = value;
		(*head)->next = NULL;
	}
	else
	{
		node_t_socket* current = (*head);
		while (current->next != NULL) {
			current = current->next;
		}

		current->next = (node_t_socket*)malloc(sizeof(node_t_socket));;
		current = current->next;
		current->value = value;
		current->next = NULL;
	}
}
void AddToList(node_t** head, HANDLE value)
{
	if ((*head) == NULL)
	{
		(*head) = (node_t*)malloc(sizeof(node_t));
		(*head)->value = value;
		(*head)->next = NULL;
	}
	else
	{
		node_t* current = (*head);
		while (current->next != NULL) {
			current = current->next;
		}

		current->next = (node_t*)malloc(sizeof(node_t));;
		current = current->next;
		current->value = value;
		current->next = NULL;
	}
}
void CreateQueue()
{
	
	msg_queue = NULL;
	msg_queue = (char *)malloc(520);//brojac slobodnih(4) brojac zauzetih(4) i poruka 512
	int min = 0;
	int max = 512;
	memcpy(msg_queue, &min, 4);
	memcpy(msg_queue+4, &max, 4);

		


}

void Enqueue(char ** queue, char* msg, int msg_size) {
	int *lenght =(int *)(*queue);
	int *ukupno = (int* )((*queue) +4);
	if (*lenght + msg_size > *ukupno)
	{
		//alociraj novu memoriju
		char* newQueue = (char*)malloc((*ukupno) * 2);
		*ukupno *= 2;
		memcpy(newQueue, (*queue), *lenght);
		free((*queue));
		(*queue) = newQueue;
		printf("\n nova memorija  ***********\n");

		int *lenght = (int *)(*queue);
		int *ukupno = (int*)((*queue) + 4);
		char*message_for_queue = (char*)malloc(msg_size + 4);
		memcpy(message_for_queue, &msg_size, 4);
		memcpy(message_for_queue + 4, msg, msg_size);

		memcpy((*queue) + (*lenght) + 8, message_for_queue, msg_size + 4);
		*lenght += (msg_size + 4);
		printf("%d\n", *lenght);

		
	}
	else
	{
		char*message_for_queue = (char*)malloc(msg_size + 4);
		memcpy(message_for_queue, &msg_size, 4);
		memcpy(message_for_queue+4, msg, msg_size);
		
		memcpy((*queue) +(*lenght)+8,message_for_queue,msg_size+4);
		*lenght += (msg_size + 4);
		printf("%d\n", *lenght);

	}
	/*message_queue_t* new_node;
	new_node = (message_queue_t*)malloc(sizeof(message_queue_t));

	new_node->message = (char*)malloc(msg_size);
	memcpy(new_node->message, (void*)msg, msg_size);

	new_node->next = *head;
	*head = new_node;*/
}