#include "common_server.h"
#include "common_error.h"
#include <byteswap.h>
#include <errno.h>
#include <string.h>

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

int server_receive(socket_t* socket, decode_t*decode, int size){
        char aux [size];
        int total_bytes=0;
        memset(aux, 0, size);
        int bytes = socket_receive(socket, aux, size);
        if (bytes < 0)  throw_sterr("Recive fails", strerror(errno) );
        // client closed
        if (bytes == 0) return bytes;
        buffer_save_data(decode->bytes, aux , size);
        total_bytes += bytes; 
        return total_bytes;
}

int server_decode(decode_t decode, int read, uint32_t msj_id, socket_t* socket){
     buffer_t buffer,params;
     decoded_create_size(&decode, &buffer,&params, read+1);
     if (server_receive(socket, &decode, read) < 0){
        decoded_destroy(&decode);
        return -1;
     }
     decode_messaje(&decode);
     decoded_output(&decode,msj_id);
     decoded_destroy(&decode);
     return 0;
}

int server_run(server_t* self, char* service){
    char message[3] = "OK\0";
    decode_t decode;
    if (socket_bind_and_listen(self->socket, service)<0) return -1;
    socket_t socket;
    socket_create(&socket);
    if (socket_accept(self->socket, &socket) < 0) {
        socket_destroy(&socket);
        return -1;
    }
    while (1){
        uint8_t buff[16]= "";
        int bytes = socket_receive(&socket,(char*)buff, sizeof(buff));
        
        if (bytes < 0) {
                socket_destroy(&socket);
                return -1;
        }
        // client closed
        if (bytes == 0) break;

        int body_size = extract_body_size((char*)buff);
        int array_size = extract_array_size((char*)buff);
        uint32_t msj_id = extract_msj_id((char*)buff);;
        int to_read = body_size + array_size;

        if (server_decode(decode,to_read,msj_id,&socket) < 0){
           socket_destroy(&socket);
           return -1;     
        } 
        if (socket_send(&socket, message, sizeof(message)) < 0){
           socket_destroy(&socket);
           return -1;
        }
    }
    socket_destroy(&socket);
    return 0;
}
