/**
 * @file asyncSocket.h
 * @brief 异步事件接口
 * @details 异步事件接口，定义读写接口
 * @author 王华林
 * @date 2022-05-10
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <memory>
#include <string>
#include "socket.h"

namespace geely {
namespace net {
    class AsyncSocket : public Socket {
    public:
        AsyncSocket()
            : Socket() {}
        AsyncSocket(const int32_t fd)
            : Socket(fd) {}
        /**
         * @brief 写数据操作，一般为写到缓存，Update(WRITE)
         * @param data 要写的数据
         */
        virtual void Write(const std::string &data) = 0;

    protected:
        /**
         * @brief 处理写事件，将缓存数据发送
         * @return int32_t 返回码
         * @retval 0 成功
         * @retval -1 错误码记录在errno中
         */
        virtual int32_t HandleWrite() = 0;
        /**
         * @brief 处理读事件
         * @return int32_t 返回码
         * @retval 0 成功
         * @retval -1 错误码记录在errno中
         */
        virtual int32_t HandleRead() = 0;

        friend class EpollLoop;
    };
} // namespace net
} // namespace geely