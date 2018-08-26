#pragma once
#include "ShellFolder.h"

class CKnownDriveBaseFolder:public CShellFolder
{
public:
	CKnownDriveBaseFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder = nullptr);
	virtual ~CKnownDriveBaseFolder() {}
	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon() override;
	virtual std::wstring GetFileNameWithoutExt() override;
	virtual std::wstring GetFileName() override;
};

class CKnownFolder:public CKnownDriveBaseFolder
{
private:
	CComPtr<IKnownFolder> m_pKnownFolder;
public:
	CKnownFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IKnownFolder>& pKnownFolder, CComPtr<IShellFolder> pShellFolder = nullptr);
	virtual ~CKnownFolder(){}
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize() override;
	virtual std::wstring GetExt() override;

	KF_CATEGORY GetCategory();
	KNOWNFOLDERID GetId();
};

class CKnownFolderManager
{
private:
	std::vector<std::shared_ptr<CKnownFolder>> m_knownFolders;
public:
	static CKnownFolderManager* GetInstance()
	{
		static CKnownFolderManager mgr;
		return &mgr;
	}

public:
	CKnownFolderManager();
	bool IsKnownFolder(CIDL& idl);
	std::vector<std::shared_ptr<CKnownFolder>> GetKnownFolders()const { return m_knownFolders; }
	std::shared_ptr<CKnownFolder> GetKnownFolderByIDL(CIDL& idl);
	std::shared_ptr<CKnownFolder> GetKnownFolderByPath(const std::wstring& path);
	std::shared_ptr<CKnownFolder> GetKnownFolderById(const KNOWNFOLDERID& id);
};