#pragma once
#include "ShellFolder.h"

class CKnownDriveBaseFolder:public CShellFolder
{
public:
	CKnownDriveBaseFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder);
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
	CKnownFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder, CComPtr<IKnownFolder>& pKnownFolder);
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

class CDriveFolder :public CKnownDriveBaseFolder
{
public:
	CDriveFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder);
	virtual ~CDriveFolder() {}
	virtual std::wstring GetExt() override;
};


class CDriveManager
{
private:
	std::vector<std::shared_ptr<CDriveFolder>> m_driveFolders;
public:
	static CDriveManager* GetInstance()
	{
		static CDriveManager mgr;
		return &mgr;
	}

public:
	CDriveManager();
	std::shared_ptr<CDriveFolder> GetDriveFolderByIDL(CIDL& idl);
	std::shared_ptr<CDriveFolder> GetDriveFolderByPath(const std::wstring& path);
};
