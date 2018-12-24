#pragma once
#include "ShellFolder.h"

class CShellZipFolder :public CShellFolder
{
public:
	CShellZipFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl);
	virtual ~CShellZipFolder(){}

	virtual std::wstring GetFileNameWithoutExt()override;
	virtual std::wstring GetFileName()override;
	virtual std::wstring GetExt()override;

	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon() override;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(std::shared_ptr<FileSizeArgs>& spArgs)override;


};
