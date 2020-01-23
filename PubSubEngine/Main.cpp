#include "PubSubEngineFunctions.h"



int  main(int argc, char** argv)
{
	
	node_t_socket* listSockets = NULL;

	SOCKET publisherListenSocket = INVALID_SOCKET;
	SOCKET subscriberListenSocket = INVALID_SOCKET;

	SOCKET publisherAcceptedSocket = INVALID_SOCKET;
	SOCKET subscriberAcceptedSocket = INVALID_SOCKET;
	InitializeCriticalSection(&cs);
	
	CreateQueue();

	int iResult;

	
	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	// Prepare address information structures
	publisherListenSocket = *CreatePublisherListenSocket();
	subscriberListenSocket= *CreateSubscriberListenSocket();
	//thread za subove
	DWORD printSubID;
	HANDLE ThreadSub;
	ThreadSub = CreateThread(NULL, 0, &ListenSubscriber, &subscriberListenSocket, 0, &printSubID);
	AddToList(&listThread, ThreadSub);



	iResult = listen(publisherListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(publisherListenSocket);
		WSACleanup();
		return 1;
	}
	iResult = ioctlsocket(publisherAcceptedSocket, FIONBIO, (u_long*)1);

	printf("Server initialized, waiting for clients.\n");
	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;



	/* primanje poruka*/

	do
	{
		// konektovanje
		FD_ZERO(&set);
		FD_SET(publisherListenSocket, &set);

		iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
		}
		else if (iResult != 0) {
			if (FD_ISSET(publisherListenSocket, &set)) {
				publisherAcceptedSocket = *CreateAcceptSocket(publisherListenSocket);
				//AddSocketToList(&listSockets, acceptedSocket);
				DWORD print1ID;
				HANDLE Thread;
				data_for_thread  temp = *((data_for_thread*)malloc(sizeof(data_for_thread)));
				temp.socket = publisherAcceptedSocket;
				temp.msgQueue = &msg_queue;

				printf("Pravljenje treda\n");

				
				Thread = CreateThread(NULL, 0, &RcvMessage, &temp, 0, &print1ID);
				AddToList(&listThread, Thread);

				//	break;
			}
		}
		///
	} while (1);

	// shutdown the connection since we're done
	iResult = shutdown(publisherAcceptedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(publisherAcceptedSocket);
		WSACleanup();
		return 1;
	}

	DeleteCriticalSection(&cs);
	///uradit 
	//oslobditi hendlove -> tredovi
	// cleanup
	closesocket(publisherListenSocket);
	closesocket(publisherAcceptedSocket);
	WSACleanup();
	getchar();

	return 0;
}
