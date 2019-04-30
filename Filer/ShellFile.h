#pragma once
#include "MyString.h"
#include "MyIcon.h"
#include "IDL.h"
//#include "FileIconCache.h"
#include <future>
#include <chrono>

class CShellFolder;
struct FileSizeArgs;
struct FileTimeArgs;


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

enum class FileTimeStatus
{
	None,
	Available,
	AvailableLoading,
	Loading,
	Unavailable,
};

class CShellFile: public std::enable_shared_from_this<CShellFile>
{
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

	std::pair<FILETIME, FileTimeStatus> m_creationTime = std::make_pair(FILETIME{ 0 }, FileTimeStatus::None);
	std::pair<FILETIME, FileTimeStatus> m_lastAccessTime = std::make_pair(FILETIME{ 0 }, FileTimeStatus::None);
	std::pair<FILETIME, FileTimeStatus> m_lastWriteTime = std::make_pair(FILETIME{ 0 }, FileTimeStatus::None);
	std::pair<ULARGE_INTEGER, FileSizeStatus> m_size = std::make_pair(ULARGE_INTEGER(), FileSizeStatus::None);
	std::pair<std::shared_ptr<CIcon>, FileIconStatus> m_icon = std::make_pair(std::shared_ptr<CIcon>(nullptr), FileIconStatus::None);

	DWORD  m_fileAttributes = 0;
	ULONG m_sfgao = 0;

	std::unique_ptr<std::thread> m_pIconThread;
	std::mutex m_mtxIcon;
public:
	//Constructor
	CShellFile() {}
	CShellFile(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl);
	//CShellFile(const std::wstring& path);

	//Destructor
	virtual ~CShellFile();

	//Signal
	//boost::signals2::signal<void(CShellFile*)> SignalFileIconChanged;
	
	//Getter 
	CComPtr<IShellFolder>& GetParentShellFolderPtr(){return m_pParentShellFolder;}
	CIDL& GetAbsoluteIdl() { return m_absoluteIdl; }
	CIDL& GetChildIdl() { return m_childIdl; }

	//Lazy Evaluation Getter
	virtual std::wstring& GetPath();
	virtual std::wstring GetFileNameWithoutExt();
	virtual std::wstring GetFileName();
	virtual std::wstring GetExt();
	std::wstring GetTypeName();	
	//std::wstring GetCreationTime();
	//std::wstring GetLastAccessTime();
	UINT GetSFGAO();
	DWORD GetAttributes();

	virtual void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt);
	virtual void SetExt(const std::wstring& wstrExt);

	//LastWrite
	bool GetFileLastWriteTime(FILETIME& time);
	static bool GetFileLastWriteTime(FILETIME& time, const CComPtr<IShellFolder>& pParentFolder, const CIDL& relativeIdl);
	virtual std::pair<FILETIME, FileTimeStatus> GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed = nullptr);

	//Size
	bool GetFileSize(ULARGE_INTEGER& size/*, std::shared_future<void> future*/);
	static bool GetFileSize(ULARGE_INTEGER& size, const CComPtr<IShellFolder>& pParentShellFolder, const CIDL& childIdl);
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed = nullptr);
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> ReadSize();

	//Icon
	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction);

	bool HasIconInCache();
	bool IsDirectory();
	virtual void Reset();

private:
	void UpdateWIN32_FIND_DATA();
	virtual void ResetIcon();
	virtual void ResetSize();

protected:
	std::shared_ptr<CIcon> GetDefaultIcon();
	std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetLockIcon();
	void SetLockIcon(std::pair<std::shared_ptr<CIcon>, FileIconStatus>& icon);
};


class CShellInvalidFile :public CShellFile
{
public:
	//Constructor
	CShellInvalidFile() {}

	//Destructor
	virtual ~CShellInvalidFile(){}

	//Icon
	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction)override;
};

