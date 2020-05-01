#include "server.h"
#include "socket.h"
#include "client.h"
#include "stdint.h"

int server_create(server_t* self){
    socket_t socket;
    socket_create(&socket);
    self->socket = &socket;
    return 0;
}

int server_destroy(server_t* self){
    socket_shutdown(self->socket, 2);
    return 0;
}

int server_run(server_t* self, char* service){
    char buff[32]= "";
    char message[3] = "OK\0";
    client_t client;

    socket_bind_and_listen(self->socket, service);
    
    client_create(&client);
    printf("socket server fd %d\n", self->socket->socket);
    printf("socket client fd %d\n", (&client)->socket->socket);
    socket_accept(self->socket, (&client)->socket);
    // en el programa real extraer la longitud
    // de los primeros bytes
    socket_receive((&client)->socket,buff, sizeof(buff));
    printf("mesaje  recibido: %s\n", buff);
    
    socket_send((&client)->socket, message, sizeof(message));

    client_destroy(&client);
    server_destroy(self);

    return 0;
}


