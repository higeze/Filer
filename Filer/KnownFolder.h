#pragma once
#include "ShellFolder.h"

class CKnownDriveBaseFolder:public CShellFolder
{
public:
	CKnownDriveBaseFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder = nullptr);
	virtual ~CKnownDriveBaseFolder() {}
//	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction) override;
	virtual std::wstring GetFileNameWithoutExt() override;
	virtual std::wstring GetDispName() override;
};

class CKnownFolder:public CKnownDriveBaseFolder
{
private:
	CComPtr<IKnownFolder> m_pKnownFolder;
public:
	CKnownFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IKnownFolder>& pKnownFolder, CComPtr<IShellFolder> pShellFolder = nullptr);
	virtual ~CKnownFolder(){}
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed = nullptr) override;
	virtual std::wstring GetDispExt() override;

	KF_CATEGORY GetCategory();
	KNOWNFOLDERID GetId();
};

class CKnownFolderManager
{
private:
	std::unordered_map<std::wstring, std::shared_ptr<CKnownFolder>> m_knownFolderMap;
	std::shared_ptr<CKnownFolder> m_desktopFolder;
public:
	static CKnownFolderManager* GetInstance()
	{
		static CKnownFolderManager mgr;
		return &mgr;
	}

public:
	CKnownFolderManager();
	bool IsExist(const std::wstring& path) const;
	std::shared_ptr<CKnownFolder> GetKnownFolderByPath(const std::wstring& path);
	std::shared_ptr<CKnownFolder> GetKnownFolderById(const KNOWNFOLDERID& id);
	std::shared_ptr<CKnownFolder> GetDesktopFolder();

};