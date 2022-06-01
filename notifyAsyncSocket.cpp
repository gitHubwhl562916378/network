#include <sys/eventfd.h>
#include <iostream>
#include "ioLoop.h"
#include "tcp/tcpSocket.h"
#include "notifyAsyncSocket.h"
namespace geely {
namespace net {

    NotifyAsyncSocket::NotifyAsyncSocket() {
        auto evFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (0 > evFd) {
            std::cout << "eventfd create failed, error " << errno << std::endl;
            ::exit(EXIT_FAILURE);
        }
        SetNativeSocket(evFd);
    }
    NotifyAsyncSocket::~NotifyAsyncSocket() {
        Close();
    }
    void NotifyAsyncSocket::WakeUp() {
        HandleWrite();
    }
    int32_t NotifyAsyncSocket::HandleWrite() {
        uint64_t one = 1;
        int32_t n = TcpSocket::Write(GetNativeSocket(), reinterpret_cast<char *>(&one),
                                     sizeof(one));
        if (sizeof(uint64_t) != n) {
            std::cout << "NotifyAsyncSocket::Write " << n << "  bytes instead of "
                      << sizeof(uint64_t) << std::endl;
        }

        return n;
    }
    int32_t NotifyAsyncSocket::HandleRead() {
        uint64_t one = -1;
        int32_t n = TcpSocket::Read(GetNativeSocket(), reinterpret_cast<char *>(&one),
                                    sizeof(uint64_t));
        if (sizeof(uint64_t) != n) {
            std::cout << "NotifyAsyncSocket::Read " << n << "  bytes instead of "
                      << sizeof(uint64_t) << std::endl;
        }

        return n;
    }
} // namespace net
} // namespace geely