#pragma once
#include "ShellFile.h"
#include "KnownFolder.h"

class CShellFolder :public CShellFile
{
private:
	CComPtr<IShellFolder> m_pShellFolder;

	std::unique_ptr<std::thread> m_pSizeThread;
	std::promise<void> m_sizePromise;
	std::shared_future<void> m_sizeFuture;
	std::mutex m_mtxSize;
	static CKnownFolderManager s_knownFolderManager;

public:
	//static std::shared_ptr<CShellFolder> CreateShellFolderFromPath(const std::wstring& path);
	CShellFolder();
	CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pDhellFolder);
	virtual ~CShellFolder();

	//	CShellFolder(const std::wstring& path);

	boost::signals2::signal<void(CShellFile*)> SignalFileSizeChanged;

	std::wstring GetFileNameWithoutExt()override;
	std::wstring GetFileName()override;
	std::wstring GetExt()override;

	CComPtr<IShellFolder> GetShellFolderPtr()const { return m_pShellFolder; }
	std::shared_ptr<CShellFolder> GetParent();
	std::shared_ptr<CShellFolder> Clone()const;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize()override;
	std::shared_ptr<CShellFile> CreateShExFileFolder(CIDL& relativeIdl);
	static std::shared_ptr<CShellFile> CreateShExFileFolder(const std::wstring& path);

private:
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetLockSize();
	void SetLockSize(std::pair<ULARGE_INTEGER, FileSizeStatus>& size);
	bool GetFolderSize(ULARGE_INTEGER& size, std::shared_future<void> future);

	virtual void ResetSize();
};
