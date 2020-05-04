#include "parser.h"
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "error.h"
#include <byteswap.h>
#include <netinet/in.h>
#include <inttypes.h>


uint8_t type_prot_des =  0x06;
uint8_t type_prot_path =  0x01;
uint8_t type_prot_inter =  0x02;
uint8_t type_prot_meth =  0x03;


uint8_t END = 0x00;
size_t POS_BODY_SIZE= 4;
size_t POS_ARR_SIZE= 12;
size_t POS_START_ARR =16;
int SIZE_END = 1;

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

int decoded_create_size(decode_t* decode, int size){
    buffer_t* buff = malloc(sizeof(buffer_t));
    buffer_create_size(buff, size);
    decode->bytes = buff;
    return 0;
}

void decoded_destroyed(decode_t* decode){
    buffer_destroyed(decode->bytes);
    free(decode->bytes);
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
    printf("FIN");
    return 0;  
}


int encode_convert_multiple( char* arg, char** arg_pad, int size_arg_pad, size_t size){
    printf("size_arg_pad: %d\n", size_arg_pad);
    //un lugar mas para guardar
    *arg_pad = malloc(sizeof(char)* size_arg_pad+1);
    memset(*arg_pad, 0, sizeof(char)*size_arg_pad+1);
    memcpy(*arg_pad,arg,size);
    //printf("arg_pad : %s\n", *arg_pad);
    //printf("arg_pad %s", arg_pad);
    memcpy(*arg_pad + size , "\0", 1);
    int padd = size_arg_pad - size ;
    //printf("pad %d\n", padd);
    for (size_t i = 0; i <= padd && padd >0 ; i++){
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
    //para poder 
    int padd = number_padd(size_param+ SIZE_END, 8);
    int size_arg_pad= size_param + SIZE_END + padd;
    encode_convert_multiple(arg,&arg_pad, size_arg_pad, strlen(arg));
    uint8_t stat = 0x01;
   
    buffer_save_data(encode->bytes, (char*)t_p, 1);
    buffer_save_data(encode->bytes, (char*)&stat ,1);
    buffer_save_data(encode->bytes, (char*)t_d, sizeof(uint16_t));
    buffer_save_data(encode->bytes, (char*)&swap_size_par, sizeof(uint32_t));
    buffer_save_data(encode->bytes,arg_pad, size_arg_pad);
    actualize_count_pad(encode,padd);
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
    int padd = number_padd(size_param+ SIZE_END, 8);
    int size_arg_pad= size_param + SIZE_END + padd ;
    encode_convert_multiple(aux,&arg_pad, size_arg_pad,size_param );

    buffer_save_data(encode->bytes,arg_pad, size_arg_pad);
    free(arg_pad);
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

int extract_body_size(char* buff){
    return encode_extract_size(buff, POS_BODY_SIZE);
}

int extract_array_size(char* buff){
    return encode_extract_size(buff, POS_ARR_SIZE);
}

uint32_t extract_msj_id(char* buff){
    return encode_extract_size(buff, 8);
}

int encode_extract_size(char* buff, int pos){
    char * len = buff + pos;
    uint32_t* size = (uint32_t*) len;
    return to_little_32(*size);
}


int decode_fill_dest(char* param_fill,  buffer_t* buff , int size ){
    param_fill = malloc(size);
    memset(param_fill, 0, size);
    memcpy(param_fill, buff->data+ buff->read, size);
    buff->read += size;
    printf("param: %s", param_fill);
    return 0;

}

int size_param(decode_t* decode, buffer_t* buff){
    int size_param = 0;
    //posicion del size es en 4 
    memcpy(size_param, buff->data + buff->read + 4 , sizeof(int) );
    buff->read += 8;
    int padd = number_padd(size_param , 8 );
    int to_read = size_param+ padd + SIZE_END;
    return 0;
    
}

int decode_dest_param(decode_t* decode, buffer_t* buff){
    int size = size_param(decode, buff);
    decode_fill_dest(decode->destino,buff,size);
    return 0;
}

int decode_path_param(decode_t* decode, buffer_t* buff){
    int size = size_param(decode, buff);
    decode_fill_dest(decode->path,buff,size);
    return 0;
}

int decode_inter_param(decode_t* decode, buffer_t* buff){
    int size = size_param(decode, buff);
    decode_fill_dest(decode->interface,buff,size);
    return 0;
}

int decode_meth_param(decode_t* decode, buffer_t* buff){
    int size = size_param(decode, buff);
    decode_fill_dest(decode->method,buff,size);
    return 0;
}


int decode_messaje(decode_t* decode){
    buffer_t* buff = decode->bytes;
    char type; 
    while(buff->read < buff->used){
        memcpy(type, buff + buff->read, 1 );
        buff->read += 1;
        const char prot_des =  0x06;
        const char prot_path =  0x01;
        const char prot_inter =  0x02;
        const char prot_meth =  0x03;
        const char prot_firm =  0x09;
        switch (type){
            case prot_des:
                decode_dets_param(decode,buff);
                break;
            case prot_path:
                decode_path_param(decode,buff);
                break;
            case prot_inter:
                decode_inter_param(decode,buff);
                break;
            case prot_meth:
                decode_meth_param(decode,buff);
                break;
            /*case prot_firm:
                decode_firm_param(buff);
                break;
            */
            default:
                return -1;
        }
    }
    return 0;
}

void decoded_output(decode_t* decode, uint32_t msj_id){
    printf("Id:" );
    printf("%" PRIu32 "\n",msj_id);
}
