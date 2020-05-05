#ifndef CLIENT_H
#define CLIENT_H
#include "socket.h"
#include "buffer.h"
#include <stdio.h>
#include "parser.h"

typedef struct client_t {
    socket_t* socket;
}client_t;

//Crear cliente
int client_create(client_t* self);

//Destruit cliente
int client_destroy(client_t* self);

//Se contecta al servidor envia el archivo y espera el resultado
int client_run(client_t* self, const char* host, const char* service, FILE* file);

//Envia el mensaje codificiado con el protocolo d-bus
int client_send_encode(client_t* self, char* buff, buffer_t* d_buff, FILE* file);

void client_output(encode_t* encode, char* server_respose);

#endif