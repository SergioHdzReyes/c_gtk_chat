//
// Created by sergio on 21/11/21.
//

#include "client_utils.h"

void startGUI()
{
    serverConnected = 0;
    userId = 0;
    historyMsg = 0;
    userName = "";
    clients = NULL;

    builder = gtk_builder_new_from_file("client.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtkStack = GTK_WIDGET(gtk_builder_get_object(builder, "stack"));

    // Vista seleccionar usuario
    gtkViewSelectUser = GTK_WIDGET(gtk_builder_get_object(builder, "selectUsersView"));
    gtkGridSelectUser = GTK_WIDGET(gtk_builder_get_object(builder, "selectUsersGrid"));
    gtk_window_set_title(GTK_WINDOW(window), "Chat UDP");

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
    if (getaddrinfo(SERVER_HOST, SERVER_PORT, &hints, &peer_address)) {
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
        struct requestStrc *response = NULL;

        int bytes_received = recvfrom(scktRecv, read, 1024,
                                      0, (struct sockaddr *) &client_address, &client_len);
        g_print("\n\nRespuesta recibida: %s\n", read);
        g_print("Bytes recibidos: %d\n", bytes_received);
        if (!bytes_received) continue;

        response = (struct requestStrc *)&read;
        g_print("ID_ASIGNADO: %d\n", response->userId);

        // Se responde a cliente
        processResponse(*response);
    }
}

// PENDIENTE
void sendConexion(struct requestStrc *request)
{
    g_print("[sendConexion] start\n");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(SERVER_HOST, SERVER_PORT, &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }

    SOCKET serverSocket;
    serverSocket = socket(peer_address->ai_family,
                         peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(serverSocket)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }

    ssize_t bytes_sent = sendto(serverSocket,
                                request, sizeof(*request),
                                0,
                                peer_address->ai_addr, peer_address->ai_addrlen);
    printf("Sent %d bytes.\n", (int)bytes_sent);

    freeaddrinfo(peer_address);
    CLOSESOCKET(serverSocket);
    printf("Finished.\n");
}

void processResponse(struct requestStrc response)
{
    g_print("[processResponse] start\n");

    switch (response.type) {
        case CH_ADD:
            printf("AGREGADO\n");
            break;
        case CH_MSG:
            g_idle_add ((GSourceFunc) msgReceived, &response   );
            printf("MENSAJE\n");
            break;
        case CH_REMOVE:
            printf("REMOVER\n");
            break;
        case CH_CONNECT:
            printf("CONECTAR\n");
            userId = response.userId;
            connectServer(response.content);
            break;
        case CH_USERS_REFRESH:
            printf("ACTUALIZAR USUARIOS\n");
            g_idle_add ((GSourceFunc) updateUsersList, response.content   );
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
    gtk_window_set_title(GTK_WINDOW(window), userName);

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
    int row = 0;
    while (button[row] != NULL) {
        gtk_grid_remove_row((GtkGrid *)gtkGridSelectUser, 0);
        row++;
    }

    row = 0;
    while (list->id != 0) {
        if (list->id == userId) {
            list++;
            continue;
        }

        g_print("INSERTANDO: user=%s, id=%d\n", list->name, list->id);
        gtk_grid_insert_row((GtkGrid *)gtkGridSelectUser, row);

        button[row] = gtk_button_new_with_label(list->name);
        gtk_grid_attach(GTK_GRID(gtkGridSelectUser), button[row], 1, row, 2, 1);
        g_signal_connect (button[row], "clicked", G_CALLBACK (selectedUser), list);

        list++;
        row++;
    }

    gtk_widget_show_all(gtkGridSelectUser);
}

void *updateUsersList(void *args)
{
    g_print("[updateUsersList] start\n");

    clients = (struct clientList *)args;

    /*int row = 0;
    while (list->id != 0) {
        gtk_grid_insert_row((GtkGrid *)gtkGridSelectUser, row);

        button[row] = gtk_button_new_with_label(list->name);
        gtk_grid_attach(GTK_GRID(gtkGridSelectUser), button[row], 1, row, 2, 1);
        g_signal_connect (button[row], "clicked", G_CALLBACK (selectedUser), list);

        list++;
        row++;
    }

    gtk_widget_show_all(gtkGridSelectUser);*/

    refreshUsersList();

    return NULL;
}

void *msgReceived(void *args)
{
    g_print("[msgReceived] start\n");
    struct requestStrc *response = args;

    char msg[200] = "";
    strcpy(msg, curUserChatName);
    strcat(msg, ": ");
    strcat(msg, response->content);

    gtk_grid_insert_row(GTK_GRID(gtkGridChat), historyMsg);
    button[historyMsg] = gtk_button_new_with_label(msg);
    gtk_grid_attach(GTK_GRID(gtkGridChat), button[historyMsg], 1, historyMsg, 1, 1);
    gtk_widget_show_all(gtkGridChat);
    historyMsg++;

    return NULL;
}

struct clientList *searchClient(int usrId)
{
    g_print("[searchClient] start\n");
    g_print("USER_ID: %d\n", usrId);

    struct clientList *list = clients;
    struct clientList *clnt = NULL;

    while (list->id > 0) {
        printf("Procesando: %s, %d\n", list->name, list->id);
        if (list->id == usrId) {
            clnt = malloc(sizeof(struct clientList));
            clnt->id = list->id;
            strcpy(clnt->name, list->name);
            break;
        }

        list++;
    }

    return clnt;
}

// SIGNALS
void onWindowDestroy()
{
    CLOSESOCKET(scktRecv);
    gtk_main_quit();
}

void selectedUser(GtkButton *btn, struct clientList *clientInfo)
{
    curUserChatId = clientInfo->id;
    curUserChatName = malloc(sizeof(clientInfo->name));
    strcpy(curUserChatName, clientInfo->name);
    g_print("[selectedUser] start\n");

    gtkFixedChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatFixed"));
    gtkLabelUserNameChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatLabelUserName"));
    gtkEntryMsgChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatEntryMsg"));

    gtkBtnSendChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatBtnSend"));
    g_signal_connect (gtkBtnSendChat, "clicked", G_CALLBACK (sendMsgClicked), NULL);

    gtk_label_set_text(GTK_LABEL(gtkLabelUserNameChat), clientInfo->name);
    gtkGridChat = GTK_WIDGET(gtk_builder_get_object(builder, "chatGrid"));

    gtk_stack_set_visible_child((GtkStack*) gtkStack, gtkFixedChat);
}

void sendMsgClicked(GtkButton *btn)
{
    g_print("[sendMsgClicked] start\n");

    // Se da formato a mensaje para pintarlo en historial
    GtkEntryBuffer *inputMsg = gtk_entry_get_buffer(GTK_ENTRY(gtkEntryMsgChat));
    char *msg = (char *)gtk_entry_buffer_get_text(inputMsg);

    // Se envia mensaje a servidor
    struct requestStrc request = {userId, CH_MSG, ""};
    struct msgStrct msgRequest = {curUserChatId, ""};
    strcpy(msgRequest.msg, msg);
    memcpy(request.content, &msgRequest, sizeof(msgRequest));
    sendConexion(&request);

    char fullMsg[200] = "";
    strcat(fullMsg, "Yo: ");
    strcat(fullMsg, msg);

    // Se pinta mensaje en historial
    gtk_grid_insert_row(GTK_GRID(gtkGridChat), historyMsg);
    button[historyMsg] = gtk_button_new_with_label(fullMsg);
    gtk_grid_attach(GTK_GRID(gtkGridChat), button[historyMsg], 1, historyMsg, 1, 1);

    gtk_entry_set_text(GTK_ENTRY(gtkEntryMsgChat), "");
    gtk_widget_show_all(gtkGridChat);
    historyMsg++;
}
