//
// Created by sergio on 20/11/21.
//

#include "server_utils.h"

int main (int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    startGUI();
    g_thread_new(NULL, (GThreadFunc)receiving, NULL);

    gtk_main();

    return EXIT_SUCCESS;
}