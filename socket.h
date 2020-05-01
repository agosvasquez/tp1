#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

typedef struct socket_t{
    int socket;
} socket_t;

// crear el socket
int socket_create(socket_t* self);

//destruir el socket
int socket_destroy(socket_t* self);

// Asignarle una direccion y lo marca como socket pasivo (server)
int socket_bind_and_listen(socket_t* self, const char* service);

// Cuando se conecta un cliente lo asigna a accepted_socket
// 0 exito -1 error
int socket_accept(socket_t* self, socket_t* accepted_socket );

// Dada una direccion y servicio/puerto se conecta a un servidor (cliente)
// 0 exito -1 error
int socket_connect(socket_t* self, const char* host_name, const char* service);

//envia "lenght" bytes utilizando el socket 
//devuelve la cantidad de bytes recibidos o -1 error
int socket_send(socket_t* self, char* buffer , size_t length);

//recibe "lenght" bytes utilizando el socket 
//devuelve la cantidad de bytes recibidos o -1 error
int socket_receive(socket_t* self, char* buffer, size_t legth);

//Cierra el "channel" (lectura/escritura) del socket
// 0 exito -1 error
void socket_shutdown(socket_t* self, int channel);

#endif
