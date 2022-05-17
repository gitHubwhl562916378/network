#pragma once
#include "asyncSocket.h"

namespace geely {
namespace net {
    class IoLoop;
    class NotifyAsyncSocket : public AsyncSocket {
    public:
        explicit NotifyAsyncSocket();
        ~NotifyAsyncSocket();
        void WakeUp();

    private:
        void Write(const std::string &data) override {}
        int32_t HandleWrite() override;
        int32_t HandleRead() override;
    };
} // namespace net
} // namespace geely