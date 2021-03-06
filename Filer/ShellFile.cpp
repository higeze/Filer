#include "ShellFile.h"
#include "MyIcon.h"
#include "MyString.h"
#include "MyCom.h"
#include "FileIconCache.h"
#include "FileSizeArgs.h"
#include "ShellFunction.h"

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

CShellFile::CShellFile(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl)
	:m_pParentShellFolder(pParentShellFolder),m_absoluteIdl(parentIdl + childIdl), m_parentIdl(parentIdl), m_childIdl(childIdl)
{
	if (!m_absoluteIdl) {
		::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, m_absoluteIdl.ptrptr());
	}
}

CShellFile::~CShellFile() = default;

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

std::pair<FileTimes, FileTimeStatus> CShellFile::GetFileTimes(std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed)
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
		m_fileAttributes = wfd.dwFileAttributes;
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
	m_wstrPath.clear();
	m_wstrFileNameWithoutExt.clear();
	m_wstrFileName.clear();
	m_wstrExt.clear();
	m_wstrType.clear();

	m_fileAttributes = 0;
	m_sfgao = 0;

	ResetSize();
	ResetTime();
}

bool CShellFile::IsDirectory()
{
	return GetAttributes() & FILE_ATTRIBUTE_DIRECTORY;
}



