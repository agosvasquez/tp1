#define _POSIX_C_SOURCE 200112L
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
int SIZE_SEP = 1;

int INIT_ENCODED_BUFF_SIZE = 32;

int encoded_create(encode_t* encode, uint32_t msj_id){
    buffer_t* buff = malloc(sizeof(buffer_t));
    buffer_create(buff);
    encode->bytes = buff;
    encode->msj_id = msj_id ;
    printf("EL ID ES %02X\n", encode->msj_id);
    encode->count_pad=0;
    return 0;
}

void encoded_destoyed(encode_t* encoded){
    buffer_destroyed(encoded->bytes);
    free(encoded->bytes);
}


int decoded_create_size(decode_t* decode, int size){
    buffer_t* buff = malloc(sizeof(buffer_t));
    buffer_create_size(buff, size);
    decode->bytes = buff;
    decode->destino = NULL;
    decode->path= NULL;
    decode->interface = NULL;
    decode->method = NULL;
    decode->params = NULL; 
    return 0;
}

void decoded_destroyed(decode_t* decode){
    buffer_destroyed(decode->bytes);
    free(decode->bytes);
    free(decode->destino);
    free(decode->interface);
    free(decode->path);
    free(decode->method);
    if(decode->params){
        buffer_destroyed(decode->params);
        free(decode->params);
    }
}

int number_padd(int size, int mult){
    if((size%mult) == 0) return 0;
    int div = (int)size/mult;
    return(( div + 1)* mult ) - size;
}


int encode_line(encode_t * encode, char* data){
    uint16_t S = to_little_16(0x73);
    uint16_t O = to_little_16(0x6F);
    char *save_ptr = NULL;
    
    encode_set_static(encode);
    char * dest = strtok_r(data, " ", &save_ptr);
    if (!dest) throw_error("destination not found");
    encode_arg(encode, dest, &type_prot_des, &S );
    char* path = strtok_r(NULL, " ",&save_ptr);
    if (!path) throw_error("path not found");
    encode_arg(encode, path, &type_prot_path, &O);
    char* interface = strtok_r(NULL, " ", &save_ptr);
    if (!interface) throw_error("interface not found");
    encode_arg(encode, interface, &type_prot_inter, &S );
    char* method = strtok_r(NULL, "(", &save_ptr);
    if (!method) throw_error("method not found");
    encode_arg(encode, method, &type_prot_meth,&S);
    // +3 por los espacios +1 por el parentesis
    size_t s_firm = strlen(dest)+ strlen(path)+ strlen(interface)+ strlen(method)+ 4;
    char* firm = data+ s_firm;
    printf("firma posta %s", firm);
    int par_len= strlen(firm);
    strcat(firm, "\0");
    char params[par_len + SIZE_END];
    memset(params,0, par_len + SIZE_END);
    strncpy(params, firm , par_len );
    if (buffer_is_finished_line(params) == 0){
        printf("PARAMS tiene null finish");
    }
    printf("PARAMS: %s\n", params);
    
    encode_params_firm(encode,firm);
    //long array
    int long_arr = encode->bytes->used - POS_START_ARR - encode->count_pad;
    uint32_t swap_long_ar= to_little_32(long_arr);
    memcpy(encode->bytes->data + POS_ARR_SIZE,(char*)&swap_long_ar, sizeof(uint32_t));

    //long body
    int size_before_body= encode->bytes->used;
    encode_body(encode, params);
    int long_param = encode->bytes->used -size_before_body;
    uint32_t swap_long_par= to_little_32(long_param);
    memcpy(encode->bytes->data + POS_BODY_SIZE, (char*)&swap_long_par, sizeof(uint32_t));
    printf("FIN");
    return 0;  
}

int encode_body(encode_t * encode, char* params){
    //printf("PARAMS: %s", params);
    char* param, *par;
    char *saveptr = NULL;
    //printf(" before strcpy params %s", params);
    //printf("params afeter strcpy %s \n", params);
    param = strtok_r(params, ",", &saveptr);
    if (!param) throw_error("body param not found");
    while (param != NULL ){
        strcat(param, "\0");
        if (buffer_is_finished_line(param) == 0)
            printf("PARAMS tiene null finish");
        int len = strlen(param);
        //printf("PRAM: %s\n", param);
        //printf("size SAVE %d\n",len);
        if (strchr(param, ')' ) != NULL ) {
            par = strtok_r(param, ")", &saveptr );
            save_param(encode,par, len -1);
            break;
        }
        save_param(encode,param, len);
        param = strtok_r(NULL, ",", &saveptr);
    }
    
    return 0;
}

int save_param(encode_t * encode, char* param, int size){
    uint32_t swap_size_par = to_little_32((uint32_t)size);
    printf("guardo len %02X \n", swap_size_par);
    buffer_save_data(encode->bytes, (char*)&swap_size_par, sizeof(uint32_t));
    printf("guardo par\n");
    buffer_save_data(encode->bytes, param, size);
    printf("guardo null terminator");
    buffer_save_data(encode->bytes, "\0", 1);
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
    char *saveptr = NULL;
    char* param = strtok_r(firm, ",", &saveptr);
    if (!param) throw_error("firm param not found");
    while (param != NULL ){
        counter_par +=1;
        param = strtok_r(NULL, ",", &saveptr);
    }
    printf("counter_par %d\n", counter_par);
    encode_firm(encode, &counter_par);
    
    return 0;
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


int decode_fill_par(char** param_fill,  buffer_t* buff , int size ){
    *param_fill = malloc(size);
    memset(*param_fill, 0, size);
    memcpy(*param_fill, buff->data+ buff->read, size);
    printf("size en fill : %d\n", size);
    buff->read += size;
    printf("param: %s\n", *param_fill);
    return 0;

}

int size_param(decode_t* decode, buffer_t* buff){
    uint32_t _size_param;
    uint32_t size_param;
    //posicion del size es en 4 
    memset(&_size_param,0, sizeof(int));
    memset(&size_param,0, sizeof(int));
    printf("buff->read %ld \n", buff->read);
    /*uint8_t * hexa = (uint8_t*) buff->data + buff->read + 3;
    for (int i = 0; i < buff->used; i++){
            if (i > 0) printf(":");
            printf("%02X", hexa[i]);
    }*/
    memcpy(&_size_param, buff->data + buff->read + 3 , sizeof(int) );
    //printf("size_param: %d\n", _size_param);
    
    size_param = to_little_32(_size_param);
    //printf("size_param: %d\n", size_param);
    buff->read += 7;
    int padd = number_padd(size_param + SIZE_END, 8 );
    //printf("padd: %d\n ", padd);
    //printf("LEN FINAL %d\n", size_param+ padd + SIZE_END);
    return size_param+ padd + SIZE_END;   
}

int decode_dest_param(decode_t* decode, buffer_t* buff){
    printf("en dest\n");
    int size = size_param(decode, buff);
    decode_fill_par(&decode->destino,buff,size);
    return 0;
}

int decode_path_param(decode_t* decode, buffer_t* buff){
    printf("en path\n");
    int size = size_param(decode, buff);
    decode_fill_par(&decode->path,buff,size);
    return 0;
}

int decode_inter_param(decode_t* decode, buffer_t* buff){
    printf("en inter\n");
    int size = size_param(decode, buff);
    decode_fill_par(&decode->interface,buff,size);
    return 0;
}

int decode_meth_param(decode_t* decode, buffer_t* buff){
    printf("en meth\n");
    int size = size_param(decode, buff);
    decode_fill_par(&decode->method,buff,size);
    return 0;
}

int save_dec_param(decode_t* decode, buffer_t* buff){
    uint32_t size_param;
    memset(&size_param,0, 1);
    printf("buff->read %ld \n", buff->read);
    uint8_t * hexa = (uint8_t*) buff->data + buff->read;
    for (int i = 0; i < buff->used - buff->read; i++){
            if (i > 0) printf(":");
            printf("%02X", hexa[i]);
    }
    printf("\n");
    memcpy(&size_param, buff->data + buff->read , sizeof(uint32_t) );
    buff->read += sizeof(uint32_t);
    //le sumo al size el end de caracter
    printf("size param %d\n", size_param);
    buffer_save_data(decode->params, buff->data + buff->read, size_param);
    buff->read += (size_param + SIZE_END);
    printf("param->used : %ld\n", decode->params->used);
    printf("param guardado: %s\n", decode->params->data);
    buffer_save_data(decode->params,",", 1 );
    printf("param->used after save ,: %ld\n", decode->params->used);
    printf("param guardado: %s\n", decode->params->data);
    return 0;
}


int decode_firm_param(decode_t* decode, buffer_t* buff){
    buffer_t* params = malloc(sizeof(buffer_t));
    buffer_create_size(params, INIT_ENCODED_BUFF_SIZE);
    decode->params = params;
     
    // PARAMETROS
    uint8_t cant_param;
    memset(&cant_param,0, 1);
    printf("buff->read %ld \n", buff->read);
    printf("buff->used %ld \n", buff->used);
    uint8_t * hexa = (uint8_t*) buff->data + buff->read + 3;
    for (int i = 0; i < buff->used - buff->read-3; i++){
            if (i > 0) printf(":");
            printf("%02X", hexa[i]);
    }
    memcpy(&cant_param, buff->data + buff->read + 3, 1 );

    //LONGITUD DE LA FIRMA ARRANCA EN 5
    int size_firma = 5;
    printf("cantidad par %d\n", cant_param);
    size_firma += (cant_param + SIZE_END );
    printf("size_firma %d\n", size_firma);
    int firma_padd_size = number_padd(size_firma, 8) + size_firma;
    printf("padd: %d\n", number_padd(size_firma, 8));
    printf("firma padd: %d\n", firma_padd_size);
    //PORQUE YA LEI UNO DE LA FIRMA
    buff->read += firma_padd_size-1;
    printf("buff->read %ld \n", buff->read);
    for(int i=0; i< cant_param; i++){
        save_dec_param(decode, buff);
    }
    return 0;
}

int decode_messaje(decode_t* decode){
    buffer_t* buff = decode->bytes;
    uint8_t type =0; 
    printf("buff-> read %ld\n", buff->read);
    printf("buff->used %ld\n", buff->used);
    while(buff->read < buff->used){
        printf("entre\n");
        memcpy(&type, buff->data + buff->read, 1 );
        buff->read += 1;
        printf("%02X\n",type);
        switch (type){
        case  0x06:
            decode_dest_param(decode,buff);
            break;
        case 0x01:
            decode_path_param(decode,buff);
            break;
        case 0x02:
            decode_inter_param(decode,buff);
            break;
        case 0x03:
            decode_meth_param(decode,buff);
            break;
        case  0x09:
            decode_firm_param(decode,buff);
            break;
        default:
            return -1;
        }
    }
    return 0;
}

void decoded_output(decode_t* decode, uint32_t msj_id){
    printf("* Id: 0x%08" PRIx16 "\n", msj_id);
    printf("* Destino: %s\n", decode->destino);
    printf("* Path: %s\n", decode->path);
    printf("* Interfaz: %s\n", decode->interface);
    printf("* Metodo: %s\n", decode->method);
    if(decode->params){
        char *save_ptr = NULL;
        printf("parametros %s\n", decode->params->data);
        printf("* Parámetros:\n");
        char* param = strtok_r(decode->params->data, ",", &save_ptr);
        //if (!param) throw_error("param not found");
        while (param != NULL ){
            printf("  %s\n",param);
            param = strtok_r(NULL, ",", &save_ptr);
        }
    }
   
}