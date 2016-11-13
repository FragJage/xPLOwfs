#include "owDevice.h"

using namespace std;

owDevice::owDevice() : m_DisplayName(), m_DeviceName(), m_Round(-1), m_Current()
{
}

owDevice::owDevice(const string& displayName, const string& deviceName, int round) : m_DisplayName(displayName), m_DeviceName(deviceName), m_Round(round), m_Current()
{
}

owDevice::owDevice(const string& displayName, const string& deviceName, int round, const string& current) : m_DisplayName(displayName), m_DeviceName(deviceName), m_Round(round), m_Current(current)
{
}

owDevice::~owDevice()
{
}

string owDevice::GetDisplayName()
{
    return m_DisplayName;
}

int owDevice::GetRound()
{
    return m_Round;
}

string owDevice::GetValue()
{
    return m_Current;
}

void owDevice::SetValue(const string& current)
{
    m_Current = current;
}
