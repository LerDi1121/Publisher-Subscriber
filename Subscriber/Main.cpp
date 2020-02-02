#include "..\common\SubscriberFunctions.h"
#include "..\common\Communication.h"

int  main(int argc, char** argv)
{
	SOCKET connectSocket = INVALID_SOCKET;
	int iResult;
	char messageBufer[DEFAULT_BUFLEN];

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
	serverAddress.sin_port = htons(DEFAULT_PORT_FOR_PUB_SUB_ENG_SEND);

	if (connect(connectSocket, (SOCKADDR*)& serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	if (SubscriberConnect(connectSocket)) {
		Sleep(1000);
		Subscribe(connectSocket);
	}

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	FD_ZERO(&set);

	while (true)
	{
		FD_SET(connectSocket, &set);
		char* message = NULL;
		int iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			printf("SOCKET_ERROR");
			break;
		}
		else if (iResult != 0) {
			if (FD_ISSET(connectSocket, &set)) {
				char* someBuff = (char*)malloc(4);
				int* bufflen;
				int iResult = recv(connectSocket, someBuff, 4, 0);
				if (iResult > 0)
				{
					bufflen = (int*)someBuff;
					int temp = *bufflen;
					message = (char*)malloc(*bufflen);
					char* mess = message;

					while (*bufflen > 0) {
						iResult = recv(connectSocket, messageBufer, DEFAULT_BUFLEN, 0);
						*bufflen -= iResult;
						memcpy(mess, messageBufer, iResult);
						mess += iResult;
					}
					PrintMessages(message, temp);
					free((void*)message);
				}
				else if (iResult == 0)
				{
					printf("Connection with client closed.\n");
					break;
				}
				else
				{
					printf("recv failed with error: %d\n", WSAGetLastError());
					break;
				}
			}
		}
	}

	CloseSocket(&connectSocket);
	WSACleanup();

	return 0;
}