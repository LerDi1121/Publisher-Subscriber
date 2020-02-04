#pragma once
#define HEADER_H
#include "PubSubEngineStruct.h"
#include <conio.h>

/*
	Funkcija: CreateSubscriber
	----------------------------
	Funkcionalnost:
	Povratna vrednost:

*/
subscriber_t* CreateSubscriber(SOCKET socket, int topic);

DWORD WINAPI RcvMessage(LPVOID param);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void AddToList(node_t** head, HANDLE* value, int id);

/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void AddSocketToList(node_t_socket** head, SOCKET* value);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void CloseAllSockets();



/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void AddSubscriberToList(subscriber_t** sub);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void AddToConcreteList(node_subscriber_t** list, subscriber_t** sub);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost:
*/
SOCKET* CreateAcceptSocket(SOCKET Listen);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost:
*/
char* Enqueue(char** queue, char* msg, int msg_size);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void CreateQueue(char** msgQueue);

/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost:
*/
SOCKET* CreatePublisherListenSocket();
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost:
*/
SOCKET* CreateSubscriberListenSocket();
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void WriteMessage(char* message);// za poseban tred koji ce upisivati u red
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost:
*/
DWORD WINAPI  AddMessageToQueue(LPVOID param);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost:
*/

DWORD WINAPI ListenSubscriber(LPVOID param);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void RemoveSubscriber(subscriber_t* sub);
/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void RemoveSubscriberFromList(int id, node_subscriber_t** list);
/*
	Funkcija: InitializeOurCriticalSection
	------------------------------
	Funkcionalnost: Inicijalizuje kriticnu sekciju
	Povratna vrednost: Nema
*/
void InitializeOurCriticalSection();
/*
	Funkcija: DeleteOurCriticalSection
	------------------------------
	Funkcionalnost: Brise kriticnu sekciju
	Povratna vrednost: Nema
*/
void DeleteOurCriticalSection();
/*
	Funkcija: LitenForPublisher
	------------------------------
	Funkcionalnost: Osluskuje da li se neki publisher konektovao i pravi poseban tred za njega u kojem se primaju poruke
	publisherListenSocket: Soket koji sluzi za slusanje
	Povratna vrednost: Nema
*/
void LitenForPublisher(SOCKET publisherListenSocket);
/*
	Funkcija:SetSocketInNonblockingMode
	------------------------------
	Funkcionalnost: Postavlja uticnicu u neblokirajuci mod
	socket: pokazivac na soket koji se postavlja u neblokirajuci mod
	Povratna vrednost: Nema
*/

void SetSocketInNonblockingMode(SOCKET* socket);

/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
void ConnectPublisher(SOCKET socket);/*
	Funkcija:
	------------------------------
	Funkcionalnost:
	Povratna vrednost: Nema
*/
int ConnectSubscriber(SOCKET socket);

void DeactivateThread(node_t **head, int id);
