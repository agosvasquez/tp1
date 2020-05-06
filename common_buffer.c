#include "common_buffer.h"
#include "common_error.h"
#include <string.h>

const int INITIAL_BUFF_SIZE = 32;
const int INCREASE_FACTOR = 2;

int buffer_create(buffer_t* buffer){
    buffer->data = malloc(INITIAL_BUFF_SIZE* sizeof(char));
    if (!buffer->data) throw_error("malloc error");
    memset(buffer->data, 0, sizeof(char)* (INITIAL_BUFF_SIZE));
    buffer->capacity= INITIAL_BUFF_SIZE+1;
    buffer->used=0;
    buffer->read=0;
    return 0;
}

int buffer_create_size(buffer_t* buffer, int size){
    buffer->data = malloc(size * sizeof(char));
    if (!buffer->data) throw_error("malloc error");
    memset(buffer->data, 0, sizeof(char)* (size));
    buffer->capacity= size+1;
    buffer->used=0;
    buffer->read=0;
    return 0;
}

void buffer_destroyed(buffer_t* buffer){
    free(buffer->data);
    buffer->data = NULL;
}


int buffer_is_finished_line(char * line){
    size_t length = strlen(line);
    for (size_t pos = 0; pos < length; pos++){
        if (line[pos] == '\n' || line[pos] == '\0'){
            return pos;
        }
    }
    return -1;  
}

int buffer_realloc(buffer_t* buffer){
    size_t new_tam= buffer->capacity * INCREASE_FACTOR;
    char *aux = (char*)realloc(buffer->data, new_tam);
    if (!aux) throw_error("realloc error\n");
    buffer->data = aux;
    buffer->capacity = new_tam;
    return 0; 
}

int buffer_save_data(buffer_t* buffer, char* data, int size){
     while (buffer->capacity <= buffer->used + size){
        buffer_realloc(buffer);
    }
    if (buffer->used > 0) memcpy(buffer->data + buffer->used, data, size);
    else
        memcpy(buffer->data, data, size);
    buffer->used += size;
    return 0;
}

int buffer_set_final_char(buffer_t* buffer, int pos){
    memcpy(buffer->data + pos, "\0", 1);
    return 0;
}

int _get_line(char * buff, char* copy){
    int pos_final = buffer_is_finished_line(buff);
    if (pos_final < 0) return -1;
    // +1 porque en pos se cuentan desde 0 osea
    // que para contar memoria es mas 1
    memcpy(copy, buff, pos_final+1);
    /* add a final null terminator */
    size_t final= strlen(copy);
    memcpy(copy + final , "\0", 1);
    return 0;
}

// devuelve la cantidad de caracteres que tiene la linea que guardo
int buffer_save_from_file(buffer_t* d_buff,char* buff, FILE* file){
    while (strchr(buff,'\n') == NULL){
        memset(buff,0, INITIAL_BUFF_SIZE);
        size_t read = fread(buff, INITIAL_BUFF_SIZE-1,1, file);
        if (read*INITIAL_BUFF_SIZE < INITIAL_BUFF_SIZE-1 && strlen(buff)>0) {
            //ultima pasada
            //drop "/0" cuando esta en un archivo 
            char aux[INITIAL_BUFF_SIZE];
            memset(aux,0, (INITIAL_BUFF_SIZE));
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

int buffer_get_line(buffer_t* d_buf, char* buff, char** line, FILE* file){
    size_t line_b=0;
    while (1){
        memset(buff,0,INITIAL_BUFF_SIZE);
        if (!feof(file))
            if ((line_b = buffer_save_from_file(d_buf,buff, file))< 0) 
                return -1;
        while (d_buf->read < d_buf->used){
            size_t line_size = strlen(d_buf->data + d_buf->read)+1;
            *line= malloc(sizeof(char)* line_size);
            memset(*line,0,sizeof(char)*(line_size));
            if (_get_line(d_buf->data+d_buf->read, *line)< 0) break;
            //strlen da bien todo lo que leyo no restar
            d_buf->read += strlen(*line);
            return 1;
        }
        if (line_b < INITIAL_BUFF_SIZE-1) break;
        if (d_buf->used == 0) break;
    }
    if (d_buf->read < d_buf->used){
        size_t line_size = strlen(d_buf->data + d_buf->read);
        line= malloc(sizeof(char)* line_size);
        memset(*line,0,sizeof(char)*(line_size));
        memcpy(*line,d_buf->data+d_buf->read,line_size);
        d_buf->read += strlen(*line)+ 1;
        return 1;
    }
    return 0;
}