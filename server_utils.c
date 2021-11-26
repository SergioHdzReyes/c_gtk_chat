//
// Created by sergio on 20/11/21.
//

#include "server_utils.h"

void startGUI()
{
    builder = gtk_builder_new_from_file("server.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    g_signal_connect(window, "gtk_main_quit", NULL, NULL);
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
    getaddrinfo(0, "1990", &hints, &bind_address);

    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
                           bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        pthread_exit(NULL);
    }

    printf("Binding socket to local address...\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        pthread_exit(NULL);
    }
    freeaddrinfo(bind_address);

    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);

    while (1) {
        char read[1024] = "";
        struct requestStrc *request = NULL;
        int bytes_received = recvfrom(socket_listen, read, 1024,
                                      0, (struct sockaddr *) &client_address, &client_len);
        request = (struct requestStrc *)&read;
        struct clientInfo clientData = {"", "", ""};
        printf("TIPO: %d, CONTENT: %s", request->type, request->content);

        getnameinfo(((struct sockaddr *) &client_address),
                    client_len,
                    clientData.host, sizeof(clientData.host),
                    clientData.port, sizeof(clientData.port),
                    NI_NUMERICHOST | NI_NUMERICSERV);
        printf("%s %s\n", clientData.host, clientData.port);

        // Se responde a cliente
        processRequest(&clientData, *request);
    }

    CLOSESOCKET(socket_listen);
    printf("Finished.\n");
};

void processRequest(struct clientInfo *clientData, struct requestStrc request)
{
    pthread_t threadId;

    switch (request.type) {
        case ADD:
            strcpy(clientData->name, request.content);
            printf("DAR DE ALTA: host=%s, port=%s, name=%s\n",
                   clientData->host, clientData->port, clientData->name);
            pthread_create(&threadId, NULL, sending, clientData);
            clientsCount++;
            break;
        case MSG:
            printf("MENSAJE\n");
            break;
        case REMOVE:
            printf("REMOVER\n");
            break;
        default:
            break;
    }
}

void *sending(void *args)
{
    struct clientInfo *clientData = args;
    printf("SENDING:\n");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo *peer_address;
    if (getaddrinfo("127.0.0.1", clientData->port, &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return NULL;
    }

    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
                         peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return NULL;
    }

    const char *message = "Mensaje enviado desde el servidor";
    printf("Sending: %s\n", message);
    int bytes_sent = sendto(socket_peer,
                            message, strlen(message),
                            0,
                            peer_address->ai_addr, peer_address->ai_addrlen);
    printf("Sent %d bytes.\n", bytes_sent);

    freeaddrinfo(peer_address);
    CLOSESOCKET(socket_peer);
    printf("Finished.\n");
}

void onWindowDestroy()
{
    gtk_main_quit();
}