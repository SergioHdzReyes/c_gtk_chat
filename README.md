# Compilación

    gcc -o client client.c `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
    gcc -o server server.c `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

    ./server
    
    ./client
    ./client
    .
    .
    .
    n