#ifndef PARSER_H
#define PARSER_H
#include <stddef.h>
#include <stdint.h>
#include "buffer.h"

typedef struct encode_t{
    buffer_t* bytes;
    uint32_t msj_id;
    size_t count_pad;
} encode_t;

typedef struct dbus_decode_t{
    char* bytes;
    char* str;
    size_t size_pad;
} dbus_decode_t;

// encode a file into dbus protocol
int encode_line(encode_t * encode, char* buff);

// decode a uint8 
//int decode_file(dbus_decode_t * decode, uint8_t* buff);

int encoded_create(encode_t* encode);

void encoded_destoyed(encode_t* encoded);

void encode_set_static(encode_t* encode);

int encode_arg(encode_t* encode, char* arg, uint8_t* t_p, uint16_t* t_d);

int decoded_create(dbus_decode_t* decode);

int decoded_destroyed(dbus_decode_t* decode);

int encode_convert_multiple( char* arg, char** arg_pad, int size_arg_pad, size_t size);

int encode_firm(encode_t* encode, int* cant_par);

int encode_params_firm( encode_t* encode,char* firm);

#endif