//#include "stdafx.h"
#include "DirectoryWatcher.h"
#include "SEHException.h"
#include "MyWin32.h"
#include "MyString.h"


CDirectoryWatcher::CDirectoryWatcher(HWND hWnd)
	:m_hWnd(hWnd), m_quitEvent(), m_dir(), m_work(), m_vData(kBufferSize, 0)
{}

CDirectoryWatcher::~CDirectoryWatcher(void)
{
	QuitWatching();
}

void CDirectoryWatcher::StartWatching()
{
	try {
		//Create event
		m_quitEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL));
		if (!m_quitEvent) {
			FILE_LINE_FUNC_TRACE;
			return;
		}
		//Create work
		m_work.reset(::CreateThreadpoolWork(CDirectoryWatcher::WatchDirectoryCallback, (PVOID)this, NULL));
		if (!m_work) {
			FILE_LINE_FUNC_TRACE;
			return;
		}
		//Submit work
		::SubmitThreadpoolWork(m_work.get());
	}
	catch (std::exception& e) {
		FILE_LINE_FUNC_TRACE;
		throw e;
	}
}

void CDirectoryWatcher::QuitWatching()
{
	try {
		//Event
		if (m_quitEvent) {
			//Throw quit event
			if (!::SetEvent(m_quitEvent.get())) {
				FILE_LINE_FUNC_TRACE;
			}
			//Close handle
			m_quitEvent.reset();
		}
		//Work
		if (m_work) {
			//Wait for submitted work
			::WaitForThreadpoolWorkCallbacks(m_work.get(), TRUE);
			//Close
			m_work.reset();
		}
		//Dir
		if (m_dir) {
			if (!::CancelIo(m_dir.get())) {
				FILE_LINE_FUNC_TRACE;
			}
			m_dir.reset();
		}
	}
	catch (std::exception& e) {
		FILE_LINE_FUNC_TRACE;
		throw e;
	}

}

VOID CALLBACK CDirectoryWatcher::WatchDirectoryCallback(PTP_CALLBACK_INSTANCE pInstance,LPVOID pvParam,PTP_WORK pWork)
{
	//Catch SEH exception as CEH
	_set_se_translator (CSEHException::TransferSEHtoCEH);

	auto that = reinterpret_cast<CDirectoryWatcher*>(pvParam);
	return that->WatchDirectoryCallback(pInstance, pWork);
}

void CDirectoryWatcher::WatchDirectoryCallback(PTP_CALLBACK_INSTANCE pInstance,PTP_WORK pWork)
{
	try{
		BOOST_LOG_TRIVIAL(trace) << "IoCompletionCallback";
		//PCTSTR pDir = (PCTSTR)pvParam;
		OVERLAPPED oi = {0};

		//Create File
		m_dir.reset(CreateFile(m_path.c_str(),
								FILE_LIST_DIRECTORY,
								FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
								NULL,
								OPEN_EXISTING,
								FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
								NULL));
		if(m_dir.get() == INVALID_HANDLE_VALUE){
			throw std::exception("CreateFile == INVALID_HANDLE_VALUE.\n");
			return;
		}
		//Create thread pool
		UniqueIOPtr  pio(::CreateThreadpoolIo(m_dir.get(),CDirectoryWatcher::IoCompletionCallback,(PVOID)this,NULL));
		BOOST_LOG_TRIVIAL(trace) << "Start watching direcotry";
		//Start observing
		IoCompletionCallback(NULL,(LPOVERLAPPED)&oi,NO_ERROR,0,pio.get());
		//Wait for quit event
		WaitForSingleObject(m_quitEvent.get(),INFINITE);
		//Cancel task in que
		WaitForThreadpoolIoCallbacks(pio.get(),TRUE);

		BOOST_LOG_TRIVIAL(trace) << "End watching directory";
	}catch(std::exception&){
		FILE_LINE_FUNC_TRACE;
		QuitWatching();
	}
}

VOID CALLBACK CDirectoryWatcher::IoCompletionCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvParam,PVOID pOverlapped,ULONG IoResult,ULONG_PTR ulBytes,PTP_IO pio)
{
	//Catch SEH exception as CEH
	_set_se_translator (CSEHException::TransferSEHtoCEH);

	auto that = reinterpret_cast<CDirectoryWatcher*>(pvParam);
	return that->IoCompletionCallback(pInstance, pOverlapped, IoResult, ulBytes, pio);

}

void CDirectoryWatcher::IoCompletionCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pOverlapped,ULONG IoResult,ULONG_PTR ulBytes,PTP_IO pio)
{
	try{
		BOOST_LOG_TRIVIAL(trace) << "IoCompletionCallback";
		PFILE_NOTIFY_INFORMATION pFileNotifyInfo = nullptr;
		if(ulBytes > 0)
		{
			pFileNotifyInfo = (PFILE_NOTIFY_INFORMATION)new BYTE[ulBytes];
			::CopyMemory((PVOID)pFileNotifyInfo,(CONST PVOID)m_vData.data(),ulBytes);
		}
		//Keep watching
		if(pOverlapped)
		{
			BOOST_LOG_TRIVIAL(trace) << "ReadDirectoryChangesW";
			//Associate iocompletionobject to thread pool
			StartThreadpoolIo(pio);
			if(!ReadDirectoryChangesW(
				m_dir.get(),
				m_vData.data(),
				m_vData.size(),
				FALSE,
				FILE_NOTIFY_CHANGE_FILE_NAME   | 
				FILE_NOTIFY_CHANGE_DIR_NAME    | 
				FILE_NOTIFY_CHANGE_ATTRIBUTES  |  
				FILE_NOTIFY_CHANGE_SIZE        |  
				FILE_NOTIFY_CHANGE_LAST_WRITE  |
				FILE_NOTIFY_CHANGE_LAST_ACCESS |
				FILE_NOTIFY_CHANGE_CREATION    |
				FILE_NOTIFY_CHANGE_SECURITY,    
				NULL,
				(LPOVERLAPPED)pOverlapped,
				NULL)){
				throw std::exception(
					("ReadDirectoryChangesW FAILED\r\n"  
					  "Last Error:" + GetLastErrorString() +
					  "Path:" + wstr2str(m_path) + "\r\n").c_str());
				return;
			}
		}
		if (ulBytes == 0) {
			FILE_LINE_FUNC_TRACE;
		}else if(ulBytes > 0){

			::SendMessage(m_hWnd, WM_DIRECTORYWATCH, (WPARAM)pFileNotifyInfo, NULL);
			//FILE_NOTIFY_INFORMATION* pInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(m_vData.data());
			//while (true) {
			//	std::wstring oldName;
			//	std::wstring fileName;
			//	memcpy(::GetBuffer(fileName, pInfo->FileNameLength / sizeof(wchar_t)), pInfo->FileName, pInfo->FileNameLength);

			//	switch (pInfo->Action) {
			//	case FILE_ACTION_ADDED:
			//		Added(fileName);
			//		break;
			//	case FILE_ACTION_MODIFIED:
			//		Modified(fileName);
			//		break;
			//	case FILE_ACTION_REMOVED:
			//		Removed(fileName);
			//		break;
			//	case FILE_ACTION_RENAMED_NEW_NAME:
			//		if (!oldName.empty()) {
			//			Renamed(oldName, fileName);
			//			oldName.clear();
			//		}
			//		else {
			//			FILE_LINE_FUNC_TRACE;
			//		}
			//		break;
			//	case FILE_ACTION_RENAMED_OLD_NAME:
			//		oldName = fileName;
			//	default:
			//		break;
			//	}

			//	if (pInfo->NextEntryOffset == 0) { break; }
			//	
			//	pInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<unsigned char*>(pInfo) + pInfo->NextEntryOffset);
			//}


			
			
			//_bstr_t filename,fullpath;
			//*(PWSTR)((PBYTE)(pFileNotifyInfo->FileName) + pFileNotifyInfo->FileNameLength) = L'\0';
			//filename = pFileNotifyInfo->FileName;
			//fullpath = m_path.c_str();
			//fullpath += (_bstr_t(TEXT("\\")) + filename);
			//_tprintf(TEXT("[THREAD:%d] [Changed]%s\n"),GetCurrentThreadId(),(PCTSTR)fullpath);
			//Changed();
			//delete [] (PBYTE)pFileNotifyInfo;
		}
	}catch(std::exception&){
		FILE_LINE_FUNC_TRACE;
		m_work.reset();
		QuitWatching();
	}
}

