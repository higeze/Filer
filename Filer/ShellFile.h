#pragma once
#include "MyString.h"
#include "MyIcon.h"
#include "IDL.h"
//#include "FileIconCache.h"
#include <future>
#include <chrono>

class CShellFolder;

tstring FileTime2String(FILETIME *pFileTime);
tstring Size2String(ULONGLONG size);
std::wstring ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit = 3);
//bool GetFileSize(CComPtr<IShellFolder>& parentFolder, CIDL childIDL, ULARGE_INTEGER& size);
//bool GetFolderSize(std::shared_ptr<CShellFolder>& pFolder, ULARGE_INTEGER& size, std::function<void()> checkExit);
//bool GetFolderSize(std::shared_ptr<CShellFolder>& pFolder, ULARGE_INTEGER& size, std::shared_future<void> future);
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
	Available,
	Loading,
};

class CShellFile: public std::enable_shared_from_this<CShellFile>
{
private:
//	static CFileIconCache s_iconCache;
protected:
	CComPtr<IShellFolder> m_pParentShellFolder;
	CIDL m_absoluteIdl;
	CIDL m_parentIdl;
	CIDL m_childIdl;

	std::wstring m_wstrPath;
	std::wstring m_wstrFileName;
	std::wstring m_wstrFileNameWithoutExt;
	std::wstring m_wstrExt;
	std::wstring m_wstrType;
	std::wstring m_wstrCreationTime;
	std::wstring m_wstrLastAccessTime;
	std::wstring m_wstrLastWriteTime;

	std::pair<ULARGE_INTEGER, FileSizeStatus> m_size = std::make_pair(ULARGE_INTEGER(), FileSizeStatus::None);
	std::pair<std::shared_ptr<CIcon>, FileIconStatus> m_icon = std::make_pair(std::shared_ptr<CIcon>(nullptr), FileIconStatus::None);

	DWORD  m_fileAttributes = 0;
	ULONG m_sfgao = 0;

	std::unique_ptr<std::thread> m_pIconThread;
	//std::promise<void> m_iconPromise;
	//std::shared_future<void> m_iconFuture;
	std::mutex m_mtxIcon;


public:
	//Constructor
	CShellFile() {}
	CShellFile(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl);
	//CShellFile(const std::wstring& path);

	//Destructor
	virtual ~CShellFile();

	//Signal
	boost::signals2::signal<void(CShellFile*)> SignalFileIconChanged;
	
	//Getter 
	CComPtr<IShellFolder>& GetParentShellFolderPtr(){return m_pParentShellFolder;}
	CIDL& GetAbsoluteIdl();
	CIDL& GetChildIdl();

	//Lazy Evaluation Getter
	virtual std::wstring& GetPath();
	virtual std::wstring GetFileNameWithoutExt();
	virtual std::wstring GetFileName();
	virtual std::wstring GetExt();
	std::wstring GetTypeName();	
	std::wstring GetCreationTime();
	std::wstring GetLastAccessTime();
	std::wstring GetLastWriteTime();
	UINT GetSFGAO();
	DWORD GetAttributes();

	void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt);
	void SetExt(const std::wstring& wstrExt);

	//Size
	bool GetFileSize(ULARGE_INTEGER& size/*, std::shared_future<void> future*/);
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize();
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> ReadSize();

	//Icon
	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon();

	bool HasIconInCache();
	bool IsDirectory();
	virtual void Reset();
private:
	void UpdateWIN32_FIND_DATA();
	std::shared_ptr<CIcon> GetDefaultIcon();

	virtual void ResetIcon();
	virtual void ResetSize();

protected:
	std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetLockIcon();
	void SetLockIcon(std::pair<std::shared_ptr<CIcon>, FileIconStatus>& icon);
};