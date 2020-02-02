#include "PublisherFunctions.h"

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