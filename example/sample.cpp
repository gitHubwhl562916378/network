#ifdef _WIN32
#include "iocp/iocpLoop.h"
#else
#include "epoll/epollLoop.h"
#endif
#include "udp/udpServer.h"
#include "udp/udpSocket.h"

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
        std::cout << data << std::endl;

        session->Write("I Recevied " + std::to_string(data.size()) + " bytes");
    }
};

bool TestAsyncUdpClientWriteInCallBack(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<TestUdpCommonServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        std::cout << "Bind port failed" << std::endl;
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
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool TestUdpServerByAsyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<TestUdpCommonServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        std::cout << "Bind port failed" << std::endl;
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
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}

bool TestUdpServerBySyncClient(std::shared_ptr<geely::net::IoLoop> loop)
{
    //使用期间,保证server的生命周期
    auto server = std::make_shared<TestUdpCommonServer>(loop);
    if (!server->Bind({"0.0.0.0", 6666}))
    {
        std::cout << "Bind port failed" << std::endl;
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

int main(int argc, char **argv)
{
#ifdef _WIN32
    std::shared_ptr<geely::net::IoLoop> loop = std::make_shared<geely::net::IocpLoop>();
#else
    std::shared_ptr<geely::net::IoLoop> loop = std::make_shared<geely::net::EpollLoop>();
#endif
    if (!loop->Run())
    {
        std::cout << "IocpLoop run failed" << std::endl;
        return -1;
    }

    TestAsyncUdpClientWriteInCallBack(loop);
    TestUdpServerByAsyncClient(loop);
    TestUdpServerBySyncClient(loop);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}