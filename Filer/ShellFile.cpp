#include "ShellFile.h"
#include "MyIcon.h"
#include "MyString.h"
#include "MyCom.h"
#include "FileIconCache.h"
#include <thread>
#include "FileSizeArgs.h"

bool GetDirSize(std::wstring path, ULARGE_INTEGER& size, std::function<void()> checkExit)
{
	try {
		size.QuadPart = 0;
		ULARGE_INTEGER childSize = { 0 };

		WIN32_FIND_DATA findData;
		std::unique_ptr <std::remove_pointer<HANDLE>::type, findclose> upFind(FindFirstFile((path + L"\\*").c_str(), &findData));
		if (upFind.get() == INVALID_HANDLE_VALUE) {
			return false;
		}

		do {
			checkExit();

			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				//Ignore "." ".."
				if (!_tcscmp(_T("."), findData.cFileName) || !_tcscmp(_T(".."), findData.cFileName)) {
					continue;
				}
				std::wstring nextPath(path + L"\\" + findData.cFileName);
				if (!GetDirSize(nextPath.c_str(), childSize, checkExit)) {
					return false;
				}
				size.QuadPart += childSize.QuadPart;
			} else {
				childSize.HighPart = findData.nFileSizeHigh;
				childSize.LowPart = findData.nFileSizeLow;
				size.QuadPart += childSize.QuadPart;
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
	// TODO depends on locale
	//return boost::lexical_cast<std::wstring>(n);
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

CShellFile::CShellFile(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl)
	:m_pParentShellFolder(pParentShellFolder),m_absoluteIdl(parentIdl + childIdl), m_parentIdl(parentIdl), m_childIdl(childIdl)
{
	if (!m_absoluteIdl) {
		::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, m_absoluteIdl.ptrptr());
	}

}

CShellFile::~CShellFile()
{
	try {
		if(m_pIconThread && m_pIconThread->joinable()) {
			BOOST_LOG_TRIVIAL(trace) << L"CShellFile::~CShellFile " + GetFileNameWithoutExt() + L" Icon thread canceled";
			m_pIconThread->join();
		}
	} catch (...) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFile::~CShellFile Exception Icon thread detached";
		if (m_pIconThread) m_pIconThread->detach();
	}
}

std::wstring& CShellFile::GetPath()
{
	if (m_wstrPath.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_FORPARSING, &strret);
		m_wstrPath = m_childIdl.STRRET2WSTR(strret);
	}
	return m_wstrPath;
}

std::wstring CShellFile::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		m_wstrFileNameWithoutExt = GetFileName();
		::PathRemoveExtension(::GetBuffer(m_wstrFileNameWithoutExt, MAX_PATH));
		::ReleaseBuffer(m_wstrFileNameWithoutExt);
	}
	return m_wstrFileNameWithoutExt;
}

std::wstring CShellFile::GetFileName()
{
	if (m_wstrFileName.empty()) {
		m_wstrFileName = ::PathFindFileName(GetPath().c_str());
	}
	return m_wstrFileName;
}

std::wstring CShellFile::GetExt()
{
	if (m_wstrExt.empty()) {
		m_wstrExt = ::PathFindExtension(GetFileName().c_str());
	}
	return m_wstrExt;
}

void CShellFile::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt)
{
	CIDL idlNew;
	if(SUCCEEDED(m_pParentShellFolder->SetNameOf(
		NULL,
		m_childIdl.ptr(),
		(wstrNameWoExt + GetExt()).c_str(),
		SHGDN_FORPARSING | SHGDN_INFOLDER,
		idlNew.ptrptr()))){
		
		m_childIdl = idlNew;
		m_absoluteIdl = m_parentIdl + idlNew;

		Reset();
	}

}

void CShellFile::SetExt(const std::wstring& wstrExt)
{
	CIDL idlNew;
	if (SUCCEEDED(m_pParentShellFolder->SetNameOf(
		NULL,
		m_childIdl.ptr(),
		(GetFileNameWithoutExt() + wstrExt).c_str(),
		SHGDN_FORPARSING | SHGDN_INFOLDER,
		idlNew.ptrptr()))) {

		m_childIdl = idlNew;
		m_absoluteIdl = m_parentIdl + idlNew;

		Reset();
	}
}

std::wstring CShellFile::GetTypeName()
{
	if (m_wstrType.empty()) {
		SHFILEINFO sfi = { 0 };
		::SHGetFileInfo((LPCTSTR)m_childIdl.ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_TYPENAME);
		m_wstrType = sfi.szTypeName;
	}
	return m_wstrType;
}

//std::wstring CShellFile::GetCreationTime()
//{
//	if (m_wstrCreationTime.empty()) {
//		UpdateWIN32_FIND_DATA();
//	}
//	return m_wstrCreationTime;
//}
//
//std::wstring CShellFile::GetLastAccessTime()
//{
//	if (m_wstrLastAccessTime.empty()) {
//		UpdateWIN32_FIND_DATA();
//	}
//	return m_wstrLastAccessTime;
//}
//
//std::wstring CShellFile::GetLastWriteTime()
//{
//	if (m_wstrLastWriteTime.empty()) {
//		UpdateWIN32_FIND_DATA();
//	}
//	return m_wstrLastWriteTime;
//}

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFile::GetLockIcon()
{
	std::lock_guard<std::mutex> lock(m_mtxIcon);
	return m_icon;
}

void CShellFile::SetLockIcon(std::pair<std::shared_ptr<CIcon>, FileIconStatus>& icon)
{
	std::lock_guard<std::mutex> lock(m_mtxIcon);
	m_icon = icon;
}

bool CShellFile::GetFileLastWriteTime(FILETIME& time)
{
	return CShellFile::GetFileLastWriteTime(time, GetParentShellFolderPtr(), GetChildIdl());
}

//static
bool CShellFile::GetFileLastWriteTime(FILETIME& time, const CComPtr<IShellFolder>& pParentFolder, const CIDL& relativeIdl)
{
	WIN32_FIND_DATA wfd = { 0 };
	if (!FAILED(::SHGetDataFromIDList(pParentFolder, relativeIdl.ptr(), SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
		time = wfd.ftLastWriteTime;
		return true;
	} else {
		time = FILETIME{ 0 };
		return false;
	}
}


bool CShellFile::GetFileSize(ULARGE_INTEGER& size/*, std::shared_future<void> future*/)
{
	return CShellFile::GetFileSize(size, m_pParentShellFolder, m_childIdl);
}


//static
bool CShellFile::GetFileSize(ULARGE_INTEGER& size, const CComPtr<IShellFolder>& pParentShellFolder, const CIDL& childIdl)
{
	WIN32_FIND_DATA wfd = { 0 };
	if (!FAILED(::SHGetDataFromIDList(pParentShellFolder, childIdl.ptr(), SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
		size.LowPart = wfd.nFileSizeLow;
		size.HighPart = wfd.nFileSizeHigh;
		return true;
	} else {
		return false;
	}
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFile::ReadSize()
{
	return m_size;
}


std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFile::GetSize(std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed)
{
	switch (m_size.second) {
	case FileSizeStatus::None:
	{
		ULARGE_INTEGER size = { 0 };
		if (GetFileSize(size/*, nullptr*/)) {
			m_size = std::make_pair(size, FileSizeStatus::Available);
		} else {
			m_size = std::make_pair(size, FileSizeStatus::Unavailable);
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

std::pair<FILETIME, FileTimeStatus> CShellFile::GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed)
{
	switch (m_lastWriteTime.second) {
	case FileTimeStatus::None:
	{
		UpdateWIN32_FIND_DATA();
	}
	break;
	case FileTimeStatus::Available:
	case FileTimeStatus::Unavailable:
		break;
	}
	return m_lastWriteTime;
}

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFile::GetIcon(std::function<void(CShellFile*)>& changedAction)
{
	switch (GetLockIcon().second) {
	case FileIconStatus::None:
		if (HasIconInCache()) {
			SetLockIcon(std::make_pair(CFileIconCache::GetInstance()->GetIcon(this), FileIconStatus::Available));
		} else {
			SetLockIcon(std::make_pair(GetDefaultIcon(), FileIconStatus::Loading));
			if (!m_pIconThread) {
				m_pIconThread.reset(new std::thread([this, changedAction]()->void {
					try {
						CCoInitializer coinit(COINIT_APARTMENTTHREADED);
						SetLockIcon(std::make_pair(CFileIconCache::GetInstance()->GetIcon(this), FileIconStatus::Available));
						changedAction(this);
					} catch (...) {
						BOOST_LOG_TRIVIAL(error) << L"CShellFile::GetIcon " + GetFileNameWithoutExt() + L" Icon thread exception";
					}
				}));
			}
		}
		break;
	case FileIconStatus::Available:
	case FileIconStatus::Loading:
		break;
	}
	return GetLockIcon();
}

std::shared_ptr<CIcon> CShellFile::GetDefaultIcon()
{
	return CFileIconCache::GetInstance()->GetDefaultIcon();
}

UINT CShellFile::GetSFGAO()
{
	if (m_sfgao == 0) {
		m_sfgao = SFGAO_CAPABILITYMASK | SFGAO_GHOSTED | SFGAO_LINK | SFGAO_SHARE | SFGAO_FOLDER | SFGAO_FILESYSTEM;
		m_pParentShellFolder->GetAttributesOf(1, (LPCITEMIDLIST*)(m_childIdl.ptrptr()), &m_sfgao);
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
	WIN32_FIND_DATA wfd = { 0 };
	if (!FAILED(::SHGetDataFromIDList(m_pParentShellFolder, m_childIdl.ptr(), SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
		m_creationTime = std::make_pair(wfd.ftCreationTime, FileTimeStatus::Available);
		m_lastAccessTime = std::make_pair(wfd.ftLastAccessTime, FileTimeStatus::Available);
		m_lastWriteTime = std::make_pair(wfd.ftLastWriteTime, FileTimeStatus::Available);
		m_fileAttributes = wfd.dwFileAttributes;
	}
}

void CShellFile::ResetIcon()
{
	if (m_pIconThread && m_pIconThread->joinable()) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFile::~CShellFile Icon thread canceled";
		m_pIconThread->join();
	}
	m_pIconThread.reset();
	SetLockIcon(std::make_pair(std::shared_ptr<CIcon>(nullptr), FileIconStatus::None));
}

void CShellFile::ResetSize()
{
	m_size = std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::None);
}

void CShellFile::Reset()
{
	m_wstrPath.clear();
	m_wstrFileNameWithoutExt.clear();
	m_wstrFileName.clear();
	m_wstrExt.clear();
	m_wstrType.clear();
	m_size = std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::None);
	m_creationTime = std::make_pair(FILETIME{ 0 }, FileTimeStatus::None);
	m_lastAccessTime = std::make_pair(FILETIME{ 0 }, FileTimeStatus::None);
	m_lastWriteTime = std::make_pair(FILETIME{ 0 }, FileTimeStatus::None);

	m_fileAttributes = 0;
	m_sfgao = 0;

	ResetIcon();
	ResetSize();
}

bool CShellFile::IsDirectory()
{
	return GetAttributes() & FILE_ATTRIBUTE_DIRECTORY;
}

bool CShellFile::HasIconInCache()
{
	return CFileIconCache::GetInstance()->Exist(this);
}

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellInvalidFile::GetIcon(std::function<void(CShellFile*)>& changedAction)
{
	switch (GetLockIcon().second) {
	case FileIconStatus::None:
		SetLockIcon(std::make_pair(GetDefaultIcon(), FileIconStatus::Available));
		break;
	default:
		break;
	}
	return GetLockIcon();
}




