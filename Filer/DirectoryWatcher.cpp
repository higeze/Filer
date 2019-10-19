//#include "stdafx.h"
#include "DirectoryWatcher.h"
#include "SEHException.h"
#include "MyWin32.h"
#include "MyString.h"
#include "ThreadPool.h"


CDirectoryWatcher::CDirectoryWatcher(HWND hWnd)
	:m_hWnd(hWnd), m_pQuitEvent(), m_vData(kBufferSize, 0)
{}

CDirectoryWatcher::~CDirectoryWatcher(void)
{
	QuitWatching();
}

void CDirectoryWatcher::StartWatching(const std::wstring& path)
{
	try {
		//Create event
		m_pQuitEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL));
		if (!m_pQuitEvent) {
			FILE_LINE_FUNC_TRACE;
			return;
		}

		//Create watch thread
		m_futureWatch = CThreadPool::GetInstance()->enqueue(&CDirectoryWatcher::WatchDirectory, this, path);
	}
	catch (std::exception& e) {
		FILE_LINE_FUNC_TRACE;
		QuitWatching();
		throw e;
	}
}

void CDirectoryWatcher::QuitWatching()
{
	try {
		//Event
		if (m_pQuitEvent) {
			//Throw quit event
			if (!::SetEvent(m_pQuitEvent.get())) {
				FILE_LINE_FUNC_TRACE;
			}
			//Close handle
			m_pQuitEvent.reset();
		}
		//Work
		//Release thread
		if (m_futureWatch.valid() && m_futureWatch.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
			m_futureWatch.get();
		}
		//Path
		m_path.clear();
	}
	catch (std::exception& e) {
		FILE_LINE_FUNC_TRACE;
		throw e;
	}

}

void CDirectoryWatcher::WatchDirectory(const std::wstring& path)
{
	try{
		spdlog::info("Start CDirectoryWatcher::WatchDirectoryCallback");

		//Create File
		m_path = path;
		UniqueHandlePtr pDir(::CreateFileW(m_path.c_str(),
								FILE_LIST_DIRECTORY,
								FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
								NULL,
								OPEN_EXISTING,
								FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
								NULL));
		if(pDir.get() == INVALID_HANDLE_VALUE){
			throw std::exception(FILE_LINE_FUNC);//TODODO
		}

		//Associate IoComp with hDir
		UniqueHandlePtr pIocp(::CreateIoCompletionPort(pDir.get(), NULL, COMPKEY_DIR, 0));
		if (pIocp.get() == INVALID_HANDLE_VALUE) {
			throw std::exception(FILE_LINE_FUNC);//TODODO
		}

		//Start thread;
		std::future<void> futureCallback(CThreadPool::GetInstance()->enqueue(&CDirectoryWatcher::IoCompletionCallback, this, pIocp.get(), pDir.get()));

		//Start observer
		OVERLAPPED overlapped = { 0 };
		::PostQueuedCompletionStatus(pIocp.get(), 0, COMPKEY_DIR, (LPOVERLAPPED)&overlapped);

		//Wait for quit event
		WaitForSingleObject(m_pQuitEvent.get(),INFINITE);

		//Terminate observer
		::PostQueuedCompletionStatus(pIocp.get(), 0, COMPKEY_QUIT, nullptr);
		//Terminate thread
		futureCallback.get();

		spdlog::info("End CDirectoryWatcher::WatchDirectoryCallback");
	}catch(std::exception&){
		FILE_LINE_FUNC_TRACE;
	}
}

void CDirectoryWatcher::IoCompletionCallback(HANDLE hIocp, HANDLE hDir)
{
	try{
		spdlog::info("Start CDirectoryWatcher::IoCompletionCallback");

		while (true) {
			DWORD dwBytes = 0L;
			ULONG_PTR ulCompKey = 0L;
			LPOVERLAPPED pOverlapped = nullptr;
			::GetQueuedCompletionStatus(hIocp, &dwBytes, &ulCompKey, &pOverlapped, INFINITE);

			if (ulCompKey == COMPKEY_QUIT) {
				break;
			}

			PFILE_NOTIFY_INFORMATION pFileNotifyInfo = nullptr;
			if (dwBytes > 0) {
				pFileNotifyInfo = (PFILE_NOTIFY_INFORMATION)new BYTE[dwBytes];
				::CopyMemory((PVOID)pFileNotifyInfo, (CONST PVOID)m_vData.data(), dwBytes);
			}

			//Keep watching
			if (pOverlapped) {
				spdlog::info("ReadDirectoryChangesW");
				//Associate iocompletionobject to thread pool
				if (!ReadDirectoryChangesW(
					hDir,
					m_vData.data(),
					m_vData.size(),
					FALSE,
					FILE_NOTIFY_CHANGE_FILE_NAME |
					FILE_NOTIFY_CHANGE_DIR_NAME |
					FILE_NOTIFY_CHANGE_ATTRIBUTES |
					FILE_NOTIFY_CHANGE_SIZE |
					FILE_NOTIFY_CHANGE_LAST_WRITE |
					//FILE_NOTIFY_CHANGE_LAST_ACCESS |
					//FILE_NOTIFY_CHANGE_SECURITY    |
					FILE_NOTIFY_CHANGE_CREATION,
					NULL,
					(LPOVERLAPPED)pOverlapped,
					NULL)) {
					throw std::exception(
						("ReadDirectoryChangesW FAILED\r\n"
							"Last Error:" + GetLastErrorString() +
							"Path:" + wstr2str(m_path) + "\r\n").c_str());
				}
			}

			if (dwBytes > 0) {
				::SendMessage(m_hWnd, WM_DIRECTORYWATCH, (WPARAM)pFileNotifyInfo, NULL);
				delete[](PBYTE)pFileNotifyInfo;
			}
		}
	}catch(std::exception&){
		FILE_LINE_FUNC_TRACE;
	}
}

