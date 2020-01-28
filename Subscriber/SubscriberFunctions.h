#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include  "..\common\AllEnums.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT_FOR_PUB_SUB_ENG_SEND 27017

void PrintMessages(char* msg, int totalSize);
bool InitializeWindowsSockets();
bool Connect(SOCKET subscribeSocket);
bool Subscribe(SOCKET subscribeSocket);
int SubscriptionChoice();