#ifndef COMMON_PARSER_H
#define COMMON_PARSER_H
#include <stddef.h>
#include <stdint.h>
#include "common_buffer.h"

typedef struct encode_t{
    buffer_t* bytes;
    uint32_t msj_id;
    size_t count_pad;
} encode_t;

typedef struct decode_t{
    buffer_t* bytes;
    char* destino;
    char* path;
    char* interface;
    char* method;
    buffer_t* params; 
} decode_t;

// encode a file into dbus protocol
int encode_line(encode_t * encode, char* buff);

// decode a uint8 
int decode_file(decode_t * decode, uint8_t* buff);

int encoded_create(encode_t* encode, uint32_t msj_id);

void encoded_destoyed(encode_t* encoded);

void encode_set_static(encode_t* encode);

int encode_arg(encode_t* encode, char* arg, uint8_t* t_p, uint16_t* t_d);

int decoded_create_size(decode_t* decode,buffer_t* buff,buffer_t* p, int size);

int decoded_create(decode_t* decode);

void decoded_destroyed(decode_t* decode);

int encode_convert_multiple(char* arg, char** arg_pad,int s_arg_p,size_t size);

int encode_firm(encode_t* encode, int* cant_par);

int encode_params_firm(encode_t* encode,char* firm);

uint16_t to_little_16(uint16_t x);

uint32_t to_little_32(uint32_t x);

int encode_extract_size(char* buff, int pos);

int extract_body_size(char* buff);

int extract_array_size(char* buff);

int number_padd(int size, int mult);

int decode_messaje(decode_t* decode);

int decode_messaje(decode_t* decode);

int decode_meth_param(decode_t* decode, buffer_t* buff);

int size_param(decode_t* decode, buffer_t* buff);

int decode_fill_par(char** param_fill,  buffer_t* buff , int size);

int decode_dest_param(decode_t* decode, buffer_t* buff);

int decode_path_param(decode_t* decode, buffer_t* buff);

int decode_inter_param(decode_t* decode, buffer_t* buff);

void decoded_output(decode_t* decode, uint32_t msj_id);

uint32_t extract_msj_id(char* buff);

int encode_body(encode_t * encode, char* params);

int save_param(encode_t * encode, char* param, int size);

int get_total_padd(decode_t* decode);

#endif
