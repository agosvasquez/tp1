#define _POSIX_C_SOURCE 201112L
#include "common_socket.h"
#include "common_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

const int QUEUE_LEN_LISTEN = 10;

void socket_settings(struct addrinfo* hints){
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_family= AF_INET;
    hints->ai_socktype= SOCK_STREAM;
    hints->ai_flags = 0;
    hints->ai_protocol= 6;
}

int socket_create(socket_t* self){
    self->socket = -1;
    return 0;
}

int socket_destroy(socket_t* self){
    self->socket = -1;
    return 0;
}

void socket_shutdown(socket_t* self, int channel){
    shutdown(self->socket, channel);
    close(self->socket);
}

int socket_bind_and_listen(socket_t* self, const char* service){
    struct addrinfo hints, *res;
    int s, sfd;
    int opt =1;
    
    socket_settings(&hints); 
    if ((s = getaddrinfo(NULL,service, &hints, &res)) != 0) 
        throw_sterr("getaddrinfo:", gai_strerror(s));
    
    if ((sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))< 0){
        freeaddrinfo(res);
        throw_error("create socket");
    }
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        freeaddrinfo(res);
        throw_error("setsockop error");
    }    
    if (bind(sfd, res->ai_addr, res->ai_addrlen) < 0){
        freeaddrinfo(res);
        throw_error("Bind error");
    }
    if (listen(sfd, QUEUE_LEN_LISTEN) < 0){
        freeaddrinfo(res);
        throw_error("Listen error");
    }
    
    freeaddrinfo(res);
    self->socket = sfd; 
    return 0;            
}

int socket_accept(socket_t* self, socket_t* accepted_socket){
    int s = accept(self->socket, NULL,NULL);
    if (s < 0) throw_error("Not accepted connection");
    accepted_socket->socket = s;
    return 0;
}

int socket_connect(socket_t* self, const char* host_name, const char* service){
    struct addrinfo hints, *res, *r;
    int s, socket_fd;

    socket_settings(&hints);

    if ((s = getaddrinfo(host_name, service, &hints,&res)) !=0) 
        throw_sterr("getaddr:", gai_strerror(s));
    for (r = res; r != NULL; r = r->ai_next) {
        socket_fd = socket(r->ai_family, r->ai_socktype,r->ai_protocol);
        if (socket_fd == -1) continue;
        if (connect(socket_fd, r->ai_addr, r->ai_addrlen) != -1){
            self->socket = socket_fd;
            break;                  
        }
        close(socket_fd);
    }
    if (!r) {
        freeaddrinfo(res); 
        throw_sterr("Could not connect\n", NULL);               
    }
    freeaddrinfo(res);  
    return 0;
}

int socket_send(socket_t* self, char* buff , size_t length){
    int sum_b = 0;
    int bytes =0;
    int to_send= length;
    while (sum_b != length){
        if ((bytes = send(self->socket,buff+sum_b,to_send,MSG_NOSIGNAL))<0) 
            throw_error("send failed");
        sum_b += bytes;
        to_send -= bytes;
    }
    return sum_b;
}

int socket_receive(socket_t* self, char* buff, size_t length){
    int sum_b = 0;
    int bytes = 0;
    int to_read = length;
    while (sum_b != length){
        if ((bytes = recv(self->socket, buff+sum_b, to_read,0)) <0) 
            throw_error("recive failed");
        // client closed
        if (bytes == 0) return 0;
        sum_b += bytes;
        to_read -= bytes;
    }
    return sum_b;
}



