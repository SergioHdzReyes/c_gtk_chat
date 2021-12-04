//
// Created by sergio on 21/11/21.
//

#include "client_utils.h"

void startGUI()
{
    serverConnected = 0;
    userId = 0;
    userName = "";
    clients = NULL;

    builder = gtk_builder_new_from_file("client.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtkStack = GTK_WIDGET(gtk_builder_get_object(builder, "stack"));

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
    *(int *)&datos->userId = 0;
    *(int *)&datos->type = CH_CONNECT;
    strcpy((char *)&datos->content, userName);

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
        g_print("Respuesta recibida: %s\n", read);
        g_print("Bytes recibidos: %d\n", bytes_received);
        if (!bytes_received) continue;

        request = (struct requestStrc *)&read;
        g_print("ID_ASIGNADO: %d", request->userId);

        // Se responde a cliente
        processResponse(*request);
    }
}

void sendConexion()
{

}

void processResponse(struct requestStrc request)
{
    g_print("[processResponse] start\n");
    pthread_t threadId;

    switch (request.type) {
        case CH_ADD:
            printf("AGREGADO\n");
            break;
        case CH_MSG:
            printf("MENSAJE\n");
            break;
        case CH_REMOVE:
            printf("REMOVER\n");
            break;
        case CH_CONNECT:
            userId = request.userId;
            connectServer(request.content);
            break;
        default:
            break;
    }
}

void connectServer(char content[512])
{
    g_print("[connectServer] start\n");
    clients = (struct clientList *)content;

    serverConnected = 1;
}

void loginBtnSendClicked(GtkButton *btn)
{
    g_print("[loginBtnSendClicked] start\n");
    GtkWidget *gtkLabelMsgs = GTK_WIDGET(gtk_builder_get_object(builder, "loginLabelMsgs"));
    GtkWidget *alias = GTK_WIDGET(gtk_builder_get_object(builder, "loginInputName"));
    GtkEntryBuffer *inputName = gtk_entry_get_buffer((GtkEntry*) alias);
    userName = (char *)gtk_entry_buffer_get_text(inputName);

    if (!strcmp(userName, "")) {
        gtk_label_set_text(GTK_LABEL(gtkLabelMsgs), (const gchar*)"Ingrese un nombre válido.");
        return;
    }

    g_thread_new(NULL, (GThreadFunc)receiveConexions, NULL);
    sleep(1);
    if (!serverConnected) {
        gtk_label_set_text(GTK_LABEL(gtkLabelMsgs), (const gchar*)"Servidor no encontrado, intente nuevamente.");
        return;
    }

    refreshUsersList();
    gtkFixedSelectUser = GTK_WIDGET(gtk_builder_get_object(builder, "selectUserFixed"));
    gtk_stack_set_visible_child((GtkStack*) gtkStack, gtkFixedSelectUser);
}

void refreshUsersList()
{
    g_print("[refreshUsersList] start\n");

    struct clientList *list = clients;
    gtkViewSelectUser = GTK_WIDGET(gtk_builder_get_object(builder, "selectUsersView"));
    gtkGridSelectUser = GTK_WIDGET(gtk_builder_get_object(builder, "selectUsersGrid"));

    int row = 0;
    while (list->id != 0) {
        gtk_grid_insert_row((GtkGrid *)gtkGridSelectUser, row);

        button[row] = gtk_button_new_with_label(list->name);
        gtk_grid_attach(GTK_GRID(gtkGridSelectUser), button[row], 1, row, 2, 1);
        g_signal_connect (button[row], "clicked", G_CALLBACK (selectedUser), list);

        /*gtkCheckBtnSelectUser[row] = gtk_check_button_new_with_label(list->name);
        gtk_grid_attach(GTK_GRID(gtkGridSelectUser), gtkCheckBtnSelectUser[row], 1, row, 1, 1);*/

        /*label[row] = gtk_label_new(list->name);
        gtk_label_set_justify(GTK_LABEL(label[row]), GTK_JUSTIFY_LEFT);
        gtk_label_set_xalign(GTK_LABEL(label[row]), 0);
        gtk_grid_attach(GTK_GRID(gtkGridSelectUser), label[row], 2, row, 1, 1);*/

        list++;
        row++;
    }

    gtk_widget_show_all(gtkGridSelectUser);
}

// SIGNALS
void onWindowDestroy()
{
    CLOSESOCKET(scktRecv);
    gtk_main_quit();
}

void selectedUser(GtkButton *btn, struct clientList *clientInfo)
{
    g_print("[selectedUser] start\n");

    gtkFixedChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatFixed"));
    gtkGridChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatGrid"));
    gtkLabelUserNameChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatLabelUserName"));
    gtkEntryMsgChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatEntryMsg"));
    gtkBtnSendChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatBtnSend"));

    gtk_label_set_text(GTK_LABEL(gtkLabelUserNameChat), clientInfo->name);
    gtk_stack_set_visible_child((GtkStack*) gtkStack, gtkFixedChat);
}