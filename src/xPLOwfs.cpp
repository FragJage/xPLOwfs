#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <algorithm>

#include "xPLOwfs.h"
#include "Plateforms/Plateforms.h"
#include "xPLLib/xPLDevice.h"
#include "xPLLib/Schemas/SchemaSensor.h"
#include "xPLLib/Schemas/SchemaControl.h"


using namespace std;

xPLOwfs::xPLOwfs() : m_AdvanceConfig(&m_xPLDevice), m_Sensors(&m_xPLDevice), m_RefreshDevicesInterval(90), m_RefreshValuesInterval(10), m_bConfigured(false)
{
    m_Log = m_xPLDevice.GetLogHandle();

    m_AdvanceConfig.AddFormat("configname", xPL::AdvanceConfig::ParamType::STRING, xPL::AdvanceConfig::ParamList::NONE);
    m_AdvanceConfig.AddFormat("displayname", xPL::AdvanceConfig::ParamType::STRING, xPL::AdvanceConfig::ParamList::NONE);
    m_AdvanceConfig.AddFormat("sensortype", xPL::AdvanceConfig::ParamType::STRING, xPL::AdvanceConfig::ParamList::SENSORTYPE);
    m_AdvanceConfig.AddFormat("nbdecimal", xPL::AdvanceConfig::ParamType::INTEGER, xPL::AdvanceConfig::ParamList::NONE);
    m_AdvanceConfig.AddFormat("unit", xPL::AdvanceConfig::ParamType::STRING, xPL::AdvanceConfig::ParamList::NONE);
    m_AdvanceConfig.SetCallBack(this);

    m_xPLDevice.AddExtension(&m_AdvanceConfig);
    m_xPLDevice.AddExtension(&m_Sensors);

    m_xPLDevice.Initialisation("fragxpl", "owfs", "default");
    m_xPLDevice.SetAppName("xPL Owfs", "1.0.0.0");
    m_bServicePause = false;
    m_bServiceStop = false;

    m_xPLDevice.AddBasicConfig("owserver", xPL::ConfigItem::Reconf, "127.0.0.1");
    m_xPLDevice.AddBasicConfig("owport", xPL::ConfigItem::Option, "4304");
    m_xPLDevice.AddBasicConfig("devicesinterval", xPL::ConfigItem::Option, "90");
    m_xPLDevice.AddBasicConfig("valuesinterval", xPL::ConfigItem::Option, "10");
    m_xPLDevice.AddBasicConfig("temperaturescale", xPL::ConfigItem::Option, "C");
    m_xPLDevice.AddBasicConfig("pressurescale", xPL::ConfigItem::Option, "Mbar");
    m_xPLDevice.AddBasicConfig("uncachedread", xPL::ConfigItem::Option, "0");
    m_xPLDevice.SetCallBackConfig(this);

	//m_xPLDevice.LoadConfig();
}

xPLOwfs::~xPLOwfs()
{
    ConfigsDelete();
}

void xPLOwfs::ConfigsDelete()
{
    m_Sensors.RemoveAllMessages();
    m_OwDevices.clear();
}

void xPLOwfs::ConfigDelete(const std::string& configName)
{
    std::map<std::string, owDevice>::iterator it;

    it = m_OwDevices.find(configName);
    if(it == m_OwDevices.end()) return;
    m_Sensors.RemoveMessage(it->second.GetDisplayName());
    m_OwDevices.erase(it);
}

void xPLOwfs::ConfigChange(const std::string& configName)
{
    std::map<std::string, owDevice>::iterator it;
    map<string, string>* config;
    string displayName;
    string sensorType;
    string unit;
    int round;


    config = m_AdvanceConfig.GetConfig(configName);
    displayName  = (*config)["displayname"];
    sensorType   = (*config)["sensortype"];
    round        = atoi((*config)["nbdecimal"].c_str());
    unit         = (*config)["unit"];

    it = m_OwDevices.find(configName);
    if(it != m_OwDevices.end())
    {
        m_Sensors.RemoveMessage(it->second.GetDisplayName());
        m_OwDevices.erase(it);
    }
    OwDeviceAdd(displayName, configName, xPL::SchemaSensorTypeUtility::toSensorType(sensorType), round, unit);
}

void xPLOwfs::AdvanceConfigure()
{
    int i, nb;
    int round;


    LOG_ENTER;

    ConfigsDelete();

    nb = m_AdvanceConfig.GetNbConfig();
    for(i=0; i<nb; i++)
    {
        std::map<std::string, std::string>* config;
        string configName;
        string displayName;
        string sensorType;
        string unit;


        config = m_AdvanceConfig.GetConfig(i);
        configName   = (*config)["configname"];
        displayName  = (*config)["displayname"];
        sensorType   = (*config)["sensortype"];
        round        = atoi((*config)["nbdecimal"].c_str());
        unit         = (*config)["unit"];

        OwDeviceAdd(displayName, configName, xPL::SchemaSensorTypeUtility::toSensorType(sensorType), round, unit);
    }
    CompleteConfig();
    m_bConfigured = true;

	LOG_EXIT_OK;
}

void xPLOwfs::Configure()
{
    xPL::ConfigItem *pConfigWebApi;
    string host;
    string value;
    int port;
    int tmp;


    LOG_ENTER;

   	pConfigWebApi = m_xPLDevice.GetConfigItem("owserver");
    if(pConfigWebApi!=NULL)
        host = pConfigWebApi->GetValue();
    else
        host = "127.0.0.1";

   	pConfigWebApi = m_xPLDevice.GetConfigItem("owport");
    if(pConfigWebApi!=NULL)
        port = atoi(pConfigWebApi->GetValue().c_str());
    else
        port = 4304;

    LOG_VERBOSE(m_Log) << "Connect to owserver "<<host<<":"<<port;
    m_owfsClient.Initialisation(host, port);

    pConfigWebApi = m_xPLDevice.GetConfigItem("devicesinterval");
    if(pConfigWebApi!=NULL)
    {
        tmp = atoi(pConfigWebApi->GetValue().c_str());
        if(tmp>0)
        {
            m_RefreshDevicesInterval = tmp;
            LOG_VERBOSE(m_Log) << "Set RefreshDevicesInterval to "<< m_RefreshDevicesInterval;
        }
    }

    pConfigWebApi = m_xPLDevice.GetConfigItem("valuesinterval");
    if(pConfigWebApi!=NULL)
    {
        tmp = atoi(pConfigWebApi->GetValue().c_str());
        if(tmp>0)
        {
            m_RefreshValuesInterval = tmp;
            LOG_VERBOSE(m_Log) << "Set RefreshValuesInterval to "<< m_RefreshValuesInterval;
        }
    }

    pConfigWebApi = m_xPLDevice.GetConfigItem("temperaturescale");
    if(pConfigWebApi!=NULL)
    {
        switch(pConfigWebApi->GetValue()[0])
        {
            case 'C' :
                m_owfsClient.SetTemperatureScale(owfscpp::Centigrade);
                LOG_VERBOSE(m_Log) << "Set SetTemperatureScale to Centigrade";
                break;
            case 'F' :
                m_owfsClient.SetTemperatureScale(owfscpp::Fahrenheit);
                LOG_VERBOSE(m_Log) << "Set SetTemperatureScale to Fahrenheit";
                break;
            case 'K' :
                m_owfsClient.SetTemperatureScale(owfscpp::Kelvin);
                LOG_VERBOSE(m_Log) << "Set SetTemperatureScale to Kelvin";
                break;
            case 'R' :
                m_owfsClient.SetTemperatureScale(owfscpp::Rankine);
                LOG_VERBOSE(m_Log) << "Set SetTemperatureScale to Rankine";
                break;
        }
    }

    pConfigWebApi = m_xPLDevice.GetConfigItem("pressurescale");
    if(pConfigWebApi!=NULL)
    {
        value = pConfigWebApi->GetValue();
        std::transform(value.begin(), value.end(),value.begin(), ::toupper);
        if(value=="MBAR")
        {
            m_owfsClient.SetPressureScale(owfscpp::Mbar);
            LOG_VERBOSE(m_Log) << "Set SetPressureScale to Mbar";
        }
        else if(value=="ATM")
        {
            m_owfsClient.SetPressureScale(owfscpp::Atm);
            LOG_VERBOSE(m_Log) << "Set SetPressureScale to Atm";
        }
        else if(value=="MMHG")
        {
            m_owfsClient.SetPressureScale(owfscpp::MmHg);
            LOG_VERBOSE(m_Log) << "Set SetPressureScale to MmHg";
        }
        else if(value=="INHG")
        {
            m_owfsClient.SetPressureScale(owfscpp::InHg);
            LOG_VERBOSE(m_Log) << "Set SetPressureScale to InHg";
        }
        else if(value=="PSI")
        {
            m_owfsClient.SetPressureScale(owfscpp::Psi);
            LOG_VERBOSE(m_Log) << "Set SetPressureScale to Psi";
        }
        else if(value=="PA")
        {
            m_owfsClient.SetPressureScale(owfscpp::Pa);
            LOG_VERBOSE(m_Log) << "Set SetPressureScale to Pa";
        }
    }

    pConfigWebApi = m_xPLDevice.GetConfigItem("uncachedread");
    if(pConfigWebApi!=NULL)
    {
        value = pConfigWebApi->GetValue();
        if(value=="1")
        {
            m_owfsClient.SetOwserverFlag(owfscpp::Uncached, true);
            LOG_VERBOSE(m_Log) << "Set Uncached read";
        }
    }

    AdvanceConfigure();

	LOG_EXIT_OK;
}

void xPLOwfs::CompleteConfig()
{
    list<string> lstDir;
    list<string>::iterator iDir;
    string device;


    try
    {
        lstDir = m_owfsClient.DirAll("/");
    }
    catch (const exception& e)
    {
        LOG_ERROR(m_Log) << "Unable to find owdevices : " << e.what();
    }

    for(iDir = lstDir.begin(); iDir != lstDir.end(); ++iDir)
    {
        if(iDir->substr(0,1)=="/")
            device = iDir->substr(1);
        else
            device = iDir->substr(0);

        if(!OwDeviceExist(device))
            OwDeviceAdd(device);
    }
}

bool xPLOwfs::OwDeviceExist(const string& device)
{
    std::map<std::string, owDevice>::iterator it;
    size_t length;


    length = device.length();
    for(it=m_OwDevices.begin(); it!=m_OwDevices.end(); ++it)
    {
        if(it->first.compare(0, length, device)==0)
            return true;
    }

    return false;
}

string xPLOwfs::OwGetValue(const string& configName, int round)
{
    string value;
    double dalue;
    ostringstream s;


    try
    {
        value = m_owfsClient.Get(configName);
    }
    catch (const exception& e)
    {
        LOG_WARNING(m_Log) << "Unable to read " << configName << " : " << e.what();
        return "";
    }

    if(round==-1) return value;

    dalue = atof(value.c_str());

    if(round==-2)
    {
        if(dalue > 0)
            return "high";
        else
            return "low";
    }

    if(round<0) return value;

    s << fixed << setprecision(round) << dalue;

    return s.str();
}

void xPLOwfs::OwDeviceAdd(const string& displayName, const string& configName, xPL::SchemaSensorTypeUtility::SensorType sensorType, int round, const string& unit)
{
    xPL::SchemaSensorBasic* pSensorBasic;
    string value;
    string id;


    if((sensorType == xPL::SchemaSensorTypeUtility::unset)||
       (sensorType == xPL::SchemaSensorTypeUtility::direction)||
       (sensorType == xPL::SchemaSensorTypeUtility::generic)||
       (sensorType == xPL::SchemaSensorTypeUtility::count)) round = -1;
    if((sensorType == xPL::SchemaSensorTypeUtility::input)||
       (sensorType == xPL::SchemaSensorTypeUtility::output)) round = -2;
    value = OwGetValue(configName, round);

    LOG_VERBOSE(m_Log) << "Device create " << displayName <<" : "<< configName <<"("<< xPL::SchemaSensorTypeUtility::ToString(sensorType) <<") = "<<value;
    m_OwDevices[configName] = owDevice(displayName, configName, round, value);
    pSensorBasic = new xPL::SchemaSensorBasic(displayName, sensorType, value, unit);
    pSensorBasic->AddValue("owfsid", configName);
    m_Sensors.AddMessage(pSensorBasic);
}

void xPLOwfs::OwDeviceAdd(const string& name)
{
	string svalue;
	int family;


	try
	{
		svalue = m_owfsClient.Get(name + "/family");
	}
	catch (const exception& e)
	{
		LOG_WARNING(m_Log) << "Unable to read family of device " << name << " : " << e.what();
		return;
	}

	istringstream iss(svalue);
    iss >> hex >> family;

    switch(family)
    {
		case 0x05 : 	//DS2405
		    OwDeviceAdd(name, name+"/PIO", xPL::SchemaSensorTypeUtility::output, 0);
			break;
		case 0x10 :		//DS18S20, DS1920
		    OwDeviceAdd(name, name+"/temperature9", xPL::SchemaSensorTypeUtility::temp, 1);
			break;
		case 0x12 :		//DS2406/07
		    OwDeviceAdd(name, name+"/PIO.A", xPL::SchemaSensorTypeUtility::output, 0);
			break;
		case 0x1D :		//DS2423
		    OwDeviceAdd(name, name+"/counters.A", xPL::SchemaSensorTypeUtility::count, 0);
			break;
		case 0x20 : 	//DS2450
		    OwDeviceAdd(name, name+"/PIO.A", xPL::SchemaSensorTypeUtility::output, 0);
			break;
		case 0x21 :		//DS1921
		    OwDeviceAdd(name, name+"/temperature9", xPL::SchemaSensorTypeUtility::temp, 1);
			break;
		case 0x22 :		//DS1822
		    OwDeviceAdd(name, name+"/temperature9", xPL::SchemaSensorTypeUtility::temp, 1);
			break;
		case 0x26 :		//DS2438
		    OwDeviceAdd(name, name+"/VDD", xPL::SchemaSensorTypeUtility::voltage, 1);
			break;
		case 0x28 : 	//DS18B20
		    OwDeviceAdd(name, name+"/temperature9", xPL::SchemaSensorTypeUtility::temp, 1);
			break;
		case 0x29 : 	//DS2408
		    OwDeviceAdd(name, name+"/PIO.BYTE", xPL::SchemaSensorTypeUtility::generic, 0);
			break;
		case 0x3A : 	//DS2413
		    OwDeviceAdd(name, name+"/PIO.A", xPL::SchemaSensorTypeUtility::output, 0);
			break;
    }
    LOG_VERBOSE(m_Log) << "Device found " << name;
}

void xPLOwfs::RefreshValues()
{
    std::map<std::string, owDevice>::iterator it;
    string value;


    for(it=m_OwDevices.begin(); it!=m_OwDevices.end(); ++it)
    {
        value = OwGetValue(it->first, it->second.GetRound());
        if(value==it->second.GetValue()) continue;
        it->second.SetValue(value);
        m_Sensors.ModifyMessage(it->second.GetDisplayName(), value);
    }
}

void xPLOwfs::Refresh()
{
	 time_t timeNow = time((time_t*)0);
	 static time_t lastRefreshDevices = timeNow;
	 static time_t lastRefreshValues = timeNow;


    if(!m_bConfigured) return;
	if(timeNow-lastRefreshDevices>=m_RefreshDevicesInterval)
    {
        lastRefreshDevices=timeNow;
        LOG_VERBOSE(m_Log) << "Refresh devices";
        CompleteConfig();
    }

	if(timeNow-lastRefreshValues>=m_RefreshValuesInterval)
    {
        lastRefreshValues=timeNow;
        LOG_VERBOSE(m_Log) << "Refresh values";
        RefreshValues();
    }
}

int xPLOwfs::ServiceStart(int argc, char* argv[])
{
    m_bServiceStop = false;
    if((argc > 1)&&(argv[1][0]!='-')) m_xPLDevice.SetConfigFileName(argv[1]);
    m_xPLDevice.LoadConfig();
    m_xPLDevice.Open();

    while(!m_bServiceStop)
    {
        if(m_bServicePause)
        {
            Plateforms::delay(500);
        }
        else
        {
            m_xPLDevice.WaitRecv(500);
            Refresh();
        }
    }

    m_xPLDevice.Close();
    return 0;
}

void xPLOwfs::ServicePause(bool bPause)
{
    m_bServicePause = bPause;
}

void xPLOwfs::ServiceStop()
{
    m_bServiceStop = true;
}

void xPLOwfs::SetSockets(ISimpleSockUDP* senderSock, ISimpleSockUDP* receiverSock)
{
    m_xPLDevice.SetSockets(senderSock, receiverSock);
}
