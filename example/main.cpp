#include <iostream>
#include "../udp/udpSocket.h"
#include "../udp/udpServer.h"
#include "../tcp/tcpServer.h"
#include "../tcp/tcpSocket.h"
#include "../epoll/epollLoop.h"

uint64_t g_filesize = 0;
namespace geely
{
    namespace net
    {
        class FileServer : public TcpServer
        {
        public:
            explicit FileServer(std::shared_ptr<IoLoop> loop, const INetHost &host)
                : TcpServer(loop, host) {}
            void OnNewConnection(std::shared_ptr<AsyncTcpSocket> session) override
            {
                std::cout << "new client arrived " << session->GetINetHost().Ip() << " " << session->GetINetHost().Port() << std::endl;
                session->Read([](std::shared_ptr<AsyncSocket> s, const std::string &data)
                              {
                g_filesize += data.size();
                if (g_filesize == 1024 * 1024 * 1024) {
                    std::cout << "receive client " << g_filesize << " bytes" << std::endl;
                    s->Write(std::to_string(g_filesize) + " bytes");
                } });
            }
        };
    } // namespace net
} // namespace geely
void TestTcpBigFile(std::shared_ptr<geely::net::IoLoop> loop)
{
    auto server = std::make_shared<geely::net::FileServer>(loop, geely::net::INetHost{"0.0.0.0", 6666});
    if (!server->Listen(1024))
    {
        std::cout << "Listen failed" << std::endl;
        return;
    }
    loop->AddAsyncSocket(server);

    auto client = std::make_shared<geely::net::AsyncTcpSocket>(loop);
    if (!client->Connect({"127.0.0.1", 6666}))
    {
        std::cout << "Connect failed" << std::endl;
    }
    else
    {
        std::cout << "Connect success" << std::endl;
    }
    client->Read([](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
                 { std::cout << "I Recevied server " << data.size() << " bytes: " << data << std::endl; });
    loop->AddAsyncSocket(client);
    std::string file(1024 * 1024 * 1024, 0); // 1G大小
    client->Write(file);

    sleep(-1);
    loop->RemoveAsyncSocket(client);
    loop->RemoveAsyncSocket(server);
}

namespace geely
{
    namespace net
    {
        class MyTcpServer : public TcpServer
        {
        public:
            explicit MyTcpServer(std::shared_ptr<IoLoop> loop, const INetHost &host)
                : TcpServer(loop, host) {}
            void OnNewConnection(std::shared_ptr<AsyncTcpSocket> session) override
            {
                std::cout << "new client arrived " << session->GetINetHost().Ip() << " " << session->GetINetHost().Port() << std::endl;
                session->Read([](std::shared_ptr<AsyncSocket> s, const std::string &data)
                              {
                std::cout << "MyTcpServer recv task " << data << std::endl;
                s->Write("MyTcpServer handle data"); });
            }
        };
    } // namespace net
} // namespace geely
void TestCustomTcpServer(std::shared_ptr<geely::net::IoLoop> loop)
{
    auto server = std::make_shared<geely::net::MyTcpServer>(loop, geely::net::INetHost{"0.0.0.0", 6666});
    if (!server->Listen(1024))
    {
        std::cout << "Listen failed" << std::endl;
        return;
    }
    loop->AddAsyncSocket(server);

    auto client = std::make_shared<geely::net::AsyncTcpSocket>(loop);
    if (!client->Connect({"127.0.0.1", 6666}))
    {
        std::cout << "Connect failed" << std::endl;
    }
    else
    {
        std::cout << "Connect success" << std::endl;
    }
    client->Read([](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
                 { std::cout << "I Recevied server " << data.size() << " bytes: " << data << std::endl; });
    loop->AddAsyncSocket(client);
    for (int i = 0; i < 10; i++)
    {
        client->Write("custom server client data " + std::to_string(10));
        sleep(1);
    }

    sleep(2);
    loop->RemoveAsyncSocket(client);
    loop->RemoveAsyncSocket(server);
}

void TestTcpServerByMultiAsyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    auto server = std::make_shared<geely::net::TcpServer>(loop, geely::net::INetHost{"0.0.0.0", 6666});
    if (!server->Listen(1024))
    {
        std::cout << "Listen failed" << std::endl;
        return;
    }
    loop->AddAsyncSocket(server);

    std::vector<std::shared_ptr<geely::net::AsyncTcpSocket>> clients;
    for (int i = 0; i < 10; i++)
    {
        auto client = std::make_shared<geely::net::AsyncTcpSocket>(loop);
        clients.push_back(client);
        if (!client->Connect({"127.0.0.1", 6666}))
        {
            std::cout << "Connect failed" << std::endl;
        }
        else
        {
            std::cout << "Connect success" << std::endl;
        }
        client->Read([](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
                     { std::cout << "I Recevied server " << data.size() << " bytes: " << data << std::endl; });
        loop->AddAsyncSocket(client);
        client->Write("client data " + std::to_string(i));
    }

    sleep(5);
    for (auto c : clients)
    {
        loop->RemoveAsyncSocket(c);
    }
    loop->RemoveAsyncSocket(server);
}

void TestTcpServerByMultiSyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    auto server = std::make_shared<geely::net::TcpServer>(loop, geely::net::INetHost{"0.0.0.0", 6666});
    if (!server->Listen(20))
    {
        std::cout << "Listen failed" << std::endl;
        return;
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
        std::string res;
        bytes = client->Read(res);
        std::cout << "I received server " << bytes << " bytes: " << res << std::endl;
    }

    sleep(5);
    loop->RemoveAsyncSocket(server);
}

void TestAsyncUdpClientWriteInCallBack(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<geely::net::UdpServer>(loop, geely::net::INetHost{"0.0.0.0", 6666});
    if (!loop->AddAsyncSocket(server))
    {
        std::cout << "sys error" << std::endl;
        return;
    }

    //使用期间,保证client的生命周期
    auto client = std::make_shared<geely::net::AsyncUdpSocket>(loop, geely::net::INetHost{"127.0.0.1", 6666});
    client->Read([loop](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
                 {
        std::cout << "Server response: " << data << std::endl;
        static int num = 0;
        if (200 < num) {
            return;
        }
        s->Write("Receive times " + std::to_string(++num)); });
    if (!loop->AddAsyncSocket(client))
    {
        std::cout << "sys error" << std::endl;
        return;
    }
    client->Write("Start");

    //异步读写，等待，确保client读完后再释放。
    sleep(2);
    loop->RemoveAsyncSocket(server);
}

void TestUdpServerByAsyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<geely::net::UdpServer>(loop, geely::net::INetHost{"0.0.0.0", 6666});
    if (!loop->AddAsyncSocket(server))
    {
        std::cout << "sys error" << std::endl;
        return;
    }

    auto client = std::make_shared<geely::net::AsyncUdpSocket>(loop, geely::net::INetHost{"127.0.0.1", 6666});
    client->Read(
        [](std::shared_ptr<geely::net::AsyncSocket> s, const std::string &data)
        { std::cout << "Server response: " << data << std::endl; });
    if (!loop->AddAsyncSocket(client))
    {
        std::cout << "sys error" << std::endl;
        return;
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
    loop->RemoveAsyncSocket(server);
}

void TestUdpServerBySyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<geely::net::UdpServer>(loop, geely::net::INetHost{"0.0.0.0", 6666});
    if (!loop->AddAsyncSocket(server))
    {
        std::cout << "sys error" << std::endl;
        return;
    }

    int32_t num = 0;
    while (true)
    {
        std::string data = "packed udp data " + std::to_string(num++);
        geely::net::UdpSocket uclient({"127.0.0.1", 6666});
        std::cout << "client write " << uclient.Write(data) << " bytes" << std::endl;
        std::string recv;
        auto bytes = uclient.Read(recv);
        std::cout << "Server response: " << recv << std::endl;

        if (200 < num)
        {
            break;
        }
    }
    loop->RemoveAsyncSocket(server);
}

int main(int argc, char **argv)
{
    auto loop = std::make_shared<geely::net::EpollLoop>();
    if (!loop->Run())
    {
        return -1;
    }

    // TestAsyncUdpClientWriteInCallBack(loop);
    // TestUdpServerByAsyncClient(loop);
    // TestUdpServerBySyncClient(loop);

    // TestTcpBigFile(loop);
    // TestTcpServerByMultiAsyncClient(loop);
    TestTcpServerByMultiSyncClient(loop);
    // TestCustomTcpServer(loop);

    ::sleep(5);
    return 0;
}