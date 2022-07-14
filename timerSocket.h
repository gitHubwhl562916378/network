/**
 * @file timerSocket.h
 * @brief 定时器封装
 * @details 使用epoll实现定时器封装
 * @author 王华林
 * @date 2022-05-25
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <functional>
#include <atomic>
#include <future>
#include "asyncSocket.h"

namespace geely {
namespace net {
    class IoLoop;
}
class TimerSocket : public net::AsyncSocket {
public:
    using TimeoutFunctor = std::function<void(const uint64_t)>;
    /**
     * @brief Construct a new Timer Socket object
     */
    explicit TimerSocket(const std::shared_ptr<net::IoLoop> loop);
    /**
     * @brief Construct a new Timer Socket object
     * @param firstInterval 第一次超时的时间(毫秒)，0表示不启用，计时器不会触发
     * @param intervalAfterFirst
     * 第一次超时后，后面每一次超时的间隔(毫秒)，0表示第一次超时后结束，总共只超时一次
     */
    explicit TimerSocket(const std::shared_ptr<net::IoLoop> loop,
                         const uint32_t firstInterval,
                         const uint32_t intervalAfterFirst = 0);
    /**
     * @brief Destroy the Timer Socket object
     */
    ~TimerSocket();
    /**
     * @brief 设置第一次超时的时间
     * @param interval 时长(毫秒)
     */
    void SetFirstInterval(const uint32_t interval);
    /**
     * @brief 第一次超时后，后面的每次超时的间隔
     * @param interval 超时间隔(毫秒)
     */
    void SetIntervalAfterFirst(const uint32_t interval);
    /**
     * @brief 启动定时器
     * @return true 成功
     * @return false 失败
     */
    bool Start();
    /**
     * @brief 停止定时器
     * @return true
     * @return false
     */
    bool Stop();
    /**
     * @brief 设置超时处理函数
     * @param cb 回调函数
     */
    void OnTimeOut(TimeoutFunctor cb);

private:
    /**
     * @brief 不需要实现
     * @param data
     */
    void Write(const std::string &data) override {}
    /**
     * @brief 直接调用timerfd_setttime，监听写事件无效
     * @return int32_t
     */
    int32_t HandleWrite() override {
        return 0;
    }
    /**
     * @brief 超时会触发读事件，处理超时事件，调用回调函数
     * @return int32_t
     * @retval ==8 成功
     * @retval !=8 失败
     */
    int32_t HandleRead() override;

    std::atomic_uint32_t m_firstInterval;
    std::atomic_uint32_t m_intervalAfterFirst;
    TimeoutFunctor m_timeOutHandler;
    std::shared_ptr<net::IoLoop> m_loop;
};
} // namespace geely
