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
    buffer->data = malloc(size);
    if (!buffer->data) throw_error("malloc error");
    memset(buffer->data, 0, (size));
    buffer->capacity= size+1;
    buffer->used=0;
    buffer->read=0;
    return 0;
}

void buffer_destroy(buffer_t* buffer){
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
