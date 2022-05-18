#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include "tcpSocket.h"

namespace geely {
namespace net {
    TcpSocket::TcpSocket() {
        auto fd = ::socket(AF_INET, SOCK_STREAM, 0);
        SetNativeSocket(fd);
    }
    bool TcpSocket::Connect(const INetHost &host) {
        sockaddr_in addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(host.Port());
        addr.sin_addr.s_addr = ::inet_addr(host.Ip().data());
        auto ret = ::connect(GetNativeSocket(), (sockaddr *)&addr, sizeof(addr));
        if (!ret) {
            SetHost(host);
            m_isConnected = true;
        }

        return !ret;
    }
    TcpSocket::~TcpSocket() {
        Close();
    }
    int32_t TcpSocket::Write(const std::string &data) {
        if (!m_isConnected) {
            return -1;
        }
        return Write(GetNativeSocket(), data.data(), data.size());
    }
    int32_t TcpSocket::Read(std::string &data) {
        if (!m_isConnected) {
            return -1;
        }

        auto bytes = Read(GetNativeSocket(), data.data(), data.size());
        if (0 >= bytes) {
            return bytes;
        }
        data.resize(bytes);
        data.shrink_to_fit();

        return bytes;
    }
    int32_t TcpSocket::Write(const int32_t fd, const char *buffer, const uint32_t bufferLen) {
        return ::write(fd, buffer, bufferLen);
    }
    int32_t TcpSocket::Read(const int32_t fd, char *buffer, const int32_t bufferLen) {
        return ::read(fd, buffer, bufferLen);
    }
} // namespace net
} // namespace geely