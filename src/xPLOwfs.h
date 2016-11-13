#ifndef XPLOWFS_H
#define XPLOWFS_H

#include<map>
#include "owDevice.h"
#include "Service/Service.h"
#include "owfscpp/owfscpp.h"
#include "xPLLib/xPLDevCfg.h"
#include "xPLLib/Schemas/SchemaSensor.h"
#include "xPLLib/Extensions/Sensors.h"
#include "xPLLib/Extensions/AdvanceConfig.h"

class xPLOwfs : public Service::IService, public xPL::AdvanceConfig::ICallBack, public xPL::BasicConfig::IConfigure
{
    public:
        xPLOwfs();
        ~xPLOwfs();

        void Configure();
        void AdvanceConfigure();
        void ConfigChange(const std::string& device);
        void ConfigDelete(const std::string& device);
        void ConfigsDelete();

        void Refresh();
        void RefreshValues();

		int ServiceStart(int argc, char* argv[]);
		void ServicePause(bool bPause);
		void ServiceStop();

        void SetSockets(ISimpleSockUDP* senderSock, ISimpleSockUDP* receiverSock);

    private:
        void CompleteConfig();
        bool OwDeviceExist(const std::string& device);
        void OwDeviceAdd(const std::string& displayName, const std::string& configName, xPL::SchemaSensorTypeUtility::SensorType, int round, const std::string& unit="");
        void OwDeviceAdd(const std::string& displayName);
        std::string OwGetValue(const std::string& configName, int round);

        SimpleLog* m_Log;
        xPL::xPLDevCfg m_xPLDevice;
        xPL::AdvanceConfig m_AdvanceConfig;
        xPL::Sensors m_Sensors;
        std::map<std::string, owDevice> m_OwDevices;
        int m_RefreshDevicesInterval;
        int m_RefreshValuesInterval;
        bool m_bConfigured;
        bool m_bServicePause;
        bool m_bServiceStop;
        owfscpp m_owfsClient;
};

#endif // XPLOWFS_H
