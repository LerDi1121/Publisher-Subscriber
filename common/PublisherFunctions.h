#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <conio.h>
#include "AllEnums.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

/*
 *	Function: ChoseAtRandom
 * --------------------
 *	Randomly picks number between 0 and 1.
 *
 *	returns: randomly chosen number
 */
int ChoseAtRandom();

/*
 *	Function: GenerateRandomTopic
 * --------------------
 *	Randomly generates topic using ChoseAtRandom function.
 *
 *	returns: randomly generated topic
 */
enum Topic GenerateRandomTopic();

/*
 *	Function: GenerateRandomType
 * --------------------
 *	Randomly generates type of topic using ChoseAtRandom function.
 *
 *	topic: topic for which type is generated for
 *
 *	returns: randomly generated type
 */
enum TypeTopic GenerateRandomType(enum Topic topic);

/*
 *	Function: PrintPublisherInfo
 * --------------------
 *	Prints basic information about publisher on the console.
 *
 *	topic: publisher's topic
 *	type: type of topic
 */
void PrintPublisherInfo(enum Topic topic, enum TypeTopic type);
