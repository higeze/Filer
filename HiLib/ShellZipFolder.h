#pragma once
#include "ShellFolder.h"

class CShellZipFolder :public CShellFolder
{
public:
	template<typename... _Args>
	CShellZipFolder(const CComPtr<IShellFolder>& pParentShellFolder, const CIDL& parentIdl, const CIDL& childIdl, _Args... args)
		:CShellFolder(pParentShellFolder, parentIdl, childIdl, args...) {}

	virtual ~CShellZipFolder() = default;

	virtual const std::wstring& GetDispName() const override;
	virtual const std::wstring& GetDispExt() const override;
	virtual const std::wstring& GetDispNameWithoutExt() const override;

	virtual void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND hWnd = NULL) override;
	virtual void SetExt(const std::wstring& wstrExt, HWND hWnd = NULL) override;

//	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction) override;
	virtual std::pair<FileTimes, FileTimeStatus> GetFileTimes(const FileTimeArgs& args, std::function<void()> changed = nullptr)override;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(const FileSizeArgs& args, std::function<void()> changed = nullptr)override;
protected:
//	virtual bool GetFolderSize(ULARGE_INTEGER& size, std::atomic<bool>& cancel, boost::timer& tim, int limit)override;
//	virtual bool GetFolderLastWriteTime(FILETIME& time, std::atomic<bool>& cancel, boost::timer& tim, int limit, bool ignoreFolderTime)override;
};
