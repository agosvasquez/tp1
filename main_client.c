#include <stdio.h>
#include "client.h"

int main(int argc, char *argv[]){
    if ( argc < 2 || argc > 3 ){
        perror("Invalid arguments");
        return 1;
    }

    FILE* input;
    client_t client;
    char* host = argv[1];
    char* port = argv[2];
    if( argc == 3){
        input = fopen(argv[3], "r");
    }else {
        input = stdin;
    }

    client_create(&client);
    client_run(&client,host,port, input);
    client_destroy(&client);

    return 0;
}