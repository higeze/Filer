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

#include "KnownFolderManager.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CShellFolder::~CShellFolder()
{
	*m_spCancelThread = true;

	try {
		if (m_futureSize.valid()) {
			m_futureSize.wait();
		}
	} catch (...) {}

	try {
		if (m_futureTime.valid()) {
			m_futureTime.wait();
		}
	} catch (...) {}
}

//CShellFolder CShellFolder::Clone() const
//{
//	CThreadSafeComPtr<IShellFolder> pParentShellFolder = shell::DesktopBindToShellFolder(m_parentIdl);
//	CThreadSafeComPtr<IShellFolder> pShellFolder = shell::DesktopBindToShellFolder(m_absoluteIdl);
//
//	return CShellFolder(pParentShellFolder, m_parentIdl, m_childIdl);
//}

CThreadSafeComPtr<IShellFolder> CShellFolder::GetShellFolderPtr() const
{
	if (!m_pShellFolder) {
		m_pShellFolder = shell::DesktopBindToShellFolder(m_absoluteIdl);
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

const std::wstring& CShellFolder::GetDispName() const
{
	if (!m_optDispName.has_value()) {
		::OutputDebugStringW(std::format(L"IDLCount:{}\r\n", CIDL::GetItemIdListCount(m_childIdl.ptr())).c_str());
		m_optDispName.emplace(shell::GetDisplayNameOf(GetParentFolderPtr()->GetShellFolderPtr(), m_childIdl.ptr(), SHGDN_NORMAL));
	}
	return m_optDispName.value();
}

const std::wstring& CShellFolder::GetDispNameWithoutExt() const
{
	return GetDispName();
}


const std::wstring& CShellFolder::GetDispExt() const
{
	if (!m_optDispExt.has_value()) {
		m_optDispExt.emplace(L"folder");
	}
	return m_optDispExt.value();
}


const std::shared_ptr<CShellFolder>& CShellFolder::GetParentFolderPtr() const
{
	if (!m_pParentFolder) {
		CIDL parentIDL = m_absoluteIdl.CloneParentIDL();
		CIDL grandParentIDL = parentIDL.CloneParentIDL();
		CThreadSafeComPtr<IShellFolder> pGrandParentFolder = nullptr;
		LPCITEMIDLIST pidl;
		HRESULT hr = SHBindToParent(
			parentIDL.ptr(),
			IID_PPV_ARGS(&pGrandParentFolder),
			&pidl
		);
		CIDL parentLastID(::ILCloneFull(pidl));

		m_pParentFolder = std::static_pointer_cast<CShellFolder>(
			CShellFileFactory::GetInstance()->CreateShellFilePtr(pGrandParentFolder, grandParentIDL, std::move(parentLastID)));
	}
	return m_pParentFolder;
	//return std::make_shared<CShellFolder>(pGrandParentFolder, grandParentIDL, parentIDL.CloneLastID(), pParentFolder);
}

std::shared_ptr<CShellFolder> CShellFolder::Clone()const
{
	return std::make_shared<CShellFolder>(m_pParentShellFolder, m_parentIdl, m_childIdl, GetShellFolderPtr());
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFolder::GetSize(const FileSizeArgs& args, std::function<void()> changed)
{
	if (args.NoFolderSize) {
		SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable));
	} else if(args.NoFolderSizeOnNetwork && ::PathIsNetworkPath(GetPath().c_str())) {
		SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable));
	} else {
		switch (GetLockSize().second) {
			case FileSizeStatus::None:
				{
					SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Calculating));
					auto limit = args.TimeLimitFolderSize ? args.TimeLimitMs : -1;
					auto fun = [](const std::shared_ptr<bool>& spCancelThread, CThreadSafeComPtr<IShellFolder> pShellFolder, CIDL folderIdl, std::wstring path, int limit, std::function<void()> sizeChanged)
					{
						// pShellFolder はコピーなのでタスク実行中に有効
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
						FILE_LINE_FUNC,
						0,
						fun,
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

std::pair<FileTimes, FileTimeStatus> CShellFolder::GetFileTimes(const FileTimeArgs& args, std::function<void()> changed)
{
	switch (GetLockFileTimes().second) {
	case FileTimeStatus::None:
		{
		if (auto times = CShellFile::GetFileTimes(); !args.IgnoreFolderTime && times.has_value()) {
				SetLockFileTimes(std::make_pair(times.value(), FileTimeStatus::AvailableLoading));
			} else {
				SetLockFileTimes(std::make_pair(FileTimes(), FileTimeStatus::Loading));
			}
			auto limit = args.TimeLimitFolderLastWrite ? args.TimeLimitMs : -1;
			auto fun = [](const std::shared_ptr<bool>& spCancelThread,
					CThreadSafeComPtr<IShellFolder> pParentFolder,
					CThreadSafeComPtr<IShellFolder> pFolder,
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
				FILE_LINE_FUNC,
				0,
				fun,	
				m_spCancelThread,
				GetParentFolderPtr()->GetShellFolderPtr(),
				GetShellFolderPtr(),
				GetChildIdl(),
				GetPath(),
				limit,
				args.IgnoreFolderTime,
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
	CThreadSafeComPtr<IShellFolder> pFolder, const std::wstring& path,
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
		if (SUCCEEDED(pFolder.Call(&IShellFolder::EnumObjects, reinterpret_cast<HWND>(NULL), SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
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
					pFolder.Call(&IShellFolder::GetDisplayNameOf, childIdl.ptr(), SHGDN_FORPARSING, &childStrret);
					std::wstring childPath = childIdl.strret2wstring(childStrret);
					std::wstring childExt = ::PathFindExtension(childPath.c_str());

					CThreadSafeComPtr<IShellFolder> pChidFolder;
					CComPtr<IEnumIDList> childEnumIdl;
					if (!boost::iequals(childExt, ".zip") &&
						SUCCEEDED(pFolder.Call(&IShellFolder::BindToObject, childIdl.ptr(), reinterpret_cast<IBindCtx*>(nullptr), IID_IShellFolder, reinterpret_cast<void**>(& pChidFolder))) &&
						SUCCEEDED(pChidFolder.Call(&IShellFolder::EnumObjects, reinterpret_cast<HWND>(NULL), SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &childEnumIdl))) {
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

std::optional<FileTimes> CShellFolder::GetFolderFileTimes(
	const std::shared_ptr<bool>& cancel,
	const CIDL& parentIdl,
	const CIDL& relativeIdl,
	const std::wstring& path,
	std::chrono::system_clock::time_point& tp,
	int limit,
	bool ignoreFolderTime)
{
	CThreadSafeComPtr<IShellFolder> pParentFolder = shell::DesktopBindToShellFolder(parentIdl);
	CThreadSafeComPtr<IShellFolder> pFolder = shell::DesktopBindToShellFolder(parentIdl + relativeIdl);

	return CShellFolder::GetFolderFileTimes(cancel, pParentFolder, pFolder, relativeIdl, path, tp, limit, ignoreFolderTime);
}

std::optional<FileTimes> CShellFolder::GetFolderFileTimes(const std::shared_ptr<bool>& cancel,
	CThreadSafeComPtr<IShellFolder> pParentFolder, CThreadSafeComPtr<IShellFolder> pFolder, const CIDL& relativeIdl, const std::wstring& path,
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
		if (SUCCEEDED(pFolder.Call(&IShellFolder::EnumObjects, reinterpret_cast<HWND>(NULL), SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDL childIdl;
			ULONG ulRet(0);
			std::vector<std::tuple<CThreadSafeComPtr<IShellFolder>, CIDL, std::wstring>> folders;
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
					pFolder.Call(&IShellFolder::GetDisplayNameOf, childIdl.ptr(), SHGDN_FORPARSING, &childStrret);
					std::wstring childPath = childIdl.strret2wstring(childStrret);
					std::wstring childExt = ::PathFindExtension(childPath.c_str());

					CThreadSafeComPtr<IShellFolder> pChidFolder;
					CComPtr<IEnumIDList> childEnumIdl;
					if (!boost::iequals(childExt, ".zip") &&
						SUCCEEDED(pFolder.Call(&IShellFolder::BindToObject, childIdl.ptr(), reinterpret_cast<IBindCtx*>(nullptr), IID_IShellFolder, reinterpret_cast<void**>(&pChidFolder))) &&
						SUCCEEDED(pChidFolder.Call(&IShellFolder::EnumObjects, reinterpret_cast<HWND>(NULL), SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &childEnumIdl))) {
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
	HRESULT hr = GetParentFolderPtr()->GetShellFolderPtr().Call(
		&IShellFolder::SetNameOf,
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

std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(CIDL&& childIdl) const
{
	return CShellFileFactory::GetInstance()->CreateShellFilePtr(GetShellFolderPtr(), GetAbsoluteIdl(), std::forward<CIDL>(childIdl));
}

//std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(const CIDL& childIdl) const
//{
//	return CShellFileFactory::GetInstance()->CreateShellFilePtr(GetShellFolderPtr(), GetAbsoluteIdl(), childIdl);
//}