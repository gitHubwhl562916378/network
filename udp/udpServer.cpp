#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <iterator>
#include <iostream>
#include <sstream>
#include "../ioLoop.h"
#include "udpServer.h"

namespace geely {
namespace net {
    UdpServer::UdpServer(std::shared_ptr<IoLoop> loop)
        : AsyncUdpSocket(loop, INetHost()) {
        int32_t option = 1;
        ::setsockopt(GetNativeSocket(), SOL_SOCKET, SO_REUSEADDR, &option,
                     sizeof(option));
    }

    UdpServer::~UdpServer() {}

    bool UdpServer::Bind(const INetHost &host) {
        sockaddr_in ser_addr;
        ::memset(&ser_addr, 0, sizeof(ser_addr));
        ser_addr.sin_family = AF_INET;
        ser_addr.sin_port = ::htons(host.Port());
        ser_addr.sin_addr.s_addr = ::inet_addr(host.Ip().data());
        Read(std::bind(&UdpServer::OnClientMessage, this, std::placeholders::_1,
                       std::placeholders::_2));
        SetHost(host);
        m_localHost = host;

        return !::bind(GetNativeSocket(), (sockaddr *)&ser_addr, sizeof(ser_addr));
    }

    INetHost UdpServer::GetLocalHost() const {
        return m_localHost;
    }

    void UdpServer::OnClientMessage(std::shared_ptr<AsyncSocket> session,
                                    const std::string &data) {
        auto remote = GetINetHost();
        std::cout << "receive client message Ip: " << remote.Ip()
                  << " Port: " << remote.Port() << " Bytes: " << data.size() << std::endl;
        std::cout << data << std::endl;

        session->Write("I Recevied " + std::to_string(data.size()) + " bytes");
    }
} // namespace net
} // namespace geely