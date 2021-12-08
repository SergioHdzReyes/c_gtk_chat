//
// Created by sergio on 21/11/21.
//

#ifndef C_GTK_SUBSCRIPTION_CLIENT_UTILS_H
#define C_GTK_SUBSCRIPTION_CLIENT_UTILS_H

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

// REQUESTS TYPE
#define CH_ADD      1
#define CH_MSG      2
#define CH_REMOVE   3
#define CH_CONNECT  4
#define CH_USERS_REFRESH 5

// Información de servidor
#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT "19900"

struct requestStrc {
    int userId;
    int type;
    char content[512];
};

struct msgStrct {
    int destUser;
    char msg[400];
};

struct clientList {
    int id;
    char name[30];
} *clients;

SOCKET scktRecv;
int serverConnected, userId, curUserChatId, historyMsg;
char *userName, *curUserChatName;


// Variables UI
GtkWidget *window;
GtkBuilder *builder;
GtkWidget *gtkStack;

// Vista "Login"
GtkWidget *gtkFixedLogin;
//GtkWidget *gtkLabelMsgs;

// Vista "Lista de usuarios disponibles"
GtkWidget *gtkFixedSelectUser;
GtkWidget *gtkViewSelectUser;
GtkWidget *gtkGridSelectUser;
//GtkWidget *gtkCheckBtnSelectUser[100];
GtkWidget *button[100];
//GtkWidget *label[100];

// Vista "Chat"
GtkWidget *gtkFixedChat;
GtkWidget *gtkLabelUserNameChat;
GtkWidget *gtkViewChat;
GtkWidget *gtkGridChat;
GtkWidget *gtkEntryMsgChat;
GtkWidget *gtkBtnSendChat;
GtkWidget *gtkLabelHistoryMsg[100];


void startGUI();
void receiveConexions();
void sendConexion(struct requestStrc *request);
void processResponse(struct requestStrc response);
void connectServer(char content[512]);
void refreshUsersList();
void *updateUsersList(void *args);
void *msgReceived(void *args);
struct clientList *searchClient();

// SIGNALS
void onWindowDestroy();
void selectedUser(GtkButton *button, struct clientList *clientInfo);
void sendMsgClicked(GtkButton *btn);

#endif //C_GTK_SUBSCRIPTION_CLIENT_UTILS_H
