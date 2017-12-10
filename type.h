#pragma once

#include<unordered_map>
#include<vector>
#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<cerrno>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<csignal>
#include<iostream>
#include<functional>
#include<string>

#define PORT "3992"
#define BACKLOG 10
#define MAXDATASIZE 1024
#define makepair(a,b) std::pair<std::string,std::string>(a,b) 

const std::string SET="SET";
const std::string GET="GET";
const std::string DEL="DEL";
const std::string EXIST="EXIST";
const std::string MULTI="MULTI";
const std::string EXEC="EXEC";

struct commandstruct 
{
  std::string opera;
  std::string key;
  std::vector<std::string *> value;
  commandstruct *next;
  commandstruct()
  {
    next = nullptr;
  }
};

struct clientstruct 
{
  int sockfd;
  struct clientstruct *next;
  struct clientstruct *pre;
  int multi;
  int delmulti;
  std::vector<commandstruct *> command;
  clientstruct()
  {
    multi = 0;
    delmulti = 0;
  }
};

struct serverstruct
{
  int listener;
  int max_fd;
  fd_set master;
  struct clientstruct *head;
  std::unordered_map<std::string,std::string> hashtable;
  serverstruct()
  {
    head = nullptr; 
  }
};


