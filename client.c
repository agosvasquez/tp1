#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include "socket.h"
#include "client.h"
#include "parser.h"
#include "buffer.h"
#include "error.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

const int INITIAL_SIZE = 32;
//const int INCREASE_FACTOR = 2;

int client_create(client_t* self){
    socket_t* socket = malloc(sizeof(socket_t));
    socket_create(socket);
    self->socket = socket;
    return 0;
}

int client_destroy(client_t* self){
    socket_shutdown(self->socket,SHUT_RDWR);
    socket_destroy(self->socket);
    free(self->socket);
    return 0;
}

int client_run(client_t* self, const char* host, const char* service, FILE* file){
    char buff[INITIAL_SIZE];
    buffer_t dinamic_buff ;
    char buf_recive[3];

    //printf("Holee\n");
    socket_connect(self->socket,host,service);
    int bytes = client_send_encode(self, buff,&dinamic_buff, file);
    //mirar si no necesita un while
    //solo para prueba
    //int bytes = socket_send(self->socket, buff, sizeof(buff));
    //printf("Bytes enviados: %d\n", bytes);
    //socket_receive(self->socket,buf_recive,sizeof(buf_recive)); 
    //printf("Respuesta service: %s\n",buf_recive);
    //client_destroy(self);
    return 0;
}

void client_increment_id(encode_t* encoded){
    encoded->msj_id += to_little_32(0x0001);
}

int client_send_encode(client_t* self, char* buff, buffer_t* d_buf, FILE* file){
    encode_t encoded;
    int bytes, line_bytes;
    char buf_recive[3];
    buffer_create(d_buf);
    uint32_t msj_id = to_little_32(0x0001);
    while(1){
        encoded_create(&encoded, msj_id);
        char* line = NULL;
        printf("encode buffer used %ld\n", (&encoded)->bytes->used);
        buffer_get_line(d_buf,buff,&line,file);
        printf("line %s", line);
        encode_line(&encoded,line);
        uint8_t * hexa = (uint8_t*)(&encoded)->bytes->data;
        int i;
        for (i = 0; i < (&encoded)->bytes->used; i++){
            if (i > 0) printf(":");
            printf("%02X", hexa[i]);
        }
        printf("\n");
        bytes = socket_send(self->socket, (&encoded)->bytes->data, (&encoded)->bytes->used);
        printf("Bytes enviados: %d\n", bytes);
        socket_receive(self->socket,buf_recive,sizeof(buf_recive)); 
        client_output(&encoded,buf_recive);
        if(bytes < 0) throw_error("error en el send"); 
        free(line);
        encoded_destoyed(&encoded);
        //pregunto si se leyo todo
        msj_id += to_little_32(0x0001);
        if(d_buf->read == d_buf->used) break;

    }
    //socket_receive(self->socket,buf_recive,sizeof(buf_recive)); 
    //printf("Respuesta service: %s\n",buf_recive);
    buffer_destroyed(d_buf);
    return 0;
}

void client_output(encode_t* encode, char* server_respose){
    printf("0x%08" PRIx16 , encode->msj_id);
    printf(": %s \n", server_respose);
}



