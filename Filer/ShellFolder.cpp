#include "ShellFolder.h"

std::shared_ptr<CShellFolder> CShellFolder::CreateShellFolderFromPath(const std::wstring& path)
{
	ULONG         chEaten;
	ULONG         dwAttributes;

	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);

	CIDLPtr pIDL;
	HRESULT hr = NULL;
	if (path == L"") {
		hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pIDL);
	}
	else {
		hr = pDesktop->ParseDisplayName(
			NULL,
			NULL,
			const_cast<LPWSTR>(path.c_str()),
			&chEaten,
			&pIDL,
			&dwAttributes);
	}
	if (SUCCEEDED(hr))
	{
		CComPtr<IShellFolder> pFolder, pParentFolder;
		::SHBindToObject(pDesktop, pIDL, 0, IID_IShellFolder, (void**)&pFolder);
		::SHBindToObject(pDesktop, pIDL.GetPreviousIDLPtr(), 0, IID_IShellFolder, (void**)&pParentFolder);

		if (!pFolder) {
			pFolder = pDesktop;
		}
		if (!pParentFolder) {
			pParentFolder = pDesktop;
		}
		return std::make_shared<CShellFolder>(pFolder, pParentFolder, pIDL);
	}
	else {
		return std::shared_ptr<CShellFolder>();
	}
}


CShellFolder::CShellFolder() :CShellFile(), m_folder()
{
	::SHGetDesktopFolder(&m_folder);
	::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &m_absolutePidl);
}
CShellFolder::CShellFolder(CComPtr<IShellFolder> pFolder, CComPtr<IShellFolder> pParentFolder, CIDLPtr absolutePidl)
	:CShellFile(pParentFolder, absolutePidl), m_folder(pFolder) {}