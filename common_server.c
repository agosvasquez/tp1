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

int server_receive(client_t* client, decode_t*decode, int size){
        char aux [size];
        int total_bytes=0;
        memset(aux, 0, size);
        int bytes = socket_receive(client->socket, aux, size);
        if (bytes < 0) throw_sterr("Recive fails", strerror(errno) );
        // client closed
        if (bytes == 0) return bytes;
        buffer_save_data(decode->bytes, aux , size);
        total_bytes += bytes; 
        return total_bytes;
}

int server_run(server_t* self, char* service){
    char message[3] = "OK\0";
    client_t client;
    decode_t decode;
    socket_bind_and_listen(self->socket, service);
    
    client_create(&client);
  
    socket_accept(self->socket, client.socket);
    // en el programa real extraer la longitud
    // de los primeros bytes
    while (1){
       
        uint8_t buff[16]= "";
        int bytes = socket_receive(client.socket,(char*)buff, sizeof(buff));
        
        if (bytes < 0) throw_sterr("Recive fails", strerror(errno) );
        // client closed
        if (bytes == 0) break;

        int body_size = extract_body_size((char*)buff);
        
        int array_size = extract_array_size((char*)buff);
        uint32_t msj_id = extract_msj_id((char*)buff);;

        int to_read = body_size + array_size;
        

        decoded_create_size(&decode, to_read+1);
        server_receive(&client, &decode, to_read);
        
        decode_messaje(&decode);
        
        decoded_output(&decode,msj_id);
        socket_send(client.socket, message, sizeof(message));
        
        decoded_destroyed(&decode);
    }
    
    client_destroy(&client);
    return 0;
}
