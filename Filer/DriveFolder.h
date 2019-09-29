#pragma once
#include "KnownFolder.h"

class CDriveFolder :public CKnownDriveBaseFolder
{
public:
	CDriveFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder = nullptr);
	virtual ~CDriveFolder() {}
	virtual std::wstring GetExt() override;
};


class CDriveFolderManager
{
private:
	std::vector<std::shared_ptr<CDriveFolder>> m_driveFolders;
public:
	static CDriveFolderManager* GetInstance()
	{
		static CDriveFolderManager mgr;
		return &mgr;
	}

public:
	CDriveFolderManager();
	void Update();
	bool IsExist(const std::wstring& path);
	std::shared_ptr<CDriveFolder> GetDriveFolderByIDL(CIDL& idl);
	std::shared_ptr<CDriveFolder> GetDriveFolderByPath(const std::wstring& path);
};
