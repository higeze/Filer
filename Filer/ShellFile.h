#pragma once
#include "MyString.h"
#include "MyIcon.h"
#include "IDLPtr.h"
#include "FileIconCache.h"

class CShellFolder;

tstring FileTime2String(FILETIME *pFileTime);
tstring Size2String(ULONGLONG size);
std::wstring ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit = 3);
bool GetDirSize(std::wstring path, ULONGLONG *pSize);

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

class CShellFile
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

	std::unique_ptr<std::thread> m_pSizeThread = nullptr;
	std::atomic<bool> m_cancelSizeThread = false;

	std::unique_ptr<std::thread> m_pIconThread = nullptr;
	std::atomic<bool> m_cancelIconThread = false;

	//std::mutex m_mtxSize;


public:
	//Constructor
	CShellFile();
	CShellFile(CComPtr<IShellFolder> pfolder, CIDLPtr absolutePidl);
	CShellFile(const std::wstring& path);

	//Destructor
	virtual ~CShellFile();

	//Signal
	boost::signals2::signal<void(CShellFile*)> SignalFileSizeChanged;
	boost::signals2::signal<void(CShellFile*)> SignalFileIconChanged;
	
	//Getter 
	CComPtr<IShellFolder>& GetParentShellFolderPtr(){return m_parentFolder;}
	CIDLPtr& GetAbsolutePidl(){return m_absolutePidl;}

	//Lazy Evaluation Getter
	std::wstring& GetPath();
	std::wstring GetName();
	std::wstring GetExt();
	std::wstring GetNameExt();
	std::wstring GetTypeName();
	
	std::wstring GetCreationTime();
	std::wstring GetLastAccessTime();
	std::wstring GetLastWriteTime();

	std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize();
	//void SetSize(ULARGE_INTEGER size, FileSizeStatus status);

	std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon();

	bool HasIconInCache();
	bool IsDirectory();
	bool IsShellFolder();


	UINT GetSFGAO();
	DWORD GetAttributes();

	void Reset();
	//
	std::shared_ptr<CShellFolder> CastShellFolder();
private:
	void UpdateWIN32_FIND_DATA();
	std::shared_ptr<CIcon> GetDefaultIcon();

	CIcon GetIconBySHGetFileInfo();


};