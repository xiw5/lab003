#pragma once

#include"type.h"

void *get_in_addr(struct sockaddr *sa);
int sendall(int s,const char *buf,int *len);
commandstruct *pro_to_v(char *p,int nbytes);
