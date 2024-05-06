#include "ShellFile.h"
#include "MyIcon.h"
#include "MyString.h"
#include "MyCom.h"
#include "FileSizeArgs.h"
#include "ShellFunction.h"
#include "KnownFolderManager.h"
#include "KnownFolder.h"

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

CShellFile::CShellFile(const std::wstring& path)
	:m_optPath(path)
{
	Load(path);
}

CShellFile::CShellFile(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl)
	:m_pParentShellFolder(pParentShellFolder),m_absoluteIdl(parentIdl + childIdl), m_parentIdl(parentIdl), m_childIdl(childIdl)
{
	if (!m_absoluteIdl) {
		::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, m_absoluteIdl.ptrptr());
	}
}

CShellFile::~CShellFile() = default;

const std::wstring& CShellFile::GetPath() const
{
	if (!m_optPath.has_value()) {
		m_optPath.emplace(shell::GetDisplayNameOf(m_pParentShellFolder, m_childIdl.ptr(), SHGDN_FORPARSING));
	}
	return m_optPath.value();
}

const std::wstring& CShellFile::GetPathName() const
{
	if (!m_optPathName.has_value()) {
		m_optPathName.emplace(::PathFindFileNameW(GetPath().c_str()));
	}
	return m_optPathName.value();
}

const std::wstring& CShellFile::GetPathNameWithoutExt() const
{
	if (!m_optPathNameWithoutExt.has_value()) {
		std::wstring ret = GetPathName();
		::PathRemoveExtensionW(::GetBuffer(ret, MAX_PATH));
		::ReleaseBuffer(ret);
		m_optPathNameWithoutExt.emplace(ret);
	}
	return m_optPathNameWithoutExt.value();
}


const std::wstring& CShellFile::GetPathExt() const
{
	if(!m_optPathExt.has_value()){
		m_optPathExt.emplace(::PathFindExtensionW(GetPath().c_str()));
	}
	return m_optPathExt.value();
}

const std::wstring& CShellFile::GetPathWithoutExt() const
{
	if (!m_optPathWithoutExt.has_value()) {
		std::wstring ret = GetPath();
		::PathRemoveExtensionW(::GetBuffer(ret, MAX_PATH));
		::ReleaseBuffer(ret);
		m_optPathWithoutExt.emplace(ret);
	}
	return m_optPathWithoutExt.value();
}

const std::wstring& CShellFile::GetDispName() const
{
	return GetPathName();
}

const std::wstring& CShellFile::GetDispNameWithoutExt() const
{
	return GetPathNameWithoutExt();
}

const std::wstring& CShellFile::GetDispExt() const
{
	return GetPathExt();
}

void CShellFile::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND hWnd)
{
	HRESULT hr = m_pParentShellFolder->SetNameOf(
		hWnd,
		m_childIdl.ptr(),
		(wstrNameWoExt + GetDispExt()).c_str(),
		SHGDN_FORPARSING | SHGDN_INFOLDER,
		nullptr);
}

void CShellFile::SetExt(const std::wstring& wstrExt, HWND hWnd)
{
	HRESULT hr = m_pParentShellFolder->SetNameOf(
		hWnd,
		m_childIdl.ptr(),
		(GetDispNameWithoutExt() + wstrExt).c_str(),
		SHGDN_FORPARSING | SHGDN_INFOLDER,
		nullptr);
}

const std::wstring& CShellFile::GetTypeName() const
{
	if (!m_optTypeName.has_value()) {
		SHFILEINFO sfi = { 0 };
		::SHGetFileInfo((LPCTSTR)m_childIdl.ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_TYPENAME);
		m_optTypeName.emplace(sfi.szTypeName);
	}
	return m_optTypeName.value();
}

const DWORD& CShellFile::GetAttributes() const
{
	if (!m_optAttributes.has_value()) {	
		WIN32_FIND_DATA wfd = { 0 };
		if (SUCCEEDED(::SHGetDataFromIDList(m_pParentShellFolder, m_childIdl.ptr(), SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
			m_optAttributes.emplace(wfd.dwFileAttributes);
		} else {
			m_optAttributes.emplace(0);
		}
	}
	return m_optAttributes.value();
}

const SFGAOF& CShellFile::GetSFGAO() const
{
	if (!m_optSFGAO.has_value()) {
		SFGAOF sfgao{SFGAO_CAPABILITYMASK | SFGAO_GHOSTED | SFGAO_LINK | SFGAO_SHARE | SFGAO_FOLDER | SFGAO_FILESYSTEM};
		m_pParentShellFolder->GetAttributesOf(1, (LPCITEMIDLIST*)(m_childIdl.ptrptr()), &sfgao);
		m_optSFGAO.emplace(sfgao);
	}
	return m_optSFGAO.value();
}

const std::wstring& CShellFile::GetIconKey() const
{
	//if (IsInvalid()) {
	//	static std::wstring def{L"DEFAULT"};
	//	return def;
	//}

	if (!m_optIconKey.has_value()) {
		static std::set<std::wstring> excludeExtSet({L".exe", L".ico", L".lnk", L"known", L"drive"});
		std::wstring key = L"DEFAULT";
		if (!m_absoluteIdl || GetPath().empty()) {
			key = L"DEFAULT";
		} else if (!GetDispExt().empty() && excludeExtSet.find(GetDispExt()) == excludeExtSet.end() && GetAttributes() != 0) {
			key = GetDispExt();
		} else {
			key = GetPath();
		}
		m_optIconKey.emplace(key);
	}
	return m_optIconKey.value();
}

CIcon CShellFile::GetIcon() const
{
	SHFILEINFO sfi = { 0 };
	::SHGetFileInfo((LPCTSTR)m_absoluteIdl.ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	return CIcon(sfi.hIcon);
}


//std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFile::GetLockIcon()
//{
//	std::lock_guard<std::mutex> lock(m_mtxIcon);
//	return m_icon;
//}
//
//void CShellFile::SetLockIcon(std::pair<std::shared_ptr<CIcon>, FileIconStatus>& icon)
//{
//	std::lock_guard<std::mutex> lock(m_mtxIcon);
//	m_icon = icon;
//}

std::optional<FileTimes> CShellFile::GetFileTimes()
{
	return shell::GetFileTimes(GetParentShellFolderPtr(), GetChildIdl());
}

bool CShellFile::GetFileSize(ULARGE_INTEGER& size/*, std::shared_future<void> future*/)
{
	return shell::GetFileSize(size, m_pParentShellFolder, m_childIdl);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFile::ReadSize()
{
	return m_size;
}


std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFile::GetSize(const FileSizeArgs& args, std::function<void()> changed)
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

std::pair<FileTimes, FileTimeStatus> CShellFile::GetFileTimes(const FileTimeArgs& args, std::function<void()> changed)
{
	switch (m_fileTimes.second) {
	case FileTimeStatus::None:
	{
		if (auto times = GetFileTimes(); times.has_value()) {
			m_fileTimes = std::make_pair(times.value(), FileTimeStatus::Available);
		} else {
			m_fileTimes = std::make_pair(FileTimes(), FileTimeStatus::Unavailable);
		}
	}
	break;
	case FileTimeStatus::Available:
	case FileTimeStatus::Unavailable:
		break;
	}
	return m_fileTimes;
}

bool CShellFile::GetIsExist()
{
	auto a = ::PathFileExistsW(GetPath().c_str());
	auto b = shell::GetDisplayNameOf(m_pParentShellFolder, m_childIdl.ptr(), SHGDN_FORPARSING);

	ULONG chEaten = 0;
	ULONG dwAttributes = 0;
	auto desktop(CKnownFolderManager::GetInstance()->GetDesktopFolder());
	CIDL absIdl;
	auto c = SUCCEEDED(desktop->GetShellFolderPtr()->ParseDisplayName(
		NULL,
		NULL,
		const_cast<LPWSTR>(GetPath().c_str()),
		&chEaten,
		absIdl.ptrptr(),
		&dwAttributes));

	if (b.empty() || !c) {
		return false;
	} else {
		return true;
	}

}


void CShellFile::ResetSize()
{
	m_size = std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::None);
}

void CShellFile::ResetTime()
{
	m_fileTimes = std::make_pair(FileTimes(), FileTimeStatus::None);
}

void CShellFile::Reset()
{
	ResetOpts();

	ResetSize();
	ResetTime();
}

bool CShellFile::IsInvalid() const
{
	return !::PathFileExists(GetPath().c_str());
}


bool CShellFile::IsDirectory() const
{
	return GetAttributes() & FILE_ATTRIBUTE_DIRECTORY;
}

void CShellFile::Execute(const wchar_t* lpVerb)
{
	SHELLEXECUTEINFO	sei = { 0 };
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOASYNC ;
	sei.lpVerb = lpVerb;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpIDList = (LPVOID)(GetAbsoluteIdl().ptr());

	::ShellExecuteEx(&sei);
}

void CShellFile::Load(const std::wstring& path)
{
	m_optPath.emplace(path);
	if (path.empty() || path[0] != L':' && !::PathFileExistsW(path.c_str())) {
		//Invalid
	} else {
		m_absoluteIdl = CIDL(path.c_str());
		m_parentIdl = m_absoluteIdl.CloneParentIDL();
		m_childIdl = m_absoluteIdl.CloneLastID();
		m_pParentShellFolder = shell::DesktopBindToShellFolder(m_parentIdl);
	}
}

void CShellFile::Open()
{
	Execute(nullptr);
}

void CShellFile::RunAs()
{
	Execute(L"RunAs");
}

void CShellFile::AddToRecentDocs()
{
	::SHAddToRecentDocs(SHARD::SHARD_PIDL, m_parentIdl.m_pIDL);
	::SHAddToRecentDocs(SHARD::SHARD_PIDL, m_absoluteIdl.m_pIDL);
}



