#ifndef OWDEVICE_H
#define OWDEVICE_H

#include<string>

class owDevice
{
    public:
        owDevice();
        owDevice(const std::string& displayName, const std::string& deviceName, int round);
        owDevice(const std::string& displayName, const std::string& deviceName, int round, const std::string& current);
        ~owDevice();

        std::string GetDisplayName();
        int GetRound();
        std::string GetValue();
        void SetValue(const std::string& current);

    private:
        std::string m_DisplayName;
        std::string m_DeviceName;
        int m_Round;
        std::string m_Current;
};

#endif // OWDEVICE_H
