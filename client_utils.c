//
// Created by sergio on 21/11/21.
//

#include "client_utils.h"

void startGUI()
{
    builder = gtk_builder_new_from_file("client.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    g_signal_connect(window, "gtk_main_quit", NULL, NULL);
    g_signal_connect(window, "destroy", (GCallback)onWindowDestroy, NULL);
    gtk_builder_connect_signals(builder, NULL);

    gtk_widget_show(window);
}

void receiveConexions()
{
    printf("Comienza configuración para enviar conexion a servidor...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo *peer_address;
    if (getaddrinfo("127.0.0.1", "1990", &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() fallido. (%d)\n", GETSOCKETERRNO());
        return;
    }

    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST | NI_NUMERICSERV);
    printf("Dirección remota: %s %s\n", address_buffer, service_buffer);
    printf("Creando socket...\n");

    scktRecv = socket(peer_address->ai_family,
                         peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(scktRecv)) {
        fprintf(stderr, "socket() fallido. (%d)\n", GETSOCKETERRNO());
        return;
    }

    struct requestStrc *datos = malloc(sizeof(struct requestStrc));
    *(int *)&datos->type = ADD;
    strcpy((char *)&datos->content, "Sergio Hernandez Reyes");

    ssize_t bytes_sent = sendto(scktRecv,
                            datos, sizeof(struct requestStrc),
                            0,
                            peer_address->ai_addr, peer_address->ai_addrlen);
    printf("Sent %d bytes.\n", (int)bytes_sent);

    // Cambio de dirección en mismo socket, ahora recibe conexiones
    while(1) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        char read[1024] = "";
        struct requestStrc *request = NULL;

        int bytes_received = recvfrom(scktRecv, read, 1024,
                                      0, (struct sockaddr *) &client_address, &client_len);
        printf("RECIBIDOS: %s\n", read);
    }
}

void onWindowDestroy()
{
    CLOSESOCKET(scktRecv);
    gtk_main_quit();
}