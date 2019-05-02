#pragma once
#include "ShellFolder.h"

class CShellZipFolder :public CShellFolder
{
public:
	CShellZipFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl);
	virtual ~CShellZipFolder(){}

	virtual std::wstring GetFileNameWithoutExt()override;
	virtual std::wstring GetFileName()override;
	virtual std::wstring GetExt()override;

	virtual void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt) override;
	virtual void SetExt(const std::wstring& wstrExt) override;

//	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction) override;
	virtual std::pair<FILETIME, FileTimeStatus> GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed = nullptr)override;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed = nullptr)override;
protected:
//	virtual bool GetFolderSize(ULARGE_INTEGER& size, std::atomic<bool>& cancel, boost::timer& tim, int limit)override;
//	virtual bool GetFolderLastWriteTime(FILETIME& time, std::atomic<bool>& cancel, boost::timer& tim, int limit, bool ignoreFolderTime)override;
};
