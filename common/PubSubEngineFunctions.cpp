#include "PubSubEngineFunctions.h"
int SubCount = 0;
int ThreadCounter;
CRITICAL_SECTION cs;
char* msg_queue;
node_subscriber_t* listAnalog = NULL;
node_subscriber_t* listStatus = NULL;
node_t_socket* listSockets = NULL;
node_t* listThreadSubs = NULL;
node_t* listThreadPubs = NULL;

#define BUFF_SIZE 515

void InitializeOurCriticalSection()
{
	InitializeCriticalSection(&cs);
}
void DeleteOurCriticalSection()
{
	DeleteCriticalSection(&cs);
}
void AddSocketToList(node_t_socket** head, SOCKET* value)
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

void CloseAllThreads()
{
	node_t* currentPub = listThreadPubs;
	node_t* currentSub = listThreadSubs;

	while (currentPub != NULL)
	{
		listThreadPubs = currentPub->next;			// Changed head 
		CloseHandle((currentPub->value));	//close handle
		free(currentPub);					// free old head 
		currentPub = listThreadPubs;
	}
	while (currentSub != NULL)
	{
		listThreadSubs = currentSub->next;			// Changed head 
		CloseHandle((currentSub->value));	//close handle
		free(currentSub);					// free old head 
		currentSub = listThreadSubs;
	}
}


void CloseInactiveThreads(node_t** head)
{

	node_t *current = *head;
	node_t* previous= NULL;
	while (current != NULL && current->Active == false)//if head thread not active, close him
	{
		*head = current->next;			// Changed head 
		CloseHandle((current->value));	//close handle
		free(current);					// free old head 
		current = *head;				// Change Temp 
	}

	// Delete other than head 
	while (current != NULL)
	{
		// Search for the inactive thread to be deleted, keep track of the 
		// previous node as we need to change 'prev->next' 
		while (current != NULL && current->Active != false)
		{
			previous = current;
			current = current->next;
		}

		// If inactiv thread was not present in linked list 
		if (current == NULL) return;

		// Unlink the node from linked list 
		previous->next = current->next;
	
		CloseHandle((current->value));	//Close handle
		free(current);					// Free memory 

		//Update Temp for next iteration of outer loop 
		current = previous->next;
	}


}

void CloseAllSockets()
{
	node_t_socket* current = listSockets;
	listSockets = NULL;
	EnterCriticalSection(&cs);
	if (current == NULL)
	{
		LeaveCriticalSection(&cs);
		return;
	}
	else {
		while (current != NULL) {
			node_t_socket* temp = current;
			CloseSocket(current->value);
			current = current->next;
			free(temp);
		}
	}
	LeaveCriticalSection(&cs);
}
void SetSocketInNonblockingMode(SOCKET* socket)
{
	unsigned long mode = 1;
	ioctlsocket(*socket, FIONBIO, &mode);
}
void LitenForPublisher(SOCKET publisherListenSocket)
{
	int iResult = listen(publisherListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(publisherListenSocket);
		WSACleanup();
		return;
	}
	SOCKET publisherAcceptedSocket = INVALID_SOCKET;
	SetSocketInNonblockingMode(&publisherAcceptedSocket);

	printf("Server initialized, waiting for Publisher.\n");
	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	do
	{
		FD_ZERO(&set);
		FD_SET(publisherListenSocket, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			CloseSocket(&publisherAcceptedSocket);
			return;
		}
		else if (iResult != 0) {
			if (FD_ISSET(publisherListenSocket, &set)) {
				publisherAcceptedSocket = *CreateAcceptSocket(publisherListenSocket);
				AddSocketToList(&listSockets, &publisherAcceptedSocket);
				FD_SET(publisherAcceptedSocket, &set);
				iResult = select(0, &set, NULL, NULL, &timeVal);

				if (iResult != 0) {
					if (FD_ISSET(publisherAcceptedSocket, &set)) {
						ConnectPublisher(publisherAcceptedSocket);
					}
				}
			}
		}

		if (_kbhit())
		{
			EnterCriticalSection(&cs);
			if (CloseApp())
			{
				CloseSocket(&publisherAcceptedSocket);
				LeaveCriticalSection(&cs);
				return;
			}
			LeaveCriticalSection(&cs);
		}

		Sleep(100);
		CloseInactiveThreads(&listThreadPubs);
	} while (1);
	CloseSocket(&publisherAcceptedSocket);
	
}

int ConnectSubscriber(SOCKET socket)
{
	char someBuff[BUFF_SIZE];
	int iResult = recv(socket, someBuff, BUFF_SIZE, 0);
	if (iResult > 0)
	{
		int* size = (int*)someBuff;
		char* msg = someBuff + (sizeof(int));
		msg[*size] = '\0';
		printf("%s\n", msg);
	}
	return iResult;
}

void ConnectPublisher(SOCKET socket)
{
	char someBuff[BUFF_SIZE];
	int iResult = recv(socket, someBuff, BUFF_SIZE, 0);
	if (iResult > 0)
	{
		int* size = (int*)someBuff;
		char* msg = someBuff + (sizeof(int));
		msg[*size] = '\0';
		printf("%s\n", msg);

		DWORD ID;
		HANDLE Thread;
		EnterCriticalSection(&cs);
		int ThreadID = ThreadCounter++;
		LeaveCriticalSection(&cs);
	
		socket_and_id_for_thred* param = (socket_and_id_for_thred*)malloc(sizeof(socket_and_id_for_thred) );

		param->ID = ThreadID;
		param->socket = socket;
		Thread = CreateThread(NULL, 0, &ReceiveMessageFromPublisher, param, 0, &ID);

		AddToList(&listThreadPubs, Thread, ThreadID);
	}
}
///primanje poruke suba i slanje na njega
DWORD WINAPI RcvMessageFromSub(LPVOID param)
{
	socket_and_id_for_thred* temp = (socket_and_id_for_thred*)param;
	int ThreadID = temp->ID;
	SOCKET acceptedSocket = temp->socket;
	free(temp);

	bool flag = false;
	subscriber_t* sub =NULL;
	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	FD_ZERO(&set);
	while (!flag)
	{
		FD_SET(acceptedSocket, &set);
		int iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			continue;
		}
		else if (iResult != 0) {
			if (FD_ISSET(acceptedSocket, &set)) {
				int iResult = ConnectSubscriber(acceptedSocket);
				if (iResult > 0)
				{
					break;//connect
				}	
				else
				{
					flag = true;
				}
			}
		}
		Sleep(500);
	}
	while (!flag)//subscribe
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
				else 
				{
					flag = true;
				}
			}
		}
		Sleep(500);
	}
	while (!flag)
	{
		Sleep(5000);
		EnterCriticalSection(&cs);
		char* queue = (sub->queue);
		if (queue == NULL)
		{
			LeaveCriticalSection(&cs);
			continue;
		}

		sub->queue = NULL;
		CreateQueue(&(sub->queue));
		LeaveCriticalSection(&cs);
		int* size = (int*)queue;
		char* messageForSend = (char*)malloc(*size + sizeof(int));
		memcpy(messageForSend, size, sizeof(int));
		memcpy(messageForSend + sizeof(int), queue + sizeof(int) * 2, *size);
		free(queue);

		int sizeOfMsg = *size + sizeof(int);
		char* msgBegin = messageForSend;
		
		while (!flag) {
			int iResult = send(acceptedSocket, messageForSend, sizeOfMsg, 0);
			if (iResult == SOCKET_ERROR)
			{
				printf("The client has unsubscribed.\n");
				flag = true;
				break;
			}
			sizeOfMsg -= iResult;
			messageForSend += iResult;
			if (sizeOfMsg <= 0)
				break;
		}
		
		free(msgBegin);
		if (flag)
			break;
		printf("Sending message to subscriber.\n");
	
		Sleep(5000);
	}

	CloseSocket(&acceptedSocket);
	RemoveSubscriber(sub);
	free(sub);
	DeactivateThread(&listThreadSubs, ThreadID);
	return -1;
}
DWORD WINAPI ListenSubscriber(LPVOID param)
{
	SOCKET subscriberListenSocket = *((SOCKET*)param);
	int iResult = listen(subscriberListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		CloseSocket(&subscriberListenSocket);
		WSACleanup();
		return 1;
	}
	SetSocketInNonblockingMode(&subscriberListenSocket);

	printf("Server initialized, waiting for Subscribers.\n");

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
			CloseSocket(&subscriberListenSocket);
			return INVALID_SOCKET;
		}
		else if (iResult != 0) {
			if (FD_ISSET(subscriberListenSocket, &setSub)) {
				subscriberAcceptedSocket = *CreateAcceptSocket(subscriberListenSocket);

				DWORD ID;
				HANDLE Thread;
				
				EnterCriticalSection(&cs);
				int ThreadID = ThreadCounter++;
				LeaveCriticalSection(&cs);

				socket_and_id_for_thred* param = (socket_and_id_for_thred*)malloc(sizeof(socket_and_id_for_thred));
				param->ID = ThreadID;
				param->socket = subscriberAcceptedSocket;

				Thread = CreateThread(NULL, 0, &RcvMessageFromSub, param, 0, &ID);
				AddToList(&listThreadSubs, Thread, ThreadID);
			}
		}
		Sleep(1000);
		CloseInactiveThreads(&listThreadSubs);
	} while (true);
}
void  WriteMessage(char* message)
{
	int* messageLength = (int*)message; // ukupna duzina poruke topic +type +text
	Topic topic = (Topic) * ((int*)(message + sizeof(int)));
	message += sizeof(int);

	HANDLE ThreadAnalog;
	HANDLE ThreadStatus;
	DWORD idAnalog;
	DWORD idStatus;
	switch (topic) {
	case 0:
	{
		data_for_thread* forAnalog = (data_for_thread*)malloc(sizeof(data_for_thread));
		forAnalog->list = &listAnalog;
		forAnalog->message = message;
		forAnalog->size = *messageLength;
		ThreadAnalog = CreateThread(NULL, 0, &AddMessageToQueue, forAnalog, 0, &idAnalog);
		Sleep(100);
		CloseHandle(ThreadAnalog);
		free(forAnalog);

		break;
	}
	case 1:
	{
		data_for_thread* forStatus = (data_for_thread*)malloc(sizeof(data_for_thread));
		forStatus->list = &listStatus;
		forStatus->message = message;
		forStatus->size = *messageLength;
		ThreadStatus = CreateThread(NULL, 0, &AddMessageToQueue, forStatus, 0, &idStatus);
		Sleep(100);
		CloseHandle(ThreadStatus);
		free(forStatus);

		break;
	}
	case 2:
	{
		//analog

		data_for_thread* forAnalog2 = (data_for_thread*)malloc(sizeof(data_for_thread));
		forAnalog2->list = &listAnalog;
		forAnalog2->message = message;
		forAnalog2->size = *messageLength;
		ThreadAnalog = CreateThread(NULL, 0, &AddMessageToQueue, forAnalog2, 0, &idAnalog);
		//status

		data_for_thread* forStatus2 = (data_for_thread*)malloc(sizeof(data_for_thread));
		forStatus2->list = &listStatus;
		forStatus2->message = message;
		forStatus2->size = *messageLength;
		ThreadStatus = CreateThread(NULL, 0, &AddMessageToQueue, forStatus2, 0, &idStatus);
		Sleep(100);

		CloseHandle(ThreadAnalog);
		CloseHandle(ThreadStatus);
		free(forAnalog2);
		free(forStatus2);

		break;
	}
	}
}

DWORD WINAPI ReceiveMessageFromPublisher(LPVOID param)
{
	socket_and_id_for_thred* temp = (socket_and_id_for_thred*)param;
	int threadID = temp->ID;
	SOCKET acceptedSocket = temp->socket;
	free(temp);
	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	bool flag =false;
	FD_ZERO(&set);

	while (!flag)
	{
		FD_SET(acceptedSocket, &set);
		int iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			flag = true;
		}
		else if (iResult != 0) {
			if (FD_ISSET(acceptedSocket, &set)) {
				char someBuff[4];
				int iResult = recv(acceptedSocket, someBuff, sizeof(int), 0);
				if (iResult > 0)
				{
					int* msgSize = (int*)someBuff;
					char* message = (char*)malloc(*msgSize);
					iResult = recv(acceptedSocket, message, *msgSize, 0);
					if (iResult > 0)
					{
						message[iResult] = '\0';
						char* messageForQueue = (char*)malloc((*msgSize) + sizeof(int));
						memcpy(messageForQueue, msgSize, sizeof(int));
						memcpy(messageForQueue + sizeof(int), message, *msgSize);
						WriteMessage(messageForQueue);
					}
					else
					{
						printf("The publisher is disconnected.\n");	
						flag =true;
					}
				}
				else
				{
					printf("The publisher is disconnected.\n");
					flag = true;
				}
			}
		}
	}
	DeactivateThread(&listThreadPubs, threadID);
	CloseSocket(&acceptedSocket);
	return 1;

}

SOCKET* CreateAcceptSocket(SOCKET listenSocket)
{
	SOCKET* acceptedSocket = (SOCKET*)malloc(sizeof(SOCKET));
	*acceptedSocket = accept(listenSocket, NULL, NULL);

	if (*acceptedSocket == INVALID_SOCKET)
	{
		printf("Accept failed with error: %d\n", WSAGetLastError());
		CloseSocket(&listenSocket);
		WSACleanup();
	}

	SetSocketInNonblockingMode(acceptedSocket);
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

		current->next = (node_subscriber_t*)malloc(sizeof(node_subscriber_t));
		current = current->next;
		current->subscriber = sub;
		current->next = NULL;
	}
}
void AddToList(node_t** head, HANDLE value, int id)
{
	if ((*head) == NULL)
	{
		(*head) = (node_t*)malloc(sizeof(node_t));
		(*head)->value = value;
		(*head)->next = NULL;
		(*head)->ID = id;
		(*head)->Active = true;
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
		current->ID = id;
		current->Active = true;
	}
}
void DeactivateThread(node_t** head, int id)
{
	node_t* current = (*head);
	while (current!= NULL) {
		if (current->ID == id)
		{
			current->Active = false;
			return;
		}

		current = current->next;
	}
}

void CreateQueue(char** msgQueue)
{
	*msgQueue = NULL;
	*msgQueue = ((char*)malloc(512 + sizeof(int)*2));//brojac slobodnih(4) brojac zauzetih(4) i poruka 512
	int min = 0;
	int max = 512;
	memcpy(*msgQueue, &min, sizeof(int));
	memcpy(*msgQueue + sizeof(int), &max, sizeof(int));
}

char* Enqueue(char** queue, char* msg, int msg_size) {
	int* lenght = (int*)(*queue);
	int* max = (int*)((*queue) + sizeof(int));

	if (*lenght + msg_size > * max)
	{
		char* newQueue = (char*)malloc((*max) * 2);
		*max *= 2;
		memcpy(newQueue, (*queue), *lenght + sizeof(int) * 2);
		free((*queue));
		(*queue) = newQueue;

		printf("\nNew memory allocated.\n\n");

		lenght = (int*)(*queue);
		max = (int*)((*queue) + sizeof(int));

		memcpy((*queue) + (*lenght) + sizeof(int) * 2, &msg_size, sizeof(int));
		*lenght += sizeof(int);
		memcpy((*queue) + (*lenght) + sizeof(int) * 2, msg, msg_size);
		*lenght += msg_size;
		
	}
	else
	{
		memcpy((*queue) + (*lenght) + sizeof(int) * 2, &msg_size, sizeof(int));
		*lenght += sizeof(int);
		memcpy((*queue) + (*lenght) + sizeof(int) * 2, msg, msg_size);
		*lenght += msg_size;
	}
	free(msg);
	return *queue;
}
SOCKET* CreatePublisherListenSocket()
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

		CloseSocket(listenSocketRetVal);
		WSACleanup();
		return invalidSocket;
	}
	SetSocketInNonblockingMode(listenSocketRetVal);
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
		CloseSocket(listenSocketRetVal);
		WSACleanup();
		return invalidSocket;
	}
	freeaddrinfo(resultingAddress);
	SetSocketInNonblockingMode(listenSocketRetVal);
	return listenSocketRetVal;
}
void RemoveSubscriberFromList(int id, node_subscriber_t** list)
{
	node_subscriber_t* current = *list;

	node_subscriber_t* previous = NULL;
	subscriber* tempSub = *(current->subscriber);
	if (current != NULL && tempSub != NULL)
	{
		if (tempSub->id == id)
		{
			EnterCriticalSection(&cs);
			*list = current->next;
			free(current);
			LeaveCriticalSection(&cs);
			return;
		}
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

DWORD WINAPI AddMessageToQueue(LPVOID param) {
	data_for_thread* temp = ((data_for_thread*)param);

	if (*(temp->list) == NULL)
		return -1;

	node_subscriber_t* current = (*(temp->list));

	while (true) {
		if (*(current->subscriber) != NULL)
		{
			char* queue = (*(current->subscriber))->queue;
			EnterCriticalSection(&cs);
			(*(current->subscriber))->queue = Enqueue(&queue, temp->message, temp->size);
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
	return 1;
}