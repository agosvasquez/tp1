#include "buffer.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

const int INITIAL_BUFF_SIZE = 32;
const int INCREASE_FACTOR = 2;

int buffer_create(buffer_t* buffer){
    buffer->data = malloc(INITIAL_BUFF_SIZE* sizeof(char));
    if (!buffer->data) throw_error("malloc error");
    memset(buffer->data, 0, sizeof(char)* (INITIAL_BUFF_SIZE+1));
    buffer->capacity= INITIAL_BUFF_SIZE+1;
    buffer->used=0;
    printf("todo ok create\n");
    return 0;
}

void buffer_destroyed(buffer_t* buffer){
    free(buffer->data);
}


int buffer_is_finished_line(char * line){
    size_t length = strlen(line);
    for (size_t pos = 0; pos < length; pos++){
        if (line[pos] == '\n' || line[pos] == '\0'){
            //printf("entcontre final fin de la iteracion\n");
            return pos;
        }
        //printf("line in pos: %c\n", line[pos]);
    }
    //printf("return -1\n");
    return -1;  
}

int buffer_realloc(buffer_t* buffer){
    size_t new_tam= buffer->capacity * INCREASE_FACTOR;
    //printf("buffer: %s\n", buffer->data);
    char *aux = (char*)realloc(buffer->data, new_tam);
    if (!aux) throw_error("realloc error\n");
    buffer->data = aux;
    buffer->capacity = new_tam;
    return 0; 
}

int buffer_save_data(buffer_t* buffer, char* data, int size){
    //printf("data:%ld \n", strlen(data));
    if (buffer->capacity <= buffer->used + strlen(data)){
        //printf("REALLOC\n");
        buffer_realloc(buffer);
    }
    //printf("memcpy save\n");
    //printf("size %d\n",size);
    //printf("buffer data antes de guardar %s\n", buffer->data);
    //printf("data %s\n", data);
    if (buffer->used > 0) memcpy(buffer->data + buffer->used, data, size);
    else memcpy(buffer->data, data, size);
    //printf("afeter memcpy save\n");
    //printf("buffer data %s\n", buffer->data);
    //printf("buffer used %ld\n", buffer->used);
    //printf("buffer capacity %ld\n", buffer->capacity);
    buffer->used += strlen(data);
    //printf("saliendo de save\n");
    //printf("data:%ld \n", strlen(data));
    //printf("buffer used %ld\n", buffer->used);
    return 0;
}

int buffer_set_final_char(buffer_t* buffer, int pos){
    memcpy(buffer->data + pos, "\0", 1);
    return 0;
}

int buffer_get_line(char * buff, char* copy){
    //printf("buffer que me llega:%s", buff);
    int pos_final = buffer_is_finished_line(buff);
    //printf("tengo la pos\n");
    if (pos_final <0) return -1;
    // +1 porque en pos se cuentan desde 0 osea
    // que para contar memoria es mas 1
    memcpy(copy, buff, pos_final+1);
    //printf("after memcpy\n");
    /* add a final null terminator */
    copy[pos_final] = 0;
    //printf("line: %s\n", copy);
    return 0;
    
}