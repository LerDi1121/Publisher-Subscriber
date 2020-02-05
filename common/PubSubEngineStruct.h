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
	int ID;
	bool Active;
	struct node* next;
} node_t;

typedef struct socket_and_id_for_thred
{
	SOCKET socket;
	int ID;

}socket_and_id_for_thred_t
;
typedef struct node_t_socket {
	SOCKET* value;

	struct node_t_socket* next;
} node_t_socket;

typedef struct data_for_thread {
	int size;
	char* message;
	node_subscriber_t** list;
}data_for_thread;

extern node_t_socket* listSockets;
extern CRITICAL_SECTION cs;
extern char* msg_queue;
extern node_t* listThreadSubs;
extern node_t* listThreadPubs;
extern node_subscriber_t* listAnalog;
extern node_subscriber_t* listStatus;

