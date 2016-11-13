/*** LICENCE ***************************************************************************************/
/*
  SimpleSock - Simple class to manage socket communication TCP or UDP

  This file is part of SimpleSock.

    SimpleSock is free software : you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SimpleSock is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SimpleSock.  If not, see <http://www.gnu.org/licenses/>.
*/
/***************************************************************************************************/
#include "SimpleSock/SimpleSockUDP.h"
#include "Plateforms/Plateforms.h"
#include "SockUdpMock.h"
using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/***************************************************************************************************/
/***                                                                                             ***/
/*** Class SockUdpMock                                                                           ***/
/***                                                                                             ***/
/***************************************************************************************************/
string SockUdpMock::g_MockRecv = "";
queue<string> SockUdpMock::g_MockSend;
mutex SockUdpMock::g_MockRecvMutex;
mutex SockUdpMock::g_MockSendMutex;

SockUdpMock::SockUdpMock()
{
    m_sockAddress.sin_family = AF_INET;
    m_sockAddress.sin_port=0;
    m_sockAddress.sin_addr.s_addr=0;
    m_isOpen = false;
}

SockUdpMock::~SockUdpMock()
{
    Close();
}

unsigned long SockUdpMock::BroadcastAddress(const std::string& interfaceName)
{
    return inet_addr("127.255.255.255");
}

std::string SockUdpMock::LocalAddress(const std::string& interfaceName)
{
    return "127.0.0.1";
}

void SockUdpMock::SetNetworkInterface(const std::string& networkInterface)
{
}

void SockUdpMock::SetSocket(SOCKET sockHandle, struct sockaddr_in sockAddress, int type)
{
    m_sockAddress = sockAddress;
    m_isOpen = true;
}

void SockUdpMock::Open(int port)
{
    Open(port, INADDR_BROADCAST);
}

void SockUdpMock::Open(int port, const std::string& ipAddress)
{
    Open(port, inet_addr(ipAddress.c_str()));
}

void SockUdpMock::Open(int port, unsigned long ipAddress)
{
    memset(&m_sockAddress, 0, sizeof(m_sockAddress));
	m_sockAddress.sin_family = AF_INET;
	m_sockAddress.sin_port=htons(port);
    m_sockAddress.sin_addr.s_addr=ipAddress;
    m_isOpen = true;
}

bool SockUdpMock::isOpen()
{
    return m_isOpen;
}

void SockUdpMock::Close()
{
    m_isOpen = false;
}

void SockUdpMock::Listen(int port)
{
    Open(port);
}

void SockUdpMock::Listen(int port, const std::string& ipAddress)
{
    Open(port, ipAddress);
}

void SockUdpMock::Listen(int port, unsigned long ipAddress)
{
    Open(port, ipAddress);
}

bool SockUdpMock::WaitRecv(int delay)
{
    if(delay<100)
    {
        if(SomethingToRecv()) return true;
        Plateforms::delay(delay);
    }
    else
    {
        for(int i=0; i<10; i++)
        {
            if(SomethingToRecv()) return true;
            Plateforms::delay(delay/10);
        }
    }

    if(SomethingToRecv()) return true;
    return false;
}

void SockUdpMock::Send(const std::string& buffer)
{
    SetSend(buffer);
}

void SockUdpMock::Send(const char* buffer, size_t bufferSize)
{
    SetSend(buffer);
}

bool SockUdpMock::Recv(std::string& buffer, int sizeMax)
{
    buffer = GetRecv();
    if(buffer=="") return false;
	return true;
}

unsigned SockUdpMock::Recv(char* buffer, size_t bufferSize)
{
	int status;
	string tmp;


    if(bufferSize==0) throw SimpleSock::Exception(0x0054, "SimpleSock::Recv: Invalid buffer size");
    if(buffer==nullptr) throw SimpleSock::Exception(0x0053, "SimpleSock::Recv: Invalid buffer pointer");
    if(!m_isOpen) throw SimpleSock::Exception(0x0052, "SimpleSock::Recv: Listen() or Connect() method is mandatory before Recv()");

    tmp = GetRecv();
    strncpy(buffer, tmp.c_str(), bufferSize);
    status = tmp.size();

	return status;
}

void SockUdpMock::Blocking(bool blocking)
{
}

int SockUdpMock::GetPort()
{
    return ntohs(m_sockAddress.sin_port);
}

std::string SockUdpMock::GetAddress()
{
    return inet_ntoa(m_sockAddress.sin_addr);
}

string SockUdpMock::GetLastSend(int delay)
{
    string tmp;
    int idelay;

    for(idelay=0; idelay<=delay; idelay++)
    {
        SockUdpMock::g_MockSendMutex.lock();
        if(!SockUdpMock::g_MockSend.empty())
        {
            tmp = SockUdpMock::g_MockSend.front();
            SockUdpMock::g_MockSend.pop();
        }
        SockUdpMock::g_MockSendMutex.unlock();
        if(tmp!="") break;
        Plateforms::delay(100);
    }

    return tmp;
}

void SockUdpMock::SetNextRecv(string value)
{

    SockUdpMock::g_MockRecvMutex.lock();
    if(SockUdpMock::g_MockRecv!="")
    {
        std::stringstream ss;
        ss << SockUdpMock::g_MockRecv << endl << value;
        SockUdpMock::g_MockRecv = ss.str();
    }
    else
        SockUdpMock::g_MockRecv += value;
    SockUdpMock::g_MockRecvMutex.unlock();
}

void SockUdpMock::SetSend(string value)
{
    SockUdpMock::g_MockSendMutex.lock();
    SockUdpMock::g_MockSend.push(value);
    SockUdpMock::g_MockSendMutex.unlock();
}

bool SockUdpMock::SomethingToRecv()
{
    bool tmp = false;

    SockUdpMock::g_MockRecvMutex.lock();
    if(SockUdpMock::g_MockRecv!="") tmp = true;
    SockUdpMock::g_MockRecvMutex.unlock();
    return tmp;
}

string SockUdpMock::GetRecv()
{
    string tmp;

    SockUdpMock::g_MockRecvMutex.lock();
    tmp = SockUdpMock::g_MockRecv;
    SockUdpMock::g_MockRecv = "";
    SockUdpMock::g_MockRecvMutex.unlock();
    return tmp;
}

#pragma GCC diagnostic pop
