# Compilación

    gcc -o client client.c client_utils.c `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
    gcc -o server server.c server_utils.c `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

    ./server
    
    ./client
    ./client
    .
    .
    .
    n