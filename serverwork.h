#pragma once

#include"type.h"


serverstruct *buildserver();
void running(serverstruct *s);
void buildclient(serverstruct *s,int newfd);
void deletecommand(commandstruct *command);
void deletecommand(commandstruct *command,clientstruct *c);
std::string *handle(char *buf,serverstruct *s,int nbytes,clientstruct *c);
