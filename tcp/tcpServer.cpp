#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <iostream>
#include <sstream>
#include "tcpServer.h"

namespace geely {
namespace net {
    TcpServer::TcpServer(std::shared_ptr<IoLoop> loop)
        : m_loop(loop) {
        auto fd = ::socket(AF_INET, SOCK_STREAM, 0);
        int32_t option = 1;
        ::setsockopt(GetNativeSocket(), SOL_SOCKET, SO_REUSEADDR, &option,
                     sizeof(option));
        int opts = ::fcntl(fd, F_GETFL);
        opts |= O_NONBLOCK;
        ::fcntl(fd, F_SETFL, opts);
        linger m;
        m.l_onoff = 1;  // close调用时，还有数据没发送完毕，允许逗留
        m.l_linger = 2; // time_wait时间,秒计算
        setsockopt(GetNativeSocket(), SOL_SOCKET, SO_LINGER, (const char *)&m, sizeof(m));
        SetNativeSocket(fd);
    }

    TcpServer::~TcpServer() {
        Close();
    }

    bool TcpServer::Bind(const INetHost &host) {
        sockaddr_in ser_addr;
        ::memset(&ser_addr, 0, sizeof(ser_addr));
        ser_addr.sin_family = AF_INET;
        ser_addr.sin_port = ::htons(host.Port());
        ser_addr.sin_addr.s_addr = ::inet_addr(host.Ip().data());
        SetHost(host);

        return !::bind(GetNativeSocket(), (sockaddr *)&ser_addr, sizeof(ser_addr));
    }

    int32_t TcpServer::Listen(int32_t num) {
        return !::listen(GetNativeSocket(), num);
    }

    std::shared_ptr<AsyncTcpSocket> TcpServer::GetSession(const int32_t connFd) {
        std::shared_ptr<AsyncTcpSocket> ret;
        {
            std::lock_guard<std::mutex> lock(m_sessionMtx);
            auto iter = m_sessions.find(connFd);
            if (iter != m_sessions.end()) {
                ret = iter->second;
            }
        }
        return ret;
    }

    void TcpServer::OnNewConnection(std::shared_ptr<AsyncTcpSocket> client) {
        std::cout << "New connection: " << client->GetINetHost().Ip() << " "
                  << client->GetINetHost().Port() << std::endl;
        client->Read([](std::shared_ptr<AsyncSocket> s, const std::string &data) {
            std::cout << "Receive client " << s->GetINetHost().Ip() << " "
                      << s->GetINetHost().Port() << " size: " << data.size() << " "
                      << data.capacity() << " data: " << data << std::endl;
            s->Write("packed data");
        });
    }

    void TcpServer::Write(const std::string &data) {
        std::map<int32_t, std::shared_ptr<AsyncTcpSocket>> temp;
        {
            std::lock_guard<std::mutex> lock(m_sessionMtx);
            temp = m_sessions;
        }
        for (auto &s : temp) {
            s.second->Write(data);
        }
    }

    int32_t TcpServer::HandleRead() {
        int32_t conn_fd;
        while (true) {
            sockaddr_in remote;
            socklen_t addrlen;
            conn_fd = ::accept(GetNativeSocket(), (sockaddr *)&remote, &addrlen);
            if (0 > conn_fd) {
                break;
            }

            std::shared_ptr<AsyncTcpSocket> session;
            auto m_sloop = m_loop.lock();
            if (nullptr == m_sloop) {
                continue;
            }

            session.reset(new AsyncTcpSocket(
                m_sloop, conn_fd,
                INetHost{::inet_ntoa(remote.sin_addr), ::ntohs(remote.sin_port)}));
            OnNewConnection(session);
            m_sloop->AddAsyncSocket(session);
            {
                std::lock_guard<std::mutex> lock(m_sessionMtx);
                m_sessions.insert({conn_fd, session});
            }
        }

        return conn_fd;
    }
} // namespace net
} // namespace geely