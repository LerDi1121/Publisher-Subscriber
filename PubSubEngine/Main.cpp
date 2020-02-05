#include "..\common\PubSubEngineFunctions.h"

int  main(int argc, char** argv)
{
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

	AddSocketToList(&listSockets, &publisherListenSocket);
	AddSocketToList(&listSockets, &subscriberListenSocket);
	DWORD printSubID;
	HANDLE ThreadSub;
	ThreadSub = CreateThread(NULL, 0, &ListenSubscriber, &subscriberListenSocket, 0, &printSubID);
	LitenForPublisher(publisherListenSocket);

	Sleep(1500);
	CloseHandle(ThreadSub);
	CloseAllSockets();

	WSACleanup();

	DeleteOurCriticalSection();

	return 0;
}