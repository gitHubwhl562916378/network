/**
 * @file tcpServer.h
 * @brief 实现TCP服务器
 * @details 实现TCP客户端连接建立与管理
 * @author 王华林
 * @date 2022-05-12
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <map>
#include "../ioLoop.h"
#include "asyncTcpSocket.h"

namespace geely {
namespace net {
    class TcpServer : public AsyncSocket {
    public:
        /**
         * @brief 使用生成fd
         *
         * @param loop IO事件循环对象
         */
        explicit TcpServer(std::shared_ptr<IoLoop> loop);
        /**
         * @brief 回收资源, 对m_sessions中已有的连接不做RemoveAsyncSocket，认为析构程序退出。运行期间，由客户端断开在
         * 事件循环中自动RemoveAsyncSocket
         * @retval
         */
        ~TcpServer();
        /**
         * @brief 绑定端口
         * @param host 端口和绑定地址
         * @return true 成功
         * @return false 失败
         */
        bool Bind(const INetHost &host);
        /**
         * @brief 绑定端口，监听端口，需要使用IoLoop::AddAsyncSocket添加到循环，并IoLoop::Update读事件
         *
         * @param num 最大监听的套接字数量
         * @return int32_t 返回码
         * @retval 0 成功
         * @retval -1 失败，原因在errno
         */
        int32_t Listen(int32_t num);
        /**
         * @brief 查找文件描述符为connFd的连接
         * @param connFd 文件描述符
         * @return std::shared_ptr<AsyncTcpSocket>
         * @retval nullptr 不存在这样的连接
         */
        std::shared_ptr<AsyncTcpSocket> GetSession(const int32_t connFd);
        /**
         * @brief 处理新建连接，此时已经将连接加入到IoLoop的事件管理中，
         * 函数中需要使用AsyncTcpSocket::Read指明数据读取后的回调函数
         *
         * @param client 建立好的连接
         */
        virtual void OnNewConnection(std::shared_ptr<AsyncTcpSocket> client);
        /**
         * @brief 向连所有接好的Session发送数据，使用AsyncTcpSocket::Write完成
         *
         * @param data
         */
        void Write(const std::string &data) override;

    private:
        /**
         * @brief TcpServer中不实现
         *
         */
        int32_t HandleWrite() override {
            return 1;
        }
        /**
         * @brief 有响应就视为有新的连接到来，
         * 创建新的连接,accept中获取客户端ip和端口，以此构建连接好的AsyncTcpSocket
         *
         */
        int32_t HandleRead() override;

        std::weak_ptr<IoLoop> m_loop;
        std::mutex m_sessionMtx;
        std::map<int32_t, std::shared_ptr<AsyncTcpSocket>> m_sessions;
    };
} // namespace net
} // namespace geely