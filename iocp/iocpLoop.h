#pragma once
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <future>
#include "../ioLoop.h"
#define IOCP_WSABUFFCOUNT 1
#define IOCP_ONEBUFLEN 1400

//https://docs.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports
namespace geely
{
    namespace net
    {
        enum class OP_TYPE
        {
            READ = 0x01,
            WRITE = 0x10
        };
        struct LIB_EXPORT IO_OPERATION_DATA
        {
            WSAOVERLAPPED overlapped;
            WSABUF wsaBuffer[IOCP_WSABUFFCOUNT];
            OP_TYPE oType;
            sockaddr_in remote_addr;
            socklen_t addr_len = sizeof(sockaddr_in);

            static IO_OPERATION_DATA *NewIoOperationData(const OP_TYPE &t)
            {
                auto data = new IO_OPERATION_DATA;
                for (int i = 0; i < IOCP_WSABUFFCOUNT; i++)
                {
                    if (OP_TYPE::READ == t) {
                        auto p = new char[IOCP_ONEBUFLEN];
                        data->wsaBuffer[i].buf = p;
                        data->wsaBuffer[i].len = IOCP_ONEBUFLEN;
                    }
                    else {
                        data->wsaBuffer[i].buf = nullptr;
                        data->wsaBuffer[i].len = 0;
                    }
                }
                data->oType = t;
                ZeroMemory(&data->overlapped, sizeof(OVERLAPPED));
                return data;
            }
            static void FreeIoOperaionData(IO_OPERATION_DATA **data)
            {
                if (nullptr == data) {
                    return;
                }

                if (OP_TYPE::READ == (*data)->oType) {
                    for (int i = 0; i < IOCP_WSABUFFCOUNT; i++)
                    {
                        delete (*data)->wsaBuffer[i].buf;
                    }
                }
                delete *data;
                *data = nullptr;
            }
        };

        class LIB_EXPORT IocpLoop : public IoLoop
        {
        public:
            explicit IocpLoop();
            ~IocpLoop() override;
            bool AddAsyncSocket(std::shared_ptr<AsyncSocket> s) override;
            bool RemoveAsyncSocket(std::shared_ptr<AsyncSocket> s) override;
            bool RemoveAsyncSocket(const int32_t fd) override;
            int32_t Update(const int32_t events, std::shared_ptr<AsyncSocket> s) override;
            bool Run() override;
            bool IsInLoopTread() override;

        private:
            void ProcessThread(std::promise<std::thread::id> &pro);

            HANDLE m_cpHandle;
            SYSTEM_INFO m_sysInfo;
            volatile bool m_shutDown = false;
            std::vector<std::thread> m_processThreads;
            std::vector<std::thread::id> m_thrIds;

            std::mutex m_fdMtx;
            std::map<int32_t, std::weak_ptr<AsyncSocket>> m_sockets;
        };
    }
}