#include "ShellFolder.h"

CShellFolder::CShellFolder() :CShellFile(), m_folder()
{
	::SHGetDesktopFolder(&m_folder);
	::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &m_absolutePidl);
}
CShellFolder::CShellFolder(CComPtr<IShellFolder> pFolder, CComPtr<IShellFolder> pParentFolder, CIDLPtr absolutePidl)
	:CShellFile(pParentFolder, absolutePidl), m_folder(pFolder) {}