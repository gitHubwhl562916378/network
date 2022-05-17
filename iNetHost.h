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

namespace geely {
namespace net {
    class INetHost final {
    public:
        INetHost();
        INetHost(const std::string &ip, const uint16_t port);
        void SetPort(const uint16_t port);
        void SetIp(const std::string &ip);
        uint16_t Port() const;
        std::string Ip() const;

    private:
        std::string m_ipAddr;
        uint16_t m_port;
    };
} // namespace net
} // namespace geely