#ifndef COMMON_CLIENT_H
#define COMMON_CLIENT_H
#include "common_socket.h"
#include "common_parser.h"

typedef struct client_t {
    socket_t* socket;
}client_t;

//Crear cliente
int client_create(client_t* self, socket_t* socket);

//Destruit cliente
int client_destroy(client_t* self);

//Se contecta al servidor envia el archivo y espera el resultado
int client_run(client_t* self, const char* h, const char* s, FILE* f);

//Envia el mensaje codificiado con el protocolo d-bus
int client_send_encode(client_t* self, char* s, buffer_t* b, FILE* f);

void client_output(encode_t* encode, char* server_respose);

#endif
