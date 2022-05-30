/**
 * @file socket.h
 * @brief 文件描述符基类，管理基本信息
 * @details 对文件描述符，网络布套接字基本信息管理
 * @author 王华林
 * @date 2022-05-25
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include "iNetHost.h"
#include "noncopyable.h"

namespace geely {
namespace net {
    class Socket : noncopyable {
    public:
        explicit Socket()
            : m_nativeSocket(0) {}
        Socket(const int32_t fd)
            : m_nativeSocket(fd) {}
        virtual ~Socket() {
            std::cout << "Socket::~Socket()" << std::endl;
        }

        int32_t GetNativeSocket() const {
            return m_nativeSocket;
        }
        /**
         * @brief 获取远端host配置
         *
         * @return INetHost
         */
        INetHost GetINetHost() const {
            return m_host;
        }
        /**
         * @brief 判断本地是否为大端序
         * @return true 大端序
         * @return false 小端序
         */
        static bool IsBigEndian() {
            union {
                int a;
                char b;
            } num;

            num.a = 0x12345678;
            return (num.b == 0x12);
        }
        /**
         * @brief 转换64位字节序
         * @param n 64位数字
         * @return int64_t 转换后的结果
         */
        static int64_t SwapEndian64(int64_t n) {
            return ((n >> 56) & 0xFF) | ((n >> 48) & 0xFF) << 8 | ((n >> 40) & 0xFF) << 16 | ((n >> 32) & 0xFF) << 24 |
                   ((n >> 24) & 0xFF) << 32 | ((n >> 16) & 0xFF) << 40 | ((n >> 8) & 0xFF) << 48 | (n & 0xFF) << 56;
        }

    protected:
        /**
         * @brief Set the Host object
         * @param host ip地址和端口
         */
        void SetHost(const INetHost &host) {
            m_host = host;
        }
        /**
         * @brief Set the Native Socket object
         * @param fd 文件描述符
         */
        void SetNativeSocket(const int32_t fd) {
            m_nativeSocket = fd;
        }
        /**
         * @brief 关闭后，如果是m_nativeSocket是在Io_Loop中,且是TCP连接,则会触发读事件，且handleRead返回0
         * IoLoop将其事件对象erase掉
         *
         * @return true
         * @return false
         */
        bool Close() const {
            return ::close(m_nativeSocket);
        }

    private:
        int32_t m_nativeSocket;
        INetHost m_host;
    };
} // namespace net
} // namespace geely