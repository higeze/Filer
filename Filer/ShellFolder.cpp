#include "ShellFolder.h"
#include "ShellZipFolder.h"
#include "KnownFolder.h"
#include "DriveFolder.h"
#include "FileIconCache.h"
#include "ConsoleTimer.h"
#include "ApplicationProperty.h"
#include "FileSizeArgs.h"
#include "FileTimeArgs.h"
#include "ThreadPool.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CShellFolder::CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CShellFile(pParentShellFolder, parentIdl, childIdl), m_pShellFolder(pShellFolder) {}


CShellFolder::~CShellFolder()
{
	*m_spCancelThread = true;

	//std::wcout << L"~CShellFolder " + GetFileName() << std::endl;
	//try {
	//	m_cancelThread.store(true);
	//	if (m_pSizeThread && m_pSizeThread->joinable()) {
	//		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFolder Size thread join : " + GetPath();
	//		m_pSizeThread->join();
	//	}
	//	SignalFileSizeChanged.disconnect_all_slots();
	//} catch (...) {
	//	BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFile Exception Size thread detached";
	//	if (m_pSizeThread) m_pSizeThread->detach();
	//}


	//try{
	//	if (m_pTimeThread && m_pTimeThread->joinable()) {
	//		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFolder Time thread join : " + GetPath();
	//		m_pTimeThread->join();
	//	}
	//	SignalTimeChanged.disconnect_all_slots();
	//} catch (...) {
	//	BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFile Exception Time thread detached";
	//	if (m_pTimeThread) m_pTimeThread->detach();
	//}
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

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFolder::GetIcon(std::function<void(CShellFile*)>& changedAction)
{
	if (GetLockIcon().second == FileIconStatus::None) {
		SetLockIcon(std::make_pair(CFileIconCache::GetInstance()->GetFolderIcon(), FileIconStatus::Available));
	}
	return GetLockIcon();
}


std::wstring CShellFolder::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
		m_wstrFileNameWithoutExt = m_childIdl.STRRET2WSTR(strret);
	}
	return m_wstrFileNameWithoutExt;
}

std::wstring CShellFolder::GetFileName()
{
	if (m_wstrFileName.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
		m_wstrFileName = m_childIdl.STRRET2WSTR(strret);
	}
	return m_wstrFileName;
}

std::wstring CShellFolder::GetExt()
{
	if (m_wstrExt.empty()) {
		m_wstrExt = L"folder";
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

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFolder::GetSize(std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed)
{
	if (spArgs->NoFolderSize) {
		SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable));
	} else if(spArgs->NoFolderSizeOnNetwork && ::PathIsNetworkPath(GetPath().c_str())) {
		SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable));
	} else {
		switch (GetLockSize().second) {
			case FileSizeStatus::None:
				{
					SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Calculating));
					auto limit = spArgs->TimeLimitFolderSize ? spArgs->TimeLimitMs : -1;
					m_futureSize = CThreadPool::GetInstance()->enqueue([](std::shared_ptr<bool> spCancelThread, CComPtr<IShellFolder> pShellFolder, CIDL folderIdl, std::wstring path, int limit, std::function<void()> sizeChanged) {
						try {
							boost::timer tim;
							ULARGE_INTEGER size = { 0 };
							if (CShellFolder::GetFolderSize(size, spCancelThread, pShellFolder, path, tim, limit)) {
								if (sizeChanged) { sizeChanged(); }
								return std::make_pair(size, FileSizeStatus::Available);
							} else {
								if (sizeChanged) { sizeChanged(); }
								return std::make_pair(size, FileSizeStatus::Unavailable);
							}
						} catch (...) {
							BOOST_LOG_TRIVIAL(trace) << L"CShellFile::GetSize Exception at size thread";
							return std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable);
						}					
					}, m_spCancelThread, GetShellFolderPtr(), GetAbsoluteIdl(), GetPath(), limit, changed);
				}
				break;
			case FileSizeStatus::Calculating:
				{
					if (m_futureSize.valid()) {
						SetLockSize(m_futureSize.get());
					}
				}
				break;
			case FileSizeStatus::Available:
			case FileSizeStatus::Unavailable:
				break;
		}
	}
	return GetLockSize();
}

std::pair<FILETIME, FileTimeStatus> CShellFolder::GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed)
{
	switch (GetLockLastWriteTime().second) {
	case FileTimeStatus::None:
		{
			FILETIME time = { 0 };
			if (!spArgs->IgnoreFolderTime, CShellFile::GetFileLastWriteTime(time)) {
				SetLockLastWriteTime(std::make_pair(time, FileTimeStatus::AvailableLoading));
			} else {
				SetLockLastWriteTime(std::make_pair(time, FileTimeStatus::Loading));
			}
			auto limit = spArgs->TimeLimitFolderLastWrite ? spArgs->TimeLimitMs : -1;
			m_futureTime = CThreadPool::GetInstance()->enqueue([](std::shared_ptr<bool> spCancelThread, 
				CComPtr<IShellFolder> pParentFolder, CComPtr<IShellFolder> pFolder, CIDL relativeIdl, std::wstring path,
				int limit, bool ignoreFolderTime, std::function<void()> timeChanged) {
				try {
					boost::timer tim;
					FILETIME time = { 0 };
					if (GetFolderLastWriteTime(time, spCancelThread, pParentFolder, pFolder, relativeIdl, path, tim, limit, ignoreFolderTime) && (time.dwLowDateTime || time.dwHighDateTime)) {
						timeChanged();
						return std::make_pair(time, FileTimeStatus::Available);
					} else {
						timeChanged();
						return std::make_pair(time, FileTimeStatus::Unavailable);
					}
				} catch (...) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFile::GetLastWriteTime Exception at time thread";
					return std::make_pair(FILETIME{ 0 }, FileTimeStatus::Unavailable);
				}
			}, m_spCancelThread, GetParentShellFolderPtr(), GetShellFolderPtr(), GetChildIdl(), GetPath(), limit, spArgs->IgnoreFolderTime, changed);
		}
		break;
	case FileTimeStatus::Loading:
	case FileTimeStatus::AvailableLoading:
		{
			if (m_futureTime.valid()) {
				SetLockLastWriteTime(m_futureTime.get());
			}
		}
		break;
	case FileTimeStatus::Available:
	case FileTimeStatus::Unavailable:

		break;
	}
	return GetLockLastWriteTime();
}

//static
bool CShellFolder::GetFolderSize(ULARGE_INTEGER& size, const std::shared_ptr<bool>& cancel, 
	const CComPtr<IShellFolder>& pFolder, const std::wstring& path, 
	const boost::timer& tim, const int limit)
{	
	if (*cancel) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Canceled at top : " + path;
		return false;
	} else if (limit > 0 && tim.elapsed() > limit/1000.0) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize TimeElapsed at top : " + path;
		return false;
	}

	try {
		//Enumerate child IDL
		size.QuadPart = 0;
		ULARGE_INTEGER childSize = { 0 };
		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDL childIdl;
			ULONG ulRet(0);
			while (SUCCEEDED(enumIdl->Next(1, childIdl.ptrptr(), &ulRet))) {
				if (*cancel) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Canceled in while : " + path;
					return false;
				} else if (limit > 0 && tim.elapsed() > limit/1000.0) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize TimeElapsed in while : " + path;
					return false;
				}

				if (!childIdl) {
					break; 
				} else {
					STRRET childStrret;
					pFolder->GetDisplayNameOf(childIdl.ptr(), SHGDN_FORPARSING, &childStrret);
					std::wstring childPath = childIdl.STRRET2WSTR(childStrret);
					std::wstring childExt = ::PathFindExtension(childPath.c_str());

					CComPtr<IShellFolder> pChidFolder;
					CComPtr<IEnumIDList> childEnumIdl;
					if (!boost::iequals(childExt, ".zip") &&
						SUCCEEDED(pFolder->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pChidFolder)) &&
						SUCCEEDED(pChidFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &childEnumIdl))) {
						if (CShellFolder::GetFolderSize(childSize, cancel,
							pChidFolder, childPath,
							tim, limit)) {
							size.QuadPart += childSize.QuadPart;
						} else {
							return false;
						}
					} else {
						if (CShellFile::GetFileSize(childSize, pFolder, childIdl)) {
							size.QuadPart += childSize.QuadPart;
						} else {
							return false;
						}
					}
					childSize.QuadPart = 0;
					childIdl.Clear();
				}
			}
		}
	} catch (...) {
		BOOST_LOG_TRIVIAL(trace) << "Exception CShellFolder::GetFolderSize " << path;
		return false;
	}
	return true;
}

bool CShellFolder::GetFolderLastWriteTime(FILETIME& time, const std::shared_ptr<bool>& cancel,
	const CComPtr<IShellFolder>& pParentFolder, const CComPtr<IShellFolder>& pFolder, const CIDL& relativeIdl, const std::wstring& path,
	boost::timer& tim, int limit, bool ignoreFolderTime)
{
	if (!ignoreFolderTime && CShellFile::GetFileLastWriteTime(time, pParentFolder, relativeIdl )) {
	} else {
		time = FILETIME{ 0 };
	}

	if (*cancel) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderLastWriteTime Canceled at top : " + path;
		return true;
	} else if (limit > 0 && tim.elapsed() > limit / 1000.0) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderLastWriteTime TimeElapsed at top : " + path;
		return true;
	}
	try {
		//Enumerate child IDL
		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDL childIdl;
			ULONG ulRet(0);
			std::vector<std::tuple<CComPtr<IShellFolder>, CIDL, std::wstring>> folders;
			while (SUCCEEDED(enumIdl->Next(1, childIdl.ptrptr(), &ulRet))) {
				if (*cancel) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderLastWriteTime Canceled in while : " + path;
					return true;
				} else if (limit > 0 && tim.elapsed() > limit / 1000.0) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderLastWriteTime TimeElapsed in while : " + path;
					return true;
				}

				if (!childIdl) {
					break;
				} else {
					FILETIME childTime = { 0 };
					STRRET childStrret;
					pFolder->GetDisplayNameOf(childIdl.ptr(), SHGDN_FORPARSING, &childStrret);
					std::wstring childPath = childIdl.STRRET2WSTR(childStrret);
					std::wstring childExt = ::PathFindExtension(childPath.c_str());

					CComPtr<IShellFolder> pChidFolder;
					CComPtr<IEnumIDList> childEnumIdl;
					if (!boost::iequals(childExt, ".zip") &&
						SUCCEEDED(pFolder->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pChidFolder)) &&
						SUCCEEDED(pChidFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &childEnumIdl))) {
						folders.emplace_back(std::make_tuple(pChidFolder, childIdl, childPath));
					} else {
						if (!CShellFile::GetFileLastWriteTime(childTime, pFolder, childIdl)) {
							childTime = { 0 };
						}
					}

					if (childTime.dwLowDateTime || childTime.dwHighDateTime) {
						ULARGE_INTEGER latest = { time.dwLowDateTime, time.dwHighDateTime };
						ULARGE_INTEGER child = { childTime.dwLowDateTime, childTime.dwHighDateTime };
						latest = latest.QuadPart > child.QuadPart ? latest : child;
						time = FILETIME{ latest.LowPart, latest.HighPart };
					}

					childIdl.Clear();
				}
			}

			for (auto childFolderArg : folders) {
				FILETIME grandchildTime = { 0 };
				if (!GetFolderLastWriteTime(grandchildTime, cancel,
					pFolder, std::get<0>(childFolderArg), std::get<1>(childFolderArg), std::get<2>(childFolderArg),
					tim, limit, ignoreFolderTime)) {
					grandchildTime = { 0 };
				}

				if (grandchildTime.dwLowDateTime || grandchildTime.dwHighDateTime) {
					ULARGE_INTEGER latest = { time.dwLowDateTime, time.dwHighDateTime };
					ULARGE_INTEGER child = { grandchildTime.dwLowDateTime, grandchildTime.dwHighDateTime };
					latest = latest.QuadPart > child.QuadPart ? latest : child;
					time = FILETIME{ latest.LowPart, latest.HighPart };
				}
			}

		}
	} catch (...) {
		BOOST_LOG_TRIVIAL(trace) << "Exception CShellFolder::GetFolderLastWriteTime " << path;
		return false;
	}
	return true;

}

void CShellFolder::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt) 
{
	CIDL idlNew;
	if (SUCCEEDED(m_pParentShellFolder->SetNameOf(
		NULL,
		m_childIdl.ptr(),
		wstrNameWoExt.c_str(),
		SHGDN_FORPARSING | SHGDN_INFOLDER,
		idlNew.ptrptr()))) {

		m_childIdl = idlNew;
		m_absoluteIdl = m_parentIdl + idlNew;

		Reset();
	}
}

void CShellFolder::SetExt(const std::wstring& wstrExt)
{
	//Do nothing
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
	return CreateShExFileFolder(GetShellFolderPtr(), GetAbsoluteIdl(), childIdl);
}

//static
std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(const CComPtr<IShellFolder>& pShellFolder, const CIDL& parentIdl, const CIDL& childIdl)
{
	boost::timer tim;

	CComPtr<IShellFolder> pFolder;
	CComPtr<IEnumIDList> enumIdl;
	STRRET strret;
	pShellFolder->GetDisplayNameOf(childIdl.ptr(), SHGDN_FORPARSING, &strret);
	std::wstring path = childIdl.STRRET2WSTR(strret);
	std::wstring ext = ::PathFindExtension(path.c_str());

	if (!childIdl) {
		return CKnownFolderManager::GetInstance()->GetDesktopFolder();
	} else if (auto spKnownFolder = CKnownFolderManager::GetInstance()->GetKnownFolderByPath(path)) {
		return spKnownFolder;
	} else if (path[0] == L':') {
		return std::make_shared<CShellFile>(pShellFolder, parentIdl, childIdl);
	} else if (auto spDriveFolder = CDriveManager::GetInstance()->GetDriveFolderByPath(path)) {
		return spDriveFolder;
	} else if (boost::iequals(ext, ".zip")) {
		return std::make_shared<CShellZipFolder>(pShellFolder, parentIdl, childIdl);
	} else if (
		//Do not use ::PathIsDirectory(path.c_str()), because it's slower
		(SUCCEEDED(pShellFolder->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pFolder)) &&
			SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)))) {
		return std::make_shared<CShellFolder>(pShellFolder, parentIdl, childIdl, pFolder);
	} else {
		return std::make_shared<CShellFile>(pShellFolder, parentIdl, childIdl);
	}

	if (tim.elapsed() > 0.1) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::CreateShExFileFolder Over0.1 " << path;
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

	if (FAILED(hr)) {//Not Exist
		return std::make_shared<CShellInvalidFile>();
	} else {
		return desktop->CreateShExFileFolder(relativeIdl);
	}
}





