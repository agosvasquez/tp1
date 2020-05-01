#include <string.h>
#include "socket.h"
#include "client.h"
#include "parser.h"
#include <stdbool.h>

int client_create(client_t* self){
    socket_t socket;
    socket_create(&socket);
    self->socket = &socket;
    return 0;
}

int client_destroy(client_t* self){
    socket_shutdown(self->socket,2);
    return 0;
}

int client_run(client_t* self, const char* host, const char* service, FILE* file){
    char buff [32]= "HOLA SERVICE\n";
    char buf_recive[3]= "";

    socket_connect(self->socket,host,service);
    
    //client_send_encode(self, buff, file);
    //mirar si no necesita un while
    //solo para prueba
    int bytes = socket_send(self->socket, buff, sizeof(buff));
    printf("Bytes enviados: %d\n", bytes);
    socket_receive(self->socket,buf_recive,sizeof(buf_recive)); 
    printf("Respuesta service: %s\n",buf_recive);
    client_destroy(self);
    return 0;
}

int client_send_encode(client_t* self, char* buff, FILE* file){
    dbus_encode_t encoded;
    int bytes;
    encoded_create(&encoded);
    while( !feof(file)){
        size_t rv = fread(buff, 1, sizeof(buff), file);
        encode_file(&encoded,buff, rv);
        bytes = socket_send(self->socket, (&encoded)->bytes,(&encoded)->size);
        if(bytes < 0){
            perror("Error when send");
            return -1;
        }     
    }
    return 0;
}