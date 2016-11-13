#include <cassert>
#include <thread>
#include <vector>
#include "xPLLib/Schemas/SchemaObject.h"
#include "Plateforms/Plateforms.h"
#include "UnitTest/UnitTest.h"
#ifdef WIN32
    #include "Thread/mingw.thread.h"
#else
    #include <thread>
#endif
#include "../src/xPLOwfs.h"
#include "SockUdpMock.h"


class TestxPLOwfs : public TestClass<TestxPLOwfs>
{
public:
    TestxPLOwfs();
    ~TestxPLOwfs();
    static void ThreadStart(xPLOwfs* pxPLDev);
    bool Start();
    bool StdConfig();
    bool SetAdvConfig();
    bool GetAdvConfig();
    bool ModifyAdvConfig();
    bool Stop();
    bool ReStart();
    bool DelAdvConfig();
    bool ReStop();

    xPLOwfs xPLDev;
    SockUdpMock UdpSendMock;
    SockUdpMock UdpReceiveMock;
    std::vector<std::string> OwDevices;
};
