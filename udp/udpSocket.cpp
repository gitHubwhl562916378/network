#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#endif // _WIN32
#include <fcntl.h>
#include <string.h>
#include "udpSocket.h"

namespace geely
{
    namespace net
    {
        UdpSocket::UdpSocket(const INetHost &host)
        {
            auto sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            // int optval = 1;
            // ::setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
            SetNativeSocket(sockfd);
            SetHost(host);
        }
        UdpSocket::~UdpSocket()
        {
            Close();
        }
        int32_t UdpSocket::Write(const std::string &data)
        {
            return Write(GetNativeSocket(), data.data(), static_cast<uint32_t>(data.size()), GetINetHost());
        }
        int32_t UdpSocket::Read(std::string &data)
        {
            INetHost remote;
            auto bytes = Read(GetNativeSocket(), const_cast<char *>(data.data()), static_cast<uint32_t>(data.size()), remote);
            if (0 >= bytes)
            {
                return bytes;
            }
            data.resize(bytes);
            data.shrink_to_fit();

            return bytes;
        }
        int32_t UdpSocket::Write(const int32_t fd, const char *buffer, const uint32_t bufferLen, const INetHost &host)
        {
            sockaddr_in target_info;
            ::memset(&target_info, 0, sizeof(target_info));
            target_info.sin_family = AF_INET;
            target_info.sin_port = ::htons(host.Port());
            target_info.sin_addr.s_addr = ::inet_addr(host.Ip().data());
            return static_cast<int32_t>(::sendto(fd, buffer, bufferLen, 0, (sockaddr *)&target_info, sizeof(target_info)));
        }

        int32_t UdpSocket::Read(const int32_t fd, char *buffer, const uint32_t bufferLen, INetHost &host)
        {
            sockaddr_in remote_info;
            socklen_t len = sizeof(remote_info);
            ::memset(&remote_info, 0, len);
            auto bytes = ::recvfrom(fd, buffer, bufferLen, 0, (sockaddr *)&remote_info, &len);
            host.SetIp(::inet_ntoa(remote_info.sin_addr));
            host.SetPort1(::ntohs(remote_info.sin_port));
            return static_cast<int32_t>(bytes);
        }
    } // namespace net
} // namespace geely