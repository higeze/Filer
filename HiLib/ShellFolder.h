#pragma once
#include "ShellFile.h"
#include <chrono>
#include <mutex> 
#include "reactive_vector.h"

class CDirectoryWatcher;
struct DirectoryWatchEvent;

class CShellFolder :public CShellFile
{
private:
	mutable std::shared_ptr<CDirectoryWatcher> m_spWatcher;

	std::shared_ptr<bool> m_spCancelThread = std::make_shared<bool>(false);
	
	std::future<std::pair<ULARGE_INTEGER, FileSizeStatus>> m_futureSize;
	std::future<std::pair<FileTimes, FileTimeStatus>> m_futureTime;

	mutable std::mutex m_mtxSize;
	mutable std::mutex m_mtxTime;

	mutable reactive_vector_ptr<std::shared_ptr<CShellFile>> Files;

public:
	reactive_vector_ptr<std::shared_ptr<CShellFile>>& GetFiles() const;
private:
	virtual void OnDirectoryWatch(const DirectoryWatchEvent& e) const;
	void Added(const std::wstring& fileName) const;
	void Modified(const std::wstring& fileName) const;
	void Removed(const std::wstring& fileName) const;
	void Renamed(const std::wstring& oldName, const std::wstring& newName) const;



public:

protected: mutable CShellComPtr<IShellFolder> m_pShellFolder; public: virtual CShellComPtr<IShellFolder> GetShellFolderPtr() const;
	DECLARE_LAZY_SHAREDPTR_GETTER(CShellFolder, ParentFolder);

public:
	template<typename... _Args>
	CShellFolder(CShellComPtr<IShellFolder> pParentShellFolder, const CIDL& parentIdl, const CIDL& childIdl, _Args... args)
		:CShellFile(pParentShellFolder, parentIdl, childIdl, args...)/*, m_pShellFolder(::get(arg<"ishellfolder"_s>(), args..., default_(nullptr)))*/{}

	virtual ~CShellFolder();

	//CShellFolder Clone() const;

	virtual const std::wstring& GetDispName() const override;
	virtual const std::wstring& GetDispNameWithoutExt() const override;
	virtual const std::wstring& GetDispExt() const override;

	virtual void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND hWnd = NULL) override;
	virtual void SetExt(const std::wstring& wstrExt, HWND hWnd = NULL) override;

	std::shared_ptr<CShellFolder> Clone()const;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(const FileSizeArgs& args, std::function<void()> changed = nullptr)override;
	virtual std::pair<FileTimes, FileTimeStatus> GetFileTimes(const FileTimeArgs& args, std::function<void()> changed = nullptr)override;
	std::shared_ptr<CShellFile> CreateShExFileFolder(CIDL&& relativeIdl) const;
	//std::shared_ptr<CShellFile> CreateShExFileFolder(const CIDL& relativeIdl) const;
	static std::optional<FileTimes> GetFolderFileTimes(const std::shared_ptr<bool>& cancel,
		CShellComPtr<IShellFolder> pParentFolder, CShellComPtr<IShellFolder> pFolder, const CIDL& relativeIdl, const std::wstring& path,
		std::chrono::system_clock::time_point& tp, int limit, bool ignoreFolderTime);
	static std::optional<FileTimes> GetFolderFileTimes(
		const std::shared_ptr<bool>& cancel,
		const CIDL& parentIdl,
		const CIDL& relativeIdl,
		const std::wstring& path,
		std::chrono::system_clock::time_point& tp,
		int limit,
		bool ignoreFolderTime);
	static bool GetFolderSize(ULARGE_INTEGER& size, const std::shared_ptr<bool>& cancel,
		CShellComPtr<IShellFolder> pFolder, const std::wstring& path,
		const std::chrono::system_clock::time_point& tp, const int limit);
private:
	std::pair<FileTimes, FileTimeStatus> GetLockFileTimes() const;
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetLockSize() const;

protected:
	void SetLockSize(const std::pair<ULARGE_INTEGER, FileSizeStatus>& size);
	void SetLockFileTimes(const std::pair<FileTimes, FileTimeStatus>& times);




};
