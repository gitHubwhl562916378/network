/**
 * @file notifyAsyncSocket.h
 * @brief
 * @details
 * @author 王华林
 * @date 2022-05-25
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include "asyncSocket.h"

namespace geely {
namespace net {
    class IoLoop;
    class NotifyAsyncSocket : public AsyncSocket {
    public:
        explicit NotifyAsyncSocket();
        ~NotifyAsyncSocket();
        /**
         * @brief 写eventfd，读端将会触发
         */
        void WakeUp();

    private:
        /**
         * @brief 不需要实现
         * @param data
         */
        void Write(const std::string &data) override {}
        /**
         * @brief 写一个uint64_t, 触发读端
         * @return int32_t
         * @retval ==8 成功
         * @retval !=8 失败
         */
        int32_t HandleWrite() override;
        /**
         * @brief 有事件可读
         * @return int32_t
         * @retval ==8 成功
         * @retval !=8 失败
         */
        int32_t HandleRead() override;
    };
} // namespace net
} // namespace geely