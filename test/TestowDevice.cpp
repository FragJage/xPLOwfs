#include "TestowDevice.h"

using namespace std;

TestowDevice::TestowDevice() : TestClass("owDevice", this)
{
	addTest("DisplayName", &TestowDevice::TestDisplayName);
	addTest("Round", &TestowDevice::TestRound);
	addTest("Value", &TestowDevice::TestValue);
}

TestowDevice::~TestowDevice()
{
}

bool TestowDevice::TestDisplayName()
{
    owDevice myOw("MyDisplayName", "", 0);
    assert("MyDisplayName"==myOw.GetDisplayName());

    return true;
}

bool TestowDevice::TestRound()
{
    return true;
}

bool TestowDevice::TestValue()
{
    return true;
}
