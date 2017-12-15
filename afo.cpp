#include"afo.h"
#include"serverwork.h"

void build_afo(serverstruct *s)
{
  std::ifstream fin("afo.txt");
  if(fin.is_open())
  {
    int len = 0;
    char command[10000];
    char ch;
    clientstruct *c = new clientstruct;
    for(;!fin.eof();)
    {
      fin.get(ch);
      if(ch == '@' && command[len-1] == '\n')
      {
        //command[len] = '\0';
        //std::cout<<command<<std::endl;
        handle(command,s,len,c);
        len = 0;
        //std::cout<<"dd"<<std::endl;
        continue;
      }
      command[len] = ch;
      len++;
    }
  }
  fin.close();
}
