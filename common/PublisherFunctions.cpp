#include "PublisherFunctions.h"

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

	//memcpy(poruka + 4, message, duzina);
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

bool PubConnect(SOCKET publishSocket)
{
	const char* initialMessageToSend = "New publisher has connected.";
	int initialMessageSize = strlen(initialMessageToSend);
	char* dataToSend = (char*)malloc(initialMessageSize + 4);
	memcpy(dataToSend, &initialMessageSize, 4);
	memcpy(dataToSend + 4, initialMessageToSend, initialMessageSize);
	int iResult = send(publishSocket, dataToSend, initialMessageSize + 4, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

enum Topic GenerateRandomTopic() {
	Topic topic = Analog;
	int randomNumber = ChoseAtRandom();

	if (randomNumber == 0)
		topic = Analog;
	else if (randomNumber == 1)
		topic = Status;

	return topic;
}

enum TypeTopic GenerateRandomType(enum Topic topic) {
	TypeTopic type = SWG;
	int randomNumber;

	if (topic == Analog) {
		type = MER;
	}
	else {
		randomNumber = ChoseAtRandom();
		if (randomNumber == 0)
			type = SWG;
		else if (randomNumber == 1)
			type = CRB;
	}

	return type;
}

int ChoseAtRandom() {
	int randomNumber;

	randomNumber = rand() % 2;

	return randomNumber;
}

void PrintPublisherInfo(enum Topic topic, enum TypeTopic type) {
	printf("\n*********************************************************\n");
	printf("New publisher:\n");

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

	printf("\n*********************************************************\n\n");
}