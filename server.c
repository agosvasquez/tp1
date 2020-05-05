#include "server.h"
#include "socket.h"
#include "stdint.h"
#include "error.h"
#include <byteswap.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int server_create(server_t* self){
    socket_t* socket = malloc(sizeof(socket_t));;
    socket_create(socket);
    self->socket = socket;
    return 0;
}

int server_destroy(server_t* self){
    socket_shutdown(self->socket, SHUT_RDWR);
    socket_destroy(self->socket);
    free(self->socket);
    return 0;
}
int server_receive(client_t* client, decode_t*decode, int size){
        char aux[size];
        memset(aux, 0, size);
        int bytes = socket_receive(client->socket, aux, size);
        if (bytes < 0) throw_sterr("Recive fails", strerror(errno) );
        // client closed
        if (bytes == 0) return bytes;
        buffer_save_data(decode->bytes, aux , size);
        return bytes;
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
        char* msj;
        int bytes = socket_receive(client.socket,(char*)buff, sizeof(buff));
        printf("bytes: %d\n", bytes);
        if (bytes < 0) throw_sterr("Recive fails", strerror(errno) );
        // client closed
        if (bytes == 0) break;
        int body_size = extract_body_size((char*)buff);
         printf("body size %d \n", body_size);
        int array_size = extract_array_size((char*)buff);
        uint32_t msj_id = extract_msj_id((char*)buff);
        int array_padd_size = number_padd(array_size, 8) + array_size;
        printf("size arr without padding %d \n", array_size);
        printf("array_pad_size %d\n", array_padd_size);

        int to_read = body_size + array_padd_size;
        printf("buff size:%d\n", to_read);

        decoded_create_size(&decode, to_read+1);
        server_receive(&client, &decode, to_read);
        // receive del cuerpo!!!
        uint8_t * hexa = (uint8_t*) (&decode)->bytes->data;
        for (int i = 0; i < to_read; i++){
                if (i > 0) printf(":");
                printf("%02X", hexa[i]);
        }
        printf("\n");

        decode_messaje(&decode);
        
        decoded_output(&decode,msj_id);
        socket_send(client.socket, message, sizeof(message));
        
        decoded_destroyed(&decode);
    }
    printf("afuera");
    client_destroy(&client);
    return 0;
}
