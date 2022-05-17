#include "iNetHost.h"

namespace geely {
namespace net {
    INetHost::INetHost()
        : m_port(0)
        , m_ipAddr("") {}
    INetHost::INetHost(const std::string &ip, const uint16_t port)
        : m_ipAddr(ip)
        , m_port(port) {}
    void INetHost::SetPort(const uint16_t port) {
        m_port = port;
    }
    void INetHost::SetIp(const std::string &ip) {
        m_ipAddr = ip;
    }
    uint16_t INetHost::Port() const {
        return m_port;
    }
    std::string INetHost::Ip() const {
        return m_ipAddr;
    }
} // namespace net
} // namespace geely