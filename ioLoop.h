/**
 * @file ioLoop.h
 * @brief IO事件循环接口
 * @details 提供事件通知、修改监听事件、添加/删除事件等事件管理功能
 * @author Hualin.Wang
 * @date 2022-05-09
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <memory>
#include "noncopyable.h"
#include "asyncSocket.h"

namespace geely {
namespace net {
    class IoLoop : noncopyable {
    public:
        virtual ~IoLoop() {}
        /**
         * @brief 添加到事件监听循环中
         *
         * @param s 套接字共享指针
         * @return true 添加成功
         * @return false 添加失败
         */
        virtual bool AddAsyncSocket(std::shared_ptr<AsyncSocket> s) = 0;
        /**
         * @brief 在事件循环中删除事件
         *
         * @param s 要删除事件的共享指针
         * @return true 删除成功
         * @return false 删除失败
         */
        virtual bool RemoveAsyncSocket(std::shared_ptr<AsyncSocket> s) = 0;
        /**
         * @brief 修改要监听的下一个事件
         *
         * @param events 要监听的下一个事件类型
         * @param s 事件对象指针
         * @return int32_t 成功为0，不成功为错误码
         */
        virtual int32_t Update(const int32_t events, std::shared_ptr<AsyncSocket> s) = 0;
        /**
         * @brief 启动IO事件循环,非阻塞
         * @return true 成功
         * @return false 失败
         */
        virtual bool Run() = 0;
        /**
         * @brief 判断是否在Io线程
         * @return true 在
         * @return false 不在
         */
        virtual bool IsInLoopTread() = 0;
    };
} // namespace net
} // namespace geely