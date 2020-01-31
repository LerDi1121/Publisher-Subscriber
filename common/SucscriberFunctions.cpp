#include "SubscriberFunctions.h"

bool Connect(SOCKET subscribeSocket)
{
	int initMessage = 1;
	int iResult = send(subscribeSocket, (char*)(&initMessage), 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(subscribeSocket);
		return false;
	}
	else {
		printf("Connection was successful.\n");
	}
	printf("*********************************************************\n\n");
	return true;
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

int SubscriptionChoice() {
	int i = 0;
	printf("1. Subscribe to Analog topic\n");
	printf("2. Subscribe to Status topic\n");
	printf("3. Subscribe to Analog and Status topic\n");

	do {
		printf("Chose topic: ");
		scanf("%d", &i);
		printf("\n");
	} while (i > 3 || i < 1);

	return i - 1;
}

void PrintMessages(char* receivedData, int receivedDataSize) {
	char* message = receivedData;
	int* messageSize = (int*)receivedData;
	char* messageText = NULL;

	while (receivedDataSize > 0) {
		messageSize = (int*)message;
		message += 4;

		Topic topic = (Topic) * ((int*)(message));
		switch ((int)topic) {
		case 0:
			printf("Topic: Analog\t");
			break;
		case 1:
			printf("Topic: Status\t");
			break;
		default:
			break;
		}

		TypeTopic type = (TypeTopic) * ((int*)(message + 4));
		switch ((int)type) {
		case 0:
			printf("Type: SWG\t");
			break;
		case 1:
			printf("Type: CRB\t");
			break;
		case 2:
			printf("Type: MER\t");
			break;
		default:
			break;
		}

		messageText = (char*)malloc(*messageSize - 8);
		memcpy(messageText, message + 8, *messageSize - 8);
		messageText[*messageSize - 8] = '\0';
		printf("Text: %s\n", messageText);
		message += (*messageSize);
		receivedDataSize -= (*messageSize + 4);
	}
	printf("\n*********************************************************\n\n");
}