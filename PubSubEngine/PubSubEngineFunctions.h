#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef HEADER_H
#define HEADER_H


#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <conio.h>
#include  "..\common\AllEnums.h"


#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT_FOR_SUB "27017"
#define DEFAULT_PORT_FOR_PUB "27016"




typedef struct Subscriber {
	SOCKET Socket;
	char** queue;
	Topic Topics[2];
} Subscriber;

typedef struct message_queue {
	char* message;
	struct message_queue* next;
} message_queue_t;

typedef struct node {
	HANDLE value;
	struct node* next;
} node_t;

typedef struct node_t_socket {
	SOCKET value;
	struct node_t_socket* next;
} node_t_socket;

typedef struct data_for_thread {
	SOCKET socket;
	char** msgQueue;
	//CRITICAL_SECTION * cs;
}data_for_thread;

extern CRITICAL_SECTION cs;
extern char* msg_queue;
extern Subscriber * queueSUb;

extern node_t* listThread;

bool InitializeWindowsSockets();
DWORD WINAPI RcvMessage(LPVOID param);
void AddToList(node_t** head, HANDLE value);
void AddSocketToList(node_t_socket** head, SOCKET value);
SOCKET* CreateAcceptSocket(SOCKET Listen);
void Enqueue(char** queue, char* msg, int msg_size);
void CreateQueue();


SOCKET * CreatePublisherListenSocket();
SOCKET * CreateSubscriberListenSocket();

DWORD WINAPI ListenSubscriber(LPVOID param);


#endif // HEADER_H