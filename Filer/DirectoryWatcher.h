#pragma once
#include "stdafx.h"
#include "Debug.h"
//#define BUFFER_SIZE 1024

//typedef struct _IocpData : OVERLAPPED
//{
//	TCHAR szDir[MAX_PATH];
//	HANDLE hDir;
//	std::vector<BYTE> vData[BUFFER_SIZE];
//} IocpData,*PIOCPDATA;

struct closethreadpoolio
{
	void operator()(PTP_IO pio)const
	{
		::CloseThreadpoolIo(pio);
	}
};

struct closehandle
{
	void operator()(HANDLE handle)const
	{
		if (!::CloseHandle(handle)) {
			FILE_LINE_FUNC_TRACE;
		}
	}

};

struct closethreadpoolwork
{
	void operator()(PTP_WORK pwork)const
	{
		::CloseThreadpoolWork(pwork);
	}
};

const UINT WM_DIRECTORYWATCH = RegisterWindowMessage(L"WM_DIRECTORYWATCH");

class CDirectoryWatcher
{
private:
	using UniqueHandlePtr = std::unique_ptr<std::remove_pointer<HANDLE>::type, closehandle>;
	using UniqueWorkPtr = std::unique_ptr<std::remove_pointer<PTP_WORK>::type, closethreadpoolwork>;
	using UniqueIOPtr = std::unique_ptr<std::remove_pointer<PTP_IO>::type, closethreadpoolio>;
	const size_t kBufferSize = 1024;

private:
	HWND m_hWnd;

	UniqueHandlePtr m_quitEvent;
	UniqueHandlePtr m_dir;
	UniqueWorkPtr m_work;

	std::wstring m_path;
	std::vector<BYTE> m_vData;

public:
	CDirectoryWatcher(HWND m_hWnd);
	~CDirectoryWatcher(void);

	//boost::signals2::signal<void(void)> Changed;
	boost::signals2::signal<void(const std::wstring&)> Added;
	boost::signals2::signal<void(const std::wstring&)> Removed;
	boost::signals2::signal<void(const std::wstring&)> Modified;
	boost::signals2::signal<void(const std::wstring&, const std::wstring&)> Renamed;




	static VOID CALLBACK WatchDirectoryCallback(PTP_CALLBACK_INSTANCE pInstance,LPVOID pvParam,PTP_WORK pWork);
	static VOID CALLBACK IoCompletionCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvParam,PVOID pOverlapped,ULONG IoResult,ULONG_PTR ulBytes,PTP_IO pio);
	void WatchDirectoryCallback(PTP_CALLBACK_INSTANCE pInstance,PTP_WORK pWork);
	void IoCompletionCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pOverlapped,ULONG IoResult,ULONG_PTR ulBytes,PTP_IO pio);

	void SetPath(const std::wstring& path){m_path = path;}
	std::wstring GetPath()const{return m_path;}
	void StartWatching();
	void QuitWatching();
};

