#include "..\common\PublisherFunctions.h"
#include "..\common\Communication.h"

int __cdecl main(int argc, char** argv)
{
	srand(time(NULL));		// potrebno za generisanje random broja

	SOCKET connectSocket = INVALID_SOCKET;
	const char* messageToSend = "this is a test";
	Topic topic = GenerateRandomTopic();
	TypeTopic type = GenerateRandomType(topic);

	PrintPublisherInfo(topic, type);

	if (argc != 2)
	{
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddress.sin_port = htons(DEFAULT_PORT);

	if (connect(connectSocket, (SOCKADDR*)& serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	if (PubConnect(connectSocket))
		while (true) {
			if (!Publish((void*)topic, (void*)type, messageToSend, connectSocket))
				break;
			Sleep(2500);
		}

	// CloseSocket(&connectSocket);
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}