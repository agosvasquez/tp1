#include <stdio.h>
#include "server.h"
#include "client.h"


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: ./server <puerto>");
        return 1;
    }
    server_t server;

    printf("llamo al create\n");
    server_create(&server);

    char* service = argv[1];
    server_run(&server, service); 
    server_destroy(&server);
}