#include"afo.h"
#include"serverwork.h"

void build_afo(serverstruct *s)
{
  std::ifstream fin("efo.txt");
  if(fin.is_open())
  {
    int len = 0;
    char command[10000];
    char ch;
    clientstruct *c = new clientstruct;
    for(;;)
    {
      fin>>ch;
      if(ch == EOF)
        break;
      if(ch == '\0')
      {
        handle(command,s,len,c);
        len = 0;
        continue;
      }
      command[len] = ch;
      len++;
    }
  }
  fin.close();
  std::ofstream fout("efo.tex",std::ios::trunc);
}
