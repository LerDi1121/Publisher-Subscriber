#include "PubSubEngineFunctions.h"

CRITICAL_SECTION cs;
char* msg_queue;
node_t* listThread = NULL;
node_subscriber_t* listAnalog = NULL;
node_subscriber_t* listStatus = NULL;

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

///primanje poruke suba i slanje na njega
DWORD WINAPI RcvMessageFromSub(LPVOID param)
{
	SOCKET acceptedSocket = *((SOCKET*)param);

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	FD_ZERO(&set);
	while (true)
	{
		FD_SET(acceptedSocket, &set);
		int iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			//printf("SOCKET_ERROR");
			continue;
		}
		else if (iResult != 0) {
			if (FD_ISSET(acceptedSocket, &set)) {
				char someBuff[4];
				int iResult = recv(acceptedSocket, someBuff, 4, 0);
				if (iResult > 0)
				{
					break;//connect
				}
				else if (iResult == 0)
				{
					printf("Connection with client closed.\n");
					closesocket(acceptedSocket);
				}
				else
				{
					printf("recv failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket);
				}
			}
		}
		Sleep(500);
	}
	while (true)//subscribe
	{
		FD_SET(acceptedSocket, &set);
		subscriber_t* sub;
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
					int* msg = (int*)someBuff;
					sub = CreateSubscriber(acceptedSocket, *msg);
					AddSubscriberToList(&sub);
				}
				else if (iResult == 0)
				{
					printf("Connection with client closed.\n");
					closesocket(acceptedSocket);
				}
				else
				{
					printf("recv failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket);
				}
			}
		}
		Sleep(500);
	}
	while (true)
	{
		//slanje poruke na Sub
		printf("Slanje poruke na Suba ****\n ");
		Sleep(5000);
	}
}

///slusanje za subove
DWORD WINAPI ListenSubscriber(LPVOID param)
{
	SOCKET subscriberListenSocket = *((SOCKET*)param);
	int iResult = listen(subscriberListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(subscriberListenSocket);
		WSACleanup();
		return 1;
	}
	unsigned long int temp = 1;
	iResult = ioctlsocket(subscriberListenSocket, FIONBIO, &temp);

	printf("Server initialized, waiting for SUBSCRIBER.\n");

	FD_SET setSub;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	SOCKET subscriberAcceptedSocket = INVALID_SOCKET;
	do
	{
		FD_ZERO(&setSub);
		FD_SET(subscriberListenSocket, &setSub);
		iResult = select(0 /* ignored */, &setSub, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			continue;
		}
		else if (iResult != 0) {
			if (FD_ISSET(subscriberListenSocket, &setSub)) {
				subscriberAcceptedSocket = *CreateAcceptSocket(subscriberListenSocket);

				DWORD print1ID;
				HANDLE Thread;
				printf("Pravljenje treda Za suba\n");

				Thread = CreateThread(NULL, 0, &RcvMessageFromSub, &subscriberAcceptedSocket, 0, &print1ID);
				AddToList(&listThread, Thread);

				//	break;
			}
		}
		Sleep(1000);
	} while (true);
}
/// primanje poruke sa puba
DWORD WINAPI RcvMessage(LPVOID param)
{
	//SOCKET acceptedSocket = *((SOCKET *)param);
	data_for_thread temp = *((data_for_thread*)param);
	SOCKET acceptedSocket = temp.socket;
	char* msgQueue = *(temp.msgQueue);
	FD_SET set;
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
					iResult = recv(acceptedSocket, Poruka, *velicinaPor, 0);
					if (iResult > 0)
					{
						Poruka[iResult] = '\0';

						//EnterCriticalSection(&cs);
					//	Enqueue(&msg_queue, Poruka, *velicinaPor);
						//LeaveCriticalSection(&cs);
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
					printf("Connection with client closed.\n");
					closesocket(acceptedSocket);
					return false;
				}
				else
				{
					printf(" 2recv failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket);
					return false;
				}
			}
		}
	}
	//return true;
}

SOCKET* CreateAcceptSocket(SOCKET  listenSocket)
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

void AddSubscriberToList(subscriber_t** sub)
{
	if ((*sub)->topic == 1) {
		AddToConcreteList(&listAnalog, sub);
	}
	if ((*sub)->topic == 2) {
		AddToConcreteList(&listStatus, sub);
	}
	if ((*sub)->topic == 3) {
		AddToConcreteList(&listAnalog, sub);
		AddToConcreteList(&listStatus, sub);
	}
}

void AddToConcreteList(node_subscriber_t** list, subscriber_t** sub) {
	if ((*list) == NULL)
	{
		(*list) = (node_subscriber_t*)malloc(sizeof(node_subscriber_t));
		(*list)->subscriber = sub;
		(*list)->next = NULL;
	}
	else
	{
		node_subscriber_t* current = (*list);
		while (current->next != NULL) {
			current = current->next;
		}

		current->next = (node_subscriber_t*)malloc(sizeof(node_subscriber_t));;
		current = current->next;
		current->subscriber = sub;
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
void CreateQueue(char** msgQueue)
{
	*msgQueue = NULL;
	*msgQueue = ((char*)malloc(520));//brojac slobodnih(4) brojac zauzetih(4) i poruka 512
	int min = 0;
	int max = 512;
	memcpy(msgQueue, &min, 4);
	memcpy(msgQueue + 4, &max, 4);
}
void Enqueue(char** queue, char* msg, int msg_size) {
	int* lenght = (int*)(*queue);
	int* ukupno = (int*)((*queue) + 4);
	if (*lenght + msg_size > * ukupno)
	{
		//alociraj novu memoriju
		char* newQueue = (char*)malloc((*ukupno) * 2);
		*ukupno *= 2;
		memcpy(newQueue, (*queue), *lenght + 8);
		free((*queue));
		(*queue) = newQueue;
		printf("\n nova memorija  ***********\n");

		int* lenght = (int*)(*queue);
		int* ukupno = (int*)((*queue) + 4);
		char* message_for_queue = (char*)malloc(msg_size + 4);
		memcpy(message_for_queue, &msg_size, 4);
		memcpy(message_for_queue + 4, msg, msg_size);

		memcpy((*queue) + (*lenght) + 8, message_for_queue, msg_size + 4);
		*lenght += (msg_size + 4);
		printf("%d\n", *lenght);
	}
	else
	{
		char* message_for_queue = (char*)malloc(msg_size + 4);
		memcpy(message_for_queue, &msg_size, 4);
		memcpy(message_for_queue + 4, msg, msg_size);

		memcpy((*queue) + (*lenght) + 8, message_for_queue, msg_size + 4);
		*lenght += (msg_size + 4);
		printf("%d\n", *lenght);
	}
}
SOCKET* CreatePublisherListenSocket()
{
	SOCKET* listenSocketRetVal = (SOCKET*)malloc(sizeof(SOCKET));

	addrinfo* resultingAddress = NULL;

	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     //

	// Resolve the server address and port
	int iResult = getaddrinfo(NULL, DEFAULT_PORT_FOR_PUB, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		//return 1;
	}

	// Create a SOCKET for connecting to server
	*listenSocketRetVal = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (*listenSocketRetVal == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		//return ;
	}

	// Setup the TCP listening socket - bind port number and local address
	// to socket
	iResult = bind(*listenSocketRetVal, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(*listenSocketRetVal);
		WSACleanup();
		//return 1;
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(*listenSocketRetVal, FIONBIO, &nonBlockingMode);// ******omoguciti  ne blokirajuci rezim
	freeaddrinfo(resultingAddress);
	return listenSocketRetVal;
}
SOCKET* CreateSubscriberListenSocket()
{
	SOCKET* listenSocketRetVal = (SOCKET*)malloc(sizeof(SOCKET));

	addrinfo* resultingAddress = NULL;

	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     //

	// Resolve the server address and port
	int iResult = getaddrinfo(NULL, DEFAULT_PORT_FOR_SUB, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		//return 1;
	}

	// Create a SOCKET for connecting to server
	*listenSocketRetVal = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (*listenSocketRetVal == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		//return ;
	}

	// Setup the TCP listening socket - bind port number and local address
	// to socket
	iResult = bind(*listenSocketRetVal, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(*listenSocketRetVal);
		WSACleanup();
		//return 1;
	}
	freeaddrinfo(resultingAddress);
	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(*listenSocketRetVal, FIONBIO, &nonBlockingMode);// ******omoguciti  ne blokirajuci rezim

	return listenSocketRetVal;
}
subscriber_t* CreateSubscriber(SOCKET socket, int topic) {
	subscriber_t* temp = (subscriber_t*)malloc(sizeof(subscriber_t));
	temp->socket = socket;
	temp->topic = topic;
	temp->queue = NULL;
	CreateQueue(&(temp->queue));
	return temp;
}