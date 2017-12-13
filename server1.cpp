#include"type.h"
#include"serverwork.h"
#include"afo.h"

int main()
{
  serverstruct *s = buildserver();
  //std::cout<<"111"<<std::endl;
  if(s == nullptr)
  {
    std::cerr<<"buildserver "<<std::endl;
    return 0;
  }
  build_afo(s);
  for(;;)
  {
    running(s);
  }
  close(s->listener);
  return 0;
}
