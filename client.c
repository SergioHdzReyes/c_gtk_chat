//
// Created by sergio on 21/11/21.
//

#include "client_utils.h"

int main (int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    startGUI();
    g_thread_new(NULL, (GThreadFunc)receiveConexions, NULL);

    gtk_main();

    return EXIT_SUCCESS;
}