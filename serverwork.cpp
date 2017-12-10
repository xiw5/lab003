#include"type.h"
#include"serverwork.h"
#include"mix.h"
#include"operator.h"

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
  fd_set read_fds =  s->master;
 
  if(select(s->max_fd+1,&read_fds,nullptr,nullptr,nullptr) == -1)
  {
    std::cerr<<"select "<<std::endl;
    return;
  }

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
      if(c->next != nullptr)
        c->next->pre = c->pre;
      continue;
    }
    std::string *p = handle(buf,s,nbytes,c);
    int len = (*p).size();                                                              
    if(sendall(c->sockfd,(*p).c_str(),&len) == -1)
      std::cerr<<"send"<<std::endl;
    delete p;
  }
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
      clientstruct *c = new clientstruct;
      c->sockfd = newfd;
      if(s->head != nullptr)
        s->head->pre = c;
      c->next = s->head;
      c->pre = nullptr;
      s->head = c;
    }
  }
}

void deletecommand(struct commandstruct *command,clientstruct *c)
{
  if(command->opera == MULTI)
    c->delmulti = 1;
  if(command->opera == EXEC)
    c->delmulti = 0;
  if(command->next != nullptr)
    deletecommand(command->next,c);
  if(command->opera == MULTI)
    c->delmulti = 0;
  
  for(auto it = command->value.begin();it != command->value.end();it++)
    delete *it;
    command->value.clear();
    delete command;
}

std::string *handle(char *p,serverstruct *s,int nbytes,clientstruct *c)
{
 // std::cout<<p<<std::endl;
  struct commandstruct *command = pro_to_v(p,nbytes);
 // std::cout<<command->next<<std::endl;
 // std::cout<<command->key<<std::endl;
//  std::cout<<*(command->value[0])<<std::endl;
  std::string *res;
  if(command->opera == SET)
    res = set(command,s,c);
  //std::cout<<"sdsd"<<std::endl;
  if(command->opera == GET)
    res = get(command,s,c);
  //std::cout<<"get "<<std::endl;
  if(command->opera == EXIST)
    res = exist(command,s,c);
  //std::cout<<"exist"<<std::endl;
  if(command->opera == DEL)
    res = del(command,s,c);
 // std::cout<<*res<<std::endl;
  if(command->opera == MULTI)
    res = multi(c);
  if(command->opera == EXEC)
    res = exec(s,c);
  struct commandstruct *tem =command;
  for(;tem->next != nullptr;)
  {
    std::string * smalls;
    //std::cout<<"22"<<std::endl;
    tem = tem->next;
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
