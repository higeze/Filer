#include "ShellFolder.h"

//std::shared_ptr<CShellFolder> CShellFolder::CreateShellFolderFromPath(const std::wstring& path)
//{
//	ULONG         chEaten;
//	ULONG         dwAttributes;
//
//	CComPtr<IShellFolder> pDesktop;
//	::SHGetDesktopFolder(&pDesktop);
//
//	CIDLPtr pIDL;
//	HRESULT hr = NULL;
//	if (path == L"") {
//		hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pIDL);
//	}
//	else {
//		hr = pDesktop->ParseDisplayName(
//			NULL,
//			NULL,
//			const_cast<LPWSTR>(path.c_str()),
//			&chEaten,
//			&pIDL,
//			&dwAttributes);
//	}
//	if (SUCCEEDED(hr))
//	{
//		CComPtr<IShellFolder> pFolder, pParentFolder;
//		::SHBindToObject(pDesktop, pIDL, 0, IID_IShellFolder, (void**)&pFolder);
//		::SHBindToObject(pDesktop, pIDL.GetPreviousIDLPtr(), 0, IID_IShellFolder, (void**)&pParentFolder);
//
//		if (!pFolder) {
//			pFolder = pDesktop;
//		}
//		if (!pParentFolder) {
//			pParentFolder = pDesktop;
//		}
//		return std::make_shared<CShellFolder>(pFolder, pParentFolder, pIDL);
//	}
//	else {
//		return std::shared_ptr<CShellFolder>();
//	}
//}

CShellFolder::CShellFolder() :CShellFile(), m_folder()
{
	::SHGetDesktopFolder(&m_folder);
}

CShellFolder::CShellFolder(CComPtr<IShellFolder> pFolder, CComPtr<IShellFolder> pParentFolder, CIDLPtr absolutePidl)
	:CShellFile(pParentFolder, absolutePidl), m_folder(pFolder) {}

CShellFolder::CShellFolder(const std::wstring& path)
	:CShellFile(path), m_folder()
{
	if (!m_absolutePidl.m_pIDL || !m_parentFolder) { m_folder = nullptr; return; }
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);

	HRESULT hr = ::SHBindToObject(pDesktop, m_absolutePidl, 0, IID_IShellFolder, (void**)&m_folder);
	if (FAILED(hr)) { m_folder = nullptr; }
}

std::shared_ptr<CShellFolder> CShellFolder::GetParent()
{
	CIDLPtr parentIDL = this->GetAbsolutePidl().GetPreviousIDLPtr();
	CIDLPtr grandParentIDL = parentIDL.GetPreviousIDLPtr();
	CComPtr<IShellFolder> pGrandParentFolder;
	//Desktop IShellFolder
	CComPtr<IShellFolder> pDesktopShellFolder;
	::SHGetDesktopFolder(&pDesktopShellFolder);
	pDesktopShellFolder->BindToObject(grandParentIDL, 0, IID_IShellFolder, (void**)&pGrandParentFolder);

	if (!pGrandParentFolder) {
		pGrandParentFolder = pDesktopShellFolder;
	}
	return std::make_shared<CShellFolder>(this->GetParentShellFolderPtr(), pGrandParentFolder, parentIDL);

}

std::shared_ptr<CShellFolder> CShellFolder::Clone()const
{
	return std::make_shared<CShellFolder>(m_folder, m_parentFolder, m_absolutePidl);
}