/**
 * @file udpServer.h
 * @brief 实现UDP服务端
 * @details 服务端接收客户端消息，并响应
 * @author 王华林
 * @date 2022-05-11
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include "asyncUdpSocket.h"

namespace geely {
namespace net {
    class UdpServer : public AsyncUdpSocket {
    public:
        /**
         * @brief 使用使用基类创建套接字,绑定端口,客户创建shared_ptr<AsyncUdpSocket>后，需要客户调用loop->AddAsyncSocket,构造函数中
         * 不能使用shared_from_this(), 构造中需要使用Read把OnClientMessage设置为读取回调
         *
         * @param loop IO事件循环
         * @param host 需要使用SetHost保存到基类，然后保存到m_localHost用于记录服务绑定信息，基类的GetHost记录客户端信息
         */
        explicit UdpServer(std::shared_ptr<IoLoop> loop, const INetHost &host);
        ~UdpServer();
        /**
         * @brief Get the Local Host object
         *
         * @return INetHost
         */
        INetHost GetLocalHost() const;

    protected:
        /**
         * @brief 实现客户端数据的打印
         *
         * @param client 客户端信息
         * @param data 客户端发送的数据
         */
        virtual void OnClientMessage(std::shared_ptr<AsyncSocket> session, const std::string &data);

    private:
        INetHost m_localHost;
    };
} // namespace net
} // namespace geely