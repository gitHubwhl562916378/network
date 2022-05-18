/**
 * @file udpSocket.h
 * @brief 实现UDP同步客户端
 * @details 同步读写
 * @author 王华林
 * @date 2022-05-11
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
    class UdpSocket : public Socket {
    public:
        /**
         * @brief 创建同步UDP套接字对象
         * @param host 服务端信息
         */
        explicit UdpSocket(const INetHost &host);
        /**
         * @brief 回收资源
         */
        ~UdpSocket();
        /**
         * @brief 发送数据
         * @param data 待发送的数据, 数据大小不要超过548
         * @return int32_t 成功发送的字节数
         * @retval >0 成功发送的字节数
         * @retval <=0 发送失败，错误码在errno中
         */
        int32_t Write(const std::string &data);
        /**
         * @brief 同步读取数据
         * @param data 返回读到的数据, 需要使用std::string(n, 0)或者resize初始化空间大小;不能使用reserve数据写不进去
         * @return int32_t 成功读取到的字节数
         * @retval >0 成功读取的字节数
         * @retval <=0 读取失败，错误码在errno中
         */
        int32_t Read(std::string &data);

        /**
         * @brief 使用fd发送数据，供全局调用
         * @param fd 文件描述符
         * @param buffer 待发送的数据地址
         * @param bufferLen 待发送的数据长度
         * @param host 远端地址信息
         * @return int32_t 成功发送的字节数
         * @retval >0 成功发送的字节数
         * @retval <=0 发送失败，错误码在errno中
         */
        static int32_t Write(const int32_t fd, const char *buffer, const uint32_t bufferLen, const INetHost &host);
        /**
         * @brief 使用fd读取数据，供全局调用
         * @param fd 文件描述符
         * @param buffer 存放读取数据的内存地址
         * @param bufferLen 读取数据的的内存大小
         * @param host 远端信息
         * @return int32_t 返回成功读取的字节数
         * @retval >0 成功读到的字节数
         * @retval <=0 读取失败，错误码在errno中
         */
        static int32_t Read(const int32_t fd, char *buffer, const uint32_t bufferLen, INetHost &host);
    };
} // namespace net
} // namespace geely