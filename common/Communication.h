#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define HEADER_H

#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <conio.h>
#include  "AllEnums.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

/*
 *	Function: InitializeWindowsSockets
 * --------------------
 *	Initializes WSAData.
 *
 *	returns: true if success, false if failure
 */
bool InitializeWindowsSockets();

/*
 *	Function: CloseSocket
 * --------------------
 *	Closes the socket.
 *
 *	socket: socket that closes
 */
void CloseSocket(SOCKET* socket);

/*
 *	Function: Subscribe
 * --------------------
 *	Allows user to subscribe to selected topic.
 *
 *	subscribeSocket: socket that connects client to pub-sub engine server
 *
 *	returns: true if success, false if failure
 */
bool Subscribe(SOCKET subscribeSocket);

/*
 *	Function: Connect
 * --------------------
 *	Connects user to the pub-sub engine server.
 *
 *	connectSocket: socket that connects client to pub-sub engine server
 *	initialMessage: initial message that informs the pub-sub engine server new connection
 *
 *	returns: true if success, false if failure
 */
bool Connect(SOCKET connectSocket, const char* initialMessage);

/*
 *	Function: Publish
 * --------------------
 *	Sends the message to the pub-sub engine server.
 *
 *	topic: publishers topic
 *	type: type of topic
 *	message: message that is sent along with type and topic
 *	conSoc: socket that connects publisher to pub-sub engine server
 *
 *	returns: true if success, false if failure
 */
bool Publish(void* topic, void* type, const char* message, SOCKET conSoc);

/*
 *	Function: CloseApp
 * --------------------
 *	Allows user to chose if he wants to exit program.
 *
 *	returns: true if user wants to exit program, false if not
 */
bool CloseApp();