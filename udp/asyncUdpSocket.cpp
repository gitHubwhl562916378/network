#pragma once
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <iostream>
#include "../ioLoop.h"
#include "udpSocket.h"
#include "asyncUdpSocket.h"

namespace geely {
namespace net {
    AsyncUdpSocket::AsyncUdpSocket(std::shared_ptr<IoLoop> loop, const INetHost &host)
        : m_loop(loop) {
        int sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
        int optval = 1;
        ::setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
        ::fcntl(sockfd, F_SETFL, O_NONBLOCK);
        SetNativeSocket(sockfd);
        SetHost(host);
    }
    AsyncUdpSocket::~AsyncUdpSocket() {
        Close();
    }
    void AsyncUdpSocket::Write(const std::string &data) {
        int32_t nwrite = UdpSocket::Write(GetNativeSocket(), data.data(), data.size(), GetINetHost());
        // UDP发送失败返回-1，成功返回整个报文大小，没有中间值;没送发送缓冲区
        if (-1 == nwrite) {
            std::cout << "EpollLoop::ProcessLoop HandleWrite error, errno: " << errno << ", " << ::strerror(errno) << " write bytes "
                      << nwrite << std::endl;
        }
    }
    void AsyncUdpSocket::Read(READ_EVENT_CB cb) {
        m_readCb = cb;
    }

    int32_t AsyncUdpSocket::HandleRead() {
        char buffer[1024]{0};
        INetHost remote;
        auto bytes = UdpSocket::Read(GetNativeSocket(), buffer, sizeof(buffer) / sizeof(char), remote);
        if (0 >= bytes) {
            return bytes;
        }

        SetHost(remote);
        if (m_readCb) {
            m_readCb(shared_from_this(), std::string(buffer, buffer + bytes));
        }

        return bytes;
    }

    int32_t AsyncUdpSocket::HandleWrite() {
        return 0;
    }
} // namespace net
} // namespace geely