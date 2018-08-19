#pragma once
#include "ShellFolder.h"

class CKnownFolder:public CShellFolder
{
private:
	CComPtr<IKnownFolder> m_pKnownFolder;
public:
	//CKnownFolder(CComPtr<IKnownFolder>& pKnownFolder, CIDL& IDL);
	CKnownFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder, CComPtr<IKnownFolder>& pKnownFolder);
//	virtual std::wstring& GetPath() override;
	virtual std::wstring GetFileNameWithoutExt() override;
	virtual std::wstring GetFileName() override;
	virtual std::wstring GetExt() override;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize() override;
	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon() override;

	KF_CATEGORY GetCategory();
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
};

