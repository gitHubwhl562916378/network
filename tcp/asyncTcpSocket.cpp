#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include "tcpSocket.h"
#include "asyncTcpSocket.h"

namespace geely {
namespace net {
    AsyncTcpSocket::AsyncTcpSocket(std::shared_ptr<IoLoop> loop)
        : m_loop(loop) {
        auto fd = ::socket(AF_INET, SOCK_STREAM, 0);
        SetNativeSocket(fd);
    }

    AsyncTcpSocket::AsyncTcpSocket(std::shared_ptr<IoLoop> loop, const int32_t fd, const INetHost &host)
        : AsyncSocket(fd)
        , m_loop(loop) {
        int opts = ::fcntl(fd, F_GETFL);
        opts |= O_NONBLOCK;
        ::fcntl(fd, F_SETFL, opts);
        linger m;
        m.l_onoff = 1;  // close调用时，还有数据没发送完毕，允许逗留
        m.l_linger = 2; // time_wait时间,秒计算
        setsockopt(GetNativeSocket(), SOL_SOCKET, SO_LINGER, (const char *)&m, sizeof(m));
        SetHost(host);
        m_isConected = true;
    }

    AsyncTcpSocket::~AsyncTcpSocket() {
        Close();
    }

    bool AsyncTcpSocket::Connect(const INetHost &host) {
        sockaddr_in addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(host.Port());
        addr.sin_addr.s_addr = ::inet_addr(host.Ip().data());
        auto ret = ::connect(GetNativeSocket(), (sockaddr *)&addr, sizeof(addr));
        if (!ret) {
            int opts = ::fcntl(GetNativeSocket(), F_GETFL);
            opts |= O_NONBLOCK;
            ::fcntl(GetNativeSocket(), F_SETFL, opts);
            linger m;
            m.l_onoff = 1;  // close调用时，还有数据没发送完毕，允许逗留
            m.l_linger = 2; // time_wait时间,秒计算
            setsockopt(GetNativeSocket(), SOL_SOCKET, SO_LINGER, (const char *)&m, sizeof(m));
            SetHost(host);
            m_isConected = true;
        }

        return !ret;
    }

    void AsyncTcpSocket::Write(const std::string &data) {
        if (!m_isConected) {
            return;
        }

        if (m_loop.lock()->IsInLoopTread()) {
            size_t remaining = data.size();
            bool faultError = false;
            int32_t nwrite = TcpSocket::Write(GetNativeSocket(), data.data(), data.size());
            if (nwrite >= 0) {
                remaining = data.size() - nwrite;
                if (0 == remaining) {
                    return;
                }
            } else // nwrote < 0
            {
                nwrite = 0;
                if (errno != EWOULDBLOCK) {
                    if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                    {
                        faultError = true;
                    }
                }
            }

            if (!faultError && remaining > 0) {
                {
                    std::lock_guard<std::mutex> lock(m_dataMtx);
                    m_writeCache += std::string(data.data() + nwrite, remaining);
                }
                m_loop.lock()->Update(EPOLLIN | EPOLLOUT | EPOLLET, shared_from_this());
            }
        } else {
            {
                std::lock_guard<std::mutex> lock(m_dataMtx);
                m_writeCache += std::move(data);
            }
            m_loop.lock()->Update(EPOLLIN | EPOLLOUT | EPOLLET, shared_from_this());
        }
    }

    void AsyncTcpSocket::Read(READ_EVENT_CB cb) {
        m_read_cb = cb;
    }

    // Write并不是很耗时，数据量过大时，加锁拷贝的处理后，再加锁更新缓存;不如加锁一次,锁全域不拷贝直接操作缓存。
    int32_t AsyncTcpSocket::HandleWrite() {
        std::string writeTemp;
        int32_t nwrite;
        {
            do {
                std::unique_lock<std::mutex> lock(m_dataMtx, std::try_to_lock);
                if (!lock.owns_lock()) {
                    continue;
                }

                bool faultError = false;
                nwrite =
                    TcpSocket::Write(GetNativeSocket(), m_writeCache.data() + m_writedCacheLen, m_writeCache.size() - m_writedCacheLen);
                if (nwrite >= 0) {
                    m_writedCacheLen += nwrite;
                    if (m_writedCacheLen == m_writeCache.size()) {
                        m_writedCacheLen = 0;
                        m_writeCache.clear();
                        m_loop.lock()->Update(EPOLLIN | EPOLLET, shared_from_this());
                    }
                } else // nwrote < 0
                {
                    nwrite = 0;
                    if (errno != EWOULDBLOCK) {
                        if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                        {
                            faultError = true;
                        }
                    }
                }

                if (!faultError && m_writedCacheLen != m_writeCache.size()) {
                    m_loop.lock()->Update(EPOLLIN | EPOLLOUT | EPOLLET, shared_from_this());
                }
                break;
            } while (true);
        }

        return nwrite;
    }

    int32_t AsyncTcpSocket::HandleRead() {
        int32_t nread = 0;
        int32_t readSize = 0;
        while (true) {
            char buffer[4096]{0};
            nread = TcpSocket::Read(GetNativeSocket(), buffer, sizeof(buffer) / sizeof(char));
            if (0 >= nread) {
                break;
            }
            readSize += nread;
            if (0 < nread && m_read_cb) {
                m_read_cb(shared_from_this(), std::string(buffer, buffer + nread));
            }
        }

        return nread;
    }
} // namespace net
} // namespace geely