/**
 * @file iNetHost.h
 * @brief 用于储存套接字信息
 * @details 储存ip、端口等
 * @author 王华林
 * @date 2022-05-11
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <string>
#include "globalDefine.h"

namespace geely
{
    namespace net
    {
        class LIB_EXPORT INetHost final
        {
        public:
            INetHost();
            INetHost(const std::string &ip, const uint16_t port);
            /**
             * @brief Set the Port object
             * @param port 端口
             * @retval
             */
            void SetPort1(const uint16_t port);
            /**
             * @brief Set the Ip object
             * @param ip 地址
             * @retval
             */
            void SetIp(const std::string &ip);
            /**
             * @brief 返回端口
             * @return uint16_t
             */
            uint16_t Port() const;
            /**
             * @brief 返回地址
             * @return std::string
             * @retval
             */
            std::string Ip() const;

        private:
            std::string m_ipAddr;
            uint16_t m_port;
        };
    } // namespace net
} // namespace geely