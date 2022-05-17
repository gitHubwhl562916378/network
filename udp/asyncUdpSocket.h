/**
 * @file asyncUdpSocket.h
 * @brief UDP异步客户端
 * @details 实现UDP客户端异步读写
 * @author 王华林
 * @date 2022-05-11
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
#include <vector>
#include <functional>
#include <mutex>
#include "../asyncSocket.h"
#include "../iNetHost.h"

namespace geely {
namespace net {
    class IoLoop;
    class AsyncUdpSocket : public AsyncSocket, public std::enable_shared_from_this<AsyncUdpSocket> {
    public:
        using READ_EVENT_CB = std::function<void(std::shared_ptr<AsyncSocket>, const std::string &)>;
        /**
         * @brief 需要使用SetHost填充host信息,客户创建shared_ptr<AsyncUdpSocket>后，需要客户调用loop->AddAsyncSocket,构造函数中
         * 不能使用shared_from_this()
         *
         * @param loop io事件循环对象
         * @param host 如果是服务端为自己的端口和监听地址，如果是客户端是目标服务器的ip和端口，
         */
        explicit AsyncUdpSocket(std::shared_ptr<IoLoop> loop, const INetHost &host);
        /**
         * @brief 完成资源回收
         */
        ~AsyncUdpSocket();
        /**
         * @brief UDP没有写缓冲区，只有完全发送，和发送失败的区别，不需要等待EAGIN，不保证先后顺序，
         * 发送一次要么成功全部发送，要么失败返回-1，直接操作系统函数发送
         * @param data 要发送的报文，长度建议小于等于548字节(Internet标准576-8-20)
         */
        void Write(const std::string &data) override;
        /**
         * @brief 设置响应数据处理函数,客户端先发送可不用IoLoop::Update(READ)
         *
         * @param cb 读事件回调函数
         */
        void Read(READ_EVENT_CB cb);

    protected:
        std::weak_ptr<IoLoop> m_loop;

    private:
        /**
         * @brief 处理读事件回调，读完后调用READ_EVENT_CB，将数据透传出去, 获取到的远端ip和port使用
         * SetHost更新到基类成员
         * @return int32_t
         * @retval >0 读入的数据
         * @retval <=0 发生错误，错误码在errno, IoLoop会将该对象其删除
         */
        int32_t HandleRead() override;
        /**
         * @brief UDP没有写缓冲区，只有完全发送，和发送失败的区别，不需要等待EAGIN，不需要先后顺序，
         * 发送一次要么成功全部发送，要么失败返回-1，空实现
         * @return int32_t
         * @retval 0 固定值
         */
        int32_t HandleWrite() override;

        READ_EVENT_CB m_readCb;
    };
} // namespace net
} // namespace geely