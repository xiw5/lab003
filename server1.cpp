#include"type.h"
#include"serverwork.h"

int main()
{
  serverstruct *s = buildserver();
  //std::cout<<"111"<<std::endl;
  if(s == nullptr)
  {
    std::cerr<<"buildserver "<<std::endl;
    return 0;
  }
  for(;;)
  {
    running(s);
  }
  return 0;
}
