
#pragma once

#define HEADER_H

#include "Communication.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT_FOR_SUB "27017"
#define DEFAULT_PORT_FOR_PUB "27016"

typedef struct subscriber {
	SOCKET socket;
	int id;
	char* queue;
	int topic;
} subscriber_t;

typedef struct node_subscriber {
	subscriber_t** subscriber;
	node_subscriber* next;
}node_subscriber_t;

typedef struct node {
	HANDLE value;
	struct node* next;
} node_t;

typedef struct node_t_socket {
	SOCKET value;
	struct node_t_socket* next;
} node_t_socket;

typedef struct data_for_thread {
	int size;
	char* message;
	node_subscriber_t** list;
	//CRITICAL_SECTION * cs;
}data_for_thread;

extern CRITICAL_SECTION cs;
extern char* msg_queue;
extern node_t* listThread;
extern node_subscriber_t* listAnalog;
extern node_subscriber_t* listStatus;
