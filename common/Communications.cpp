#include "Communication.h"

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void CloseSocket(SOCKET* socket)
{
	int iResult = shutdown(* socket, SD_SEND);
	closesocket(*socket);
}