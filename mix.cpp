#include"type.h"
#include"mix.h"

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

int sendall(int s,const char *buf,int *len)
{
  int total = 0;
  int bytesleft = *len;
  int n;
  for(;total < *len;)
  {
    n = send(s,buf+total,bytesleft,0);
    if(n == -1)
      break;
    total += n;
    bytesleft -= n;
  }
  *len = total;
  return n==-1?-1:0;
}

commandstruct *pro_to_v(char *buf,int nbytes)
{
  //std::cout<<buf<<std::endl;
  commandstruct *command = new commandstruct;
  commandstruct *nowcommand = command;
  int now = 1;
  for(;now+1 < nbytes;)
  {
    if(now != 1)
    {
      commandstruct *newcommand = new commandstruct;
      nowcommand->next = newcommand;
      nowcommand = newcommand;
    }
    int osum = 0;
    for(;buf[now] != '\r';now++)
    {
      osum = osum*10+buf[now]- '0';
    }
    now++;
    for(;buf[now] != '\r';now++);
      now += 2;
    nowcommand->opera ="";
    for(;buf[now] != '\r';now++)
     nowcommand->opera += buf[now];
    //std::cout<<nowcommand->opera<<std::endl;
    now++;
    if(osum > 1)
    {
      for(;buf[now] != '\r';now++);
        now += 2;
      nowcommand->key = "";
      for(;buf[now] != '\r';now++)
        nowcommand->key += buf[now];
      now++;
    }
  //std::cout<<osum<<std::endl;
    for(int i=2;i<osum;i++)
    {
      std::string *p = new std::string;
      *p = "";
    //std::cout<<now<<std::endl;
      for(;buf[now] != '\r';now++);
        now += 2;
    //std::cout<<buf+now<<std::endl;
      for(;buf[now] != '\r';now++)
        *p += buf[now];
    //std::cout<<*p<<std::endl;
      now++;
      nowcommand->value.push_back(p);
    }
    now += 2;
    //std::cout<<buf[now]<<std::endl;
  }
  return command;
}
