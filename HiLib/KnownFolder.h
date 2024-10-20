#pragma once
#include "ShellFolder.h"

class CKnownDriveBaseFolder:public CShellFolder
{
public:
	CKnownDriveBaseFolder(const CComPtr<IShellFolder>& pParentShellFolder, const CIDL& parentIdl, const CIDL& childIdl, const CComPtr<IShellFolder>& pShellFolder)
		:CShellFolder(pParentShellFolder, parentIdl, childIdl, arg<"ishellfolder"_s>() = pShellFolder) {}
	virtual ~CKnownDriveBaseFolder() {}
//	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction) override;
	//virtual const std::wstring& GetDispNameWithoutExt() const override;
	//virtual const std::wstring& GetDispName() const override;
};

class CKnownFolder:public CKnownDriveBaseFolder
{
private:
	CComPtr<IKnownFolder> m_pKnownFolder;
public:
	CKnownFolder(const CComPtr<IShellFolder>& pParentShellFolder, const CIDL& parentIdl, const CIDL& childIdl, const CComPtr<IKnownFolder>& pKnownFolder, const CComPtr<IShellFolder>& pShellFolder)
		:CKnownDriveBaseFolder(pParentShellFolder, parentIdl, childIdl, pShellFolder), m_pKnownFolder(pKnownFolder) {}
	virtual ~CKnownFolder(){}
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(const FileSizeArgs& args, std::function<void()> changed = nullptr) override;
	virtual const std::wstring& GetDispExt() const override;

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