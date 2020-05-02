#include <string.h>
#include "socket.h"
#include "client.h"
#include "parser.h"
#include "buffer.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

const int INITIAL_SIZE = 32;
//const int INCREASE_FACTOR = 2;

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
    buffer_t dinamic_buff ;
    char buf_recive[3];

    //printf("Holee\n");
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

// devuelve la cantidad de caracteres que tiene la linea que guardo
int client_save_in_buffer(char* buff, buffer_t* d_buff, FILE* file){
    //printf("LO QUE ESTA GUAR %s\n", d_buff->data);
    //printf("JEEE\n");
    //printf("buff antes while %s\n", buff);
    //printf("LO QUE ESTA GUAR ANTES DEL READ %s\n", d_buff->data);
    while(strchr(buff,'\n') == NULL){
        //printf("entro bien\n");
        //printf("buff antes del memset %s\n", buff);
        memset(buff,0,sizeof(char)*(INITIAL_SIZE));
        //printf("antes de print");
        //printf("LO QUE ESTA GUAR ANTES DEL READ %s\n", d_buff->data);
        fread(buff, INITIAL_SIZE-1,1, file);
        if(strlen(buff) < INITIAL_SIZE-1) {
            //ultima pasada
            //drop "/0"
            //printf("desde aca\n");
            char aux[INITIAL_SIZE];
            //printf("buff in final: %s\n", buff );
            memset(aux,0,sizeof(char)*(INITIAL_SIZE));
            memcpy(aux, buff, strlen(buff)-1);
            //printf("aux:%s", aux);
            buffer_save_data(d_buff,aux,strlen(buff));
            //printf("d_buff final size %ld\n", d_buff->used);
            //printf("que verga rompe\n");
            break;
        }
        //printf("guardo\n");
        //printf("leido %s\n", buff);
        //printf("leido %ld\n", strlen(buff));
        buffer_save_data(d_buff,buff,INITIAL_SIZE);
        //printf("Guardado %s\n", d_buff->data);
        //printf("d_buff size %ld\n", d_buff->used);
        //printf("saliendo\n");
        
    }
    //printf("antes de set final\n");
    buffer_set_final_char(d_buff, d_buff->used);
    printf("d_buff size %ld\n", d_buff->used);
    //printf("despues de set final\n");
    return strlen(buff);
}

int client_send_encode(client_t* self, char* buff, buffer_t* d_buf, FILE* file){
    dbus_encode_t encoded;
    int bytes, line_bytes;
    size_t size = 0;
    encoded_create(&encoded);
    buffer_create(d_buf);
    while(1){
        //printf("LLAMO DEDE AFUERA CON EL BUFF LOCAL %s\n", buff);
        memset(buff,0,sizeof(char)*(INITIAL_SIZE));
        if((line_bytes = client_save_in_buffer(buff, d_buf, file))< 0) return -1;
        while(size < d_buf->used){
            //printf("size:%ld\n", size);
            //printf("strlen:%ld\n", d_buf->used);
            size_t line_size = strlen(d_buf->data +size);
            char line[line_size];
            memset(line,0,sizeof(char)*(line_size));
            //printf("entre al splipz");
           // printf("new_line: %s\n", d_buf->data+size);
            //voy corriendome hasta que termine de split la linea
            if (buffer_get_line(d_buf->data+size, line)< 0) break;
            printf("start %s is this\n", line);
            size += strlen(line)+ 1;
            
        }
        //printf("line_bytes: %d\n", line_bytes);
        //printf("AFUERA %s IS THIS\n", d_buf->data);
        //printf(" CUANTO LEI %d\n", line_bytes);
        if(line_bytes < INITIAL_SIZE-1) break;
        //encode_file(&encoded,d_buf, line_bytes);
        //bytes = socket_send(self->socket, (&encoded)->bytes,(&encoded)->size);
        //if(bytes < 0){
        //    perror("Error when send");
        //    return -1;
        //}     
    }
    //printf("buffer dinamico: %s\n", d_buf->data);
    return 0;
}



