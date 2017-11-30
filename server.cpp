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
#define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 100
std::string SET="SET";
std::string GET="GET";
std::string DEL="DEL";
std::string EXIST="EXIST";
std::hash<std::string> h;
struct dictEntry
{
  std::string key;
  std::string value;
  dictEntry *next;
};
struct dictht
{
  dictEntry **table;
  unsigned long size;
  unsigned long sizemask;
  unsigned long used;
  dictht()
  {
    size=4;
    sizemask=3;
    used=0;
    table=new dictEntry * [4];
    for(int i=0;i<4;i++)
      table[i]=nullptr;
  }
  dictht(unsigned long n)
  {
    size=n;
    sizemask=n-1;
    used=0;
    table=new dictEntry * [n];
    for(unsigned long i=0;i<n;i++)
      table[i]=nullptr;
  }
};
struct dict
{
  dictht ht[2];
  int rehashidx;
  dict()
  {
    rehashidx=-1;
  }
}dictionary;
std::string extract(std::string fin,std::string::size_type &i)
{
  std::string s;
  for(;i!=fin.size()&&isspace(fin[i]);i++);
  for(;i!=fin.size()&&!isspace(fin[i]);i++)
    s+=fin[i];
  return s;
}
std::string re;
std::string  set(std::string key,std::string value)
{
  unsigned long index=dictionary.ht[0].sizemask&h(key);
  dictEntry *newdE=new dictEntry;
  newdE->key=key;
  newdE->value=value;
  newdE->next=dictionary.ht[0].table[index];
  dictionary.ht[0].table[index]=newdE;
  dictionary.ht[0].used++;
  re="OK";
  return re;
}
std::string redel(dictEntry *now,std::string key)
{
  if(now->next==nullptr)
  {
    re="KEY doesn't exist";
    return re;
  }
  if(now->next->key==key)
  {
    now->next=now->next->next;
    re ="OK";
    return re;
  }
  return redel(now->next,key);
}
std::string del(std::string key)
{
  unsigned long index = dictionary.ht[0].sizemask&h(key);
  if(dictionary.ht[0].table[index]==nullptr)
  {
    re ="KEY doesn't exist";
    return re;
  }
  if(dictionary.ht[0].table[index]->key==key)
  {
    dictionary.ht[0].table[index]=dictionary.ht[0].table[index]->next;
    re  = "OK";
    return re;
  }
  else 
    return redel(dictionary.ht[0].table[index],key);
}
std::string exist(std::string key)
{
  unsigned long index=dictionary.ht[0].sizemask&h(key);
  for(dictEntry *now=dictionary.ht[0].table[index];now!=nullptr;now=now->next)
  {
    if(now->key==key)
    {
      re ="KEY exist";
      return re;
    }
  }
  re ="KEY doesn't exist";
  return re;
}
std::string get(std::string key)
{
  unsigned long index=dictionary.ht[0].sizemask&h(key);
  for(dictEntry *now=dictionary.ht[0].table[index];now!=nullptr;now=now->next)
  {
    if(now->key==key)
    {
      return now->value;
    }
  }
  re ="KEY doesn't exist";
  return re;
}
std::string  handle(std::string fin)
{
  re ="FORMAT ERROR!";
  std::string::size_type i = 0;
  std::string opera = extract(fin,i);
  std::string key = extract(fin,i);
  if(key.size() == 0||opera.size() == 0)
  {
    return re;
  }
  if(opera == SET)
  {
    for(;i != fin.size()&&isspace(fin[i]);i++);
    if(i == fin.size())
    {
      return re;
    }
    fin=fin.erase(0,i);
    return set(key,fin);
  }
  if(opera == DEL)
  {
    return del(key);
  }
  if(opera==EXIST)
  {
    return exist(key);
  }
  if(opera==GET)
  {
    return get(key);
  }
  return re;
}

void sigchld_handle(int s)
{
  while(waitpid(-1,nullptr,WNOHANG)>0);
}

void *get_in_addr(struct sockaddr *sa)
{
  if(sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  else
  {
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }
}

int main()
{
  struct addrinfo hints;
  memset(&hints,0,sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *servinfo;
  int rv = getaddrinfo(nullptr,PORT,&hints,&servinfo);
  if(rv != 0)
  {
    std::cerr<<"getaddrinfo: "<<gai_strerror(rv)<<std::endl;
    return 1;
  }

  struct addrinfo *p;
  int sockfd;
  for(p = servinfo;p != nullptr;p = p->ai_next)
  {
    sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
    if(sockfd == -1)
    {
      std::cerr<<"server: socket"<<std::endl;
      continue;
    }
    if(bind(sockfd,p->ai_addr,p->ai_addrlen) == -1)
    {
      close(sockfd);
      std::cerr<<"server: bind"<<std::endl;
      continue;
    }
    break;
  }

  if(p == nullptr)
  {
    std::cerr<<"server: failed to bind"<<std::endl;
    return 2;
  }

  freeaddrinfo(servinfo);

  if(listen(sockfd,BACKLOG) == -1)
  {
    std::cerr<<"listen"<<std::endl;
    return 1;
  }

  struct sigaction sa;
  sa.sa_handler = sigchld_handle;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD,&sa,nullptr) == -1)
  {
    std::cerr<<"sigaction"<<std::endl;
    return 1;
  }

  std::cout<<"server: waiting for connections..."<<std::endl;

  socklen_t sin_size;
  struct sockaddr_storage their_addr;
  char s[INET6_ADDRSTRLEN];
  int new_fd;
  for(;;)
  {
    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd,(struct sockaddr *)&their_addr,&sin_size);

    if(new_fd == -1)
    {
      std::cerr<<"accept"<<std::endl;
      continue;
    }

    inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s,sizeof(s));
    std::cout<<"server: got connection from "<<s<<std::endl;
      
    for(int i=0;;i++)
    {
      char buf[MAXDATASIZE];
      int numbytes = recv(new_fd,buf,MAXDATASIZE-1,0);
      //std::cout<<buf<<std::endl;
      if(numbytes == -1)
      {
        std::cerr<<"recv"<<i<<std::endl;
      }
      buf[numbytes]='\0';
      if(buf[0] == 'Q'&&strlen(buf) == 1)
      {
        break;
      }
      std::string input = buf;
     // std::cout<<input<<std::endl;
      input = handle(input);
      //std::cout<<input<<std::endl;
      //std::cout<<"server: recived '"<<buf<<"'"<<std::endl;

      if(send(new_fd,input.c_str(),input.size(),0) == -1)
        std::cerr<<"send 2 "<<i<<std::endl;
    }
    close(new_fd);
  }

  return 0;
}
