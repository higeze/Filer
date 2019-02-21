#pragma once
#include "ShellFile.h"
#include <boost/timer.hpp>

class CShellFolder :public CShellFile
{
private:
	CComPtr<IShellFolder> m_pShellFolder;

	std::unique_ptr<std::thread> m_pSizeThread;
	std::unique_ptr<std::thread> m_pTimeThread;
	std::atomic<bool> m_cancelThread = false;
	//std::promise<void> m_sizePromise;
	//std::shared_future<void> m_sizeFuture;
	std::mutex m_mtxSize;
	std::mutex m_mtxTime;

public:
//	CShellFolder();
	CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder = nullptr);
	virtual ~CShellFolder();

	//	CShellFolder(const std::wstring& path);

	boost::signals2::signal<void(CShellFile*)> SignalFileSizeChanged;
	boost::signals2::signal<void(CShellFile*)> SignalTimeChanged;

	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon() override;

	virtual std::wstring GetFileNameWithoutExt()override;
	virtual std::wstring GetFileName()override;
	virtual std::wstring GetExt()override;

	CComPtr<IShellFolder> GetShellFolderPtr();
	std::shared_ptr<CShellFolder> GetParent();
	std::shared_ptr<CShellFolder> Clone()const;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(std::shared_ptr<FileSizeArgs>& spArgs)override;
	virtual std::pair<FILETIME, FileTimeStatus> GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs)override;
	std::shared_ptr<CShellFile> CreateShExFileFolder(CIDL& relativeIdl);
	static std::shared_ptr<CShellFile> CreateShExFileFolder(const std::wstring& path);

private:
	std::pair<FILETIME, FileTimeStatus> GetLockLastWriteTime();
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetLockSize();

	//virtual void ResetSize();

protected:
	void SetLockSize(std::pair<ULARGE_INTEGER, FileSizeStatus>& size);
	void SetLockLastWriteTime(std::pair<FILETIME, FileTimeStatus>& time);
	virtual bool GetFolderSize(ULARGE_INTEGER& size, std::atomic<bool>& cancel, boost::timer& tim, int limit);
	virtual bool GetFolderLastWriteTime(FILETIME& time, std::atomic<bool>& cancel, boost::timer& tim, int limit);

};
