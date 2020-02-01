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
	Funkcija: InitializeWindowsSockets
	----------------------------------
	Funkcionalnost : Inicijalizuje WsaData podatke
	Povratna vrednost : Uspesnost akcije
*/
bool InitializeWindowsSockets();
/*
	Funkcija: CloseSocket
	----------------------------------
	Funkcionalnost : Gasenje i zatvaranje uticnice
	socket: Soket koji treba da se ugasi i zatvori
	Povratna vrednost : Nema
*/

void CloseSocket(SOCKET* socket);
