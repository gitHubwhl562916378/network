/**
 * @file epollLoop.h
 * @brief 使用epoll的IO事件循环实现
 * @details 使用epoll对IoLoop接口实现
 * @author 王华林
 * @date 2022-05-09
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <functional>
#include <thread>
#include <future>
#include <vector>
#include <condition_variable>
#include <map>
#include "../ioLoop.h"

namespace geely {
namespace net {
    class NotifyAsyncSocket;
    class EpollLoop : public IoLoop {
    public:
        using Functor = std::shared_ptr<std::packaged_task<int32_t()>>;

        explicit EpollLoop();
        /**
         * @brief 回收资源
         */
        ~EpollLoop();
        /**
         * @brief 添加监听套接字
         * @param s 要监听的套接字对象，生命周期由调用方管理，本类保存weak_ptr
         * @return true 添加成功
         * @return false 添加失败
         */
        bool AddAsyncSocket(std::shared_ptr<AsyncSocket> s) override;
        /**
         * @brief 删除监听套接字
         * @param s 要监听的套接字对象，生命周期由调用方管理，本类保存weak_ptr
         * @return true 删除成功
         * @return false 删除失败
         */
        bool RemoveAsyncSocket(std::shared_ptr<AsyncSocket> s) override;
        /**
         * @brief 删除要监听的套接字
         * @param fd 要监听的文件描述符
         * @return true 成功
         * @return false 失败
         */
        bool RemoveAsyncSocket(const int32_t fd) override;
        /**
         * @brief 修改监听事件
         * @param events 要监听的事件
         * @param s 要监听的套接字对象
         * @return int32_t 返回码
         * @retval 0 成功
         * @retval 非0 失败
         */
        int32_t Update(const int32_t events, std::shared_ptr<AsyncSocket> s) override;
        /**
         * @brief 启动事件循环
         * @return true 成功
         * @return false 失败
         */
        bool Run() override;

    private:
        /**
         * @brief 判断是否在Io线程
         * @return true 在
         * @return false 不在
         */
        bool IsInLoopTread() override;
        /**
         * @brief 在IO线程ProcessLoop函数中执行的函数
         * @param cb 要执行的函数
         */
        void RunInLoop(Functor cb);
        /**
         * @brief 执行挂起的Functor
         */
        void DoPendingFunctors();
        /**
         * @brief IO事件循环线程函数
         */
        void ProcessLoop(std::promise<void> &promise);

        std::thread m_ioThread;
        std::thread::id m_threadId;
        std::mutex m_functorMtx;
        std::vector<Functor> m_pendingFunctors;
        int32_t m_epollFd;
        std::shared_ptr<NotifyAsyncSocket> m_notifySocket;
        volatile bool m_shutDown = false;
        std::map<int32_t, std::weak_ptr<AsyncSocket>> m_evSockets;
    };
} // namespace net
} // namespace geely