#pragma once
#include "ShellFile.h"
#include <boost/timer.hpp>

class CShellFolder :public CShellFile
{
private:
	CComPtr<IShellFolder> m_pShellFolder;

	std::unique_ptr<std::thread> m_pSizeThread;
	std::atomic<bool> m_cancelSize = false;
	//std::promise<void> m_sizePromise;
	//std::shared_future<void> m_sizeFuture;
	std::mutex m_mtxSize;

public:
//	CShellFolder();
	CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder = nullptr);
	virtual ~CShellFolder();

	//	CShellFolder(const std::wstring& path);

	boost::signals2::signal<void(CShellFile*)> SignalFileSizeChanged;

	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon() override;

	virtual std::wstring GetFileNameWithoutExt()override;
	virtual std::wstring GetFileName()override;
	virtual std::wstring GetExt()override;

	CComPtr<IShellFolder> GetShellFolderPtr();
	std::shared_ptr<CShellFolder> GetParent();
	std::shared_ptr<CShellFolder> Clone()const;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(std::shared_ptr<FileSizeArgs>& spArgs)override;
	std::shared_ptr<CShellFile> CreateShExFileFolder(CIDL& relativeIdl);
	static std::shared_ptr<CShellFile> CreateShExFileFolder(const std::wstring& path);

private:
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetLockSize();

	virtual void ResetSize();

protected:
	void SetLockSize(std::pair<ULARGE_INTEGER, FileSizeStatus>& size);
	bool GetFolderSize(ULARGE_INTEGER& size, std::atomic<bool>& cancel, boost::timer& tim, int limit);

};
