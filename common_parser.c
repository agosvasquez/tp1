#define _POSIX_C_SOURCE 200112L
#include "common_parser.h"
#include <string.h>
#include "common_error.h"
#include <byteswap.h>
#include <netinet/in.h>
#include <inttypes.h>


uint8_t type_prot_des =  0x06;
uint8_t type_prot_path =  0x01;
uint8_t type_prot_inter =  0x02;
uint8_t type_prot_meth =  0x03;


uint8_t END = 0x00;
size_t POS_BOD_S= 4;
size_t POS_ARR_S= 12;
size_t POS_START_ARR =16;
int SIZE_END = 1;
int SIZE_SEP = 1;

int INIT_ENCODED_BUFF_SIZE = 32;
int SIZE_PARAM = 100;

int encoded_create(encode_t* encode, uint32_t msj_id){
    buffer_t* buff = malloc(sizeof(buffer_t));
    buffer_create(buff);
    encode->bytes = buff;
    encode->msj_id = msj_id;
    encode->count_pad=0;
    return 0;
}

void encoded_destoyed(encode_t* encoded){
    buffer_destroy(encoded->bytes);
    free(encoded->bytes);
}


int decoded_create_size(decode_t* decode,buffer_t* buff,buffer_t* p, int size){
    buffer_create_size(buff, size);
    buffer_create_size(p, INIT_ENCODED_BUFF_SIZE);
    decode->bytes = buff;
    decode->destino = NULL;
    decode->path= NULL;
    decode->interface = NULL;
    decode->method = NULL;
    decode->params = p; 
    return 0;
}

void decoded_destroy(decode_t* decode){
    buffer_destroy(decode->bytes);
    buffer_destroy(decode->params);
    free(decode->destino);
    free(decode->interface);
    free(decode->path);
    free(decode->method);
}

int number_padd(int size, int mult){
    if ((size%mult) == 0) return 0;
    int div = (int)size/mult;
    return(( div + 1)* mult ) - size;
}

void initialize(char* arr1, char* arr2, char*arr3, char* arr4){
    memset(arr1,0, SIZE_PARAM);
    memset(arr2,0,SIZE_PARAM);
    memset(arr3,0,SIZE_PARAM);
    memset(arr4,0,SIZE_PARAM);
}

void set_array_len(encode_t* encode){
    int long_arr = encode->bytes->used - POS_START_ARR;
    uint32_t swap_size_a= to_little_32(long_arr);
    memcpy(encode->bytes->data+POS_ARR_S,(char*)&swap_size_a,sizeof(uint32_t));
}

void process_body(encode_t* encode, char* params){
    int size_before_body= encode->bytes->used;

    encode_body(encode, params);

    int long_param = encode->bytes->used - size_before_body;
    uint32_t swap_size_p= to_little_32(long_param);
    memcpy(encode->bytes->data+POS_BOD_S,(char*)&swap_size_p,sizeof(uint32_t));
}


int encode_line(encode_t * encode, char* data){
    uint16_t S = to_little_16(0x73);
    uint16_t O = to_little_16(0x6F);
    size_t s_firm, s_meth;
    char dest[SIZE_PARAM],path[SIZE_PARAM],inter[SIZE_PARAM],met[SIZE_PARAM];
    encode_set_static(encode);
    initialize(dest,path,inter,met);
    sscanf(data, "%s %s %s" ,dest, path, inter);
    s_meth= strlen(dest)+ strlen(path)+ strlen(inter);
    // +3 por los espacios
    sscanf(data + s_meth+3, "%[^(](", met);
    encode_arg(encode, dest, &type_prot_des, &S);
    encode_arg(encode, path, &type_prot_path, &O);
    encode_arg(encode, inter, &type_prot_inter, &S);
    encode_arg(encode, met, &type_prot_meth,&S);
    // +3 por los espacios +1 por el parentesis
    s_firm= s_meth+ strlen(met)+ 4;
    char* firm = data+ s_firm;
    int par_len= strlen(firm);
    memcpy(firm+par_len, "\0", sizeof(char));
    char params[SIZE_PARAM];
    memset(params,0,SIZE_PARAM);
    snprintf(params, par_len + SIZE_END , "%s", firm);
    
    encode_params_firm(encode,firm);
    set_array_len(encode);

    process_body(encode,params);
    return 0;  
}

int encode_body(encode_t * encode, char* params){
    char param[SIZE_PARAM];
    char par[SIZE_PARAM];
    if (params[0] == ')')return 0;    
    while (sscanf(params, "%[^,],", param) == 1){
        int len = strlen(param);
        if (strchr(param, ')') != NULL) {
            sscanf(param, "%[^)])", par);
            save_param(encode,par, len -1);
            break;
        }
        save_param(encode,param, len);
        params+=len+1;
        memset(param,0,SIZE_PARAM);
    }
    return 0;
}

int save_param(encode_t * encode, char* param, int size){
    uint32_t swap_size_par = to_little_32((uint32_t)size);
    buffer_save_data(encode->bytes, (char*)&swap_size_par, sizeof(uint32_t));
    buffer_save_data(encode->bytes, param, size);
    buffer_save_data(encode->bytes, "\0", 1);
    return 0;
}
    

int encode_convert_multiple(char* arg, char** arg_p, int s_arg_p, size_t size){
    //un lugar mas para guardar
    *arg_p = malloc(sizeof(char)* s_arg_p+1);
    memset(*arg_p, 0, sizeof(char)*s_arg_p+1);
    memcpy(*arg_p,arg,size);
    memcpy(*arg_p + size , "\0", 1);
    //revisar
    int padd = s_arg_p - size - SIZE_END;
    for (size_t i = 0; i <= padd && padd >0 ; i++){
        memcpy(*arg_p + size+1 ,(char*)&END, sizeof(uint8_t));
    }
    return 0;
}

void actualize_count_pad(encode_t* encode,  int pad){
    encode->count_pad+= pad;
}

void encode_set_static(encode_t* encode){
    uint8_t type = 0x01;
    uint8_t flags = 0x00;
    // dejo el lugar guardado porque todavia no se
    //agregarle a todo Hton
    uint32_t len= to_little_32(0x0000);
    buffer_save_data(encode->bytes,"l",1);
    buffer_save_data(encode->bytes, (char*)&type, 1);
    buffer_save_data(encode->bytes, (char*)&flags,1);
    buffer_save_data(encode->bytes, (char*)&type, 1);
    buffer_save_data(encode->bytes, (char*)&len, sizeof(uint32_t));
    buffer_save_data(encode->bytes,(char*)&(encode->msj_id),sizeof(uint32_t));
    buffer_save_data(encode->bytes,(char*)&len, sizeof(uint32_t) );
}


int encode_arg(encode_t* encode, char* arg, uint8_t* t_p, uint16_t* t_d){
    size_t size_param = strlen(arg);
    uint32_t swap_size_par = to_little_32((uint32_t)size_param);
    //para poder 
    int padd = number_padd(size_param+ SIZE_END, 8);

    int size_arg_pad= size_param + SIZE_END + padd;
    char * arg_pad [size_arg_pad];
    encode_convert_multiple(arg, arg_pad, size_arg_pad, strlen(arg));
    uint8_t stat = 0x01;
    

    buffer_save_data(encode->bytes, (char*)t_p, 1);
    buffer_save_data(encode->bytes, (char*)&stat ,1);
    buffer_save_data(encode->bytes, (char*)t_d, sizeof(uint16_t));
    buffer_save_data(encode->bytes, (char*)&swap_size_par, sizeof(uint32_t));
    buffer_save_data(encode->bytes,*arg_pad, size_arg_pad);
    actualize_count_pad(encode,padd);
    free(*arg_pad);
    return 0;
}

int encode_firm(encode_t* encode, int* cant_par){
    uint8_t t_p = 0x09;
    uint8_t stat = 0x01;
    uint8_t s = 0x73;
    uint16_t G = to_little_16(0x67);
    size_t size_param = *cant_par +5;

    char aux[SIZE_PARAM];
    memset(aux,0,SIZE_PARAM);
    memcpy(aux, (char*)&t_p, 1);
    memcpy(aux+1,(char*)&stat ,1);
    memcpy(aux+2, (char*)&G, sizeof(uint16_t));
    memcpy(aux+4, (char*)cant_par, sizeof(uint8_t));
    for (int i=1; i <= *cant_par; i++){
        memcpy(aux+4+i, (char*)&s, sizeof(uint8_t));
    }

    int padd = number_padd(size_param+ SIZE_END, 8);
    int size_arg_pad= size_param + SIZE_END + padd;
    char* arg_pad[size_arg_pad];
    encode_convert_multiple(aux,arg_pad, size_arg_pad,size_param);

    buffer_save_data(encode->bytes,*arg_pad, size_arg_pad);
    actualize_count_pad(encode,padd);
    free(*arg_pad);
    return 0;
}

int encode_params_firm(encode_t* encode,char* firm){
    int counter_par =0;
    char param [SIZE_PARAM];
    if (firm[0] != ')'){
        while (sscanf(firm, "%[^,],", param)){
            counter_par +=1;
            if (!strchr(firm, ','))break;  
            firm += strlen(param)+1;
            memset(param,0,SIZE_PARAM);
        }
    }
    encode_firm(encode, &counter_par);
    
    return 0;
}

uint16_t to_little_16(uint16_t x){
    uint16_t to_net =htons(x);
    return bswap_16(to_net);
}

uint32_t to_little_32(uint32_t x){
    uint32_t to_net =htonl(x);
    return bswap_32(to_net);
}

int extract_body_size(char* buff){
    return encode_extract_size(buff, POS_BOD_S);
}

int extract_array_size(char* buff){
    return encode_extract_size(buff, POS_ARR_S);
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
    buff->read += size;
    return 0;
}

int size_param(decode_t* decode, buffer_t* buff){
    uint32_t _size_param;
    uint32_t size_param;
    //posicion del size es en 4 
    memset(&_size_param,0, sizeof(int));
    memset(&size_param,0, sizeof(int));
    memcpy(&_size_param, buff->data + buff->read + 3 , sizeof(int) );
    
    size_param = to_little_32(_size_param);

    buff->read += 7;
    int padd = number_padd(size_param + SIZE_END, 8);
    return size_param+ padd + SIZE_END;   
}

int decode_dest_param(decode_t* decode, buffer_t* buff){
    int size = size_param(decode, buff);
    decode_fill_par(&decode->destino,buff,size);
    return 0;
}

int decode_path_param(decode_t* decode, buffer_t* buff){
    int size = size_param(decode, buff);
    decode_fill_par(&decode->path,buff,size);
    return 0;
}

int decode_inter_param(decode_t* decode, buffer_t* buff){
    int size = size_param(decode, buff);
    decode_fill_par(&decode->interface,buff,size);
    return 0;
}

int decode_meth_param(decode_t* decode, buffer_t* buff){
    int size = size_param(decode, buff);
    decode_fill_par(&decode->method,buff,size);
    return 0;
}

int save_dec_param(decode_t* decode, buffer_t* buff){
    uint32_t size_param;
    memset(&size_param,0, 1);
    memcpy(&size_param, buff->data + buff->read , sizeof(uint32_t) );
    buff->read += sizeof(uint32_t);
    //le sumo al size el end de caracter
    buffer_save_data(decode->params, buff->data + buff->read, size_param);
    buff->read += (size_param + SIZE_END);
    buffer_save_data(decode->params,",", 1);
    return 0;
}


int decode_firm_param(decode_t* decode, buffer_t* buff){ 
    // PARAMETROS
    uint8_t cant_param;
    memset(&cant_param,0, 1);
    memcpy(&cant_param, buff->data + buff->read + 3, 1);

    //LONGITUD DE LA FIRMA ARRANCA EN 5
    int size_firma = 5;
    size_firma += (cant_param + SIZE_END);
    int firma_padd_size = number_padd(size_firma, 8) + size_firma;

    //PORQUE YA LEI UNO DE LA FIRMA
    buff->read += firma_padd_size-1;
    for (int i=0; i< cant_param; i++){
        save_dec_param(decode, buff);
    }
    return 0;
}

int decode_messaje(decode_t* decode){
    buffer_t* buff = decode->bytes;
    uint8_t type =0; 
    while (buff->read < buff->used){
        memcpy(&type, buff->data + buff->read, 1);
        buff->read += 1;
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
    printf("* Id: 0x%08" PRIx16 "\n", to_little_32(msj_id));
    printf("* Destino: %s\n", decode->destino);
    printf("* Ruta: %s\n", decode->path);
    printf("* Interfaz: %s\n", decode->interface);
    printf("* Metodo: %s\n", decode->method);
    if (decode->params->data[0] != 0){     
        int size = 0;
        printf("* Parametros:\n");
        char param[SIZE_PARAM];
        memset(param,0, SIZE_PARAM);
        buffer_t* aux = decode->params;
        memcpy(aux->data+ aux->used, "\0", sizeof(char));
        while (sscanf(aux->data + size, "%[^,]", param) == 1){
            printf("    * %s\n",param);
            size += strlen(param)+1;
            memset(param,0, SIZE_END);
        }
    }
    printf("\n");
}
