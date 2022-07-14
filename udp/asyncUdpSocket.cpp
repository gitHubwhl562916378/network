#ifdef _WIN32
#include "../iocp/iocpLoop.h"
#else
#include <sys/socket.h>
#include <sys/epoll.h>
#endif // _WIN32
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include "../ioLoop.h"
#include "udpSocket.h"
#include "asyncUdpSocket.h"

namespace geely
{
    namespace net
    {
        AsyncUdpSocket::AsyncUdpSocket(std::shared_ptr<IoLoop> loop, const INetHost &host)
            : m_loop(loop)
        {
            int optval = 1;
#ifdef _WIN32
            int sockfd = WSASocket(AF_INET, SOCK_DGRAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
            ::setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&optval), sizeof(int));
            unsigned long noblocking = 1;
            ioctlsocket(sockfd, FIONBIO, &noblocking);
            m_ioRData = IO_OPERATION_DATA::NewIoOperationData(OP_TYPE::READ);
            m_ioWData = IO_OPERATION_DATA::NewIoOperationData(OP_TYPE::WRITE);
#else
            int sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
            ::setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
            ::fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif // _WIN32
            SetNativeSocket(sockfd);
            SetHost(host);
        }
        AsyncUdpSocket::~AsyncUdpSocket()
        {
            m_loop->RemoveAsyncSocket(GetNativeSocket());
            Close();
#ifdef _WIN32
            IO_OPERATION_DATA::FreeIoOperaionData(&m_ioRData);
            IO_OPERATION_DATA::FreeIoOperaionData(&m_ioWData);
#endif
        }

        void AsyncUdpSocket::Write(const std::string &data)
        {
#ifdef __linux__
            int32_t nwrite =
                UdpSocket::Write(GetNativeSocket(), data.data(), static_cast<uint32_t>(data.size()), GetINetHost());
            // UDP发送失败返回-1，成功返回整个报文大小，没有中间值;没送发送缓冲区
            if (-1 == nwrite)
            {
                std::cout << "AsyncUdpSocket::Write errno: " << errno
                          << ", " << ::strerror(errno) << ",write bytes " << nwrite
                          << std::endl;
            }
#else
            DWORD bytesSended = 0;
            sockaddr_in target_info;
            ::memset(&target_info, 0, sizeof(target_info));
            target_info.sin_family = AF_INET;
            target_info.sin_port = ::htons(GetINetHost().Port());
            target_info.sin_addr.s_addr = ::inet_addr(GetINetHost().Ip().data());
            m_ioWData->remote_addr = target_info;
            if (0 == IOCP_WSABUFFCOUNT)
            {
                std::cout << "IOCP_WSABUFFCOUNT " << IOCP_WSABUFFCOUNT << " ,to small" << std::endl;
                return;
            }

            m_ioWData->wsaBuffer[0].buf = const_cast<char *>(data.data());
            m_ioWData->wsaBuffer[0].len = data.size();
            auto code = WSASendTo(GetNativeSocket(), m_ioWData->wsaBuffer, IOCP_WSABUFFCOUNT, &bytesSended, 0, (SOCKADDR *)&m_ioWData->remote_addr, sizeof(sockaddr_in), nullptr, nullptr);
            if (0 != code)
            {
                std::cout << "AsyncUdpSocket::Write errno: " << GetLastError() << std::endl;
                return;
            }
            //对于客户端程序，需要通过WSASendTo、sendto、WSAJoinLeaf的调用隐式绑定到本地(形如服务端的bind)，然后才能将WSARecvFrom添加到IOCP中
            //`https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsarecvfrom`
            if (m_isFirstSend)
            {
                HandleRead(nullptr, 0);
                m_isFirstSend = false;
            }
#endif
        }
        void AsyncUdpSocket::Read(READ_EVENT_CB cb)
        {
            m_readCb = cb;
        }
#ifdef _WIN32
        int32_t AsyncUdpSocket::HandleWrite(IO_OPERATION_DATA *ioData, const size_t bytesSend)
        {
            return 0;
        }

        void AsyncUdpSocket::HandleRead(IO_OPERATION_DATA *ioData, const size_t bytesReceived)
        {
            if (nullptr == ioData && 0 == bytesReceived)
            {
                ZeroMemory(&m_ioRData->overlapped, sizeof(OVERLAPPED));
                DWORD flags = 0;
                WSARecvFrom(GetNativeSocket(), m_ioRData->wsaBuffer, IOCP_WSABUFFCOUNT, nullptr, &flags, (sockaddr *)&m_ioRData->remote_addr, &m_ioRData->addr_len, &m_ioRData->overlapped, nullptr);
                return;
            }

            if (nullptr == ioData)
            {
                return;
            }

            std::string payLoad;
            for (int i = 0; i < IOCP_WSABUFFCOUNT; i++)
            {
                if (payLoad.size() < bytesReceived)
                {
                    auto leftB = bytesReceived - payLoad.size();
                    auto cpB = leftB > ioData->wsaBuffer[i].len ? ioData->wsaBuffer[i].len : leftB;
                    payLoad.append(ioData->wsaBuffer[i].buf, cpB);
                }
            }
            INetHost remote;
            remote.SetIp(::inet_ntoa(ioData->remote_addr.sin_addr));
            remote.SetPort1(::ntohs(ioData->remote_addr.sin_port));
            SetHost(remote);

            if (m_readCb)
            {
                m_readCb(shared_from_this(), payLoad);
            }

            ZeroMemory(&ioData->overlapped, sizeof(OVERLAPPED));
            DWORD flags = 0;
            WSARecvFrom(GetNativeSocket(), ioData->wsaBuffer, IOCP_WSABUFFCOUNT, nullptr, &flags, (sockaddr *)&ioData->remote_addr, &ioData->addr_len, &ioData->overlapped, nullptr);
        }
#else
        int32_t AsyncUdpSocket::HandleWrite()
        {
            return 0;
        }
        int32_t AsyncUdpSocket::HandleRead()
        {
            int32_t nread = 0;
            int32_t readSize = 0;
            while (true)
            {
                std::string buffer(1500, 0);
                INetHost remote;
                nread = UdpSocket::Read(GetNativeSocket(), const_cast<char *>(buffer.data()),
                                        static_cast<uint32_t>(buffer.size()), remote);
                if (0 >= nread)
                {
                    break;
                }
                buffer.resize(nread);
                buffer.shrink_to_fit();
                SetHost(remote);
                readSize += nread;
                if (0 < nread && m_readCb)
                {
                    m_readCb(shared_from_this(), buffer);
                }
            }

            return nread;
        }
#endif // _WIN32
    }  // namespace net
} // namespace geely