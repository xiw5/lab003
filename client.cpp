#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<unistd.h>
#include<errno.h>
#include<cstring>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define PORT "3490"
#define MAXDATASIZE 100

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

int main(int argc,char *argv[])
{
  if(argc != 2)
  {
    std::cerr<<"usage: client hostname"<<std::endl;
    return 1;
  }

  struct addrinfo hints;
  memset(&hints,0,sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *servinfo;
  int rv = getaddrinfo(argv[1],PORT,&hints,&servinfo);
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
      std::cerr<<"client: socket"<<std::endl;
      continue;
    }

    if(connect(sockfd,p->ai_addr,p->ai_addrlen) == -1)
    {
      close(sockfd);
      std::cerr<<"client: connect"<<std::endl;
      continue;
    }

    break;
  }

  if(p==nullptr)
  {
    std::cerr<<"client: fail to connect"<<std::endl;
    return 2;
  }

  char s[INET6_ADDRSTRLEN];
  inet_ntop(p->ai_family,get_in_addr((struct sockaddr *)p->ai_addr),s,sizeof(s));
  std::cout<<"client: connecting to "<<s<<std::endl;

  freeaddrinfo(servinfo);
  
  int numbtyes;
  char buf[MAXDATASIZE];
  for(int i=0;;i++)
  {
    std::cout<<"INPUT: ";
    
    std::cin.getline(buf,MAXDATASIZE-1);
    //std::cout<<"send "<<buf<<std::endl;
    if(buf[0] == 'Q'&&strlen(buf) == 1)
      break;
    if(send(sockfd,buf,strlen(buf),0) == -1)
      std::cerr<<"send"<<i<<std::endl;

    numbtyes = recv(sockfd,buf,MAXDATASIZE-1,0);
    if(numbtyes == -1)
    {
      std::cerr<<"recv"<<i<<std::endl;
      return 1;
    }
    buf[numbtyes]='\0';
    std::cout<<buf<<std::endl;
  }
  close(sockfd);

  return 0;
}
