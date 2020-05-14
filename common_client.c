#include "common_client.h"
#include "common_error.h"
#include <inttypes.h>
#include <string.h>

const int INITIAL_SIZE = 32;
const int COTA = 100;

int client_create(client_t* self, socket_t* socket){
    socket_create(socket);
    self->socket = socket;
    return 0;
}

int client_destroy(client_t* self){
    socket_shutdown(self->socket,SHUT_RDWR);
    socket_destroy(self->socket);
    return 0;
}

int client_run(client_t* self, const char* h, const char* s, FILE* file){
    char buff[INITIAL_SIZE];
    buffer_t dinamic_buff;

    socket_connect(self->socket,h,s);
    client_send_encode(self,buff,&dinamic_buff, file);
  
    return 0;
}



int client_send_encode(client_t* self, char* buff, buffer_t* d_buf, FILE* file){
    encode_t encode;
    int bytes;
    char buf_recive[3];
    buffer_create(d_buf);
    uint32_t msj_id = 0x0001;
    while (1){
        encoded_create(&encode, msj_id);
        char* line = NULL;

        client_get_line(d_buf,buff,&line,file);
        if (!line){
            free(line);
            encoded_destoyed(&encode);
            return -1;
        }
        encode_line(&encode,line);
        bytes=socket_send(self->socket,encode.bytes->data,encode.bytes->used);
        if (bytes < 0) {
            free(line);
            encoded_destoyed(&encode);
            return -1;
        }
        if (socket_receive(self->socket,buf_recive,sizeof(buf_recive)) <0){
            free(line);
            encoded_destoyed(&encode);
            return -1;
        }
        client_output(&encode,buf_recive);
        free(line);
        encoded_destoyed(&encode);
        msj_id += 1;
        if (d_buf->read == d_buf->used) break;
    }
    buffer_destroy(d_buf);
    return 0;
}

size_t _set_line(buffer_t* d_buf, char** line){
    size_t line_size = strlen(d_buf->data + d_buf->read)+1;
    *line= malloc(sizeof(char)* line_size);
    memset(*line,0,sizeof(char)*(line_size));
    return line_size;
}

int _is_finished_line(char * line){
    size_t length = strlen(line);
    for (size_t pos = 0; pos < length; pos++){
        if (line[pos] == '\n' || line[pos] == '\0'){
            return pos;
        }
    }
    return -1;  
}

int _get_line(char * buff, char* copy){
    int pos_final = _is_finished_line(buff);
    if (pos_final < 0) return -1;
    // +1 porque en pos se cuentan desde 0 osea
    // que para contar memoria es mas 1
    memcpy(copy, buff, pos_final+1);
    /* add a final null terminator */
    size_t final= strlen(copy);
    memcpy(copy + final , "\0", 1);
    return 0;
}


int client_save_file(buffer_t* d_buff,char* buff, FILE* file , int size){
    while (strchr(buff,'\n') == NULL){
        memset(buff,0, size);
        size_t read = fread(buff, size-1,1, file);
        if (read*size < size-1 && strlen(buff)>0) {
            char aux[COTA];
            memset(aux,0, (size));
            memcpy(aux, buff, strlen(buff) -1);
            buffer_save_data(d_buff,aux,strlen(aux));
            break;
        }
        buffer_save_data(d_buff,buff,strlen(buff));
        if (read == 0) break;
    }
    buffer_set_final_char(d_buff, d_buff->used);
    return strlen(buff);
}


int client_get_line(buffer_t* d_buf, char* buff, char** line, FILE* file){
    size_t line_b=0;
    while (1){
        memset(buff,0,INITIAL_SIZE);
        if (!feof(file))
            if ((line_b = client_save_file(d_buf,buff, file, INITIAL_SIZE))< 0) 
                return -1;
        while (d_buf->read < d_buf->used){
            _set_line(d_buf, line);
            if (_get_line(d_buf->data+d_buf->read, *line)< 0) break;
            d_buf->read += strlen(*line);
            return 0;
        }
        if (line_b < INITIAL_SIZE-1) break;
        if (d_buf->used == 0) break;
    }
    free(*line);
    if (d_buf->read < d_buf->used){
        size_t line_size = _set_line(d_buf, line);
        memcpy(*line,d_buf->data+d_buf->read,line_size);
        d_buf->read += strlen(*line);
    }
    return 0;
}
void client_output(encode_t* encode, char* server_respose){
    printf("0x%08" PRIx16 , to_little_32(encode->msj_id));
    printf(": %s\n", server_respose);
}



