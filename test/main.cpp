#include <exception>
#include "UnitTest/UnitTest.h"
#include "TestowDevice.h"
#include "TestxPLOwfs.h"

using namespace std;

int main()
{
    UnitTest unitTest;
    int ret = 0;


    try
    {
        unitTest.addTestClass(new TestowDevice());
        unitTest.addTestClass(new TestxPLOwfs());
    }
    catch(const exception &e)
    {
        unitTest.displayError(e.what());
        ret = -1;
    }

    if(ret!=-1)
        if(!unitTest.run()) ret = 1;

	return ret;
}
