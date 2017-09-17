#include "stdafx.h"
#include "DirectoryWatcher.h"
#include "SEHException.h"
#include "MyWin32.h"
#include "MyString.h"


CDirectoryWatcher::CDirectoryWatcher(void):m_hQuitEvent(NULL),m_pWork(nullptr),m_hDir(NULL),m_vData(BUFFER_SIZE, 0)
{
}


CDirectoryWatcher::~CDirectoryWatcher(void)
{
}

void CDirectoryWatcher::StartWatching()
{
	//Create event
	if(m_hQuitEvent != NULL)::CloseHandle(m_hQuitEvent);
	m_hQuitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	//Create work
	if(m_pWork != nullptr)::CloseThreadpoolWork(m_pWork);
	m_pWork = ::CreateThreadpoolWork(CDirectoryWatcher::WatchDirectoryCallback,(PVOID)this,NULL);
	//Submit work
	::SubmitThreadpoolWork(m_pWork);
}

void CDirectoryWatcher::QuitWatching()
{
	if(m_hQuitEvent!=NULL && m_pWork!=nullptr){
		//Throw quit event
		::SetEvent(m_hQuitEvent);
		//Wait for submitted work
		::WaitForThreadpoolWorkCallbacks(m_pWork,FALSE);
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
		m_hDir = CreateFile(m_path.c_str(),
								FILE_LIST_DIRECTORY,
								FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
								NULL,
								OPEN_EXISTING,
								FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
								NULL);
		if(m_hDir == INVALID_HANDLE_VALUE){
			throw std::exception("CreateFile == INVALID_HANDLE_VALUE.\n");
			return;
		}
		//Create thread pool
		PTP_IO pio = ::CreateThreadpoolIo(m_hDir,CDirectoryWatcher::IoCompletionCallback,(PVOID)this,NULL);
		_tprintf(TEXT("Start watching direcotry\n"));
		//Start observing
		IoCompletionCallback(NULL,(LPOVERLAPPED)&oi,NO_ERROR,0,pio);
		//Wait for quit event
		WaitForSingleObject(m_hQuitEvent,INFINITE);
		//Cancel task in que
		WaitForThreadpoolIoCallbacks(pio,TRUE);
		//Close handle
		::CloseHandle(m_hQuitEvent);m_hQuitEvent = NULL;
		::CloseThreadpoolWork(m_pWork);m_pWork = nullptr;
		::CloseHandle(m_hDir);m_hDir = NULL;
		::CloseThreadpoolIo(pio);
		_tprintf(TEXT("End watching directory\n"));
	}catch(std::exception& ex){
		std::cout<<"Exception CDirectoryWatcher::WatchDirectoryCallback"<<std::endl;
		MessageBoxA(NULL, ex.what(), "Exception in WatchDirectoryCallback", MB_ICONWARNING);
		::CloseHandle(m_hQuitEvent);m_hQuitEvent = NULL;
		::CloseThreadpoolWork(m_pWork);m_pWork = nullptr;
		::CloseHandle(m_hDir);m_hDir = NULL;
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
				m_hDir,
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
		std::cout<<"Exception CDirectoryWatcher::IoCompletionCallback"<<std::endl;
		MessageBoxA(NULL, ex.what(), "Exception in IoCompletionCallback", MB_ICONWARNING);
		if(m_hQuitEvent)::SetEvent(m_hQuitEvent);
	}
}

