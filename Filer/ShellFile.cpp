#include "ShellFile.h"
#include "MyIcon.h"
#include "MyString.h"
#include "MyCom.h"
#include "ShellFolder.h"
#include <thread>
#include <regex>

CFileIconCache CShellFile::s_iconCache;

std::shared_ptr<CShellFile> CreateShExFileFolder(CComPtr<IShellFolder>& pParentShellFolder, CIDLPtr& absolutePidl)
{
	CComPtr<IShellFolder> pFolder;
	CComPtr<IEnumIDList> enumIdl;
	if (SUCCEEDED(pParentShellFolder->BindToObject(absolutePidl.GetLastIDLPtr(), 0, IID_IShellFolder, (void**)&pFolder)) &&
		SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl))){
		return std::make_shared<CShellFolder>(pFolder, pParentShellFolder, absolutePidl);
	} else {
		return std::make_shared<CShellFile>(pParentShellFolder, absolutePidl);
	}
}

std::shared_ptr<CShellFile> CreateShExFileFolder(const std::wstring& path)
{
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);
	CIDLPtr absolutePidl;

	ULONG         chEaten;
	ULONG         dwAttributes;
	HRESULT hr = pDesktop->ParseDisplayName(
		NULL,
		NULL,
		const_cast<LPWSTR>(path.c_str()),
		&chEaten,
		&absolutePidl,
		&dwAttributes);
	if (FAILED(hr)) { throw std::exception("CreateShExFileFolder"); }

	return ::CreateShExFileFolder(pDesktop, absolutePidl);
}



//
//bool IsShExFolder(CComPtr<IShellFolder>& pParentFolder, CIDLPtr& pidlChild) 
//{
//	CComPtr<IShellFolder> pFolder;
//	if (SUCCEEDED(pParentFolder->BindToObject(pidlChild, 0, IID_IShellFolder, (void**)&pFolder))) {
//		CComPtr<IEnumIDList> enumIdl;
//		return SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl));
//	}
//	return false;
//}
//
//bool IsShExFolder(CIDLPtr& pidlAbsolute)
//{
//	//Try BindToObject and EnumObjects to identify folder
//	CComPtr<IShellFolder> pDesktop;
//	::SHGetDesktopFolder(&pDesktop);
//	CComPtr<IShellFolder> pFolder;
//	HRESULT hr = S_OK;
//
//	if (pidlAbsolute.m_pIDL->mkid.cb == 0) {
//		pFolder = pDesktop;
//	} else {
//		hr = pDesktop->BindToObject(pidlAbsolute, 0, IID_IShellFolder, (void**)&pFolder);
//	}
//
//	if (SUCCEEDED(hr)) {
//		CComPtr<IEnumIDList> enumIdl;
//		hr = pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl);
//		return SUCCEEDED(hr);
//	} else {
//		return false;
//	}
//}

bool GetFileSize(CComPtr<IShellFolder>& parentFolder, CIDLPtr childIDL, ULARGE_INTEGER& size)
{
	WIN32_FIND_DATA wfd = { 0 }; 
	if (!FAILED(SHGetDataFromIDList(parentFolder, childIDL, SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
		size.LowPart = wfd.nFileSizeLow;
		size.HighPart = wfd.nFileSizeHigh;
		return true;
	} else {
		return false;
	}
}

bool GetFolderSize(std::shared_ptr<CShellFolder>& pFolder, ULARGE_INTEGER& size, std::shared_future<void> future /*bool& cancel*//* std::function<void()> checkExit*/)
{
	try {
		//Enumerate child IDL
		size.QuadPart = 0;
		ULARGE_INTEGER childSize = { 0 };

		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(pFolder->GetShellFolderPtr()->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDLPtr nextIdl;
			ULONG ulRet(0);
			while (1) {
				if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
					std::wcout << "::GetFolderSize canceled" << std::endl;
					return false;
				}
				::Sleep(1000);
			}

			//while (SUCCEEDED(enumIdl->Next(1, &nextIdl, &ulRet))) {
			//	if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
			//		std::wcout << "::GetFolderSize canceled" << std::endl;
			//		return false;
			//	}
			//	if (!nextIdl) { break; }
			//	auto spFile(std::make_shared<CShellFile>(pFolder->GetShellFolderPtr(), ::ILCombine(pFolder->GetAbsolutePidl(), nextIdl)));
			//	if (spFile->IsShellFolder()) {
			//		if (auto spFolder = spFile->CastShellFolder()) {
			//			if (GetFolderSize(spFolder, childSize, future)) {
			//				size.QuadPart += childSize.QuadPart;
			//			} else {
			//				return false;
			//			}
			//		}
			//	} else {
			//		if (GetFileSize(pFolder->GetShellFolderPtr(), spFile->GetAbsolutePidl().GetLastIDLPtr(), childSize)) {
			//			size.QuadPart += childSize.QuadPart;
			//		} else {
			//			return false;
			//		}
			//	}
			//	nextIdl.Clear();
			//}
		}
	} catch (...) {
		std::wcout << "::GetFolderSize Exception" << std::endl;
		return false;
	}
	return true;

}


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
			//if (cancel()) {
			//	std::wcout << L"GetDirSize canceled" << std::endl;
			//	return false;
			//}

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
	//std::wcout << L"~CShellFile " + GetName() << std::endl;
	try {
		if(m_pIconThread && m_pIconThread->joinable()) {
			std::wcout << L"CShellFile::~CShellFile " + GetFileNameWithoutExt() + L" Icon thread canceled" << std::endl;
			m_iconPromise.set_value();
			m_pIconThread->join();
		}
		SignalFileIconChanged.disconnect_all_slots();
	} catch (...) {
		std::wcout << L"CShellFile::~CShellFile Exception" << std::endl;
		if (m_pIconThread) m_pIconThread->detach();
	}
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

std::wstring CShellFile::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		CIDLPtr childPidl = m_absolutePidl.GetLastIDLPtr();
		STRRET strret;
		m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_FOREDITING | SHGDN_INFOLDER, &strret);
		m_wstrFileNameWithoutExt = childPidl.STRRET2WSTR(strret);
	}
	return m_wstrFileNameWithoutExt;
}

std::wstring CShellFile::GetFileName()
{
	if (m_wstrFileName.empty()) {
		CIDLPtr childPidl = m_absolutePidl.GetLastIDLPtr();
		STRRET strret;
		m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_FORPARSING | SHGDN_INFOLDER, &strret);
		m_wstrFileName = childPidl.STRRET2WSTR(strret);
	}
	return m_wstrFileName;
}

std::wstring CShellFile::GetExt()
{
	if (m_wstrExt.empty()) {
		auto name(GetFileName());
		auto nameWoExt(GetFileNameWithoutExt());
		if (name.size() >= nameWoExt.size() &&
			std::equal(std::begin(nameWoExt), std::end(nameWoExt), std::begin(name))) {
			std::copy(std::next(name.begin(), nameWoExt.size()), name.end(), std::back_inserter(m_wstrExt));
		} else {
			m_wstrExt = ::PathFindExtension(GetPath().c_str());
		}
	}
	return m_wstrExt;
}

void CShellFile::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt)
{
	CIDLPtr pIdlNew;
	if(SUCCEEDED(GetParentShellFolderPtr()->SetNameOf(
		NULL,
		GetAbsolutePidl().FindLastID(),
		wstrNameWoExt.c_str(),
		SHGDN_FOREDITING | SHGDN_INFOLDER,
		&pIdlNew))){
		
		m_absolutePidl = GetAbsolutePidl().GetPreviousIDLPtr() + pIdlNew;

		Reset();
	}

}

void CShellFile::SetExt(const std::wstring& wstrExt)
{
	CIDLPtr pIdlNew;
	if (SUCCEEDED(GetParentShellFolderPtr()->SetNameOf(
		NULL,
		GetAbsolutePidl().FindLastID(),
		(GetFileNameWithoutExt() + wstrExt).c_str(),
		SHGDN_FORPARSING | SHGDN_INFOLDER,
		&pIdlNew))) {

		m_absolutePidl = GetAbsolutePidl().GetPreviousIDLPtr() + pIdlNew;

		Reset();
	}


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

bool CShellFile::GetFileSize(ULARGE_INTEGER& size, std::shared_future<void> future)
{
	return ::GetFileSize(m_parentFolder, m_absolutePidl.GetLastIDLPtr(), size);
}


std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFile::GetSize()
{
	switch (m_size.second) {
	case FileSizeStatus::None:
	{
		ULARGE_INTEGER size = { 0 };
		if (::GetFileSize(m_parentFolder, m_absolutePidl.GetLastIDLPtr(), size)) {
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

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFile::GetIcon()
{
	switch (GetLockIcon().second) {
	case FileIconStatus::None:
		if (HasIconInCache()) {
			SetLockIcon(std::make_pair( s_iconCache.GetIcon(this), FileIconStatus::Available));
		} else {
			SetLockIcon(std::make_pair(GetDefaultIcon(), FileIconStatus::Loading));
			if (!m_pIconThread) {
				//std::weak_ptr<CShellFile> wpFile(shared_from_this());

				m_iconFuture = m_iconPromise.get_future();
				m_pIconThread.reset(new std::thread([this]()->void {
					//if (auto sp = wpFile.lock()) {
						SetLockIcon(std::make_pair(s_iconCache.GetIcon(this), FileIconStatus::Available));
						SignalFileIconChanged(this);
					//}
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
		//m_size.first.LowPart = wfd.nFileSizeLow;
		//m_size.first.HighPart = wfd.nFileSizeHigh;
		//if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		//	m_wstrSize = L"dir";
		//}
		//else {
		//	m_wstrSize = Size2String(m_size.QuadPart);
		//}
	}
}
//
//bool CShellFile::IsShellFolder()
//{
//	if (!m_isShellFolder) {
//		//Try BindToObject and EnumObjects to identify folder
//		CComPtr<IShellFolder> pDesktop;
//		::SHGetDesktopFolder(&pDesktop);
//		CComPtr<IShellFolder> pFolder;
//		HRESULT hr = S_OK;
//
//		if (m_absolutePidl.m_pIDL->mkid.cb == 0) {
//			pFolder = pDesktop;
//		} else {
//			hr = pDesktop->BindToObject(m_absolutePidl, 0, IID_IShellFolder, (void**)&pFolder);
//		}
//
//		if (SUCCEEDED(hr)) {
//			CComPtr<IEnumIDList> enumIdl;
//			hr = pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl);
//			m_isShellFolder =  SUCCEEDED(hr);
//		} else {
//			m_isShellFolder = false;
//		}
//	}
//	return m_isShellFolder.value();
//
//}
//
//std::shared_ptr<CShellFolder> CShellFile::CastShellFolder()
//{
//	if (IsShellFolder()) {
//		CComPtr<IShellFolder> pFolder;
//		HRESULT hr = 0;
//		if (m_absolutePidl.m_pIDL->mkid.cb == 0) {
//			if (SUCCEEDED(::SHGetDesktopFolder(&pFolder))) {
//				return std::make_shared<CShellFolder>(pFolder, m_parentFolder, m_absolutePidl);
//			}
//		}else {
//			CComPtr<IShellFolder> pDesktop;
//			if (SUCCEEDED(::SHGetDesktopFolder(&pDesktop))) {
//				if (SUCCEEDED(pDesktop->BindToObject(m_absolutePidl, 0, IID_IShellFolder, (void**)&pFolder))) {
//					return std::make_shared<CShellFolder>(pFolder, m_parentFolder, m_absolutePidl);
//				}
//			}
//		}
//		return nullptr;
//	}
//	else {
//		return nullptr;
//	}
//}

void CShellFile::ResetIcon()
{
	if (m_pIconThread && m_pIconThread->joinable()) {
		std::wcout << L"CShellFile::~CShellFile Icon thread canceled" << std::endl;
		m_iconPromise.set_value();
		m_pIconThread->join();
	}
	m_pIconThread.reset();
	m_iconPromise = std::promise<void>();
	m_iconFuture = std::future<void>();
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
	m_wstrCreationTime.clear();
	m_wstrLastAccessTime.clear();
	m_wstrLastWriteTime.clear();

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
	return s_iconCache.Exist(this);
}



