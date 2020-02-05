#pragma once
#define HEADER_H
#include "PubSubEngineStruct.h"
#include <conio.h>

/*
 *	Function: CreateSubscriber
 *  --------------------
 *	Creates new instance of subscriber structure.
 *
 *  socket: socket that new subscriber is connected on
 *  topic: topic that subscriber has subscribed on
 *
 *	returns: pointer to subscriber structure
 */
subscriber_t* CreateSubscriber(SOCKET socket, int topic);


/*
 *	Function: ReceiveMessageFromPublisher
 *  --------------------
 *	Receives messages from publisher in new thread and writes it in subscriber's queue.
 *
 *  param: socket that publisher is connected on and id of thread (needed for closing handle)
 */
DWORD WINAPI ReceiveMessageFromPublisher(LPVOID param);

/*
 *	Function: AddToList
 *  --------------------
 *	Adds new thread handle to list in order to close all handles after program ends.
 *
 *  head: list of handles
 *  value: handle
 *  id: id of thread
 */
void AddToList(node_t** head, HANDLE value, int id);

/*
 *	Function: AddSocketToList
 *  --------------------
 *	Adds new socket to list in order to close all sockets after program ends.
 *
 *  head: list of sockets
 *  value: socket
 */
void AddSocketToList(node_t_socket** head, SOCKET* value);

/*
 *	Function: CloseAllSockets
 *  --------------------
 *	Closes all sockets from global list of sockets.
 *
 */
void CloseAllSockets();

/*
 *	Function: AddSubscriberToList
 *  --------------------
 *	Adds subscriber to list of analog or digital topics depending on subscribers chosen topic.
 *
 *  sub: subscriber structure
 */
void AddSubscriberToList(subscriber_t** sub);

/*
 *	Function: AddToConcreteList
 *  --------------------
 *	Adds subscriber to concrete list.
 *
 *  list: concrete list of subscribers
 *  sub: subscriber structure
 */
void AddToConcreteList(node_subscriber_t** list, subscriber_t** sub);

/*
 *	Function: CreateAcceptSocket
 *  --------------------
 *	Creates accepted socket when client connects.
 *
 *  listenSocket: socket that was listening for connection
 *
 *  returns: accpted socket
 */
SOCKET* CreateAcceptSocket(SOCKET listenSocket);

/*
 *	Function: Enqueue
 *  --------------------
 *	Adds new message to the queue.
 *
 *  queue: queue that receives message
 *  msg: message
 *  msg_size: size of message
 *
 *  returns: pointer to queue
 */
char* Enqueue(char** queue, char* msg, int msg_size);

/*
 *	Function: CreateQueue
 *  --------------------
 *	Creates new queue.
 *
 *  msgQueue: queue that is allocated
 */
void CreateQueue(char** msgQueue);

/*
 *	Function: CreatePublisherListenSocket
 *  --------------------
 *	Creates new listen socket for publisher.
 *
 *  returns: pointer to created socket
 */
SOCKET* CreatePublisherListenSocket();

/*
 *	Function: CreateSubscriberListenSocket
 *  --------------------
 *	Creates new listen socket for subscriber.
 *
 *  returns: pointer to created socket
 */
SOCKET* CreateSubscriberListenSocket();

/*
 *	Function: WriteMessage
 *  --------------------
 *	Writes messages received from publisher to each subscribed client.
 *
 *  message: pointer to created socket
 */
void WriteMessage(char* message);

/*
 *	Function: AddMessageToQueue
 *  --------------------
 *	Adds message to queue of each subscriber of that message topic.
 *
 *  param: message and queue
 */
DWORD WINAPI AddMessageToQueue(LPVOID param);

/*
 *	Function: ListenSubscriber
 *  --------------------
 *	Listens for new subscriber and when conenction happens, creates thread for that sub.
 *
 *  param: listen socket
 */
DWORD WINAPI ListenSubscriber(LPVOID param);

/*
 *	Function: RemoveSubscriber
 *  --------------------
 *	Removes subscriber from list if he unsubscribes.
 *
 *  sub: subscriber
 */
void RemoveSubscriber(subscriber_t* sub);

/*
 *	Function: RemoveSubscriberFromList
 *  --------------------
 *	Removes subscriber from concrete list.
 *
 *  list: list of subscribers
 *  id: subscriber id
 */
void RemoveSubscriberFromList(int id, node_subscriber_t** list);

/*
 *	Function: InitializeOurCriticalSection
 *  --------------------
 *	Initializes critical section.
 */
void InitializeOurCriticalSection();

/*
 *	Function: DeleteOurCriticalSection
 *  --------------------
 *	Deletes critical section.
 */
void DeleteOurCriticalSection();


/*
 *	Function: LitenForPublisher
 *  --------------------
 *	Listens for new publisher connections. If connection hapens, creates thread for new pub.
 *
 *  publisherListenSocket: listen socket
 */
void LitenForPublisher(SOCKET publisherListenSocket);

/*
 *	Function: SetSocketInNonblockingMode
 *  --------------------
 *	Sets socket in non-blocking mode.
 *
 *  socket: socket that is set in non-blocking mode
 */
void SetSocketInNonblockingMode(SOCKET* socket);

/*
 *	Function: ConnectPublisher
 *  --------------------
 *	Creates new thread that serves one publisher.
 *
 *  socket: socket that new publisher is connected on
 */
void ConnectPublisher(SOCKET socket);

/*
 *	Function: ConnectSubscriber
 *  --------------------
 *	Receives and prints initial message from new subscriber.
 *
 *  socket: socket that new subscriber is connected on
 */
int ConnectSubscriber(SOCKET socket);

/*
 *	Function: DeactivateThread
 *  --------------------
 *	Marks the given thread as inactive.
 *
 *  head: list of all handles
 *  id: id of thread
 */
void DeactivateThread(node_t** head, int id);

/*
 *	Function: CloseInactiveThreads
 *  --------------------
 *	Closes all threads that are marked as inactive.
 *
 *  head: list of all handles
 */
void CloseInactiveThreads(node_t** head);

/*
 *	Function: CloseInactiveThreads
 *  --------------------
 *	Closes all active threads.
 */
void CloseAllThreads();
