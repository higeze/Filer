#pragma once
#include "MyString.h"
#include "MyIcon.h"
#include "IDLPtr.h"
#include "FileIconCache.h"
#include <future>
#include <chrono>

class CShellFolder;

tstring FileTime2String(FILETIME *pFileTime);
tstring Size2String(ULONGLONG size);
std::wstring ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit = 3);
bool GetFileSize(CComPtr<IShellFolder>& parentFolder, CIDLPtr childIDL, ULARGE_INTEGER& size);
//bool GetFolderSize(std::shared_ptr<CShellFolder>& pFolder, ULARGE_INTEGER& size, std::function<void()> checkExit);
bool GetFolderSize(std::shared_ptr<CShellFolder>& pFolder, ULARGE_INTEGER& size, std::shared_future<void> future);
bool GetDirSize(std::wstring path, ULARGE_INTEGER& size, std::function<void()> cancel);

struct findclose
{
	void operator()(HANDLE handle)const
	{
		if (!::FindClose(handle)) {
			//FILE_LINE_FUNC_TRACE;
		}
	}

};

enum class FileSizeStatus
{
	None,
	Available,
	Calculating,
	Unavailable,
};

enum class FileIconStatus
{
	None,
	Avilable,
	Loading,
};

class CShellFile: public std::enable_shared_from_this<CShellFile>
{
private:
	static CFileIconCache s_iconCache;
protected:
	CComPtr<IShellFolder> m_parentFolder;
	CIDLPtr m_absolutePidl;

	std::wstring m_wstrPath;
	std::wstring m_wstrName;
	std::wstring m_wstrExt;
	std::wstring m_wstrNameExt;
	std::wstring m_wstrType;
	std::wstring m_wstrCreationTime;
	std::wstring m_wstrLastAccessTime;
	std::wstring m_wstrLastWriteTime;

	boost::optional<bool> m_isShellFolder = boost::none;

	std::pair<ULARGE_INTEGER, FileSizeStatus> m_size = std::make_pair(ULARGE_INTEGER(), FileSizeStatus::None);
	std::pair<std::shared_ptr<CIcon>, FileIconStatus> m_icon = std::make_pair(std::shared_ptr<CIcon>(nullptr), FileIconStatus::None);

	DWORD  m_fileAttributes = 0;
	ULONG m_sfgao = 0;

	std::unique_ptr<std::thread> m_pSizeThread;
	std::promise<void> m_sizePromise;
	std::shared_future<void> m_sizeFuture;

	std::unique_ptr<std::thread> m_pIconThread;
	std::promise<void> m_iconPromise;
	std::shared_future<void> m_iconFuture;

	std::mutex m_mtxSize;
	std::mutex m_mtxIcon;


public:
	//Constructor
	CShellFile();
	CShellFile(CComPtr<IShellFolder> pfolder, CIDLPtr absolutePidl);
	CShellFile(const std::wstring& path);

	//Destructor
	virtual ~CShellFile();

	//Signal
	boost::signals2::signal<void(std::weak_ptr<CShellFile>)> SignalFileSizeChanged;
	boost::signals2::signal<void(std::weak_ptr<CShellFile>)> SignalFileIconChanged;
	
	//Getter 
	CComPtr<IShellFolder>& GetParentShellFolderPtr(){return m_parentFolder;}
	CIDLPtr& GetAbsolutePidl(){return m_absolutePidl;}
//	bool GetCancelSizeThread(){ return m_cancelSizeThread.load(); }
//	bool GetCancelIconThread(){ return m_cancelIconThread.load(); }

	//Lazy Evaluation Getter
	std::wstring& GetPath();
	std::wstring GetName();
	std::wstring GetExt();
	std::wstring GetNameExt();
	std::wstring GetTypeName();	
	std::wstring GetCreationTime();
	std::wstring GetLastAccessTime();
	std::wstring GetLastWriteTime();
	UINT GetSFGAO();
	DWORD GetAttributes();

	//Size
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize();
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetLockSize();
	void SetLockSize(std::pair<ULARGE_INTEGER, FileSizeStatus>& size);

	//Icon
	std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon();
	std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetLockIcon();
	void SetLockIcon(std::pair<std::shared_ptr<CIcon>, FileIconStatus>& icon);

	bool HasIconInCache();
	bool IsDirectory();
	bool IsShellFolder();



	void Reset();
	std::shared_ptr<CShellFolder> CastShellFolder();
private:
	void UpdateWIN32_FIND_DATA();
	std::shared_ptr<CIcon> GetDefaultIcon();
};