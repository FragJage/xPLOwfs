#include "TestxPLOwfs.h"

using namespace std;

TestxPLOwfs::TestxPLOwfs() : TestClass("xPLOwfs", this)
{
	addTest("Start", &TestxPLOwfs::Start);
	addTest("StdConfig", &TestxPLOwfs::StdConfig);
	addTest("SetAdvConfig", &TestxPLOwfs::SetAdvConfig);
	addTest("GetAdvConfig", &TestxPLOwfs::GetAdvConfig);
	addTest("ModifyAdvConfig", &TestxPLOwfs::ModifyAdvConfig);
	addTest("Stop", &TestxPLOwfs::Stop);
	addTest("ReStart", &TestxPLOwfs::ReStart);
	addTest("DelAdvConfig", &TestxPLOwfs::DelAdvConfig);
	addTest("ReStop", &TestxPLOwfs::ReStop);

	xPLDev.SetSockets(&UdpSendMock, &UdpReceiveMock);

    if(remove("config")==0)
        cout << termcolor::yellow << "Remove old config file" << endl << termcolor::grey;
}

TestxPLOwfs::~TestxPLOwfs()
{
    if(remove("config")!=0)
        cout << termcolor::red << "Unable to remove config file" << endl << termcolor::grey;
}

void TestxPLOwfs::ThreadStart(xPLOwfs* pxPLDev)
{
    char exeName[] = "test";
    char confName[] = "config";
    char* argv[2];

    argv[0]= exeName;
    argv[1]= confName;
    pxPLDev->ServiceStart(2, argv);
}

bool TestxPLOwfs::Start()
{
    string msg;
    xPL::SchemaObject sch;


    thread integrationTest(ThreadStart, &xPLDev);
    integrationTest.detach();

    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("xPL Owfs"==sch.GetValue("appname"));

    return true;
}

bool TestxPLOwfs::StdConfig()
{
    int nbTrig=0;
    int nbOther=0;
    string msg;
    string type;
    xPL::SchemaObject sch;
    xPL::SchemaObject schCfg(xPL::SchemaObject::cmnd, "config", "response");

    schCfg.SetValue("interval", "30");
    schCfg.SetValue("newconf", "test");
    schCfg.SetValue("owserver", "127.0.0.1");
    schCfg.SetValue("owport", "4304");

    msg = schCfg.ToMessage("fragxpl-test.default", "fragxpl-owfs.default");
    SockUdpMock::SetNextRecv(msg);

    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("hbeat" == sch.GetClass());
    assert("end" == sch.GetType());
    assert("fragxpl-owfs.default" == sch.GetSource());

    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("hbeat" == sch.GetClass());
    assert("app" == sch.GetType());
    assert("fragxpl-owfs.test" == sch.GetSource());

    while(true)
    {
        msg = SockUdpMock::GetLastSend(10);
        if(msg=="") break;
        sch.Parse(msg);
        if(sch.GetMsgTypeStr()=="xpl-trig")
        {
            nbTrig++;
            OwDevices.push_back(sch.GetValue("owfsid"));
        }
        else
        {
            nbOther++;
        }
    }

    assert(3==nbTrig);
    assert(0==nbOther);

    return true;
}

bool TestxPLOwfs::SetAdvConfig()
{
    string msg;
    xPL::SchemaObject sch;
    xPL::SchemaObject schAdvCfg(xPL::ISchema::cmnd, "advanceconfig", "request");


    schAdvCfg.SetValue("configname", OwDevices[0]);
    schAdvCfg.SetValue("displayname", "temp1");
    schAdvCfg.SetValue("sensortype", "temp");
    schAdvCfg.SetValue("nbdecimal", "2");
    schAdvCfg.SetValue("unit", "°");
    msg = schAdvCfg.ToMessage("fragxpl-test.default", "fragxpl-owfs.test");
    SockUdpMock::SetNextRecv(msg);
    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("temp1"==sch.GetValue("device"));

    schAdvCfg.SetValue("configname", OwDevices[2]);
    schAdvCfg.SetValue("displayname", "output1");
    schAdvCfg.SetValue("sensortype", "output");
    schAdvCfg.SetValue("nbdecimal", "0");
    schAdvCfg.SetValue("unit", "");
    msg = schAdvCfg.ToMessage("fragxpl-test.default", "fragxpl-owfs.test");
    SockUdpMock::SetNextRecv(msg);
    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("output1"==sch.GetValue("device"));

    return true;
}

bool TestxPLOwfs::GetAdvConfig()
{
    string msg;
    xPL::SchemaObject sch;
    xPL::SchemaObject schAdvCfg(xPL::ISchema::cmnd, "advanceconfig", "current");

    schAdvCfg.SetValue("command", "request");
    schAdvCfg.SetValue("configname", OwDevices[0]);
    msg = schAdvCfg.ToMessage("fragxpl-test.default", "fragxpl-owfs.test");
    SockUdpMock::SetNextRecv(msg);

    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("advanceconfig"==sch.GetClass());
    assert("current"==sch.GetType());
    assert(OwDevices[0]==sch.GetValue("configname"));
    assert("temp1"==sch.GetValue("displayname"));
    assert("temp"==sch.GetValue("sensortype"));
    assert("2"==sch.GetValue("nbdecimal"));
    assert("°"==sch.GetValue("unit"));

    return true;
}

bool TestxPLOwfs::ModifyAdvConfig()
{
    string msg;
    xPL::SchemaObject sch;
    xPL::SchemaObject schAdvCfgReq(xPL::ISchema::cmnd, "advanceconfig", "request");
    xPL::SchemaObject schAdvCfgCur(xPL::ISchema::cmnd, "advanceconfig", "current");

    schAdvCfgReq.SetValue("configname", OwDevices[2]);
    schAdvCfgReq.SetValue("displayname", "myoutput");
    schAdvCfgReq.SetValue("sensortype", "output");
    schAdvCfgReq.SetValue("nbdecimal", "0");
    schAdvCfgReq.SetValue("unit", "X");
    msg = schAdvCfgReq.ToMessage("fragxpl-test.default", "fragxpl-owfs.test");
    SockUdpMock::SetNextRecv(msg);

    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("myoutput"==sch.GetValue("device"));

    schAdvCfgCur.SetValue("command", "request");
    schAdvCfgCur.SetValue("configname", OwDevices[2]);
    msg = schAdvCfgCur.ToMessage("fragxpl-test.default", "fragxpl-owfs.test");
    SockUdpMock::SetNextRecv(msg);

    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("advanceconfig"==sch.GetClass());
    assert("current"==sch.GetType());
    assert(OwDevices[2]==sch.GetValue("configname"));
    assert("myoutput"==sch.GetValue("displayname"));
    assert("output"==sch.GetValue("sensortype"));
    assert("0"==sch.GetValue("nbdecimal"));
    assert("X"==sch.GetValue("unit"));

    return true;
}

bool TestxPLOwfs::Stop()
{
    string msg;
    xPL::SchemaObject sch;

    xPLDev.ServicePause(true);
    Plateforms::delay(800);
    xPLDev.ServicePause(false);
    xPLDev.ServiceStop();

    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("hbeat"==sch.GetClass());
    assert("end"==sch.GetType());
    Plateforms::delay(200);
    return true;
}

bool TestxPLOwfs::ReStart()
{
    string msg;

    thread integrationTest(ThreadStart, &xPLDev);
    integrationTest.detach();

    bool ok = true;             //Pass all messages
    while(ok)
    {
        msg = SockUdpMock::GetLastSend(10);
        if(msg=="") ok=false;
    }
    return true;
}

bool TestxPLOwfs::DelAdvConfig()
{
    string msg;
    xPL::SchemaObject sch;
    xPL::SchemaObject schAdvCfg(xPL::ISchema::cmnd, "advanceconfig", "current");

    schAdvCfg.SetValue("command", "delete");
    schAdvCfg.SetValue("configname", OwDevices[2]);
    msg = schAdvCfg.ToMessage("fragxpl-test.default", "fragxpl-owfs.test");
    SockUdpMock::SetNextRecv(msg);
    Plateforms::delay(500);

    schAdvCfg.ClearValues();
    schAdvCfg.SetValue("command", "delete");
    msg = schAdvCfg.ToMessage("fragxpl-test.default", "fragxpl-owfs.test");
    SockUdpMock::SetNextRecv(msg);
    Plateforms::delay(500);

    return true;
}

bool TestxPLOwfs::ReStop()
{
    string msg;
    xPL::SchemaObject sch;

    xPLDev.ServiceStop();

    msg = SockUdpMock::GetLastSend(10);
    sch.Parse(msg);
    assert("hbeat"==sch.GetClass());
    assert("end"==sch.GetType());
    Plateforms::delay(200);

    return true;
}

