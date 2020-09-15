//#include "stdafx.h"
#include "DirectoryWatcher.h"
#include "SEHException.h"
#include "MyWin32.h"
#include "MyString.h"
#include "ThreadPool.h"
#include "D2DWWindow.h"
#include "D2DWControl.h"

#include "ShellFunction.h"
#include <algorithm>


CDirectoryWatcher::CDirectoryWatcher(CD2DWControl* pControl, std::function<void(const DirectoryWatchEvent&)> callback)
	:m_pControl(pControl), m_pQuitEvent(), m_vData(kBufferSize, 0), m_callback(callback)
{}

CDirectoryWatcher::~CDirectoryWatcher(void)
{
	QuitWatching();
}

void CDirectoryWatcher::StartWatching(const std::wstring& path, const CIDL& absIdl)
{
	m_path = path;
	m_absIdl = absIdl;

	try {
		//Create event
		m_pQuitEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL));
		if (!m_pQuitEvent) {
			SPDLOG_INFO("");
			return;
		}

		//Create watch thread
		m_futureWatch = CThreadPool::GetInstance()->enqueue(&CDirectoryWatcher::WatchDirectory, this);
	}
	catch (std::exception& e) {
		SPDLOG_INFO("");
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
				SPDLOG_INFO("");
			}
			//Close handle
			m_pQuitEvent.reset();
		}
		//Work
		//Release thread
		if (m_futureWatch.valid() /*&& m_futureWatch.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready*/) {
			m_futureWatch.get();
		}
		//Path
		m_path.clear();
		m_absIdl.Clear();
	}
	catch (std::exception& e) {
		SPDLOG_INFO("");
		throw e;
	}

}

void CDirectoryWatcher::WatchDirectory()
{
	try{
		SPDLOG_INFO("Start CDirectoryWatcher::WatchDirectoryCallback");

		//Create File
		m_names = GetFileNamesInDirectory(m_absIdl);
		std::sort(m_names.begin(), m_names.end());
		UniqueHandlePtr pDir(::CreateFileW(m_path.c_str(),
								FILE_LIST_DIRECTORY,
								FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
								NULL,
								OPEN_EXISTING,
								FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
								NULL));
		if(pDir.get() == INVALID_HANDLE_VALUE){
			throw std::exception(FILE_LINE_FUNC);//TODOLOW
		}

		//Associate IoComp with hDir
		UniqueHandlePtr pIocp(::CreateIoCompletionPort(pDir.get(), NULL, COMPKEY_DIR, 0));
		if (pIocp.get() == INVALID_HANDLE_VALUE) {
			throw std::exception(FILE_LINE_FUNC);//TODOLOW
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

		SPDLOG_INFO("End CDirectoryWatcher::WatchDirectoryCallback");
	}catch(std::exception&){
		SPDLOG_INFO("");
	}
}

std::vector<std::wstring> CDirectoryWatcher::GetFileNamesInDirectory(CIDL absIdl)
{
	std::vector<std::wstring> names;

	CComPtr<IShellFolder> pFolder = shell::DesktopBindToShellFolder(absIdl);
	shell::for_each_idl_in_shellfolder(m_pControl->GetWndPtr()->m_hWnd, pFolder, [&names, &pFolder](const CIDL& idl) {
		std::wstring path = shell::GetDisplayNameOf(pFolder, idl, SHGDN_FORPARSING);
		if (!path.empty() && path[0] != L':') {
			names.emplace_back(::PathFindFileName(path.c_str()));
		}
	});
	return names;
}

void CDirectoryWatcher::IoCompletionCallback(HANDLE hIocp, HANDLE hDir)
{
	try{
		SPDLOG_INFO("Start CDirectoryWatcher::IoCompletionCallback");

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
				SPDLOG_INFO("ReadDirectoryChangesW");
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
				//Scan
				//::Sleep(100);
				std::vector<std::wstring> fileNames = GetFileNamesInDirectory(m_absIdl);
				std::sort(fileNames.begin(), fileNames.end());
				//Diff
				std::vector<std::wstring> addDiff, remDiff;
				std::set_difference(fileNames.begin(), fileNames.end(), m_names.begin(), m_names.end(), std::back_inserter(addDiff));
				std::set_difference(m_names.begin(), m_names.end(), fileNames.begin(), fileNames.end(), std::back_inserter(remDiff));

				auto pInfo = pFileNotifyInfo;
				std::vector<std::pair<DWORD, std::wstring>> infos;
				auto oldIter = remDiff.end();
				while (true) {
					std::wstring fileName;
					memcpy(::GetBuffer(fileName, pInfo->FileNameLength / sizeof(wchar_t)), pInfo->FileName, pInfo->FileNameLength);
					::ReleaseBuffer(fileName);

					switch (pInfo->Action) {
					case FILE_ACTION_ADDED:
						SPDLOG_INFO("FILE_ACTION_ADDED");
						{
							auto iter = std::find(addDiff.begin(), addDiff.end(), fileName);
							if (iter != addDiff.end()) {
								infos.emplace_back(FILE_ACTION_ADDED, fileName);
								addDiff.erase(iter);
							}
							break;
						}
					case FILE_ACTION_MODIFIED:
						SPDLOG_INFO("FILE_ACTION_MODIFIED");
						{
							auto iter = std::find(fileNames.begin(), fileNames.end(), fileName);
							if (iter != fileNames.end()) {
								infos.emplace_back(FILE_ACTION_MODIFIED, fileName);
							}
							break;
						}
						break;
					case FILE_ACTION_REMOVED:
						SPDLOG_INFO("FILE_ACTION_REMOVED");
						{
							auto iter = std::find(remDiff.begin(), remDiff.end(), fileName);
							if (iter != remDiff.end()) {
								infos.emplace_back(FILE_ACTION_REMOVED, fileName);
								remDiff.erase(iter);
							}
							break;
						}
						break;
					case FILE_ACTION_RENAMED_NEW_NAME:
						SPDLOG_INFO("FILE_ACTION_RENAMED_NEW_NAME");
						{
							auto newIter = std::find(addDiff.begin(), addDiff.end(), fileName);
							if (newIter != addDiff.end() && oldIter != remDiff.end()) {
								infos.emplace_back(FILE_ACTION_RENAMED_NEW_NAME, *oldIter + L"/" + *newIter);
								addDiff.erase(newIter);
								remDiff.erase(oldIter);
							} else {
								SPDLOG_INFO("");
							}
							oldIter = remDiff.end();
						}
						break;
					case FILE_ACTION_RENAMED_OLD_NAME:
						SPDLOG_INFO("FILE_ACTION_RENAMED_OLD_NAME");
						{
							oldIter = std::find(remDiff.begin(), remDiff.end(), fileName);
						}
						break;
					default:
						break;
					}

					if (pInfo->NextEntryOffset == 0) { break; }

					pInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<PBYTE>(pInfo) + pInfo->NextEntryOffset);
				}

				//Check if there is mis-catch
				if (!addDiff.empty()) {
					for (const auto& add : addDiff) {
						infos.emplace_back(FILE_ACTION_ADDED, add);
					}
				}

				if (!remDiff.empty()) {
					for (const auto& rem : remDiff) {
						infos.emplace_back(FILE_ACTION_REMOVED, rem);
					}
				}

				//std::sort(infos.begin(), infos.end());
				//infos.erase(std::unique(infos.begin(), infos.end()), infos.end());
				if (!infos.empty()) {
					//::SendMessage(m_hWnd, WM_DIRECTORYWATCH, (WPARAM)&infos, NULL);
					m_callback(DirectoryWatchEvent(infos));
				}
				delete[](PBYTE)pFileNotifyInfo;
				m_names = fileNames;
			}
		}
	}catch(std::exception&){
		SPDLOG_INFO("");
	}
}

