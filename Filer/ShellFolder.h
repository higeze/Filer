#pragma once
#include "ShellFile.h"

class CShellFolder :public CShellFile
{
private:
	CComPtr<IShellFolder> m_folder;
public:
	//static std::shared_ptr<CShellFolder> CreateShellFolderFromPath(const std::wstring& path);
	CShellFolder();
	CShellFolder(CComPtr<IShellFolder> pFolder, CComPtr<IShellFolder> pParentFolder, CIDLPtr absolutePidl);
	CShellFolder(const std::wstring& path);

	CComPtr<IShellFolder>& GetShellFolderPtr() { return m_folder; }
	std::shared_ptr<CShellFolder> GetParent();
};
