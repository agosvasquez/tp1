#include "parser.h"
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "error.h"

uint16_t S = 0x73;
uint16_t O = 0x6F;
uint16_t G = 0x67;

int INIT_ENCODED_BUFF_SIZE = 32;

int encoded_create(encode_t* encode){
    buffer_t* buff = malloc(sizeof(buffer_t));
    buffer_create(buff);
    encode->bytes = buff;
    encode->msj_id = 0x0001;
    return 0;
}

void encoded_destoyed(encode_t* encoded){
    buffer_destroyed(encoded->bytes);
    free(encoded->bytes);
}

void encoded_increment_id(encode_t* encoded){
    encoded->msj_id += 0x0001;
}

int decoded_create(dbus_decode_t* decode){
    return 0;
}

int decoded_destroyed(dbus_decode_t* decode){
    return 0;
}

int encode_line(encode_t * encode, char* data){
    uint8_t type_prot_des =  0x06;
    uint8_t type_prot_path =  0x01;
    uint8_t type_prot_inter =  0x02;
    uint8_t type_prot_meth =  0x03;
    uint8_t type_prot_par =  0x09;

    printf("entro\n");
    printf("data %s", data);
    encode_set_static(encode);
    printf("antes strk\n");
    char * dest = strtok(data, " ");
    printf("dest: %s", dest);
    printf("guardo dest\n");
    if (!dest) throw_error("destination not found");
    encode_arg(encode, dest, &type_prot_des, &S );
    char* path = strtok(NULL, " ");
    if (!path) throw_error("path not found");
    encode_arg(encode, path, &type_prot_path, &O);
    char* interface = strtok(NULL, " ");
    if (!interface) throw_error("interface not found");
    encode_arg(encode, interface, &type_prot_inter, &S );
    char* method = strtok(NULL, "(");
    if (!method) throw_error("method not found");
    encode_arg(encode, method, &type_prot_meth,&S);
    // +3 por los espacios +1 por el parentesis
    size_t s_firm = strlen(dest)+ strlen(path)+ strlen(interface)+ strlen(method)+ 4;
    //llamo sin hacer split
    char* firm = data+ s_firm;
    if (strcmp(firm, ")" ) != 0 ) {
        char* param = strtok(firm, ",");
        if (!param) throw_error("param not found");
        while (param != NULL ){
            encode_arg(encode, param, &type_prot_par,&S);
            param = strtok(NULL, ",");
        }
    } 
    // agregar longitud de paramentros
    //agregar longitud del array 
    return 0;  
}

/*
int split_line(char* string, char sep, char** splited){
        char * token = strtok(string, sep);
        size_t size = 0;
        //loop through the string to extract all other tokens
        while( token != NULL ) {
            *splited +size = token
            size += strlen(token);
            
            token = strtok(NULL, " ");
        }
}
*/
//int get_parameters_size(char* arg){
//
//}

void encode_set_static(encode_t* encode){
    //buffer_t* encode->bytes = encode->bytes;
    uint8_t type = 0x01;
    uint8_t flags = 0x00;
    uint32_t len= 0x0000;// dejo el lugar guardado porque todavia no se
    //agregarle a todo Hton
    printf("guardo l\n");
    printf("encode buffer used %ld\n", encode->bytes->used);
    buffer_save_data(encode->bytes,"l",1);
    printf("guardo type\n");
    buffer_save_data(encode->bytes, (char*)&type, 1);
    printf("guardo flahs\n");
    buffer_save_data(encode->bytes, (char*)&flags,1);
    printf("guardo type\n");
    buffer_save_data(encode->bytes, (char*)&type, 1);
    printf("guardo len\n");
    buffer_save_data(encode->bytes, (char*)&len, sizeof(uint32_t));
    printf("guardo id\n");
    buffer_save_data(encode->bytes,(char*)&(encode->msj_id),sizeof(uint32_t));
    printf("guardo otro len\n");
    buffer_save_data(encode->bytes,(char*)&len, sizeof(uint32_t) );
}

int encode_arg(encode_t* encode, char* arg, uint8_t* t_p, uint16_t* t_d){
    //buffer_t* encode->bytes = encode->bytes;
    uint8_t stat = 0x01;
    printf("get uint\n");
    printf("arg: %s", arg);
    size_t size_param = strlen(arg);

    printf("todo ok\n");

    buffer_save_data(encode->bytes, (char*)t_p, 1);
    buffer_save_data(encode->bytes, (char*)&stat ,1);
    buffer_save_data(encode->bytes, (char*)t_d, sizeof(uint16_t));
    if (size_param <= sizeof(uint32_t))
        buffer_save_data(encode->bytes, (char*)&size_param, sizeof(uint32_t));
    else buffer_save_data(encode->bytes, (char*)&size_param, sizeof(uint32_t));

    
    return 0;
}

