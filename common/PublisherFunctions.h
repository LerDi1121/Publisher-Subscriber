#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <conio.h>
#include "..\common\AllEnums.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

/*
	Funkcija: ChoseAtRandom
	----------------------------
	Funkcionalnost: Nasumicno generise broj 0 ili 1.
	Povratna vrednost: Generisan broj.

*/
int ChoseAtRandom();

/*
	Funkcija: GenerateRandomTopic
	----------------------------
	Funkcionalnost: Na osnovu ChoseAtRandom funkcije, nasumicno se bira topic.
	Povratna vrednost: Odabrani topic.

*/
enum Topic GenerateRandomTopic();

/*
	Funkcija: GenerateRandomType
	----------------------------
	Funkcionalnost: Na osnovu ChoseAtRandom funkcije, nasumicno se bira type ako se radi o Status topic-u.
	Povratna vrednost: Odabrani type.

*/
enum TypeTopic GenerateRandomType(enum Topic topic);

/*
	Funkcija: PrintPublisherInfo
	----------------------------
	Funkcionalnost: Ispisuje informacije o publisher-u.
	Povratna vrednost: Nema.

*/
void PrintPublisherInfo(enum Topic topic, enum TypeTopic type);
