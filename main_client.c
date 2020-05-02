#include <stdio.h>
#include "client.h"

int main(int argc, char *argv[]){

    if ( argc < 3 || argc > 4 ){
        perror("Invalid arguments");
        return 1;
    }
    
    FILE* input;
    client_t client;
    char* host = argv[1];
    char* port = argv[2];
    if( argc == 4){
        input = fopen(argv[3], "r");
    }else {
        input = stdin;
    }

    client_create(&client);
    client_run(&client,host,port, input);

    if (input != stdin) fclose(input);
    
    client_destroy(&client);

    return 0;
}