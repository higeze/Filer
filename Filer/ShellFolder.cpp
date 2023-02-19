#include "ShellFolder.h"
#include "Debug.h"
#include "ShellZipFolder.h"
#include "KnownFolder.h"
#include "DriveFolder.h"
#include "Debug.h"
#include "ApplicationProperty.h"
#include "FileSizeArgs.h"
#include "FileTimeArgs.h"
#include "ShellFileFactory.h"
#include "ShellFunction.h"
#include <format>
#include "ThreadPool.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CShellFolder::CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CShellFile(pParentShellFolder, parentIdl, childIdl), m_pShellFolder(pShellFolder) {}


CShellFolder::~CShellFolder()
{
	*m_spCancelThread = true;
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

std::pair<FileTimes, FileTimeStatus> CShellFolder::GetLockFileTimes() const
{
	std::lock_guard<std::mutex> lock(m_mtxTime);
	return m_fileTimes;
}

void CShellFolder::SetLockFileTimes(const std::pair<FileTimes, FileTimeStatus>& times)
{
	std::lock_guard<std::mutex> lock(m_mtxTime);
	m_fileTimes = times;
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFolder::GetLockSize() const
{
	std::lock_guard<std::mutex> lock(m_mtxSize);
	return m_size;
}

void CShellFolder::SetLockSize(const std::pair<ULARGE_INTEGER, FileSizeStatus>& size)
{
	std::lock_guard<std::mutex> lock(m_mtxSize);
	m_size = size;
}
//
//std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFolder::GetIcon(std::function<void(CShellFile*)>& changedAction)
//{
//	if (GetLockIcon().second == FileIconStatus::None) {
//		SetLockIcon(std::make_pair(CFileIconCache::GetInstance()->GetFolderIcon(), FileIconStatus::Available));
//	}
//	return GetLockIcon();
//}


std::wstring CShellFolder::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
		m_wstrFileNameWithoutExt = m_childIdl.strret2wstring(strret);
	}
	return m_wstrFileNameWithoutExt;
}

std::wstring CShellFolder::GetDispName()
{
	if (m_wstrFileName.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
		m_wstrFileName = m_childIdl.strret2wstring(strret);
	}
	return m_wstrFileName;
}

std::wstring CShellFolder::GetDispExt()
{
	if (m_wstrExt.empty()) {
		m_wstrExt = L"folder";
	}
	return m_wstrExt;
}


std::shared_ptr<CShellFolder> CShellFolder::GetParent()
{
	::OutputDebugStringA(std::format("Size:{}", ::ILGetSize(m_absoluteIdl.ptr())).c_str());
	CIDL parentIDL = m_absoluteIdl.CloneParentIDL();
	CIDL grandParentIDL = parentIDL.CloneParentIDL();
	CComPtr<IShellFolder> pParentFolder = shell::DesktopBindToShellFolder(parentIDL);
	CComPtr<IShellFolder> pGrandParentFolder = shell::DesktopBindToShellFolder(grandParentIDL);
	return std::static_pointer_cast<CShellFolder>(CShellFileFactory::GetInstance()->CreateShellFilePtr(pGrandParentFolder, grandParentIDL, parentIDL.CloneLastID()));
	//return std::make_shared<CShellFolder>(pGrandParentFolder, grandParentIDL, parentIDL.CloneLastID(), pParentFolder);
}

std::shared_ptr<CShellFolder> CShellFolder::Clone()const
{
	return std::make_shared<CShellFolder>(m_pParentShellFolder, m_parentIdl, m_childIdl, m_pShellFolder);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFolder::GetSize(const std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed)
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
					auto fun = [](const std::shared_ptr<bool>& spCancelThread, const CComPtr<IShellFolder>& pShellFolder, const CIDL& folderIdl, const std::wstring& path, const int& limit, const std::function<void()>& sizeChanged)
					{
						std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
						ULARGE_INTEGER size = { 0 };
						if (CShellFolder::GetFolderSize(size, spCancelThread, pShellFolder, path, tp, limit)) {
							if (sizeChanged) { sizeChanged(); }
							return std::make_pair(size, FileSizeStatus::Available);
						} else {
							if (sizeChanged) { sizeChanged(); }
							return std::make_pair(size, FileSizeStatus::Unavailable);
						}
					};
					m_futureSize = CThreadPool::GetInstance()->enqueue(
						fun,
						0,
						//std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable),
						m_spCancelThread,
						GetShellFolderPtr(),
						GetAbsoluteIdl(),
						GetPath(),
						limit,
						changed);
				}
				break;
			case FileSizeStatus::Calculating:
				{
					if (m_futureSize.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
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

std::pair<FileTimes, FileTimeStatus> CShellFolder::GetFileTimes(const std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed)
{
	switch (GetLockFileTimes().second) {
	case FileTimeStatus::None:
		{
		if (auto times = CShellFile::GetFileTimes(); !spArgs->IgnoreFolderTime && times.has_value()) {
				SetLockFileTimes(std::make_pair(times.value(), FileTimeStatus::AvailableLoading));
			} else {
				SetLockFileTimes(std::make_pair(FileTimes(), FileTimeStatus::Loading));
			}
			auto limit = spArgs->TimeLimitFolderLastWrite ? spArgs->TimeLimitMs : -1;
			auto fun = [](const std::shared_ptr<bool>& spCancelThread,
					const CComPtr<IShellFolder>& pParentFolder,
					const CComPtr<IShellFolder>& pFolder,
					const CIDL& relativeIdl,
					const std::wstring& path,
					const int& limit,
					const bool& ignoreFolderTime,
					const std::function<void()>& timeChanged)
			{
				std::chrono::time_point tim = std::chrono::system_clock::now();
				auto times = GetFolderFileTimes(spCancelThread, pParentFolder, pFolder, relativeIdl, path, tim, limit, ignoreFolderTime);
				if (times.has_value()) {
					timeChanged();
					return std::make_pair(times.value(), FileTimeStatus::Available);
				} else {
					timeChanged();
					return std::make_pair(times.value(), FileTimeStatus::Unavailable);
				}
			};
			m_futureTime = CThreadPool::GetInstance()->enqueue(
				fun,	
				0,
				//std::make_pair(FileTimes(), FileTimeStatus::Unavailable),
				m_spCancelThread,
				GetParentShellFolderPtr(),
				GetShellFolderPtr(),
				GetChildIdl(),
				GetPath(),
				limit,
				spArgs->IgnoreFolderTime,
				changed);
		}
		break;
	case FileTimeStatus::Loading:
	case FileTimeStatus::AvailableLoading:
		{
			if (m_futureTime.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				SetLockFileTimes(m_futureTime.get());
			}
		}
		break;
	case FileTimeStatus::Available:
	case FileTimeStatus::Unavailable:

		break;
	}
	return GetLockFileTimes();
}

//static
bool CShellFolder::GetFolderSize(ULARGE_INTEGER& size, const std::shared_ptr<bool>& cancel, 
	const CComPtr<IShellFolder>& pFolder, const std::wstring& path, 
	const std::chrono::system_clock::time_point& tp, const int limit)
{	
	if (*cancel) {
		LOG_2("CShellFolder::GetFolderSize Canceled at top :", wstr2str(path));
		return false;
	} else if (limit > 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tp).count() > limit) {
		LOG_2("CShellFolder::GetFolderSize TimeElapsed at top :", wstr2str(path));
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
					LOG_2("CShellFolder::GetFolderSize Canceled in while :", wstr2str(path));
					return false;
				}else if (limit > 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tp).count() > limit) {
					LOG_2("CShellFolder::GetFolderSize TimeElapsed in while :", wstr2str(path));
					return false;
				}

				if (!childIdl) {
					break; 
				} else {
					STRRET childStrret;
					pFolder->GetDisplayNameOf(childIdl.ptr(), SHGDN_FORPARSING, &childStrret);
					std::wstring childPath = childIdl.strret2wstring(childStrret);
					std::wstring childExt = ::PathFindExtension(childPath.c_str());

					CComPtr<IShellFolder> pChidFolder;
					CComPtr<IEnumIDList> childEnumIdl;
					if (!boost::iequals(childExt, ".zip") &&
						SUCCEEDED(pFolder->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pChidFolder)) &&
						SUCCEEDED(pChidFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &childEnumIdl))) {
						if (CShellFolder::GetFolderSize(childSize, cancel,
							pChidFolder, childPath,
							tp, limit)) {
							size.QuadPart += childSize.QuadPart;
						} else {
							return false;
						}
					} else {
						if (shell::GetFileSize(childSize, pFolder, childIdl)) {
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
		LOG_2("Exception CShellFolder::GetFolderSize", wstr2str(path));
		return false;
	}
	return true;
}

std::optional<FileTimes> CShellFolder::GetFolderFileTimes(const std::shared_ptr<bool>& cancel,
	const CComPtr<IShellFolder>& pParentFolder, const CComPtr<IShellFolder>& pFolder, const CIDL& relativeIdl, const std::wstring& path,
	std::chrono::system_clock::time_point& tp, int limit, bool ignoreFolderTime)
{
	FileTimes times = shell::GetFileTimes(pParentFolder, relativeIdl).value_or(FileTimes());
	if (ignoreFolderTime) {
		times = FileTimes();
	}

	if (*cancel) {
		LOG_2("CShellFolder::GetFolderFileTimes Canceled at top :", wstr2str(path));
		return times;
	} else if (limit > 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-tp).count() > limit) {
		LOG_2("CShellFolder::GetFolderFileTimes TimeElapsed at top :", wstr2str(path));
		return times;
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
					LOG_2("CShellFolder::GetFolderFileTimes Canceled in while :", wstr2str(path));
					return times;
				} else if (limit > 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tp).count() > limit) {
					LOG_2("CShellFolder::GetFolderFileTimes TimeElapsed in while :", wstr2str(path));
					return times;
				}

				if (!childIdl) {
					break;
				} else {
					FileTimes childTimes;
					STRRET childStrret;
					pFolder->GetDisplayNameOf(childIdl.ptr(), SHGDN_FORPARSING, &childStrret);
					std::wstring childPath = childIdl.strret2wstring(childStrret);
					std::wstring childExt = ::PathFindExtension(childPath.c_str());

					CComPtr<IShellFolder> pChidFolder;
					CComPtr<IEnumIDList> childEnumIdl;
					if (!boost::iequals(childExt, ".zip") &&
						SUCCEEDED(pFolder->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pChidFolder)) &&
						SUCCEEDED(pChidFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &childEnumIdl))) {
						folders.emplace_back(std::make_tuple(pChidFolder, childIdl, childPath));
					} else {
						childTimes = shell::GetFileTimes(pFolder, childIdl).value_or(FileTimes());
					}

					times.LastWriteTime = ::CompareFileTime(&times.LastWriteTime, &childTimes.LastWriteTime) >= 0 ?
						times.LastWriteTime : childTimes.LastWriteTime;
					childIdl.Clear();
				}
			}

			for (auto childFolderArg : folders) {
				FileTimes grandchildTime = GetFolderFileTimes(cancel, pFolder, std::get<0>(childFolderArg),
					std::get<1>(childFolderArg), std::get<2>(childFolderArg), tp, limit, ignoreFolderTime).value_or(FileTimes());

				times.LastWriteTime = ::CompareFileTime(&times.LastWriteTime, &grandchildTime.LastWriteTime) >= 0 ?
					times.LastWriteTime : grandchildTime.LastWriteTime;
			}

		}
	} catch (...) {
		LOG_2("Exception CShellFolder::GetFolderLastWriteTime", wstr2str(path));
		return std::nullopt;
	}
	return times;

}

void CShellFolder::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND  hWnd) 
{
	HRESULT hr = m_pParentShellFolder->SetNameOf(
		hWnd,
		m_childIdl.ptr(),
		wstrNameWoExt.c_str(),
		SHGDN_FORPARSING | SHGDN_INFOLDER,
		nullptr);
}

void CShellFolder::SetExt(const std::wstring& wstrExt, HWND hWnd)
{
	//Do nothing
}

std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(const CIDL& childIdl)
{
	return CShellFileFactory::GetInstance()->CreateShellFilePtr(GetShellFolderPtr(), GetAbsoluteIdl(), childIdl);
}