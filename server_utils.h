//
// Created by sergio on 20/11/21.
//

#ifndef C_GTK_SUBSCRIPTION_SERVER_UTILS_H
#define C_GTK_SUBSCRIPTION_SERVER_UTILS_H

//   -----------------------------------------------------
#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
//   -----------------------------------------------------

#include <gtk/gtk.h>


// REQUESTS TYPE
#define ADD     1
#define MSG     2
#define REMOVE  3
#define MAX_CLIENTS 100

struct requestStrc {
    int type;
    char content[100];
};

struct clientInfo {
    char host[20];
    char port[20];
    char name[30];
};

pthread_t clientId, serverId;
struct clientInfo clients[MAX_CLIENTS];
int clientsCount;

// Variables UI
GtkWidget *window;
GtkBuilder *builder;
GtkWidget *fixed;

void startGUI();
void *receiving(void *threadId);
void *sending(void *threadId);
void processRequest(struct clientInfo *, struct requestStrc);
void onWindowDestroy();

#endif //C_GTK_SUBSCRIPTION_SERVER_UTILS_H
