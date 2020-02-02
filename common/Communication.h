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
	Povratna vrednost : Uspesnost izvrsavanja funkcije.
*/
bool InitializeWindowsSockets();
/*
	Funkcija: CloseSocket
	----------------------------------
	Funkcionalnost : Gasenje i zatvaranje uticnice
	Socket: Socket koji treba da se ugasi i zatvori
	Povratna vrednost : Nema
*/
void CloseSocket(SOCKET* socket);

/*
	Funkcija: SubscriberConnect
	----------------------------------
	Funkcionalnost: Uspostavlja konekciju sa PubSub Engine-om i salje inicijalnu poruku.
	Povratna vrednost: Uspesnost izvrsavanja funkcije.
*/
bool SubscriberConnect(SOCKET subscribeSocket);

/*
	Funkcija: Subscribe
	----------------------------------
	Funkcionalnost: Izvrsava subskripciju na odabranu temu.
	Povratna vrednost: Uspesnost izvrsavanja funkcije.
*/
bool Subscribe(SOCKET subscribeSocket);

/*
	Funkcija: PublisherConnect
	----------------------------------
	Funkcionalnost: Uspostavlja konekciju sa PubSub Engine-om i salje inicijalnu poruku.
	Povratna vrednost: Uspesnost izvrsavanja funkcije.
*/
bool PublisherConnect(SOCKET connectSocket, const char* initialMessage);

/*
	Funkcija: Publish
	----------------------------------
	Funkcionalnost: Salje poruku koja se sastoji od topic-a, type-a i teksta na odabrani socket.
	Povratna vrednost: Uspesnost izvrsavanja funkcije.
*/
bool Publish(void* topic, void* type, const char* message, SOCKET conSoc);