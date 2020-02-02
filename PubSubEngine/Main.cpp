#include "..\common\PubSubEngineFunctions.h"

int  main(int argc, char** argv)
{
	node_t_socket* listSockets = NULL;
	int iResult;

	SOCKET publisherListenSocket = INVALID_SOCKET;
	SOCKET subscriberListenSocket = INVALID_SOCKET;

	InitializeOurCriticalSection();

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	publisherListenSocket = *CreatePublisherListenSocket();
	subscriberListenSocket = *CreateSubscriberListenSocket();

	DWORD printSubID;
	HANDLE ThreadSub;
	ThreadSub = CreateThread(NULL, 0, &ListenSubscriber, &subscriberListenSocket, 0, &printSubID);
	AddToList(&listThread, ThreadSub);

	LitenForPublisher(publisherListenSocket);

	DeleteOurCriticalSection();
	closesocket(publisherListenSocket);
	WSACleanup();

	return 0;
}