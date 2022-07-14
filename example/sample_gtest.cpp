#include <iostream>
#include <gtest/gtest.h>
#include "epoll/epollLoop.h"
#include "udp/udpSocket.h"
#include "udp/udpServer.h"
#include "tcp/tcpServer.h"
#include "tcp/tcpSocket.h"
#include "timerSocket.h"

class TestTcpCommServer : public geely::net::TcpServer
{
public:
    explicit TestTcpCommServer(std::shared_ptr<geely::net::IoLoop> loop)
        : TcpServer(loop) {}

protected:
    void OnNewConnection(std::shared_ptr<geely::net::AsyncTcpSocket> session) override
    {
        std::cout << "new client arrived " << session->GetINetHost().Ip() << " " << session->GetINetHost().Port() << std::endl;
        session->Read([](std::shared_ptr<AsyncSocket> s, const std::string &data)
                      {
            std::cout << "received " << data.size() << " bytes, string capacity " << data.capacity() << std::endl;
            s->Write("this is your request data"); });
    }
};

uint64_t g_fileSize = 0;
uint64_t g_totalSize = 0;
namespace geely
{
    namespace net
    {
        class FileServer : public TcpServer
        {
        public:
            explicit FileServer(std::shared_ptr<IoLoop> loop)
                : TcpServer(loop) {}
            void OnNewConnection(std::shared_ptr<AsyncTcpSocket> session) override
            {
                std::cout << "new client arrived " << session->GetINetHost().Ip() << " " << session->GetINetHost().Port() << std::endl;
                session->Read([](std::shared_ptr<AsyncSocket> s, const std::string &data)
                              {
                g_fileSize += data.size();
                if (g_fileSize == g_totalSize) {
                    std::cout << "receive client " << g_fileSize << " bytes" << std::endl;
                    s->Write(std::to_string(g_fileSize) + " bytes");
                } });
            }
        };
    } // namespace net
} // namespace geely
int64_t TestTcpBigFile(std::shared_ptr<geely::net::IoLoop> loop, const int64_t fileSize)
{
    auto server = std::make_shared<geely::net::FileServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        return -1;
    }
    if (!server->Listen(1024))
    {
        return -1;
    }
    loop->AddAsyncSocket(server);

    auto client = std::make_shared<geely::net::AsyncTcpSocket>(loop);
    if (!client->Connect({"127.0.0.1", 6666}))
    {
        return -1;
    }
    client->Read([](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
                 { std::cout << "TestTcpBigFile Received server " << data.size() << " bytes, data: " << data << " capacity " << data.capacity() << std::endl; });
    loop->AddAsyncSocket(client);
    g_totalSize = fileSize;
    std::string file(g_totalSize, 0); // 1G大小
    client->Write(file);

    sleep(5);
    auto ret = g_fileSize;
    return ret;
}

namespace geely
{
    namespace net
    {
        class MyTcpServer : public TcpServer
        {
        public:
            explicit MyTcpServer(std::shared_ptr<IoLoop> loop)
                : TcpServer(loop) {}
            void OnNewConnection(std::shared_ptr<AsyncTcpSocket> session) override
            {
                std::cout << "new client arrived " << session->GetINetHost().Ip() << " " << session->GetINetHost().Port() << std::endl;
                session->Read([](std::shared_ptr<AsyncSocket> s, const std::string &data)
                              {
                std::cout << "recv task " << data << " size " << data.size() << " capacity " << data.capacity() << std::endl;
                s->Write("MyTcpServer handle data"); });
            }
        };
    } // namespace net
} // namespace geely
bool TestCustomTcpServer(std::shared_ptr<geely::net::IoLoop> loop)
{
    auto server = std::make_shared<geely::net::MyTcpServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        return false;
    }
    if (!server->Listen(1024))
    {
        std::cout << "Listen failed" << std::endl;
        return false;
    }
    loop->AddAsyncSocket(server);

    auto client = std::make_shared<geely::net::AsyncTcpSocket>(loop);
    if (!client->Connect({"127.0.0.1", 6666}))
    {
        return false;
    }
    client->Read([](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
                 { std::cout << "I received server " << data.size() << " bytes, capacity " << data.capacity() << std::endl; });
    loop->AddAsyncSocket(client);
    for (int i = 0; i < 10; i++)
    {
        client->Write("custom server client data " + std::to_string(10));
        sleep(1);
    }

    sleep(2);
    return true;
}

bool TestTcpServerByMultiAsyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    auto server = std::make_shared<TestTcpCommServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        return false;
    }
    if (!server->Listen(1024))
    {
        return false;
    }
    loop->AddAsyncSocket(server);

    std::vector<std::shared_ptr<geely::net::AsyncTcpSocket>> clients;
    for (int i = 0; i < 10; i++)
    {
        auto client = std::make_shared<geely::net::AsyncTcpSocket>(loop);
        clients.push_back(client);
        if (!client->Connect({"127.0.0.1", 6666}))
        {
            continue;
        }
        client->Read(
            [](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
            {
                std::cout << "I received server " << data.size() << " bytes, capacity " << data.capacity() << std::endl;
            });
        loop->AddAsyncSocket(client);
        client->Write("client data " + std::to_string(i));
    }

    sleep(5);
    return true;
}

bool TestTcpServerByMultiSyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    auto server = std::make_shared<TestTcpCommServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        return false;
    }
    if (!server->Listen(20))
    {
        return false;
    }
    loop->AddAsyncSocket(server);

    for (int i = 0; i < 10; i++)
    {
        auto client = std::make_shared<geely::net::TcpSocket>();
        if (!client->Connect({"127.0.0.1", 6666}))
        {
            continue;
        }
        auto bytes = client->Write("client data " + std::to_string(i));
        std::string res(1500, 0);
        bytes = client->Read(res);
        std::cout << "I received server " << bytes << " bytes, data: " << res << std::endl;
    }

    sleep(5);
    return true;
}

class TestUdpCommonServer : public geely::net::UdpServer
{
public:
    explicit TestUdpCommonServer(std::shared_ptr<geely::net::IoLoop> loop)
        : UdpServer(loop) {}

protected:
    void OnClientMessage(std::shared_ptr<AsyncSocket> session,
                         const std::string &data) override
    {
        auto remote = session->GetINetHost();
        std::cout << "receive client message Ip: " << remote.Ip() << " port " << remote.Port() << " bytes " << data.size() << std::endl;

        session->Write("I Recevied " + std::to_string(data.size()) + " bytes");
    }
};
bool TestAsyncUdpClientWriteInCallBack(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<TestUdpCommonServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        return false;
    }
    if (!loop->AddAsyncSocket(server))
    {
        return false;
    }

    //使用期间,保证client的生命周期
    auto client = std::make_shared<geely::net::AsyncUdpSocket>(
        loop, geely::net::INetHost{"127.0.0.1", 6666});
    client->Read(
        [loop](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
        {
            std::cout << "Server response " << data << " size " << data.size() << " capacity " << data.capacity() << std::endl;
            static int num = 0;
            if (200 < num)
            {
                return;
            }
            s->Write("Receive times " + std::to_string(++num));
        });
    if (!loop->AddAsyncSocket(client))
    {
        return false;
    }
    client->Write("Start");

    //异步读写，等待，确保client读完后再释放。
    sleep(2);
    return true;
}

bool TestUdpServerByAsyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<TestUdpCommonServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        return false;
    }
    if (!loop->AddAsyncSocket(server))
    {
        return false;
    }

    auto client = std::make_shared<geely::net::AsyncUdpSocket>(
        loop, geely::net::INetHost{"127.0.0.1", 6666});
    client->Read([](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
                 { std::cout << "Server response " << data << " size " << data.size() << " capacity " << data.capacity() << std::endl; });
    if (!loop->AddAsyncSocket(client))
    {
        return false;
    }
    int32_t num = 0;
    while (true)
    {
        std::string data = "packed udp data " + std::to_string(num++);
        client->Write(data);

        if (200 < num)
        {
            break;
        }
    }

    //异步读写，等待，确保client读完后再释放。
    sleep(2);
    return true;
}

bool TestUdpServerBySyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<TestUdpCommonServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        return false;
    }
    if (!loop->AddAsyncSocket(server))
    {
        return false;
    }

    int32_t num = 0;
    while (true)
    {
        std::string data = "packed udp data " + std::to_string(num++);
        geely::net::UdpSocket uclient({"127.0.0.1", 6666});
        std::cout << "client write " << uclient.Write(data) << " bytes" << std::endl;
        std::string recv(1500, 0);
        auto bytes = uclient.Read(recv);
        std::cout << "Server response " << bytes << " bytes data:" << recv << ", capacity " << recv.capacity() << std::endl;

        if (200 < num)
        {
            break;
        }
    }

    return true;
}

bool TestTimerStop(std::shared_ptr<geely::net::IoLoop> loop, const uint32_t firstInterval,
                   const uint32_t afterFirstInterval)
{
    auto timer =
        std::make_shared<geely::TimerSocket>(loop, firstInterval, afterFirstInterval);
    if (!loop->AddAsyncSocket(timer))
    {
        std::cout << "TestTimerStop AddAsyncSocket failed" << std::endl;
        return false;
    }
    timer->OnTimeOut([](const uint64_t l)
                     { std::cout << "TestTimerStop timeout occured " << l << std::endl; });
    timer->Start();
    ::sleep(5);
    timer->Stop();
    std::cout << "TestTimerStop Timer Stoped" << std::endl;
    ::sleep(5);
    std::cout << "TestTimerStop Timer Timer prepear destroy" << std::endl;

    return true;
}

class NetTestSuit : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        std::cout << "NetTestSuit Starting" << std::endl;
    }
    static void TearDownTestCase()
    {
        ::sleep(2);
        std::cout << "NetTestSuit TearDown" << std::endl;
    }

    virtual void SetUp() override
    {
        if (!m_loop)
        {
            m_loop = std::make_shared<geely::net::EpollLoop>();
            if (!m_loop->Run())
            {
                throw std::runtime_error("EpollLoop run failed");
            }
        }
    }

    virtual void TearDown() override {}

    std::shared_ptr<geely::net::IoLoop> m_loop;
};

TEST_F(NetTestSuit, Udp)
{
    EXPECT_TRUE(TestAsyncUdpClientWriteInCallBack(m_loop));
    EXPECT_TRUE(TestUdpServerByAsyncClient(m_loop));
    EXPECT_TRUE(TestUdpServerBySyncClient(m_loop));
}

TEST_F(NetTestSuit, Tcp)
{
    int64_t fileSize = 1024 * 1024 * 1024;
    EXPECT_EQ(fileSize, TestTcpBigFile(m_loop, fileSize));
    EXPECT_TRUE(TestTcpServerByMultiAsyncClient(m_loop));
    EXPECT_TRUE(TestTcpServerByMultiSyncClient(m_loop));
    EXPECT_TRUE(TestCustomTcpServer(m_loop));
}

TEST_F(NetTestSuit, Timer)
{
    EXPECT_TRUE(TestTimerStop(m_loop, 1000, 1000));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}