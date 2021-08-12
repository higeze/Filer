#pragma once
#include "MyString.h"
#include "MyIcon.h"
#include "IDL.h"
#include "ShellFunction.h"
#include <future>
#include <chrono>
#include <optional>

class CShellFolder;
struct FileSizeArgs;
struct FileTimeArgs;

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

	std::pair<FileTimes, FileTimeStatus> m_fileTimes = std::make_pair(FileTimes(), FileTimeStatus::None);

	std::pair<ULARGE_INTEGER, FileSizeStatus> m_size = std::make_pair(ULARGE_INTEGER(), FileSizeStatus::None);
	//std::pair<std::shared_ptr<CIcon>, FileIconStatus> m_icon = std::make_pair(std::shared_ptr<CIcon>(nullptr), FileIconStatus::None);

	DWORD  m_fileAttributes = 0;
	ULONG m_sfgao = 0;

public:
	//Constructor
	CShellFile() {}
	CShellFile(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl);
	//CShellFile(const std::wstring& path);

	//Destructor
	virtual ~CShellFile();
	
	//Getter 
	CComPtr<IShellFolder>& GetParentShellFolderPtr(){return m_pParentShellFolder;}
	CIDL& GetAbsoluteIdl() { return m_absoluteIdl; }
	CIDL& GetChildIdl() { return m_childIdl; }

	//Lazy Evaluation Getter
	virtual std::wstring GetPath();
	virtual std::wstring GetFileNameWithoutExt();
	virtual std::wstring GetDispName();
	virtual std::wstring GetDispExt();
	virtual std::wstring GetPathName();
	virtual std::wstring GetPathNameWithoutExt();
	virtual std::wstring GetPathExt();

	std::wstring GetTypeName();	
	UINT GetSFGAO();
	DWORD GetAttributes();

	//Non-lazy
	bool GetIsExist();

	virtual void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND hWnd = NULL);
	virtual void SetExt(const std::wstring& wstrExt, HWND hWnd = NULL);

	//LastWrite
	std::optional<FileTimes> GetFileTimes();
	virtual std::pair<FileTimes, FileTimeStatus> GetFileTimes(const std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed = nullptr);

	//Size
	bool GetFileSize(ULARGE_INTEGER& size/*, std::shared_future<void> future*/);
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(const std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed = nullptr);
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> ReadSize();

	//Icon
	//virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction);

	//bool HasIconInCache();
	bool IsDirectory();
	virtual void Reset();
	virtual void Open();

private:
	void UpdateWIN32_FIND_DATA();
	//virtual void ResetIcon();
	virtual void ResetSize();
	virtual void ResetTime();

	//std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetLockIcon();
	//void SetLockIcon(std::pair<std::shared_ptr<CIcon>, FileIconStatus>& icon);
};


class CShellInvalidFile :public CShellFile
{
public:
	//Constructor
	CShellInvalidFile() {}

	//Destructor
	virtual ~CShellInvalidFile(){}

	virtual std::wstring GetPath() override{ return m_wstrPath; }
	virtual std::wstring GetFileNameWithoutExt()override { return m_wstrFileNameWithoutExt; }
	virtual std::wstring GetDispName() override { return m_wstrFileName; }
	virtual std::wstring GetDispExt() override { return m_wstrExt; }
	virtual void Open() override {/* Do Nothing */ }

	//Icon
//	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction)override;
};

