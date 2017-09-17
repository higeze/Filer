#pragma once
#include "stdafx.h"

#define BUFFER_SIZE 1024

//typedef struct _IocpData : OVERLAPPED
//{
//	TCHAR szDir[MAX_PATH];
//	HANDLE hDir;
//	std::vector<BYTE> vData[BUFFER_SIZE];
//} IocpData,*PIOCPDATA;

class CDirectoryWatcher
{
private:
	HANDLE m_hQuitEvent;
	HANDLE m_hDir;
	PTP_WORK m_pWork;

	std::wstring m_path;
	std::vector<BYTE> m_vData;


public:
	CDirectoryWatcher(void);
	~CDirectoryWatcher(void);

	boost::signals2::signal<void(void)> Changed;

	static VOID CALLBACK WatchDirectoryCallback(PTP_CALLBACK_INSTANCE pInstance,LPVOID pvParam,PTP_WORK pWork);
	static VOID CALLBACK IoCompletionCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvParam,PVOID pOverlapped,ULONG IoResult,ULONG_PTR ulBytes,PTP_IO pio);
	void WatchDirectoryCallback(PTP_CALLBACK_INSTANCE pInstance,PTP_WORK pWork);
	void IoCompletionCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pOverlapped,ULONG IoResult,ULONG_PTR ulBytes,PTP_IO pio);

	void SetPath(const std::wstring& path){m_path = path;}
	std::wstring GetPath()const{return m_path;}
	void StartWatching();
	void QuitWatching();
};

