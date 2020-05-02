#include "error.h"


int throw_error(char* msj){
    perror(msj);
    exit (-1);
}

int throw_sterr( char* msj, const char* param ){
    printf("aca throw");
    if (param) fprintf(stderr, "%s\n", param);
    else fprintf(stderr,"%s", msj);
    exit (-1);
   
}