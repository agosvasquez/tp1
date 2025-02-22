#ifndef SERVER_H
#define SERVER_H
#include "common_client.h"

typedef struct server_t{
    socket_t* socket;
} server_t;
 
// crea el servicio. Devuelve 0 con exito -1 error
int server_create(server_t* self);

// destruye el servicio. Devuelve 0 con exito -1 error
int server_destroy(server_t* self);

//Verifica si tiene algun cliente conectandose, y se pone en modo escucha
int server_run(server_t* self, char* service);

int server_receive(socket_t* socket, decode_t*decode, int size);

#endif
