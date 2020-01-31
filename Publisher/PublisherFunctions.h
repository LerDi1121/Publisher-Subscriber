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
#include "..\common\AllEnums.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

int ChoseAtRandom();
bool InitializeWindowsSockets();
bool Connect(SOCKET conSoc);
bool Publish(void* topic, void* type, const char* message, SOCKET conSoc);
enum Topic GenerateRandomTopic();
enum TypeTopic GenerateRandomType(enum Topic topic);
void PrintPublisherInfo(enum Topic topic, enum TypeTopic type);
