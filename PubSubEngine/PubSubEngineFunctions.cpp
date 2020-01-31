#include "PubSubEngineFunctions.h"
int SubCount = 0;
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
	subscriber_t* sub;
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
					break;
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
		Sleep(5000);
		EnterCriticalSection(&cs);
		char * red = (sub->queue);
		if (red == NULL)
		{
			LeaveCriticalSection(&cs);
			continue;
		}
		
		sub->queue = NULL;
		CreateQueue(&(sub->queue));
		LeaveCriticalSection(&cs);
		int * size = (int *)red;
		char * messageForSend = (char *)malloc(*size + sizeof(int));
		memcpy(messageForSend, size, sizeof(int));
		memcpy(messageForSend+ sizeof(int), red+ sizeof(int)*2, *size);
		int sizeOfMsg = *size + sizeof(int);
		char* msgBegin = messageForSend;
		bool flag = FALSE;
		while (true) {
			
			int iResult = send(acceptedSocket, messageForSend, sizeOfMsg, 0);
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				flag = TRUE;
				break;
			}
			sizeOfMsg -= iResult;
			messageForSend += iResult;
			if (sizeOfMsg <= 0)
				break;
		}
		free(red);

		if (flag)
			break;
		printf("Slanje poruke na Suba ****\n ");
		free(msgBegin);
		Sleep(4000);
	}
	subscriber_t *temp = sub;
	sub = NULL;
	RemoveSubscriber(temp);
	free(temp);
	
	
	closesocket(acceptedSocket);
	return -1;
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
			closesocket(subscriberListenSocket);
			return INVALID_SOCKET;
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

void  WriteMessage(char* message)
{
	int* messageLength = (int*)message; // ukupna duzina poruke topic +type +text
	Topic t = (Topic) * ((int*)(message + 4));
	TypeTopic tt = (TypeTopic) * ((int*)(message + 8));
	message += 4;
	
	switch (t) {
	case 0:
	{
		DWORD print1ID;
		HANDLE Thread;
		data_for_thread *forAnalog = (data_for_thread*)malloc(sizeof(data_for_thread));
		forAnalog->list = &listAnalog;
		forAnalog->message = message;
		forAnalog->size = *messageLength;
		Thread = CreateThread(NULL, 0, &AddMessageToQueue, forAnalog, 0, &print1ID);
		AddToList(&listThread, Thread);
		break;
	}
	case 1:
	{
		DWORD print2ID;
		HANDLE Thread1;
		data_for_thread *forStatus = (data_for_thread*)malloc(sizeof(data_for_thread));
		forStatus->list = &listStatus;
		forStatus->message = message;
		forStatus->size = *messageLength;
		Thread1 = CreateThread(NULL, 0, &AddMessageToQueue, forStatus, 0, &print2ID);
		AddToList(&listThread, Thread1);

		break;
	}
	case 2:
	{
		//analog
		DWORD printAnalog;
		HANDLE ThreadAnalog;
		data_for_thread *forAnalog2 = (data_for_thread*)malloc(sizeof(data_for_thread));
		forAnalog2->list = &listAnalog;
		forAnalog2->message = message;
		forAnalog2->size = *messageLength;
		ThreadAnalog = CreateThread(NULL, 0, &AddMessageToQueue, forAnalog2, 0, &printAnalog);
		AddToList(&listThread, ThreadAnalog);
		//status 
		DWORD printStatus;
		HANDLE ThreadStatus;
		data_for_thread *forStatus2 = (data_for_thread*)malloc(sizeof(data_for_thread));
		forStatus2->list = &listStatus;
		forStatus2->message = message;
		forStatus2->size = *messageLength;
		ThreadStatus = CreateThread(NULL, 0, &AddMessageToQueue, forStatus2, 0, &printStatus);
		AddToList(&listThread, ThreadStatus);

		break;
	}
	}
}
/// primanje poruke sa puba
DWORD WINAPI RcvMessage(LPVOID param)
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
			closesocket(acceptedSocket);
			return -1 ;

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

						char* messageForQueue = (char*)malloc((*velicinaPor) + 4);
						memcpy(messageForQueue, velicinaPor, 4);
						memcpy(messageForQueue + 4, Poruka, *velicinaPor);
						WriteMessage(messageForQueue);
					
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
	if ((*sub)->topic == 0) {
		AddToConcreteList(&listAnalog, sub);
	}
	if ((*sub)->topic == 1) {
		AddToConcreteList(&listStatus, sub);
	}
	if ((*sub)->topic == 2) {
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
	memcpy(*msgQueue, &min, 4);
	memcpy(*msgQueue + 4, &max, 4);
}
char* Enqueue(char** queue, char* msg, int msg_size) {
	

	int* lenght = (int*)(*queue);
	int* max = (int*)((*queue) + 4);

	if (* lenght + msg_size >  *max)
	{
		 char* newQueue = (char*)malloc((*max) * 2);
		 *max *= 2;
		 memcpy(newQueue, (*queue), *lenght + 8);
		 free((*queue));
		 (*queue) = newQueue;

		printf("\n nova memorija  ***********\n");

		lenght = (int*)(*queue);
		max = (int*)((*queue) + 4);
		memcpy((*queue) + (*lenght) + sizeof(int) * 2, &msg_size, sizeof(int));
		*lenght +=sizeof(int);
		memcpy((*queue) + (*lenght) + sizeof(int) * 2, msg, msg_size);
		*lenght += msg_size;
		printf("%d\n", *lenght);
	}
	else
	{
		memcpy((*queue) + (*lenght) + sizeof(int) * 2, &msg_size, sizeof(int));
		*lenght += sizeof(int);
		memcpy((*queue) + (*lenght) + sizeof(int) * 2, msg, msg_size);
		*lenght += msg_size;
		printf("%d\n", *lenght);
	}

	return *queue;
}
SOCKET* CreatePublisherListenSocket()
{
	SOCKET* listenSocketRetVal = (SOCKET*)malloc(sizeof(SOCKET));
	SOCKET* invalidSocket= NULL;
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
		return invalidSocket;
	}

	*listenSocketRetVal = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (*listenSocketRetVal == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return invalidSocket;
	}

	iResult = bind(*listenSocketRetVal, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(*listenSocketRetVal);
		WSACleanup();
		return invalidSocket;
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(*listenSocketRetVal, FIONBIO, &nonBlockingMode);// ******omoguciti  ne blokirajuci rezim
	freeaddrinfo(resultingAddress);
	return listenSocketRetVal;
}
SOCKET* CreateSubscriberListenSocket()
{
	SOCKET* listenSocketRetVal = (SOCKET*)malloc(sizeof(SOCKET));
	SOCKET* invalidSocket = NULL;
	addrinfo* resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     //

	int iResult = getaddrinfo(NULL, DEFAULT_PORT_FOR_SUB, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return invalidSocket;
	}
	*listenSocketRetVal = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (*listenSocketRetVal == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return invalidSocket;
	}

	iResult = bind(*listenSocketRetVal, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(*listenSocketRetVal);
		WSACleanup();
		return invalidSocket;
	}
	freeaddrinfo(resultingAddress);
	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(*listenSocketRetVal, FIONBIO, &nonBlockingMode);// ******omoguciti  ne blokirajuci rezim

	return listenSocketRetVal;
}
void RemoveSubscriberFromList(int id, node_subscriber_t** list)
{
	node_subscriber_t* current = *list;

	node_subscriber_t* previous=NULL;
	subscriber * tempSub = *(current->subscriber);
	if (current != NULL && tempSub->id == id)
	{
		EnterCriticalSection(&cs);
		*list = current->next;
		free(current);
		LeaveCriticalSection(&cs);
		return;

	}
	while (current->next != NULL && tempSub->id != id) {

		previous = current;
		current = current->next;
		tempSub = *(current->subscriber);

	}
	if (current == NULL) return;

	EnterCriticalSection(&cs);
	previous->next = current->next;
	free(current);
	LeaveCriticalSection(&cs);
}
void RemoveSubscriber(subscriber_t* sub)
{
	if (sub->topic == 0)
	{
		RemoveSubscriberFromList(sub->id, &listAnalog);
	}
	else if (sub->topic == 1)
	{
		RemoveSubscriberFromList(sub->id, &listStatus);
	}
	else
	{
		RemoveSubscriberFromList(sub->id, &listStatus);
		RemoveSubscriberFromList(sub->id, &listAnalog);
	}
}

subscriber_t* CreateSubscriber(SOCKET socket, int topic) {
	subscriber_t* temp = (subscriber_t*)malloc(sizeof(subscriber_t));
	temp->socket = socket;
	temp->topic = topic;
	temp->queue = NULL;
	temp->id = SubCount++;
	CreateQueue(&(temp->queue));
	return temp;
}

DWORD WINAPI  AddMessageToQueue(LPVOID param) {
	data_for_thread *temp = ((data_for_thread*)param);
	
	if (*(temp->list) == NULL)
		//free(temp);
		return -1;
	node_subscriber_t* current = (*(temp->list));

	while (true) {
		if (*(current->subscriber )!= NULL)/////////////////////////*****************************************
		{
			char* queue = (*(current->subscriber))->queue;
			EnterCriticalSection(&cs);
			(*(current->subscriber))->queue=Enqueue(&queue, temp->message, temp->size);
			LeaveCriticalSection(&cs);
			if (current->next == NULL) {
				break;
			}
			else {
				current = current->next;
			}
		}
		else
		{
			current = current->next;
		}
	}
	//free( temp);
	return 1;
}
