#pragma once

#include"type.h"

std::string *set(commandstruct *command,serverstruct *s,clientstruct *c);
std::string *get(commandstruct *command,serverstruct *s,clientstruct *c);
std::string *exist(commandstruct *command,serverstruct *s,clientstruct *c);
std::string *del(commandstruct *command,serverstruct *s,clientstruct *c);
std::string *multi(clientstruct *c);
std::string *exec(serverstruct *s,clientstruct *c);
std::string *discard(clientstruct *c);
std::string *nottrue();
