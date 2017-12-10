#include"type.h"
#include"operator.h"

std::string *set(commandstruct *command,serverstruct *s,clientstruct *c)
{
  std::string *p = new std::string;
  if(c->multi == 1)
  {
    *p = "+QUEUED";
    *p += '\r';
    *p += '\n';
    c->command.push_back(command);
    //std::cout<<c->command[0]->opera<<std::endl;
    return p;
  }
  std::cout<<command->key<<"  "<<command->opera<<"  "<<*(command->value[0])<<std::endl;
  std::unordered_map<std::string,std::string>::iterator it = s->hashtable.find(command->key);
  if(it != s->hashtable.end())
    s->hashtable.erase(it);
  //std::cout<<*(command->value[0])<<std::endl;
  s->hashtable.insert(makepair(command->key,*(command->value[0])));
  *p = "+OK";
  *p += '\r';
  *p += '\n';
  return p;
}

std::string *get(commandstruct *command,serverstruct *s,clientstruct *c)
{
  std::string *p = new std::string;
  if(c->multi == 1)
  {
    *p = "+QUEUED";
    *p += '\r';
    *p += '\n';
    c->command.push_back(command);
    std::cout<<c->command.size()<<std::endl;
    return p;
  }
  //std::cout<<"ni zen me jin lai le"<<std::endl;
  char csum[100];
  *p = "$";
  std::unordered_map<std::string,std::string>::iterator it = s->hashtable.find(command->key);
  if(it == s->hashtable.end())
    *p += "-1";
  else
  {
   // std::cout<<it->second<<std::endl;
    sprintf(csum,"%d",it->second.size());
    *p += csum;
    *p += '\r';
    *p += '\n';
    *p += it->second;
  }
  *p += '\r';
  *p += '\n';
  //std::cout<<*p<<std::endl;
  return p;
}

std::string *exist(commandstruct *command,serverstruct *s,clientstruct *c)
{
  std::string *p = new std::string;
  if(c->multi == 1)
  {
    *p = "+QUEUED";
    *p += '\r';
    *p += '\n';
    c->command.push_back(command);
    return p;
  }
  char csum[100];
  int sum = 0;
  std::unordered_map<std::string,std::string>::iterator it = s->hashtable.find(command->key);
  if(it != s->hashtable.end())
    sum++;
  for(auto ite = command->value.begin();ite != command->value.end(); ite++)
    if(s->hashtable.find(**ite) != s->hashtable.end())
      sum++;
  sprintf(csum,"%d",sum);
  *p = ":";
  *p += csum;
  *p += '\r';
  *p += '\n';
  return p;
}

std::string *del(commandstruct *command,serverstruct *s,clientstruct *c)
{
  std::string *p = new std::string;
  if(c->multi == 1)
  {
    *p = "+QUEUED";
    *p += '\r';
    *p += '\n';
    c->command.push_back(command);
    return p;
  }
  char csum[100];
  int sum = 0;
  std::unordered_map<std::string,std::string>::iterator it = s->hashtable.find(command->key);
  if(it != s->hashtable.end())
  {
    sum++;
    s->hashtable.erase(it);
  }
  for(auto ite = command->value.begin();ite != command->value.end(); ite++)
    if(s->hashtable.find(**ite) != s->hashtable.end())
    {
      sum++;
      s->hashtable.erase(s->hashtable.find(**ite));
    }
  sprintf(csum,"%d",sum);
  *p = ":";
  *p += csum;
  *p += '\r';
  *p += '\n';
  return p;
}

std::string *nottrue()
{
  std::string *p = new std::string;
  *p = "-false";
  return p;
}

std::string *multi(clientstruct *c)
{
  c->multi = 1;
  c->command.clear();
  std::string *p = new std::string;
  *p = "+OK";
  *p += '\r';
  *p += '\n';
  return p;
}

std::string *exec(serverstruct *s,clientstruct *c)
{
  std::cout<<"111"<<std::endl;
  c->multi = 0;
  std::string *res;
  std::cout<<c->command[1]->opera<<std::endl; 
  for(auto it=(c->command.begin());it != c->command.end();it++)
  {
    std::cout<<(*it)->opera<<std::endl;
    if(it == c->command.begin())
    {
      if((*it)->opera == SET)
        res = set(*it,s,c);
      if((*it)->opera == GET)
        res = get(*it,s,c);
      if((*it)->opera == DEL)
        res = del(*it,s,c);
      if((*it)->opera == EXIST)
        res = exist(*it,s,c);
    }
    else
    {
      std::cout<<"333"<<std::endl;
      std::string *smalls;
      if((*it)->opera == SET)
        smalls = set(*it,s,c);
      if((*it)->opera == GET)
        smalls = get((*it),s,c);
      if((*it)->opera == DEL)
        smalls = del((*it),s,c);
      if((*it)->opera == EXIST)
        smalls = exist((*it),s,c);
      //std::cout<<(*it)->opera<<std::endl;
      *res += *smalls;
      //std::cout<<"234"<<std::endl;
      //delete smalls;
    }
    //delete *it;
  }
  std::cout<<*res<<std::endl;
  c->command.clear();
  return res;
}
