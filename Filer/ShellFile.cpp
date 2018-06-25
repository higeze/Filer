#include "ShellFile.h"
#include "MyIcon.h"
#include "MyString.h"
#include "MyCom.h"
#include "ShellFolder.h"
#include <thread>
//#include "ThreadPool.h"
//#include "ThreadHelper.h"

//extern std::unique_ptr<ThreadPool> g_pThreadPool;

CFileIconCache CShellFile::s_iconCache;

bool GetDirSize(std::wstring path, ULONGLONG *pSize, std::function<bool()> cancel = nullptr)
{
	try {
		*pSize = 0;
		ULONGLONG size = 0;

		WIN32_FIND_DATA findData;
		std::unique_ptr <std::remove_pointer<HANDLE>::type, findclose> upFind(FindFirstFile((path + L"\\*").c_str(), &findData));
		if (upFind.get() == INVALID_HANDLE_VALUE) {
			return false;
		}

		do {
			if (cancel && cancel()) {
				return false;
			}

			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				//Ignore "." ".."
				if (!_tcscmp(_T("."), findData.cFileName) || !_tcscmp(_T(".."), findData.cFileName)) {
					continue;
				}
				std::wstring nextPath(path + L"\\" + findData.cFileName);
				if (!GetDirSize(nextPath.c_str(), &size, cancel)) {
					return false;
				}
				*pSize += size;
			} else {
				ULARGE_INTEGER plus = { findData.nFileSizeLow , findData.nFileSizeHigh };
				*pSize += plus.QuadPart;
			}
		} while (FindNextFile(upFind.get(), &findData));

		if (GetLastError() != ERROR_NO_MORE_FILES) {
			return false;
		} else {
			return true;
		}
	} catch (...) {
		return false;
	}
}


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


CShellFile::CShellFile() :m_parentFolder(), m_absolutePidl()
{
	::SHGetDesktopFolder(&m_parentFolder);
	::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &m_absolutePidl);
}

CShellFile::CShellFile(const std::wstring& path) : m_parentFolder(), m_absolutePidl()
{
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);

	ULONG         chEaten;
	ULONG         dwAttributes;
	HRESULT hr = pDesktop->ParseDisplayName(
		NULL,
		NULL,
		const_cast<LPWSTR>(path.c_str()),
		&chEaten,
		&m_absolutePidl,
		&dwAttributes);
	if (FAILED(hr)) { m_parentFolder = nullptr; m_absolutePidl = CIDLPtr(); return; }
	hr = ::SHBindToObject(pDesktop, m_absolutePidl.GetPreviousIDLPtr(), 0, IID_IShellFolder, (void**)&m_parentFolder);
	if (FAILED(hr)) { m_parentFolder = nullptr; m_absolutePidl = CIDLPtr(); return; }
}

CShellFile::CShellFile(CComPtr<IShellFolder> pfolder, CIDLPtr absolutePidl)
	:m_parentFolder(pfolder), m_absolutePidl(absolutePidl)
{}

CShellFile::~CShellFile()
{
	if (m_pSizeThread && m_pSizeThread->joinable()) {
		m_cancelSizeThread = true;
		m_pSizeThread->join();
	}
	if (m_pIconThread && m_pIconThread->joinable()) {
		m_cancelIconThread = true;
		m_pIconThread->join();
	}

	//g_pThreadPool->remove_if([this](const std::function<void()>& fun)->bool {
	//	return getAddress(m_sizeThreadFun) == getAddress(fun);
	//});

}


std::wstring& CShellFile::GetPath()
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
		m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_NORMAL, &strret);
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

std::wstring CShellFile::GetNameExt()
{
	if (m_wstrNameExt.empty()) {
		CIDLPtr childPidl = m_absolutePidl.GetLastIDLPtr();
		STRRET strret;
		m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_FORPARSING | SHGDN_INFOLDER, &strret);
		m_wstrNameExt = childPidl.STRRET2WSTR(strret);
	}
	return m_wstrNameExt;
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

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFile::GetSize()
{
	switch (m_size.second) {
	case FileSizeStatus::None:
		if (!IsDirectory()) {
			UpdateWIN32_FIND_DATA();
			m_size.second = FileSizeStatus::Available;
		} else {
			m_size.second = FileSizeStatus::Calculating;
			if (!m_pSizeThread) {
				m_pSizeThread = std::make_unique<std::thread>([this]()->void {
					ULARGE_INTEGER size;
					if (::GetDirSize(GetPath(), &size.QuadPart, [this]()->bool {return m_cancelSizeThread; })) {
						m_size.first = size;
						m_size.second = FileSizeStatus::Available;
						//SetSize(size, FileSizeStatus::Available);
					} else {
						m_size.first.QuadPart = 0;
						m_size.second = FileSizeStatus::Unavailable;
						//SetSize(size, FileSizeStatus::Unavailable);
					}
					//::Sleep(1000);
					SignalFileSizeChanged(this);
				});
			}
		}
		break;
	case FileSizeStatus::Available:
	case FileSizeStatus::Unavailable:
	case FileSizeStatus::Calculating:
		break;
	}
	return m_size;
}

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFile::GetIcon()
{
	switch (m_icon.second) {
	case FileIconStatus::None:
		if (HasIconInCache()) {
			m_icon.first = s_iconCache.GetIcon(this);
			m_icon.second = FileIconStatus::Avilable;
		} else {
			m_icon.first = GetDefaultIcon();
			m_icon.second = FileIconStatus::Loading;
			if (!m_pIconThread) {
				m_pIconThread = std::make_unique<std::thread>([this]()->void {
					m_icon.first = s_iconCache.GetIcon(this);
					m_icon.second = FileIconStatus::Avilable;
					SignalFileIconChanged(this);
				});
			}
		}
		break;
	case FileIconStatus::Avilable:
	case FileIconStatus::Loading:
		break;
	}
	return m_icon;
}

std::shared_ptr<CIcon> CShellFile::GetDefaultIcon()
{
	return s_iconCache.GetDefaultIcon();
}

UINT CShellFile::GetSFGAO()
{
	if (m_sfgao == 0) {
		auto childPidl = m_absolutePidl.GetLastIDLPtr();
		m_sfgao = SFGAO_CAPABILITYMASK | SFGAO_GHOSTED | SFGAO_LINK | SFGAO_SHARE | SFGAO_FOLDER | SFGAO_FILESYSTEM;
		m_parentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&childPidl, &m_sfgao);
	}
	return m_sfgao;

}

DWORD CShellFile::GetAttributes()
{
	if (m_fileAttributes == 0) {	
		UpdateWIN32_FIND_DATA();
	}
	return m_fileAttributes;
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
		m_size.first.LowPart = wfd.nFileSizeLow;
		m_size.first.HighPart = wfd.nFileSizeHigh;
		//if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		//	m_wstrSize = L"dir";
		//}
		//else {
		//	m_wstrSize = Size2String(m_size.QuadPart);
		//}
	}
}

bool CShellFile::IsShellFolder()
{
	if (!m_isShellFolder) {
		//Try BindToObject and EnumObjects to identify folder
		CComPtr<IShellFolder> pDesktop;
		::SHGetDesktopFolder(&pDesktop);
		CComPtr<IShellFolder> pFolder;
		HRESULT hr = S_OK;

		if (m_absolutePidl.m_pIDL->mkid.cb == 0) {
			pFolder = pDesktop;
		} else {
			hr = pDesktop->BindToObject(m_absolutePidl, 0, IID_IShellFolder, (void**)&pFolder);
		}

		if (SUCCEEDED(hr)) {
			CComPtr<IEnumIDList> enumIdl;
			hr = pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl);
			m_isShellFolder =  SUCCEEDED(hr);
		} else {
			m_isShellFolder = false;
		}
	}
	return m_isShellFolder.value();

}

std::shared_ptr<CShellFolder> CShellFile::CastShellFolder()
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
	m_wstrNameExt.clear();
	m_wstrType.clear();
	m_wstrCreationTime.clear();
	m_wstrLastAccessTime.clear();
	m_wstrLastWriteTime.clear();
	m_size = std::make_pair(ULARGE_INTEGER(), FileSizeStatus::None);

	m_isShellFolder = boost::none;

	m_size.first.QuadPart = 0;
	m_fileAttributes = 0;
	m_icon = std::make_pair(std::shared_ptr<CIcon>(nullptr), FileIconStatus::None);
	m_sfgao = 0;

}

bool CShellFile::IsDirectory()
{
	return GetAttributes() & FILE_ATTRIBUTE_DIRECTORY;
}

bool CShellFile::HasIconInCache()
{
	return s_iconCache.Exist(this);
}



