/**
 * @file tcpSocket.h
 * @brief Tcp同步客户端实现
 * @details Tcp同步读写
 * @author 王华林
 * @date 2022-05-10
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <vector>
#include "../socket.h"
#include "../iNetHost.h"

namespace geely {
namespace net {
    class TcpSocket : public Socket {
    public:
        /**
         * @brief 创建TCP套接字
         */
        explicit TcpSocket();
        /**
         * @brief 回收资源
         * @retval
         */
        ~TcpSocket();
        /**
         * @brief 连接服务器
         * @param host 服务器地址、端口
         * @return true 连接成功
         * @return false 连接失败
         */
        bool Connect(const INetHost &host);
        /**
         * @brief 同步发送数据
         * @param data 发送内容
         * @return int32_t 已成功发送的字节数
         * @retval >0 成功发送的字节数
         * @retval <=0 发送失败，错误吗记录在errno中
         */
        int32_t Write(const std::string &data);
        /**
         * @brief 同步读取数据
         * @param data 返回读取到的数据
         * @return int32_t 返回读取到的字节数
         * @retval >0 成功读取的字节数
         * @retval <=0 读取失败，错误码记录在errno中
         */
        int32_t Read(std::string &data);

        /**
         * @brief 使用fd发送数据，对系统接口封装,供其他地方调用
         * @param fd 文件描述符
         * @param buffer 待发送的数据地址
         * @param bufferLen 待发送的数据长度
         * @return int32_t 返回成功发送的字节数
         * @retval >0 成功发送的字节数
         * @retval <=0 发送失败，错误码记录在errno中
         */
        static int32_t Write(const int32_t fd, const char *buffer, const uint32_t bufferLen);
        /**
         * @brief 使用fd读取数据，对系统接口封装
         * @param fd 文件描述符
         * @param buffer 存放读取数据的内存地址
         * @param bufferLen 读取数据的的内存大小
         * @return int32_t 返回成功读取到的字节数
         * @retval >0 成功读取到的字节数
         * @retval <=0 读取失败，错误码记录在errno中
         */
        static int32_t Read(const int32_t fd, char *buffer, const int32_t bufferLen);
    };
} // namespace net
} // namespace geely