#include "Communication.h"

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void CloseSocket(SOCKET* socket)
{
	shutdown(* socket, SD_SEND);
	closesocket(*socket);
}