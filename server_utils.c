//
// Created by sergio on 20/11/21.
//

#include "server_utils.h"

int setConfigs()
{
    listClients = NULL;
    clientsCount = 0;

    return TRUE;
}

void startGUI()
{
    builder = gtk_builder_new_from_file("server.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    g_signal_connect(window, "destroy", (GCallback)onWindowDestroy, NULL);
    gtk_builder_connect_signals(builder, NULL);
    fixed = GTK_WIDGET(gtk_builder_get_object(builder, "fixed"));

    gtk_widget_show(window);
}

void *receiving(void *threadId)
{
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *bind_address;
    getaddrinfo(0, PORT, &hints, &bind_address);

    printf("Creating socket...\n");
    socketListen = socket(bind_address->ai_family,
                           bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socketListen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        pthread_exit(NULL);
    }

    printf("Binding socket to local address...\n");
    if (bind(socketListen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        pthread_exit(NULL);
    }
    freeaddrinfo(bind_address);

    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);

    while (1) {
        char read[1024] = "";
        struct requestStrc *request = NULL;
        int bytes_received = recvfrom(socketListen, read, 1024,
                                      0, (struct sockaddr *) &client_address, &client_len);
        g_print("\nNueva conexion [receiving]\n");
        request = (struct requestStrc *)&read;
        struct clientInfo clientData = {0, "", "", ""};
        printf("TIPO: %d, CONTENT: %s\n", request->type, request->content);

        getnameinfo(((struct sockaddr *) &client_address),
                    client_len,
                    clientData.host, sizeof(clientData.host),
                    clientData.port, sizeof(clientData.port),
                    NI_NUMERICHOST | NI_NUMERICSERV);
        printf("%s %s\n", clientData.host, clientData.port);

        // Se responde a cliente
        processRequest(&clientData, *request);
    }
};

void processRequest(struct clientInfo *clientData, struct requestStrc request)
{
    g_print("[processRequest] start\n");

    pthread_t threadId;
    clientData->id = request.userId;

    switch (request.type) {
        case CH_ADD_USER:
            printf("AÑADIR\n");
            strcpy(clientData->name, request.content);
            pthread_create(&threadId, NULL, addClient, clientData);
            clientsCount++;
            break;
        case CH_MSG:
            printf("MENSAJE\n");
            break;
        case CH_REMOVE:
            printf("REMOVER\n");
            break;
        case CH_CONNECT:
            strcpy(clientData->name, request.content);
            pthread_create(&threadId, NULL, connectClient, clientData);
            printf("CONNECT\n");
            break;
        default:
            break;
    }
}

void sending(struct requestStrc *response, char *host, char *port)
{
    g_print("[sending] start\n");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(host, port, &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }

    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
                         peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }

    ssize_t bytes_sent = sendto(socket_peer,
                            response, sizeof(*response),
                            0,
                            peer_address->ai_addr, peer_address->ai_addrlen);
    printf("Sent %d bytes.\n", (int)bytes_sent);

    freeaddrinfo(peer_address);
    CLOSESOCKET(socket_peer);
    printf("Finished.\n");
}

void *addClient(void *args)
{
    struct clientInfo *clientData = args;
    printf("[addClient] %s, %s, %s", clientData->host, clientData->port, clientData->name);
    insertClient(&listClients, clientData);

    struct requestStrc response = {CH_ADD_USER, (char)1};
    sending(&response, clientData->host, clientData->port);
}

void *connectClient(void *args)
{
    g_print("[connectClient] start\n");

    struct clientInfo *clientData = args;
    printf("[connectClient] %s, %s, %s, %d\n", clientData->host, clientData->port, clientData->name, clientData->id);
    insertClient(&listClients, clientData);

    struct clientList list[10];

    // Se prepara listado de clientes
    struct clientsStruct *clients = listClients;

    int count = 0;
    while (clients != NULL) {
        if (clientData->id == clients->clientDt.id) {
            clients = clients->next;
            continue;
        }

        list[count].id = clients->clientDt.id;
        strcpy(list[count].name, clients->clientDt.name);
        printf("[SHR] %s, %s, %s, %d\n", clients->clientDt.host, clients->clientDt.port, clients->clientDt.name, clients->clientDt.id);

        clients = clients->next;
        count++;
    }

    // Se prepara respuesta a cliente
    struct requestStrc response;
    response.userId = clientData->id;
    response.type = CH_CONNECT;
    memcpy(response.content, list, sizeof(list));

    sending(&response, clientData->host, clientData->port);
    refreshUsersList();
}

void refreshUsersList()
{
    struct clientList list[10] = {};

    // Se prepara listado de clientes
    struct clientsStruct *clients = listClients;

    int count = 0;
    while (clients != NULL) {
        list[count].id = clients->clientDt.id;
        strcpy(list[count].name, clients->clientDt.name);
        printf("[SHR] %s, %s, %s, %d\n", clients->clientDt.host, clients->clientDt.port, clients->clientDt.name, clients->clientDt.id);

        clients = clients->next;
        count++;
    }

    // Se prepara respuesta a cliente
    struct requestStrc response = {};
    response.userId = 0;
    response.type = CH_USERS_REFRESH;
    memcpy(response.content, list, sizeof(list));

    struct clientsStruct *clients2 = listClients;
    while (clients2 != NULL) {
        sending(&response, clients2->clientDt.host, clients2->clientDt.port);
        clients2 = clients2->next;
    }
}

void onWindowDestroy()
{
    CLOSESOCKET(socketListen);
    gtk_main_quit();
}

void insertClient(struct clientsStruct **clients, struct clientInfo *clientData)
{
    g_print("[insertClient] start\n");

    clientsCount++;
    clientData->id = clientsCount;
    printf("Se inserta cliente: nombre-%s, host-%s, puerto-%s\n", clientData->name, clientData->host, clientData->port);

    struct clientsStruct *newClient = (struct clientsStruct*) malloc(sizeof(struct clientsStruct));
    newClient->clientDt = *clientData;
    newClient->next = (*clients);

    (*clients) = newClient;
}