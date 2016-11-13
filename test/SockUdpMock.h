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

#ifndef SOCKUDPMOCK_H
#define SOCKUDPMOCK_H

//#include <iostream>
#include <queue>
#include <string>
#include <mutex>
#ifdef WIN32
    #include "Thread/mingw.mutex.h"
#else
    #include <thread>
#endif
#include "SimpleSock/SimpleSockUDP.h"

class SockUdpMock : public ISimpleSockUDP
{
    public:
        SockUdpMock();
        ~SockUdpMock();
        unsigned long BroadcastAddress(const std::string& interfaceName);
        std::string LocalAddress(const std::string& interfaceName);
        void SetNetworkInterface(const std::string& networkInterface);
        void SetSocket(SOCKET sockHandle, struct sockaddr_in sockAddress, int type);
        void Open(int port);
        void Open(int port, const std::string& ipAddress);
        void Open(int port, unsigned long ipAddress);
        bool isOpen();
        void Close();
        void Listen(int port);
        void Listen(int port, const std::string& ipAddress);
        void Listen(int port, unsigned long ipAddress);
        bool WaitRecv(int delay);
        void Send(const std::string& buffer);
        void Send(const char* buffer, size_t bufferSize);
        bool Recv(std::string& buffer, int sizeMax=-1);
        unsigned Recv(char* buffer, size_t bufferSize);
        void Blocking(bool blocking);
        int GetPort();
        std::string GetAddress();

        static std::string GetLastSend(int delay);
        static void SetNextRecv(std::string value);

    private:
        bool m_isOpen;
        struct sockaddr_in m_sockAddress;

        static bool SomethingToRecv();
        static std::string GetRecv();
        static void SetSend(std::string buffer);

        static std::string g_MockRecv;
        static std::queue<std::string> g_MockSend;
        static std::mutex g_MockRecvMutex;
        static std::mutex g_MockSendMutex;

};

#endif // SOCKUDPMOCK_H
