#include "server.h"
#include "socket.h"
#include "client.h"
#include "stdint.h"
#include "parser.h"
#include "error.h"
#include <byteswap.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int server_create(server_t* self){
    socket_t socket;
    socket_create(&socket);
    self->socket = &socket;
    return 0;
}

int server_destroy(server_t* self){
    socket_shutdown(self->socket, SHUT_RDWR);
    return 0;
}

int server_run(server_t* self, char* service){
    char message[3] = "OK\0";
    client_t client;
    decode_t decode;
    socket_bind_and_listen(self->socket, service);
    
    client_create(&client);
    //printf("socket server fd %d\n", self->socket->socket);
    //printf("socket client fd %d\n", (&client)->socket->socket);
    socket_accept(self->socket, client.socket);
    // en el programa real extraer la longitud
    // de los primeros bytes
    while(1){
        printf("arranco\n");
        uint8_t buff[16]= "";
        int bytes = socket_receive(client.socket,(char*)buff, sizeof(buff));
        printf("bytes: %d", bytes);
        if (bytes < 0) throw_sterr("Recive fails", strerror(errno) );
        // client closed
        if (bytes == 0) break;
        int body_size = extract_array_size((char*)buff);
        int array_size = extract_array_size((char*)buff);
        int array_padd_size = number_padd(array_size, 8) + array_size;
        for (int i = 0; i < sizeof(buff); i++){
                if (i > 0) printf(":");
                printf("%02X", buff[i]);
        }
        printf("\n");
        printf("size:%d", array_padd_size);
        decoded_create_size(&decode, array_padd_size+1);
        socket_receive(client.socket, (&decode)->bytes->data, array_padd_size);
        uint8_t * hexa = (uint8_t*) (&decode)->bytes->data;
        for (int i = 0; i < array_padd_size; i++){
                if (i > 0) printf(":");
                printf("%02X", hexa[i]);
        }
        printf("\n");
        socket_send(client.socket, message, sizeof(message));
        decoded_destroyed(&decode);
    }
    printf("afuera");
    client_destroy(&client);
    server_destroy(self);

    return 0;
}

