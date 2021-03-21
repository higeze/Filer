#pragma once
#include "IDL.h"
#include "Debug.h"
#include <future>

struct closehandle
{
	void operator()(HANDLE handle)const
	{
		if (!::CloseHandle(handle)) {
			LOG_THIS_1("");
		}
	}

};

class CD2DWControl;

struct DirectoryWatchEvent
{
	std::vector<std::pair<DWORD, std::wstring>> Infos;
	DirectoryWatchEvent(const std::vector<std::pair<DWORD, std::wstring>>& infos):Infos(infos){}
};

class CDirectoryWatcher
{
private:
	using UniqueHandlePtr = std::unique_ptr<std::remove_pointer<HANDLE>::type, closehandle>;
	const size_t kBufferSize = 1024;
	const ULONG_PTR COMPKEY_QUIT = -1;
	const ULONG_PTR COMPKEY_DIR = 1;

private:
	CD2DWControl* m_pControl;
	std::function<void(const DirectoryWatchEvent&)> m_callback;

	UniqueHandlePtr m_pQuitEvent;
	std::future<void> m_futureWatch;
	std::wstring m_path;
	CIDL m_absIdl;
	std::vector<BYTE> m_vData;

	std::vector<std::wstring> GetFileNamesInDirectory(CIDL absIdl);
	std::vector<std::wstring> m_names;

public:
	CDirectoryWatcher(CD2DWControl* pControl, std::function<void(const DirectoryWatchEvent&)> callback);
	~CDirectoryWatcher(void);

	std::wstring GetPath()const { return m_path; }

	boost::signals2::signal<void(const std::wstring&)> Added;
	boost::signals2::signal<void(const std::wstring&)> Removed;
	boost::signals2::signal<void(const std::wstring&)> Modified;
	boost::signals2::signal<void(const std::wstring&, const std::wstring&)> Renamed;

	void StartWatching(const std::wstring& path, const CIDL& absIdl);
	void QuitWatching();
	void WatchDirectory();
	void IoCompletionCallback(HANDLE hIocp, HANDLE hDir);
};

