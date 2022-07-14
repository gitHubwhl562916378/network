#include <functional>

#include "iocpLoop.h"

geely::net::IocpLoop::IocpLoop()
{
}

geely::net::IocpLoop::~IocpLoop()
{
	m_shutDown = true;
	for (int i = 0; i < m_processThreads.size(); i++) {
		PostQueuedCompletionStatus(m_cpHandle, DWORD(0), 0, nullptr);
	}
	for (auto& t : m_processThreads) {
		if (t.joinable()) {
			t.join();
		}
	}
	WSACleanup();
}

bool geely::net::IocpLoop::AddAsyncSocket(std::shared_ptr<AsyncSocket> s)
{
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(s->GetNativeSocket()), m_cpHandle, static_cast<ULONG_PTR>(s->GetNativeSocket()), 0);

	{
		std::lock_guard<std::mutex> lock(m_fdMtx);
		auto iter = m_sockets.find(s->GetNativeSocket());
		if (iter != m_sockets.end()) {
			return true;
		}

		m_sockets.insert(std::make_pair(s->GetNativeSocket(), s));
	}

	return true;
}

bool geely::net::IocpLoop::RemoveAsyncSocket(std::shared_ptr<AsyncSocket> s)
{
	std::lock_guard<std::mutex> lock(m_fdMtx);
	auto iter = m_sockets.find(s->GetNativeSocket());
	if (iter == m_sockets.end()) {
		return true;
	}

	m_sockets.erase(iter);
	return true;
}

bool geely::net::IocpLoop::RemoveAsyncSocket(const int32_t fd)
{
	std::lock_guard<std::mutex> lock(m_fdMtx);
	auto iter = m_sockets.find(fd);
	if (iter == m_sockets.end()) {
		return true;
	}

	m_sockets.erase(iter);
	return true;
}

int32_t geely::net::IocpLoop::Update(const int32_t events, std::shared_ptr<AsyncSocket> s)
{
	return 0;
}

bool geely::net::IocpLoop::Run()
{
	WSADATA wsaData;
	auto ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != ret) {
		std::cerr << "WSAStartup failed" << std::endl;
		return false;
	}

	if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion)) {
		WSACleanup();
		std::cerr << "Request windows socket version 2.2 error" << std::endl;
		return false;
	}

	m_cpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (nullptr == m_cpHandle) {
		std::cerr << "CreateIoCompletionPort failed, error: " << GetLastError() << std::endl;
		return false;
	}

	GetSystemInfo(&m_sysInfo);
	for (int i = 0; i < m_sysInfo.dwNumberOfProcessors * 2; i++) {
		std::promise<std::thread::id> pro;
		auto fut = pro.get_future();
		m_processThreads.push_back(std::thread(std::bind(&IocpLoop::ProcessThread, this, std::ref(pro))));
		m_thrIds.push_back(fut.get());
	}

	return true;
}

bool geely::net::IocpLoop::IsInLoopTread()
{
	return m_thrIds.end() != std::find(m_thrIds.begin(), m_thrIds.end(), std::this_thread::get_id());
}

void geely::net::IocpLoop::ProcessThread(std::promise<std::thread::id>& pro)
{
	pro.set_value(std::this_thread::get_id());

	DWORD bytesTransfered;
	LPOVERLAPPED lpOverlapped;
	ULONG_PTR key = 0;
	IO_OPERATION_DATA* ioData = nullptr;
	bool result = false;
	while (!m_shutDown)
	{
		result = GetQueuedCompletionStatus(m_cpHandle, &bytesTransfered, &key, reinterpret_cast<LPOVERLAPPED*>(&lpOverlapped), INFINITE);
		if (false == result) {
			auto code = GetLastError();
			if (ERROR_OPERATION_ABORTED == code) {
				continue;
			}

			std::cerr << "GetQueuedCompletionStatus error: " << code << std::endl;
			continue;
		}

		if (0 == key && nullptr == lpOverlapped) {
			continue;
		}

		std::shared_ptr<AsyncSocket> sock;
		do {
			std::unique_lock<std::mutex> lock(m_fdMtx, std::try_to_lock);
			if (!lock.owns_lock()) {
				continue;
			}

			auto iter = m_sockets.find(static_cast<int32_t>(key));
			if (iter != m_sockets.end()) {
				sock = iter->second.lock();
			}
			break;
		} while (true);

		if (nullptr == sock) {
			continue;
		}

		ioData = CONTAINING_RECORD(lpOverlapped, IO_OPERATION_DATA, overlapped);
		if (0 == bytesTransfered) {
			RemoveAsyncSocket(sock);
			continue;
		}

		if (OP_TYPE::READ == ioData->oType) {
			sock->HandleRead(ioData, bytesTransfered);
		}
		else {
			sock->HandleWrite(ioData, bytesTransfered);
			int a = bytesTransfered;
			int c = 0;
		}
	}
}
