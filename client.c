#include <string.h>
#include "socket.h"
#include "client.h"
#include "parser.h"
#include <stdbool.h>
#include <stdlib.h>

const int INITIAL_SIZE = 32;
const int INCREASE_FACTOR = 2;

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
    char buff[INITIAL_SIZE];
    char* dinamic_buff = malloc(INITIAL_SIZE* sizeof(char));
    char buf_recive[3];

    printf("Holee\n");
    // socket_connect(self->socket,host,service);
    
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

 int is_finished_line(char * line){
    size_t length = strlen(line);
    for (size_t pos = 0; pos < length; pos++){
        if (line[pos] == '\n' || line[pos] == EOF){
            return 0;
        }
    }
    //printf("return -1\n");
    return -1;  
 }

// devuelve la cantidad de caracteres que tiene la linea que guardo
int client_save_in_buffer(char* buff, char** d_buff, FILE* file){
    memset(*d_buff, '\0', sizeof(char)*(INITIAL_SIZE+1)); 
    size_t capacity = INITIAL_SIZE+1;
    int saved = 0;

    while(is_finished_line(buff)< 0  ){
        memset(buff,0,sizeof(char)*(INITIAL_SIZE));
        fread(buff, INITIAL_SIZE-1,1, file);
        if (capacity <= saved + strlen(buff)){
            size_t new_tam= strlen(*d_buff)*INCREASE_FACTOR;
            char *aux = (char*)realloc(*d_buff, new_tam);
            if (aux) {
                *d_buff = aux;
                capacity = new_tam;
            }
        }
        memcpy(*d_buff+saved, buff, INITIAL_SIZE);
        saved += strlen(buff);
    }
    memcpy(*d_buff+saved-2, "\0", 1);
    return saved;
}

int client_send_encode(client_t* self, char* buff, char** d_buf, FILE* file){
    dbus_encode_t encoded;
    int bytes, line_bytes;
    encoded_create(&encoded);
    while(!feof(file)){
        if((line_bytes = client_save_in_buffer(buff, d_buf, file))< 0) return -1;
        //printf("line_bytes: %d\n", line_bytes);
        printf("start %s is this\n", *d_buf);
        //encode_file(&encoded,d_buf, line_bytes);
        //bytes = socket_send(self->socket, (&encoded)->bytes,(&encoded)->size);
        //if(bytes < 0){
        //    perror("Error when send");
        //    return -1;
        //}     
    }
    //printf("buffer dinamico: %s\n", d_buf);
    return 0;
}



