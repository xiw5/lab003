#include"type.h"
#include"serverwork.h"
#include"mix.h"
#include"operator.h"
#include"afo.h"

serverstruct *buildserver()
{
  serverstruct *s = new serverstruct;
  struct addrinfo hints;
  memset(&hints,0,sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *servinfo;
  int rv = getaddrinfo(nullptr,PORT,&hints,&servinfo);
  if(rv != 0)
  {
    std::cerr<<"getaddrinfo"<<std::endl;
    return nullptr;
  }

  struct addrinfo *p;
  int sockfd;
  for(p = servinfo;p != nullptr;p = p->ai_next)
  {
    sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
    if(sockfd == -1)
    {
      std::cerr<<"socket"<<std::endl;
      continue;
    }
    if(bind(sockfd,p->ai_addr,p->ai_addrlen) == -1)
    {
      close(sockfd);
      std::cerr<<"bind"<<std::endl;
      continue;
    }
    break;
  }

  if(p == nullptr)
  {
    std::cerr<<"failed to bind"<<std::endl;
    return nullptr;
  }

  freeaddrinfo(servinfo);

  if(listen(sockfd,BACKLOG) == -1)
  {
    std::cerr<<"listen"<<std::endl;
    return nullptr;
  }
  std::cout<<"waitting for connections ...."<<std::endl;
  s->listener = sockfd;
  s->max_fd = s->listener;
  FD_SET(s->listener,&(s->master));
}

void running(serverstruct *s)
{
  std::ofstream fout("eof.txt",std::ios::out|std::ios::app);
  fd_set read_fds =  s->master;
//  std::cout<<"11"<<std::endl;
  if(select(s->max_fd+1,&read_fds,nullptr,nullptr,nullptr) == -1)
  {
    std::cerr<<"select "<<std::endl;
    return;
  }
//  std::cout<<"22"<<std::endl;
  struct sockaddr_storage remoteaddr;
  char remoteIP[INET6_ADDRSTRLEN];
  for(clientstruct *c = s->head;c != nullptr;c = c->next)
  {
    //std::cout<<"11"<<std::endl;
    if(!FD_ISSET(c->sockfd,&read_fds))
      continue;
    char buf[MAXDATASIZE];
    int nbytes = recv(c->sockfd,buf,sizeof(buf),0);
    buf[nbytes] = '\0';
    //std::cout<<buf<<std::endl;
    if(nbytes == -1)
    {
      std::cerr<<"recv"<<std::endl;
      continue;
    } 
    if(nbytes == 0)
    {
      std::cout<<c->sockfd<<" hang up "<<std::endl;
      close(c->sockfd);
      FD_CLR(c->sockfd, &s->master);
      if(c->pre != nullptr)
        c->pre->next = c->next;
      else
        s->head = c->next;
      if(c->next != nullptr)
        c->next->pre = c->pre;
      c->delmulti = 0;
  //    std::cout<<"444"<<std::endl;
      for(auto cit = c->command.begin();cit != c->command.end();cit++)
      {
        deletecommand(*cit);
        delete *cit;
      }
    //  std::cout<<"555"<<std::endl;
      c->command.clear();
      delete c;
      continue;
    }
    for(int i = 0;i<nbytes;i++)
      fout<<buf[i];
    fout<<"\0";
    std::string *p = handle(buf,s,nbytes,c);
    int len = (*p).size();                                                              
    if(sendall(c->sockfd,(*p).c_str(),&len) == -1)
      std::cerr<<"send"<<std::endl;
    delete p;
  }
  //std::cout<<"33"<<std::endl;
  if(FD_ISSET(s->listener,&read_fds))
  {
    socklen_t addrlen = sizeof(remoteaddr);
    int newfd = accept(s->listener,(struct sockaddr *)&remoteaddr,&addrlen);
    if(newfd == -1)
    {
        std::cerr<<"accept"<<std::endl;
       
    }
    else
    {
      if(newfd > s->max_fd)
        s->max_fd = newfd;
      FD_SET(newfd,&(s->master));
      std::cout<<"connect form: "<<inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr *)&remoteaddr),remoteIP,INET6_ADDRSTRLEN)<<" socket: "<<newfd<<std::endl;
      buildclient(s,newfd);
    }
  }
}

void buildclient(serverstruct *s,int newfd)
{
   clientstruct *c = new clientstruct;
   c->sockfd = newfd;
   if(s->head != nullptr)
     s->head->pre = c;
   c->next = s->head;
   c->pre = nullptr;
   s->head = c;
}


void deletecommand(struct commandstruct *command)
{
  for(auto it = command->value.begin();it != command->value.end();it++)
    delete *it;
  command->value.clear();
}

void deletecommand(struct commandstruct *command,clientstruct *c)
{
  commandstruct *nowcommand;
  for(;;)
  {
    if(command == nullptr)
      break;
    nowcommand = command->next;
    if(command->opera == EXEC || command->opera == DISCARD)
    {
      c->delmulti = 0;
      for(auto cit = c->command.begin();cit != c->command.end();cit++)
      {
        deletecommand(*cit);
        delete *cit;
      }
      c->command.clear();
    }
    if(c->delmulti == 0)
    {
      deletecommand(command);
      if(command->opera == MULTI)
        c->delmulti =1;
      delete command;
    }
    command = nowcommand;
  }
}

std::string *handle(char *p,serverstruct *s,int nbytes,clientstruct *c)
{
 // std::cout<<p<<std::endl;
  struct commandstruct *command = pro_to_v(p,nbytes);
 // std::cout<<command->next<<std::endl;
 // std::cout<<command->key<<std::endl;
//  std::cout<<*(command->value[0])<<std::endl;
  std::string *res = new std::string;
  *res = "*";
  struct commandstruct *tem = command;
  int num = 0;
  char cnum[100];
  for(;tem !=nullptr;)
  {
    num++;
    tem = tem->next;
  }
  sprintf(cnum,"%d",num);
  *res += cnum;
  *res += '\r';
  *res += '\n';
  tem = command;
  for(;tem != nullptr;)
  {
    std::string * smalls;
    //std::cout<<"22"<<std::endl;
    if(tem->opera == SET)
    {
   //std::cout<<*(command->value[0])<<std::endl;
      smalls = (set(tem,s,c));
    }
 // std::cout<<"sss"<<std::endl;
    if(tem->opera == GET)
      smalls = (get(command,s,c));
    if(tem->opera == EXIST)
      smalls = (exist(command,s,c));
    if(tem->opera == DEL)
      smalls = (del(command,s,c));
    if(tem->opera == MULTI)
      smalls = multi(c);
    if(tem->opera == EXEC)
      smalls = exec(s,c);
    if(tem->opera == DISCARD)
      smalls = discard(c);
    tem = tem->next;
    *res += *smalls;
    delete smalls;
    //for(auto it = command->value.begin();it != command->value.end();it++)
      //delete *it;
    //command->value.clear();
    //delete command;
    //return nottrue();
  }
  deletecommand(command,c);
  return res;
}
