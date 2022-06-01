#include <sys/epoll.h>
#include <signal.h>
#include <string.h>
#include <iostream>
#include "../notifyAsyncSocket.h"
#include "epollLoop.h"

namespace geely {
namespace net {
    EpollLoop::EpollLoop() {}
    EpollLoop::~EpollLoop() {
        std::cout << "EpollLoop::~EpollLoop In" << std::endl;
        m_shutDown = true;
        m_notifySocket->WakeUp();
        if (m_ioThread.joinable()) {
            m_ioThread.join();
        }

        std::cout << "EpollLoop::~EpollLoop Out" << std::endl;
    }

    bool EpollLoop::AddAsyncSocket(std::shared_ptr<AsyncSocket> s) {
        auto task = std::make_shared<std::packaged_task<int32_t()>>([&] {
            auto iter = m_evSockets.find(s->GetNativeSocket());
            if (iter != m_evSockets.end()) {
                return true;
            }

            epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = s->GetNativeSocket();
            auto ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, s->GetNativeSocket(), &ev);
            if (0 != ret) {
                return false;
            }

            return m_evSockets.insert({s->GetNativeSocket(), s}).second;
        });

        if (IsInLoopTread()) {
            (*task)();
        } else {
            RunInLoop(task);
        }

        return task->get_future().get();
    }

    bool EpollLoop::RemoveAsyncSocket(std::shared_ptr<AsyncSocket> s) {
        auto task = std::make_shared<std::packaged_task<int32_t()>>([&] {
            auto iter = m_evSockets.find(s->GetNativeSocket());
            if (iter == m_evSockets.end()) {
                return true;
            }

            epoll_event ev;
            ev.data.fd = s->GetNativeSocket();
            auto ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, s->GetNativeSocket(), &ev);
            if (0 != ret) {
                return false;
            }

            m_evSockets.erase(iter);
            return true;
        });

        if (IsInLoopTread()) {
            (*task)();
        } else {
            RunInLoop(task);
        }

        return task->get_future().get();
    }

    bool EpollLoop::RemoveAsyncSocket(const int32_t fd) {
        auto task = std::make_shared<std::packaged_task<int32_t()>>([&] {
            auto iter = m_evSockets.find(fd);
            if (iter == m_evSockets.end()) {
                return true;
            }

            epoll_event ev;
            ev.data.fd = fd;
            auto ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, &ev);
            if (0 != ret) {
                return false;
            }

            m_evSockets.erase(iter);
            return true;
        });

        if (IsInLoopTread()) {
            (*task)();
        } else {
            RunInLoop(task);
        }

        return task->get_future().get();
    }

    int32_t EpollLoop::Update(const int32_t events, std::shared_ptr<AsyncSocket> s) {
        auto task = std::make_shared<std::packaged_task<int32_t()>>([&] {
            auto iter = m_evSockets.find(s->GetNativeSocket());
            if (iter == m_evSockets.end()) {
                return -1;
            }

            epoll_event ev;
            ev.events = events;
            ev.data.fd = s->GetNativeSocket();
            return ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, s->GetNativeSocket(), &ev);
        });

        if (IsInLoopTread()) {
            (*task)();
        } else {
            RunInLoop(task);
        }

        return task->get_future().get();
    }

    bool EpollLoop::Run() {
        m_threadId = std::this_thread::get_id();
        ::sigset_t set;
        ::sigemptyset(&set);
        ::sigaddset(&set, SIGPIPE);
        ::sigprocmask(SIG_BLOCK, &set, nullptr);
        m_epollFd = ::epoll_create1(0);
        if (-1 == m_epollFd) {
            return false;
        }

        m_notifySocket = std::make_shared<NotifyAsyncSocket>();
        if (!AddAsyncSocket(m_notifySocket)) {
            return false;
        }

        std::promise<void> started;
        auto future = started.get_future();
        m_ioThread =
            std::thread(std::bind(&EpollLoop::ProcessLoop, this, std::ref(started)));
        future.get();

        return true;
    }

    bool EpollLoop::IsInLoopTread() {
        return m_threadId == std::this_thread::get_id();
    }

    void EpollLoop::RunInLoop(Functor cb) {
        {
            std::lock_guard<std::mutex> lock(m_functorMtx);
            m_pendingFunctors.push_back(std::move(cb));
        }
        m_notifySocket->WakeUp();
    }

    void EpollLoop::DoPendingFunctors() {
        std::vector<Functor> functors;
        {
            std::lock_guard<std::mutex> lock(m_functorMtx);
            std::swap(functors, m_pendingFunctors);
        }
        for (auto &func : functors) {
            (*func)();
        }
    }

    void EpollLoop::ProcessLoop(std::promise<void> &promise) {
        promise.set_value();
        m_threadId = std::this_thread::get_id();

        epoll_event events[1024];
        while (!m_shutDown) {
            auto nfds =
                ::epoll_wait(m_epollFd, events, sizeof(events) / sizeof(epoll_event), -1);
            if (-1 == nfds) {
                if (errno == EINTR) {
                    continue;
                } else {
                    std::cout << "FATAL epoll_wait failed errno " << errno << ", "
                              << ::strerror(errno) << std::endl;
                    ::exit(EXIT_FAILURE);
                }
            }

            for (int i = 0; i < nfds; i++) {
                auto fd_iter = m_evSockets.find(events[i].data.fd);
                if (fd_iter == m_evSockets.end()) {
                    continue;
                }

                auto ptr = fd_iter->second.lock();
                if (nullptr == ptr) {
                    continue;
                }
                if (EPOLLIN & events[i].events) {
                    auto nread = ptr->HandleRead();
                    if (0 == nread) {
                        RemoveAsyncSocket(ptr);
                    }

                    if (-1 == nread && errno != EAGAIN && errno != ECONNABORTED &&
                        errno != EPROTO && errno != EINTR) {
                        if (errno == ECONNRESET) {
                            RemoveAsyncSocket(ptr);
                        }
                        std::cout
                            << "EpollLoop::ProcessLoop HandleRead error, errno: " << errno
                            << ", " << ::strerror(errno) << " read bytes " << nread
                            << std::endl;
                    }
                }

                if (EPOLLOUT & events[i].events) {
                    auto nwrite = ptr->HandleWrite();
                    if (-1 == nwrite && errno != EAGAIN && errno != ECONNABORTED &&
                        errno != EPROTO && errno != EINTR) {
                        std::cout << "EpollLoop::ProcessLoop HandleWrite error, errno: "
                                  << errno << ", " << ::strerror(errno) << " write bytes "
                                  << nwrite << std::endl;
                    }
                }
            }

            DoPendingFunctors();
        }
    }
} // namespace net
} // namespace geely