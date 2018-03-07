#include "ShellFile.h"
#include "MyIcon.h"
#include "MyString.h"
#include "MyCom.h"
#include "ShellFolder.h"
#include <thread>
#include "ThreadPool.h"

extern std::unique_ptr<ThreadPool> g_pThreadPool;

std::wstring ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit)

{
    bool is_minus = n < 0;
    is_minus ? n *= -1 : 0;

    std::wstringstream ss;
    ss << n;
    std::wstring snum = ss.str();
    std::reverse(snum.begin(), snum.end());
    std::wstringstream  ss_csnum;
    for(int i = 0, len = snum.length(); i <= len;) {
        ss_csnum << snum.substr(i, separate_digit);
        if((i += separate_digit) >= len)
            break;
        ss_csnum << ',';
    }
    if(is_minus){
		ss_csnum << '-';
	}

    std::wstring cs_num = ss_csnum.str();
    std::reverse(cs_num.begin(), cs_num.end());
    return cs_num;
}

tstring FileTime2String(FILETIME* pFileTime)
{
	FILETIME ft;
	SYSTEMTIME st;

	FileTimeToLocalFileTime(pFileTime, &ft);
	FileTimeToSystemTime(&ft, &st);
	tstring str;
	wsprintf(GetBuffer(str,16),L"%04d/%02d/%02d %02d:%02d",
			st.wYear, st.wMonth, st.wDay,st.wHour, st.wMinute);
	ReleaseBuffer(str);
	return str;

}

tstring Size2String(ULONGLONG size)
{	
	return ConvertCommaSeparatedNumber(size);	
}

std::wstring CShellFile::GetPath()
{
	if (m_wstrPath.empty()) {
		CIDLPtr childPidl = m_absolutePidl.GetLastIDLPtr();
		STRRET strret;
		m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_FORPARSING, &strret);
		m_wstrPath = childPidl.STRRET2WSTR(strret);
	}
	return m_wstrPath;
}

std::wstring CShellFile::GetName()
{
	if (m_wstrName.empty()) {
		CIDLPtr childPidl = m_absolutePidl.GetLastIDLPtr();
		STRRET strret;
		m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_NORMAL|SHGDN_INFOLDER|SHGDN_FORPARSING, &strret);
		m_wstrName = childPidl.STRRET2WSTR(strret);
	}
	return m_wstrName;
}

std::wstring CShellFile::GetExt()
{
	if (m_wstrExt.empty()) {
		m_wstrExt = ::PathFindExtension(GetPath().c_str());
	}
	return m_wstrExt;
}

std::wstring CShellFile::GetTypeName()
{
	if (m_wstrType.empty()) {
		SHFILEINFO sfi = { 0 };
		::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_TYPENAME);
		m_wstrType = sfi.szTypeName;
	}
	return m_wstrType;
}

std::wstring CShellFile::GetCreationTime()
{
	if (m_wstrCreationTime.empty()) {
		UpdateWIN32_FIND_DATA();
	}
	return m_wstrCreationTime;
}

std::wstring CShellFile::GetLastAccessTime()
{
	if (m_wstrLastAccessTime.empty()) {
		UpdateWIN32_FIND_DATA();
	}
	return m_wstrLastAccessTime;
}

std::wstring CShellFile::GetLastWriteTime()
{
	if (m_wstrLastWriteTime.empty()) {
		UpdateWIN32_FIND_DATA();
	}
	return m_wstrLastWriteTime;
}

std::wstring CShellFile::GetSizeString()
{
	if (m_wstrSize.empty()) {
		UpdateWIN32_FIND_DATA();
	}
	return m_wstrSize;
}

ULARGE_INTEGER CShellFile::GetSize()
{
	if (!m_size.HighPart && !m_size.LowPart) {
		UpdateWIN32_FIND_DATA();
	}
	return m_size;
}

//void CShellFile::LoadIcon()
//{
//	SHFILEINFO sfi = { 0 };
//	::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
//	//::SHGetFileInfo(GetPath().c_str(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
//	m_icon = CIcon(sfi.hIcon);
//}


CIcon CShellFile::GetIcon(bool load)
{
	if (!m_icon && load) {
		SHFILEINFO sfi = { 0 };
		::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
		//::SHGetFileInfo(GetPath().c_str(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
		m_icon = CIcon(sfi.hIcon);
		return m_icon;
	}
	else {
		return m_icon;
	}
	//if ((HICON)m_icon == NULL && !m_isAsyncIcon) {
	//	//if (false && !(GetAttributes() & (SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR))) {
	//	//	SHFILEINFO sfi = { 0 };
	//	//	::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	//	//	//::SHGetFileInfo(GetPath().c_str(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	//	//	m_icon = CIcon(sfi.hIcon);
	//	//}else {
	//		m_isAsyncIcon = true;
	//		g_pThreadPool->add([this]
	//		{
	//			//if (FAILED(::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	//			//	return;
	//			SHFILEINFO sfi = { 0 };
	//			::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	//			//::SHGetFileInfo(GetPath().c_str(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	//			m_icon = CIcon(sfi.hIcon);
	//			m_isAsyncIcon = false;

	//			//::CoUninitialize();
	//			//CComPtr<IExtractIcon> pEI;
	//			//UINT reserved = 0;
	//			//LPITEMIDLIST lastPIDL = m_absolutePidl.FindLastID();
	//			//HRESULT hr = m_parentFolder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)(&lastPIDL), IID_IExtractIcon, &reserved, (LPVOID*)&pEI);
	//			//if (SUCCEEDED(hr)) {
	//			//	std::wstring buff;
	//			//	::GetBuffer(buff, MAX_PATH);
	//			//	UINT flags = 0;
	//			//	int index = 0;
	//			//	hr = pEI->GetIconLocation(GIL_FORSHELL, (PWSTR)buff.data(), MAX_PATH, &index, &flags);
	//			//	HICON smallIcon = NULL;
	//			//	pEI->Extract(buff.data(), index, NULL, &smallIcon, MAKELONG(32, 16));
	//			//	m_icon = CIcon(smallIcon);
	//			//}
	//			//else {
	//			//	m_icon = GetIconBySHGetFileInfo();
	//			//}
	//			//m_isAsyncIcon = false;

	//		});
	//	}

		//if (GetAttributes() & (SFGAO_LINK | SFGAO_GHOSTED | SFGAO_HIDDEN | SFGAO_SHARE) || 
		//	!(GetAttributes() & (SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR))) {
		//	m_icon = GetIconBySHGetFileInfo();
		//}
		//else {
		//	CComPtr<IExtractIcon> pEI;
		//	UINT reserved = 0;
		//	LPITEMIDLIST lastPIDL = m_absolutePidl.FindLastID();
		//	HRESULT hr = m_parentFolder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)(&lastPIDL), IID_IExtractIcon, &reserved, (LPVOID*)&pEI);
		//	if (SUCCEEDED(hr)) {
		//		std::wstring buff;
		//		::GetBuffer(buff, MAX_PATH);
		//		UINT flags = 0;
		//		int index = 0;
		//		hr = pEI->GetIconLocation(GIL_FORSHELL | GIL_ASYNC, (PWSTR)buff.data(), MAX_PATH, &index, &flags);
		//		switch (hr) {

		//		case E_PENDING:
		//		{
		//			std::cout << "E_PENDING" << std::endl;
		//			m_isAsyncIcon = true;
		//			std::thread th([this]
		//			{
		//				CComPtr<IExtractIcon> pEI;
		//				UINT reserved = 0;
		//				LPITEMIDLIST lastPIDL = m_absolutePidl.FindLastID();
		//				HRESULT hr = m_parentFolder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)(&lastPIDL), IID_IExtractIcon, &reserved, (LPVOID*)&pEI);
		//				if (SUCCEEDED(hr)) {
		//					std::wstring buff;
		//					::GetBuffer(buff, MAX_PATH);
		//					UINT flags = 0;
		//					int index = 0;
		//					hr = pEI->GetIconLocation(GIL_FORSHELL, (PWSTR)buff.data(), MAX_PATH, &index, &flags);
		//					HICON smallIcon = NULL;
		//					pEI->Extract(buff.data(), index, NULL, &smallIcon, MAKELONG(32, 16));
		//					m_icon = CIcon(smallIcon);
		//				}
		//				else {
		//					m_icon = GetIconBySHGetFileInfo();
		//				}
		//				m_isAsyncIcon = false;

		//			});
		//			th.detach();
		//		}
		//		break;
		//		case NOERROR:
		//		{
		//			HICON smallIcon = NULL;
		//			pEI->Extract(buff.data(), index, NULL, &smallIcon, MAKELONG(32, 16));
		//			if (smallIcon) {
		//				m_icon = CIcon(smallIcon);
		//			}
		//			else {
		//				m_icon = GetIconBySHGetFileInfo();
		//			}
		//		}
		//		break;
		//		case S_FALSE:
		//		{
		//			m_icon = GetIconBySHGetFileInfo();
		//		}
		//		default:
		//			break;
		//		}
		//	}
		//}
	//}
	return m_icon;
}

UINT CShellFile::GetAttributes()
{
	if (m_ulAttributes == 0) {
		auto childPidl = m_absolutePidl.GetLastIDLPtr();
		m_ulAttributes = SFGAO_CAPABILITYMASK | SFGAO_GHOSTED | SFGAO_LINK | SFGAO_SHARE | SFGAO_FOLDER | SFGAO_FILESYSTEM;
		m_parentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&childPidl, &m_ulAttributes);
	}
	return m_ulAttributes;

}

void CShellFile::UpdateWIN32_FIND_DATA()
{
	auto childPidl = m_absolutePidl.GetLastIDLPtr();
	WIN32_FIND_DATA wfd = { 0 };
	if (!FAILED(SHGetDataFromIDList(m_parentFolder, childPidl, SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
		m_wstrCreationTime = FileTime2String(&wfd.ftCreationTime);
		m_wstrLastAccessTime = FileTime2String(&wfd.ftLastAccessTime);
		m_wstrLastWriteTime = FileTime2String(&wfd.ftLastWriteTime);
		m_fileAttributes = wfd.dwFileAttributes;
		m_size.LowPart = wfd.nFileSizeLow;
		m_size.HighPart = wfd.nFileSizeHigh;
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			m_wstrSize = L"dir";
		}
		else {
			m_wstrSize = Size2String(m_size.QuadPart);
		}
	}
}

CShellFile::CShellFile() :m_parentFolder(), m_absolutePidl(), m_isAsyncIcon(false)
{
	::SHGetDesktopFolder(&m_parentFolder);
	::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &m_absolutePidl);
}

CShellFile::CShellFile(const std::wstring& path) : m_parentFolder(), m_absolutePidl(), m_isAsyncIcon(false)
{
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);

	HRESULT hr = 0;
	if (path == L"") {
		::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &m_absolutePidl);
		m_parentFolder = pDesktop;
	}
	else {
		ULONG         chEaten;
		ULONG         dwAttributes;
		hr = pDesktop->ParseDisplayName(
			NULL,
			NULL,
			const_cast<LPWSTR>(path.c_str()),
			&chEaten,
			&m_absolutePidl,
			&dwAttributes);
		if (SUCCEEDED(hr))
		{
			::SHBindToObject(pDesktop, m_absolutePidl.GetPreviousIDLPtr(), 0, IID_IShellFolder, (void**)&m_parentFolder);
			if (!m_parentFolder) {
				m_parentFolder = pDesktop;
			}
		}
	}
}

CShellFile::CShellFile(CComPtr<IShellFolder> pfolder, CIDLPtr absolutePidl)
	:m_parentFolder(pfolder),m_absolutePidl(absolutePidl), m_isAsyncIcon(false) {}

bool CShellFile::IsShellFolder()const
{
	//Try BindToObject and EnumObjects to identify folder
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);
	CComPtr<IShellFolder> pFolder;
	HRESULT hr = S_OK;

	if (m_absolutePidl.m_pIDL->mkid.cb == 0) {
		pFolder = pDesktop;
	}
	else {
		hr = pDesktop->BindToObject(m_absolutePidl, 0, IID_IShellFolder, (void**)&pFolder);
	}

	if (SUCCEEDED(hr)) {
		CComPtr<IEnumIDList> enumIdl;
		hr = pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl);
		return SUCCEEDED(hr);
	}
	return false;

}

std::shared_ptr<CShellFolder> CShellFile::GetShellFolder()const
{
	if (IsShellFolder()) {
		CComPtr<IShellFolder> pDesktop;
		::SHGetDesktopFolder(&pDesktop);
		CComPtr<IShellFolder> pFolder;
		HRESULT hr = 0;
		if (m_absolutePidl.m_pIDL->mkid.cb == 0) {
			pFolder = pDesktop;
		}
		else {
			HRESULT hr = pDesktop->BindToObject(m_absolutePidl, 0, IID_IShellFolder, (void**)&pFolder);
		}
		return std::make_shared<CShellFolder>(pFolder, m_parentFolder, m_absolutePidl);
	}
	else {
		return std::shared_ptr<CShellFolder>();
	}
}

void CShellFile::Reset()
{
	m_wstrPath.clear();
	m_wstrName.clear();
	m_wstrExt.clear();
	m_wstrType.clear();
	m_wstrCreationTime.clear();
	m_wstrLastAccessTime.clear();
	m_wstrLastWriteTime.clear();
	m_wstrSize.clear();

	m_size.LowPart = 0;
	m_size.HighPart = 0;
	m_fileAttributes = 0;
	m_icon = nullptr;
	m_ulAttributes = 0;

	bool m_isAsyncIcon = false;


}



