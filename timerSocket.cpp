#include <sys/timerfd.h>
#include "ioLoop.h"
#include "tcp/tcpSocket.h"
#include "timerSocket.h"

namespace geely
{
    TimerSocket::TimerSocket(const std::shared_ptr<net::IoLoop> loop)
        : AsyncSocket(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)), m_loop(loop) {}

    TimerSocket::TimerSocket(const std::shared_ptr<net::IoLoop> loop,
                             const uint32_t firstInterval, const uint32_t intervalAfterFirst)
        : AsyncSocket(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)), m_loop(loop)
    {
        SetFirstInterval(firstInterval);
        SetIntervalAfterFirst(intervalAfterFirst);
    }

    TimerSocket::~TimerSocket()
    {
        m_loop->RemoveAsyncSocket(GetNativeSocket());
        Close();
    }

    void TimerSocket::SetFirstInterval(const uint32_t interval)
    {
        m_firstInterval = interval;
    }

    void TimerSocket::SetIntervalAfterFirst(const uint32_t interval)
    {
        m_intervalAfterFirst = interval;
    }

    bool TimerSocket::Start()
    {
        itimerspec fut;
        fut.it_value.tv_sec = m_firstInterval.load() / 1000;
        fut.it_value.tv_nsec = (m_firstInterval.load() - fut.it_value.tv_sec * 1000) * 1e6;
        fut.it_interval.tv_sec = m_intervalAfterFirst.load() / 1000;
        fut.it_interval.tv_nsec =
            (m_intervalAfterFirst.load() - fut.it_interval.tv_sec * 1000) * 1e6;

        return -1 != ::timerfd_settime(GetNativeSocket(), 0, &fut, nullptr);
    }

    bool TimerSocket::Stop()
    {
        itimerspec fut;
        fut.it_value.tv_sec = 0;
        fut.it_value.tv_nsec = 0;
        fut.it_interval.tv_sec = 0;
        fut.it_interval.tv_nsec = 0;

        return -1 != ::timerfd_settime(GetNativeSocket(), 0, &fut, nullptr);
    }

    void TimerSocket::OnTimeOut(TimeoutFunctor cb)
    {
        m_timeOutHandler = cb;
    }

    int32_t TimerSocket::HandleRead()
    {
        uint64_t exp;
        auto bytes = net::TcpSocket::Read(GetNativeSocket(), reinterpret_cast<char *>(&exp),
                                          sizeof(uint64_t));
        if (8 > bytes)
        {
            return bytes;
        }

        if (m_timeOutHandler)
        {

            m_timeOutHandler(exp);
        }

        return bytes;
    }
} // namespace geely
