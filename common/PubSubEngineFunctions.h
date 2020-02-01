#pragma once
#define HEADER_H
#include "PubSubEngineStruct.h"

subscriber_t* CreateSubscriber(SOCKET socket, int topic);

DWORD WINAPI RcvMessage(LPVOID param);
void AddToList(node_t** head, HANDLE value);
void AddSubscriberToList(subscriber_t** sub);
void AddToConcreteList(node_subscriber_t** list, subscriber_t** sub);
SOCKET* CreateAcceptSocket(SOCKET Listen);
char* Enqueue(char** queue, char* msg, int msg_size);
void CreateQueue(char** msgQueue);

SOCKET* CreatePublisherListenSocket();
SOCKET* CreateSubscriberListenSocket();
/////*******
void WriteMessage(char* message);// za poseban tred koji ce upisivati u red
DWORD WINAPI  AddMessageToQueue(LPVOID param);

DWORD WINAPI ListenSubscriber(LPVOID param);
void RemoveSubscriber(subscriber_t* sub);
void RemoveSubscriberFromList(int id, node_subscriber_t** list);
void InitializeOurCriticalSection();
void DeleteOurCriticalSection();
void LitenForPublisher(SOCKET publisherListenSocket);

