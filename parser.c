#include "parser.h"
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "error.h"
#include <byteswap.h>
#include <netinet/in.h>


uint8_t type_prot_des =  0x06;
uint8_t type_prot_path =  0x01;
uint8_t type_prot_inter =  0x02;
uint8_t type_prot_meth =  0x03;
uint8_t END = 0x00;
size_t POS_BODY_SIZE= 4;
size_t POS_ARR_SIZE= 12;
size_t POS_START_ARR =16;

int INIT_ENCODED_BUFF_SIZE = 32;

int encoded_create(encode_t* encode){
    buffer_t* buff = malloc(sizeof(buffer_t));
    buffer_create(buff);
    encode->bytes = buff;
    encode->msj_id = to_little_32(0x0001);
    printf("EL ID ES %02X\n", encode->msj_id);
    encode->count_pad=0;
    return 0;
}


void encoded_destoyed(encode_t* encoded){
    buffer_destroyed(encoded->bytes);
    free(encoded->bytes);
}

void encoded_increment_id(encode_t* encoded){
    encoded->msj_id += to_little_32(0x0001);
}

int decoded_create(dbus_decode_t* decode){
    return 0;
}

int decoded_destroyed(dbus_decode_t* decode){
    return 0;
}

int number_padd(int size, int mult){
    if((size%mult) == 0) return 0;
    int div = (int)size/mult;
    return(( div + 1)* mult ) - size;
}


int encode_line(encode_t * encode, char* data){
    uint16_t S = to_little_16(0x73);
    uint16_t O = to_little_16(0x6F);
    
    encode_set_static(encode);
    char * dest = strtok(data, " ");
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
    char* firm = data+ s_firm;
    printf("firma: %s\n", firm);
    int long_param = encode_params_firm(encode,firm);
    uint32_t swap_long_par= to_little_32(long_param);
    memcpy(encode->bytes->data + POS_BODY_SIZE, (char*)&swap_long_par, sizeof(uint32_t));
    int long_arr = encode->bytes->used - POS_START_ARR - encode->count_pad;
    uint32_t swap_long_ar= to_little_32(long_arr);
    memcpy(encode->bytes->data + POS_ARR_SIZE,(char*)&swap_long_ar, sizeof(uint32_t));
    return 0;  
}


int encode_convert_multiple( char* arg, char** arg_pad, int size_arg_pad, size_t size){
    //printf("padd: %d\n", padd);
    //uno por el caracter nulo
    printf("size_arg_pad: %d\n", size_arg_pad);
    *arg_pad = malloc(sizeof(char)* size_arg_pad);
    memset(*arg_pad, 0, sizeof(char)*size_arg_pad);
    memcpy(*arg_pad,arg,size);
    //printf("arg_pad : %s\n", *arg_pad);
    //printf("arg_pad %s", arg_pad);
    memcpy(*arg_pad + size , "\0", 1);
    int padd = size_arg_pad - size -1;
    for (size_t i = 0; i < padd; i++){
        memcpy(*arg_pad + size+1 ,(char*)&END, sizeof(uint8_t));
    }
    return 0;
}

void actualize_count_pad(encode_t* encode,  int pad){
    encode->count_pad+= pad;
}

void encode_set_static(encode_t* encode){
    uint8_t type = 0x01;
    uint8_t flags = 0x00;
    uint32_t len= to_little_32(0x0000);// dejo el lugar guardado porque todavia no se
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
    char* arg_pad = NULL;
    size_t size_param = strlen(arg);
    uint32_t swap_size_par = to_little_32((uint32_t)size_param);
    //para poder hacer menor directo
    int padd = number_padd(size_param+1, 8) +1;
    int size_arg_pad= size_param + padd;
    encode_convert_multiple(arg,&arg_pad, size_arg_pad, strlen(arg));
    uint8_t stat = 0x01;
   
    buffer_save_data(encode->bytes, (char*)t_p, 1);
    buffer_save_data(encode->bytes, (char*)&stat ,1);
    buffer_save_data(encode->bytes, (char*)t_d, sizeof(uint16_t));
    buffer_save_data(encode->bytes, (char*)&swap_size_par, sizeof(uint32_t));
    buffer_save_data(encode->bytes,arg_pad, size_arg_pad);
    actualize_count_pad(encode,padd);
    actualize_count_pad(encode, 1);
    free(arg_pad);
    return 0;
}

int encode_firm(encode_t* encode, int* cant_par){
    uint8_t t_p = 0x09;
    uint8_t stat = 0x01;
    uint8_t s = 0x73;
    uint16_t G = to_little_16(0x67);
    size_t size_param = *cant_par +5 ;

    char aux[size_param];
    memset(aux,0, *cant_par+5);
    memcpy(aux, (char*)&t_p, 1);
    memcpy(aux+1,(char*)&stat ,1);
    memcpy(aux+2, (char*)&G, sizeof(uint16_t));
    memcpy(aux+4, (char*)cant_par, sizeof(uint8_t));
    for (int i=1; i <= *cant_par; i++){
        memcpy(aux+4+i, (char*)&s, sizeof(uint8_t));
    }
    char* arg_pad = NULL;
    printf("size_param %ld\n",size_param);
    printf("size_param %ld\n",sizeof(aux));
    int padd = number_padd(size_param+1, 8) +1;
    int size_arg_pad= size_param  + padd;
    encode_convert_multiple(aux,&arg_pad, size_arg_pad,size_param );
     for (int i = 0; i < *cant_par+5; i++){
        if (i > 0) printf(":");
        printf("%02X", arg_pad[i]);
    }
    printf("\n");
    buffer_save_data(encode->bytes,arg_pad, size_arg_pad);
    free(arg_pad);
    //for end of string
    actualize_count_pad(encode, 1);
    return 0;
}


int encode_params_firm( encode_t* encode,char* firm){
    int counter_par =0;
    int long_param = 0;
    if (strcmp(firm, ")" ) != 0 ) {
        char* param = strtok(firm, ",");
        if (!param) throw_error("param not found");
        while (param != NULL ){
            counter_par +=1;
            long_param += strlen(param);
            printf("param: %s\n", param);
            param = strtok(NULL, ",");
        }
        printf("counter_par %d\n", counter_par);
        encode_firm(encode, &counter_par);
    } 
    return long_param;
}


uint16_t to_little_16( uint16_t x){
    uint16_t to_net =htons(x);
    return bswap_16(to_net);
}


uint32_t to_little_32( uint32_t x){
    uint32_t to_net =htonl(x);
    return bswap_32(to_net);
}