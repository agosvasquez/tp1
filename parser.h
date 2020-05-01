#ifndef PARSER_H
#define PARSER_H
#include <stddef.h>

typedef struct dbus_encode_t{
    char* bytes;
    size_t size;
} dbus_encode_t;

typedef struct dbus_decode_t{
    char* bytes;
    char* str;
    size_t size;
} dbus_decode_t;

// encode a file into dbus protocol
int encode_file(dbus_encode_t * encode, char* buff, size_t lenght);

// decode a uint8 
//int decode_file(dbus_decode_t * decode, uint8_t* buff);

int encoded_create(dbus_encode_t* encode);

int encoded_destoyed(dbus_encode_t* encoded);

int decoded_create(dbus_decode_t* decode);

int decoded_destroyed(dbus_decode_t* decode);

#endif