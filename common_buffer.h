#ifndef COMMON_BUFFER_H
#define COMMON_BUFFER_H
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct buffer_t{
    char* data;
    size_t capacity;
    size_t used;
    size_t read;
} buffer_t;

int buffer_create(buffer_t* buffer);

int buffer_create_size(buffer_t* buffer, int size);

void buffer_destroy(buffer_t* buffer);

int buffer_realloc(buffer_t* buffer);

int buffer_save_data(buffer_t* buffer, char* data, int size);

int buffer_set_final_char(buffer_t* buffer, int pos);


#endif
