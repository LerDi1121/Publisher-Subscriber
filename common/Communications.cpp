#include "Communication.h"
#include "SubscriberFunctions.h"

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void CloseSocket(SOCKET* socket)
{
	printf("Shutting down socket...\n\n");
	shutdown(*socket, SD_BOTH);
	closesocket(*socket);
	Sleep(500);
}

bool Subscribe(SOCKET subscribeSocket)
{
	int subChoice = SubscriptionChoice();
	int iResult = send(subscribeSocket, (char*)(&subChoice), 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("subscribe failed with error: %d\n", WSAGetLastError());
		closesocket(subscribeSocket);
		return false;
	}
	else
	{
		switch (subChoice) {
		case 0:
			printf("User has subscribed to Analog topic successfully.\n");
			break;
		case 1:
			printf("User has subscribed to Status topic successfully.\n");
			break;
		case 2:
			printf("User has subscribed to both topics successfully.\n");
			break;
		default:
			break;
		}
	}
	printf("*********************************************************\n\n");
	return true;
}

bool Publish(void* topic, void* type, const char* message, SOCKET publishSocket)
{
	int topicSize = sizeof(Topic);
	int typeSize = sizeof(TypeTopic);
	int dataToSendSize = strlen(message) + topicSize + typeSize;
	int messageSize = strlen(message);
	char* dataToSend = (char*)malloc(dataToSendSize);

	memcpy(dataToSend, &dataToSendSize, 4);
	memcpy(dataToSend + 4, &topic, topicSize);
	memcpy(dataToSend + 4 + topicSize, &type, typeSize);
	memcpy(dataToSend + 4 + topicSize + typeSize, message, messageSize);

	int iResult = send(publishSocket, dataToSend, dataToSendSize + 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(publishSocket);
		return false;
	}

	printf("Bytes Sent: %ld\n", iResult);
	return true;
}

bool Connect(SOCKET connectSocket, const char* initialMessage)
{
	int initialMessageSize = strlen(initialMessage);
	char* dataToSend = (char*)malloc(initialMessageSize + 4);
	memcpy(dataToSend, &initialMessageSize, 4);
	memcpy(dataToSend + 4, initialMessage, initialMessageSize);
	int iResult = send(connectSocket, dataToSend, initialMessageSize + 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		return false;
	}
	else {
		printf("Connection was successful.\n");
	}
	printf("*********************************************************\n\n");
	return true;
}

bool CloseApp()
{
	printf("Are you sure you want exit? Y/N ");
	char temp = getchar();
	if (temp == 'Y' || temp == 'y')
		return true;
	return false;
}