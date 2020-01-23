#include "PubSubEngineFunctions.h"


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
	data_for_thread temp = *((data_for_thread*)param);
	SOCKET acceptedSocket = temp.socket;
	char* msgQueue = *(temp.msgQueue);

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
						char* start = Poruka;
						Topic t = (Topic) * ((int*)Poruka);

						TypeTopic tt = (TypeTopic) * ((int*)(Poruka + 4));

						int MessSize = *velicinaPor - (sizeof(Topic) + sizeof(TypeTopic));
						char* Message = (char*)malloc(MessSize);
						Poruka = Poruka + sizeof(Topic) + sizeof(TypeTopic);

						memcpy(Message, (void*)Poruka, MessSize);

						Message[MessSize] = '\0';

						EnterCriticalSection(&cs);
						Enqueue(&msg_queue, start, *velicinaPor);
						LeaveCriticalSection(&cs);


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
	msg_queue = (char*)malloc(520);//brojac slobodnih(4) brojac zauzetih(4) i poruka 512
	int min = 0;
	int max = 512;
	memcpy(msg_queue, &min, 4);
	memcpy(msg_queue + 4, &max, 4);
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