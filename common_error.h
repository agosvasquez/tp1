#ifndef COMMON_ERROR_H
#define COMMON_ERROR_H
#include <stdio.h>
#include <stdlib.h>

int throw_error(char* msj);

int throw_sterr(char* msj, const char* param);

#endif
