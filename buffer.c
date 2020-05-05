#include "buffer.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
    memset(buffer->data, 0, sizeof(char)* (INITIAL_BUFF_SIZE));
    buffer->capacity= INITIAL_BUFF_SIZE+1;
    buffer->used=0;
    buffer->read=0;
    printf("todo ok create\n");
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
            printf("entcontre final fin de la iteracion\n");
            //printf("line in pos: %c\n", line[pos]);
            return pos;
        }
        printf("line in pos: %c\n", line[pos]);
    }
    //printf("return -1\n");
    return -1;  
}

int buffer_realloc(buffer_t* buffer){
    size_t new_tam= buffer->capacity * INCREASE_FACTOR;
    //printf("buffer: %s\n", buffer->data);
    //printf("buffer cap: %ld\n", buffer->capacity);
    //printf("buffer strlen: %ld\n", strlen(buffer->data));
    char *aux = (char*)realloc(buffer->data, new_tam);
    if (!aux) throw_error("realloc error\n");
    buffer->data = aux;
    buffer->capacity = new_tam;
    return 0; 
}

int buffer_save_data(buffer_t* buffer, char* data, int size){
    printf("buffer capacity %ld\n", buffer->capacity);
    //printf("data size:%d \n", size);
    //printf("strlen data:%ld\n", strlen(data));
    //printf("buffer used %ld\n", buffer->used);
     while(buffer->capacity <= buffer->used + size){
        printf("REALLOC\n");
        buffer_realloc(buffer);
    }
    //printf("memcpy save\n");
    printf("size to save %d\n",size);
    //printf("buffer data antes de guardar %s\n", buffer->data);
    //printf("data %s\n", data);
    if (buffer->used > 0) memcpy(buffer->data + buffer->used, data, size);
    else memcpy(buffer->data, data, size);
    //printf("afeter memcpy save\n");
    //printf("buffer data %s\n", buffer->data);
    printf("buffer used %ld\n", buffer->used);
    printf("buffer capacity %ld\n", buffer->capacity);
    buffer->used += size;
    //printf("saliendo de save\n");
    //printf("data:%ld \n", strlen(data));
    //printf("buffer despues de actualiar used %ld\n", buffer->used);
    return 0;
}

int buffer_set_final_char(buffer_t* buffer, int pos){
    memcpy(buffer->data + pos, "\0", 1);
    return 0;
}

int _get_line(char * buff, char* copy){
    printf("buffer que me llega:%s\n", buff);
    int pos_final = buffer_is_finished_line(buff);
    printf("termino la linea?: %d \n ", pos_final < 0);
    printf("tengo la pos: %d\n", pos_final);
    
    if (pos_final < 0) return -1;
    
    // +1 porque en pos se cuentan desde 0 osea
    // que para contar memoria es mas 1
    memcpy(copy, buff, pos_final+1);
    //printf("after memcpy\n");
    /* add a final null terminator */
    size_t final= strlen(copy);
    memcpy(copy + final , "\0", 1);
    printf("line: %s\n", copy);
    //if (buffer_is_finished_line(copy) > 0) printf("SIII\n");
    return 0;
    
}

// devuelve la cantidad de caracteres que tiene la linea que guardo
int buffer_save_from_file(buffer_t* d_buff,char* buff, FILE* file){
    //printf("LO QUE ESTA GUAR %s\n", d_buff->data);
    //printf("JEEE\n");
    //printf("buff antes while %s\n", buff);
    //printf("LO QUE ESTA GUAR ANTES DEL READ %s\n", d_buff->data);
    while(strchr(buff,'\n') == NULL){
        //printf("entro bien\n");
        //printf("buff antes del memset %s\n", buff);
        memset(buff,0,sizeof(char)*(INITIAL_BUFF_SIZE));
        //printf("antes de print");
        //printf("LO QUE ESTA GUAR ANTES DEL READ %s\n", d_buff->data);
        fread(buff, INITIAL_BUFF_SIZE-1,1, file);
        if(strlen(buff) < INITIAL_BUFF_SIZE-1 && file != stdin) {
            //ultima pasada
            //drop "/0" cuando esta en un archivo 
            printf("desde aca\n");
            char aux[INITIAL_BUFF_SIZE];
            //printf("buff in final: %s\n", buff );
            memset(aux,0,sizeof(char)*(INITIAL_BUFF_SIZE));
            memcpy(aux, buff, strlen(buff)-1);
            //printf("aux:%s", aux);
            buffer_save_data(d_buff,aux,strlen(aux));
            //printf("d_buff final size %ld\n", d_buff->used);
            //printf("que verga rompe\n");
            break;
        }
        //printf("guardo\n");
        //printf("leido %s\n", buff);
        //printf("leido %ld\n", strlen(buff));
        buffer_save_data(d_buff,buff,strlen(buff));
        //printf("Guardado %s\n", d_buff->data);
        //printf("d_buff size %ld\n", d_buff->used);
        //printf("saliendo\n");
        
    }
    printf("antes de set final\n");
    buffer_set_final_char(d_buff, d_buff->used);
    printf("d_buff size %ld\n", d_buff->used);
    //printf("despues de set final\n");
    return strlen(buff);
}

int buffer_get_line(buffer_t* d_buf, char* buff, char** line, FILE* file){
    size_t line_bytes=0;
    while(1){
        memset(buff,0,sizeof(char)*(INITIAL_BUFF_SIZE));
        if(!feof(file))
            if((line_bytes = buffer_save_from_file(d_buf,buff, file))< 0) return -1;
        while(d_buf->read < d_buf->used){
            //size_t line_size=INITIAL_BUFF_SIZE;
            //mas 1 para guardar el '/0'
            size_t line_size = strlen(d_buf->data + d_buf->read)+1;
            *line= malloc(sizeof(char)* line_size);
            memset(*line,0,sizeof(char)*(line_size));
            printf("corrido :%s\n", d_buf->data+d_buf->read);
            if (_get_line(d_buf->data+d_buf->read, *line)< 0) break;
            //strlen da bien todo lo que leyo no restar
            d_buf->read += strlen(*line);
            //printf("lineee %s\n", *line);
            return 1;
        }
        if(line_bytes < INITIAL_BUFF_SIZE-1) break;
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
