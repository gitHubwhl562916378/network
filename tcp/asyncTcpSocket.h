/**
 * @file asyncTcpSocket.h
 * @brief 异步TCP客户端
 * @details 实现TCP异步读写
 * @author 王华林
 * @date 2022-05-12
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <vector>
#include <functional>
#include <mutex>
#include "../ioLoop.h"
#include "../asyncSocket.h"
#include "../iNetHost.h"

namespace geely {
namespace net {
    class AsyncTcpSocket : public AsyncSocket, public std::enable_shared_from_this<AsyncTcpSocket> {
    public:
        using READ_EVENT_CB = std::function<void(std::shared_ptr<AsyncSocket>, const std::string &)>;
        /**
         * @brief 生成套接字，构建对象
         *
         * @param loop 事件循环对象，保存
         */
        explicit AsyncTcpSocket(std::shared_ptr<IoLoop> loop);
        /**
         * @brief 回收资源
         */
        ~AsyncTcpSocket();
        /**
         * @brief 连接服务端
         *
         * @return true 调用方使用IoLoop::AddAsyncSocket添加到循环中
         * @return false
         */
        bool Connect(const INetHost &host);
        /**
         * @brief 写入数据到本地缓存，并使用IoLoop::Update触发IO_EVENT::WRITE
         *
         * @param data 要写入的数据
         * @return int32_t
         */
        void Write(const std::string &data) override;
        /**
         * @brief 设置响应数据处理函数
         *
         * @param cb 读事件回调函数
         */
        void Read(READ_EVENT_CB cb);

    private:
        /**
         * @brief 使用已连接好的fd构建对象, 服务端使用
         *
         * @param loop 事件循环对象，保存
         * @param fd 文件描述符
         */
        AsyncTcpSocket(std::shared_ptr<IoLoop> loop, const int32_t fd, const INetHost &host);
        /**
         * @brief 处理写回调，将缓存数据发送，发送完后，使用IoLoop::Update监听读事件
         *
         */
        int32_t HandleWrite() override;
        /**
         * @brief 处理读事件回调，读完后调用READ_EVENT_CB，将数据透传出去,
         *
         */
        int32_t HandleRead() override;

        friend class TcpServer;
        std::weak_ptr<IoLoop> m_loop;
        std::mutex m_dataMtx;
        std::string m_writeCache;
        uint64_t m_writedCacheLen = 0;
        bool m_isConected = false;
        READ_EVENT_CB m_read_cb;
    };
} // namespace net
} // namespace geely