#include "..\common\PubSubEngineFunctions.h"

int  main(int argc, char** argv)
{
	node_t_socket* listSockets = NULL;

	SOCKET publisherListenSocket = INVALID_SOCKET;
	SOCKET subscriberListenSocket = INVALID_SOCKET;

	
	SOCKET subscriberAcceptedSocket = INVALID_SOCKET;
	InitializeOurCriticalSection();

	//CreateQueue();

	int iResult;

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	// Prepare address information structures
	publisherListenSocket = *CreatePublisherListenSocket();
	subscriberListenSocket = *CreateSubscriberListenSocket();
	//thread za subove
	DWORD printSubID;
	HANDLE ThreadSub;
	ThreadSub = CreateThread(NULL, 0, &ListenSubscriber, &subscriberListenSocket, 0, &printSubID);
	AddToList(&listThread, ThreadSub);

	LitenForPublisher(publisherListenSocket); 

	// shutdown the connection since we're done
	

	
	DeleteOurCriticalSection();
	///uradit
	//oslobditi hendlove -> tredovi
	// cleanup
	closesocket(publisherListenSocket);
	
	WSACleanup();
	getchar();

	return 0;
}