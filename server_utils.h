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
//   -----------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <gtk/gtk.h>

// Tipos de solicitudes
#define CH_ADD_USER 1
#define CH_MSG      2
#define CH_REMOVE   3
#define CH_CONNECT  4
#define CH_USERS_REFRESH 5
#define PORT "19900"

struct requestStrc {
    int userId;
    int type;
    char content[512];
};

struct msgStrct {
    int destUser;
    char msg[400];
};

struct clientInfo {
    int id;
    char host[20];
    char port[20];
    char name[30];
};

struct clientsStruct {
    struct clientInfo clientDt;
    struct clientsStruct *next;
} *listClients;

struct clientList {
    int id;
    char name[30];
};

SOCKET socketListen;

pthread_t clientId, serverId;
int clientsCount;

// Variables UI
GtkWidget *window;
GtkBuilder *builder;
GtkWidget *fixed;

// Establece ambiente inicial
int setConfigs();

// Inicializa interfaz gráfica
void startGUI();

// Se reciben conexiones entrantes tipo UDP
void *receiving(void *threadId);

// Se procesa la información recibida para escoger
// el tipo de transacción a realizar
void processRequest(struct clientInfo *, struct requestStrc);

// Se envía respuesta a cliente
void sending(struct requestStrc *response, char *host, char *port);

// Agrega un cliente a la BD
void *addClient(void *args);

// Conecta a un cliente
void *connectClient(void *args);

// Envía mensaje a cliente destino
void *sendMsg(void *args);

// Envia listado de usuarios disponibles a todos los clientes
void refreshUsersList();

// Evento para cerrar la ventana
void onWindowDestroy();

// Funciones para manipular lista enlazada de clientes
void insertClient(struct clientsStruct **clients, struct clientInfo *clientData);
struct clientInfo searchClient();
void removeClient(struct clientInfo *clientData);

#endif //C_GTK_SUBSCRIPTION_SERVER_UTILS_H
