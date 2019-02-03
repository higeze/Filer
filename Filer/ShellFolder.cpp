#include "ShellFolder.h"
#include "ShellZipFolder.h"
#include "KnownFolder.h"
#include "DriveFolder.h"
#include "FileIconCache.h"
#include "ConsoleTimer.h"
#include "ApplicationProperty.h"
#include "FileSizeArgs.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CShellFolder::CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CShellFile(pParentShellFolder, parentIdl, childIdl), m_pShellFolder(pShellFolder) {}


CShellFolder::~CShellFolder()
{
	//std::wcout << L"~CShellFolder " + GetFileName() << std::endl;
	try {
		m_cancelThread.store(true);
		if (m_pSizeThread && m_pSizeThread->joinable()) {
			BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFolder Size thread join : " + GetPath();
			m_pSizeThread->join();
		}
		if (m_pTimeThread && m_pTimeThread->joinable()) {
			BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFolder Time thread join : " + GetPath();
			m_pTimeThread->join();
		}
		SignalFileSizeChanged.disconnect_all_slots();
	} catch (...) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFile Exception Thread detached";
		if (m_pSizeThread) m_pSizeThread->detach();
		if (m_pTimeThread) m_pTimeThread->detach();
	}
}

CComPtr<IShellFolder> CShellFolder::GetShellFolderPtr()
{
	if (!m_pShellFolder) {
		if (FAILED(m_pParentShellFolder->BindToObject(m_childIdl.ptr(), 0, IID_IShellFolder, (void**)&m_pShellFolder))){
			throw std::exception("CShellFolder::GetShellFolderPtr");
		}
	}
	return m_pShellFolder;
}

std::pair<FILETIME, FileTimeStatus> CShellFolder::GetLockLastWriteTime()
{
	std::lock_guard<std::mutex> lock(m_mtxTime);
	return m_lastWriteTime;
}

void CShellFolder::SetLockLastWriteTime(std::pair<FILETIME, FileTimeStatus>& time)
{
	std::lock_guard<std::mutex> lock(m_mtxTime);
	m_lastWriteTime = time;
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFolder::GetLockSize()
{
	std::lock_guard<std::mutex> lock(m_mtxSize);
	return m_size;
}

void CShellFolder::SetLockSize(std::pair<ULARGE_INTEGER, FileSizeStatus>& size)
{
	std::lock_guard<std::mutex> lock(m_mtxSize);
	m_size = size;
}

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFolder::GetIcon()
{
	if (GetLockIcon().second == FileIconStatus::None) {
		SetLockIcon(std::make_pair(CFileIconCache::GetInstance()->GetFolderIcon(), FileIconStatus::Available));
	}
	return GetLockIcon();
}


std::wstring CShellFolder::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		if(::PathIsDirectory(GetPath().c_str()) || GetPath()[0] == L':'){
			STRRET strret;
			m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
			m_wstrFileNameWithoutExt = m_childIdl.STRRET2WSTR(strret);
		} else {
			return CShellFile::GetFileNameWithoutExt();
		}
	}
	return m_wstrFileNameWithoutExt;
}

std::wstring CShellFolder::GetFileName()
{
	if (m_wstrFileName.empty()) {
		if (::PathIsDirectory(GetPath().c_str()) || GetPath()[0] == L':') {
			STRRET strret;
			m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
			m_wstrFileName = m_childIdl.STRRET2WSTR(strret);
		} else {
			return CShellFile::GetFileName();
		}
	}
	return m_wstrFileName;
}

std::wstring CShellFolder::GetExt()
{
	if (m_wstrExt.empty()) {
		if (::PathIsDirectory(GetPath().c_str()) ||  std::wstring(::PathFindFileName(GetPath().c_str()))[0] == L':') {
			m_wstrExt = L"";
		} else {
			return CShellFile::GetExt();
		}
	}
	return m_wstrExt;
}


std::shared_ptr<CShellFolder> CShellFolder::GetParent()
{
	CIDL parentIDL = m_absoluteIdl.CloneParentIDL();
	CIDL grandParentIDL = parentIDL.CloneParentIDL();
	CComPtr<IShellFolder> pParentFolder;
	CComPtr<IShellFolder> pGrandParentFolder;
	CComPtr<IShellFolder> pDesktopShellFolder;
	::SHGetDesktopFolder(&pDesktopShellFolder);
	pDesktopShellFolder->BindToObject(parentIDL.ptr(), 0, IID_IShellFolder, (void**)&pParentFolder);
	pDesktopShellFolder->BindToObject(grandParentIDL.ptr(), 0, IID_IShellFolder, (void**)&pGrandParentFolder);
	if (!pParentFolder) {
		pParentFolder = pDesktopShellFolder;
	}
	if (!pGrandParentFolder) {
		pGrandParentFolder = pDesktopShellFolder;
	}
	return std::make_shared<CShellFolder>(pGrandParentFolder, grandParentIDL, parentIDL.CloneLastID(), pParentFolder);
}

std::shared_ptr<CShellFolder> CShellFolder::Clone()const
{
	return std::make_shared<CShellFolder>(m_pParentShellFolder, m_parentIdl, m_childIdl, m_pShellFolder);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFolder::GetSize(std::shared_ptr<FileSizeArgs>& spArgs)
{
	if (spArgs->NoFolderSize) {
		SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable));
	} else if(spArgs->NoFolderSizeOnNetwork && ::PathIsNetworkPath(GetPath().c_str())) {
		SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable));
	} else {
		switch (GetLockSize().second) {
			case FileSizeStatus::None:
				SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Calculating));

				if (!m_pSizeThread) {
					auto limit = spArgs->TimeLimitFolderSize ? spArgs->TimeLimitMs : -1;
					m_pSizeThread.reset(new std::thread([this, limit]()->void {
						try {
							boost::timer tim;
							ULARGE_INTEGER size = { 0 };
							if (GetFolderSize(size, m_cancelThread, tim, limit)) {
								SetLockSize(std::make_pair(size, FileSizeStatus::Available));
							} else {
								SetLockSize(std::make_pair(size, FileSizeStatus::Unavailable));
							}
							SignalFileSizeChanged(this);
						} catch (...) {
							BOOST_LOG_TRIVIAL(trace) << L"CShellFile::GetSize Exception at size thread";
						}
					}));
				} else {
					OutputDebugString(L"Already run");
				}
				break;
			case FileSizeStatus::Available:
			case FileSizeStatus::Unavailable:
			case FileSizeStatus::Calculating:
				break;
		}
	}
	return GetLockSize();
}

std::pair<FILETIME, FileTimeStatus> CShellFolder::GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs)
{
	switch (GetLockLastWriteTime().second) {
	case FileTimeStatus::None:
	{
		FILETIME time = { 0 };
		if (CShellFile::GetFileLastWriteTime(time)) {
			SetLockLastWriteTime(std::make_pair(time, FileTimeStatus::AvailableLoading));
		} else {
			SetLockLastWriteTime(std::make_pair(time, FileTimeStatus::Loading));
		}
		if (!m_pTimeThread) {
			auto limit = spArgs->TimeLimitFolderLastWrite ? spArgs->TimeLimitMs : -1;
			m_pTimeThread.reset(new std::thread([this, limit]()->void {
				try {
					boost::timer tim;
					FILETIME time = { 0 };
					if (GetFolderLastWriteTime(time, m_cancelThread, tim, limit) && (time.dwLowDateTime || time.dwHighDateTime)) {
						SetLockLastWriteTime(std::make_pair(time, FileTimeStatus::Available));
					} else {
						SetLockLastWriteTime(std::make_pair(time, FileTimeStatus::Unavailable));
					}
					SignalTimeChanged(this);
				} catch (...) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFile::GetLastWriteTime Exception at time thread";
				}
			}));
		} else {
			OutputDebugString(L"Already run");
		}
	}
		break;
	case FileTimeStatus::Available:
	case FileTimeStatus::Unavailable:
	case FileTimeStatus::Loading:
		break;
	}
	return GetLockLastWriteTime();
}

bool CShellFolder::GetFolderSize(ULARGE_INTEGER& size, std::atomic<bool>& cancel, boost::timer& tim, int limit)
{
	if (cancel.load()) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Canceled at top : " + GetPath();
		return false;
	} else if (limit > 0 && tim.elapsed() > limit/1000.0) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Timer elapsed at top : " + GetPath();
		return false;
	}
	try {
		//Enumerate child IDL
		size.QuadPart = 0;
		ULARGE_INTEGER childSize = { 0 };

		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(GetShellFolderPtr()->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDL nextIdl;
			ULONG ulRet(0);
			while (SUCCEEDED(enumIdl->Next(1, nextIdl.ptrptr(), &ulRet))) {
				if (cancel.load()) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Canceled in while : " + GetPath();
					return false;
				} else if (limit > 0 && tim.elapsed() > limit/1000.0) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Timer elapsed in while : " + GetPath();
					return false;
				}
				if (!nextIdl) { break; }

				auto spFile(CreateShExFileFolder(nextIdl));
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
					if (spFolder->GetFolderSize(childSize, cancel, tim, limit)) {
						size.QuadPart += childSize.QuadPart;
					} else {
						return false;
					}
				} else {
					if (spFile->GetFileSize(childSize)) {
						size.QuadPart += childSize.QuadPart;
					} else {
						return false;
					}
				}
				childSize.QuadPart = 0;
				nextIdl.Clear();
			}
		}
	} catch (...) {
		BOOST_LOG_TRIVIAL(trace) << "Exception CShellFolder::GetFolderSize " << GetPath();
		return false;
	}
	return true;

}

bool CShellFolder::GetFolderLastWriteTime(FILETIME& time, std::atomic<bool>& cancel, boost::timer& tim, int limit)
{
	if (CShellFile::GetFileLastWriteTime(time)) {
	} else {
		time = FILETIME{ 0 };
	}

	if (cancel.load()) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderLastWriteTime Canceled at top : " + GetPath();
		return true;
	} else if (limit > 0 && tim.elapsed() > limit / 1000.0) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderLastWriteTime Timer elapsed at top : " + GetPath();
		return true;
	}
	try {
		//Enumerate child IDL
		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(GetShellFolderPtr()->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDL nextIdl;
			ULONG ulRet(0);
			while (SUCCEEDED(enumIdl->Next(1, nextIdl.ptrptr(), &ulRet))) {
				if (cancel.load()) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderLastWriteTime Canceled in while : " + GetPath();
					return true;
				} else if (limit > 0 && tim.elapsed() > limit / 1000.0) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderLastWriteTime Timer elapsed in while : " + GetPath();
					return true;
				}

				if (!nextIdl) { break; }

				auto spFile(CreateShExFileFolder(nextIdl));
				FILETIME childTime = { 0 };
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
					if (!spFolder->GetFolderLastWriteTime(childTime, cancel, tim, limit)) {
						childTime = { 0 };
					}
				} else {
					if (!spFile->GetFileLastWriteTime(childTime)) {
						childTime = { 0 };
					}
				}

				if (childTime.dwLowDateTime || childTime.dwHighDateTime) {
					ULARGE_INTEGER latest = { time.dwLowDateTime, time.dwHighDateTime };
					ULARGE_INTEGER child = { childTime.dwLowDateTime, childTime.dwHighDateTime };
					latest = latest.QuadPart > child.QuadPart ? latest : child;
					//if (latest.QuadPart > child.QuadPart) {
					//	latest = latest;
					//} else {
					//	latest = child;
					//}
					time = FILETIME{ latest.LowPart, latest.HighPart };
				}

				nextIdl.Clear();
			}
		}
	} catch (...) {
		BOOST_LOG_TRIVIAL(trace) << "Exception CShellFolder::GetFolderLastWriteTime " << GetPath();
		return false;
	}
	return true;

}

//void CShellFolder::ResetSize()
//{
//	if (m_pSizeThread && m_pSizeThread->joinable()) {
//		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::ResetIcon Icon thread canceled";
//		//m_sizePromise.set_value();
//		m_cancelThread.store(true);
//		m_pSizeThread->join();
//	}
//	m_pSizeThread.reset();
//	m_cancelThread.store(false);
//	SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::None));
//}

std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(CIDL& childIdl)
{
	boost::timer tim;
//	CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"CreateShExFileFolder")

	CComPtr<IShellFolder> pFolder;
	CComPtr<IEnumIDList> enumIdl;	
	STRRET strret;
	GetShellFolderPtr()->GetDisplayNameOf(childIdl.ptr(), SHGDN_FORPARSING, &strret);
	std::wstring path =  childIdl.STRRET2WSTR(strret);
	std::wstring ext = ::PathFindExtension(path.c_str());

	if (!childIdl) {
		return CKnownFolderManager::GetInstance()->GetDesktopFolder();
	} else if (auto spKnownFolder = CKnownFolderManager::GetInstance()->GetKnownFolderByPath(path)) {
		return spKnownFolder;
	} else if (auto spDriveFolder = CDriveManager::GetInstance()->GetDriveFolderByPath(path)) {
		return spDriveFolder;
	} else if (boost::iequals(ext, ".zip")) {
		return std::make_shared<CShellZipFolder>(GetShellFolderPtr(), m_absoluteIdl, childIdl);
	} else if (
		//::PathIsDirectory(path.c_str()) ||
		(SUCCEEDED(GetShellFolderPtr()->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pFolder)) &&
		SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)))) {
		return std::make_shared<CShellFolder>(GetShellFolderPtr(), m_absoluteIdl, childIdl, pFolder);
	} else {
		return std::make_shared<CShellFile>(GetShellFolderPtr(), m_absoluteIdl, childIdl);
	}

	if (tim.elapsed() > 0.1) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::CreateShExFileFolder Over0.1 "<< path;
	}
}


//static
std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(const std::wstring& path)
{
	auto desktop(CKnownFolderManager::GetInstance()->GetDesktopFolder());
	CIDL relativeIdl;

	ULONG         chEaten;
	ULONG         dwAttributes;
	HRESULT hr = desktop->GetShellFolderPtr()->ParseDisplayName(
		NULL,
		NULL,
		const_cast<LPWSTR>(path.c_str()),
		&chEaten,
		relativeIdl.ptrptr(),
		&dwAttributes);
	if (FAILED(hr)) {
		return desktop;
	}

	return desktop->CreateShExFileFolder(relativeIdl);
}





