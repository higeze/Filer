#include "stdafx.h"
#include "DirectoryWatcher.h"
#include "SEHException.h"
#include "MyWin32.h"
#include "MyString.h"
#include "Debug.h"


CDirectoryWatcher::CDirectoryWatcher(void)
	:m_quitEvent(), m_dir(), m_work(), m_vData(kBufferSize, 0)
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
		if (m_quitEvent) {
			//Throw quit event
			if (!::SetEvent(m_quitEvent.get())) {
				FILE_LINE_FUNC_TRACE;
			}
			//Close handle
			m_quitEvent.reset();
		}
		if (m_work) {
			//Wait for submitted work
			::WaitForThreadpoolWorkCallbacks(m_work.get(), FALSE);
			//Close
			m_work.reset();
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
		_tprintf(TEXT("IoCompletionCallback\n"));
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
		_tprintf(TEXT("Start watching direcotry\n"));
		//Start observing
		IoCompletionCallback(NULL,(LPOVERLAPPED)&oi,NO_ERROR,0,pio.get());
		//Wait for quit event
		WaitForSingleObject(m_quitEvent.get(),INFINITE);
		//Cancel task in que
		WaitForThreadpoolIoCallbacks(pio.get(),TRUE);
		//Close handle
		m_dir.reset();
		_tprintf(TEXT("End watching directory\n"));
	}catch(std::exception& ex){
		FILE_LINE_FUNC_TRACE;
		m_dir.reset();
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
		_tprintf(TEXT("IoCompletionCallback\n"));
		PFILE_NOTIFY_INFORMATION pFileNotifyInfo = nullptr;
		if(ulBytes > 0)
		{
			PFILE_NOTIFY_INFORMATION pfni = (PFILE_NOTIFY_INFORMATION)m_vData.data();
			SIZE_T cb = sizeof(FILE_NOTIFY_INFORMATION) + pfni->FileNameLength;
			pFileNotifyInfo = (PFILE_NOTIFY_INFORMATION)new BYTE[cb];
			CopyMemory((PVOID)pFileNotifyInfo,(CONST PVOID)pfni,cb);
		}
		//Keep watching
		if(pOverlapped)
		{
			_tprintf(TEXT("ReadDirectoryChangesW\n"));
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
		if(ulBytes > 0)
		{
			//_bstr_t filename,fullpath;
			//*(PWSTR)((PBYTE)(pFileNotifyInfo->FileName) + pFileNotifyInfo->FileNameLength) = L'\0';
			//filename = pFileNotifyInfo->FileName;
			//fullpath = m_path.c_str();
			//fullpath += (_bstr_t(TEXT("\\")) + filename);
			//_tprintf(TEXT("[THREAD:%d] [Changed]%s\n"),GetCurrentThreadId(),(PCTSTR)fullpath);
			Changed();
			delete [] (PBYTE)pFileNotifyInfo;
		}
	}catch(std::exception& ex){
		FILE_LINE_FUNC_TRACE;
		m_dir.get();
		QuitWatching();
	}
}

