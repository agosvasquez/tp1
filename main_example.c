#include <stdio.h>
#include "socket.h"

void stream(void * ctx){
    socket_t* s = ctx;
    while(!feof(stdin)){
        char buff[5]="";
        size_t rv = fread(buff, 1, sizeof(buff), stdin);
        socket_send(s, buff, rv);
    } 

}

int main(int argc, char const *argv[]){
    socket_t socket;
    socket_create(&socket);
    socket_connect(&socket, "localhost", "7777");
    stream(&socket);
    socket_destroy(&socket);
    return 0;
}