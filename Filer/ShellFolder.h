#pragma once
#include "ShellFile.h"

class CShellFolder :public CShellFile
{
private:
	CComPtr<IShellFolder> m_folder;

	std::unique_ptr<std::thread> m_pSizeThread;
	std::promise<void> m_sizePromise;
	std::shared_future<void> m_sizeFuture;
	std::mutex m_mtxSize;

public:
	//static std::shared_ptr<CShellFolder> CreateShellFolderFromPath(const std::wstring& path);
	CShellFolder();
	virtual ~CShellFolder();
	CShellFolder(CComPtr<IShellFolder> pFolder, CComPtr<IShellFolder> pParentFolder, CIDLPtr absolutePidl);
	CShellFolder(const std::wstring& path);

	boost::signals2::signal<void(CShellFile*)> SignalFileSizeChanged;


	CComPtr<IShellFolder>& GetShellFolderPtr() { return m_folder; }
	std::shared_ptr<CShellFolder> GetParent();
	std::shared_ptr<CShellFolder> Clone()const;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize()override;
private:
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetLockSize();
	void SetLockSize(std::pair<ULARGE_INTEGER, FileSizeStatus>& size);
	bool GetFolderSize(ULARGE_INTEGER& size, std::shared_future<void> future);

	virtual void ResetSize();
};
