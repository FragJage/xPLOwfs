#include <iostream>
#include <exception>
#include "xPLOwfs.h"

int main(int argc, char* argv[])
{
  int res = 0;

  try
  {
    xPLOwfs xPLDev;

  	Service* pService = Service::Create("xPLOwfs", "xPL protocol bridge for Owfs", &xPLDev);
  	res = pService->Start(argc, argv);
  	Service::Destroy();
  }
  catch(const exception &e)
  {
      std::cout << e.what();
  }
	return res;
}

