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
#include "DirectoryWatcher.h"


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

	try {
		if (m_spWatcher) { 
			m_spWatcher->QuitWatching();
		}
	} catch (...){}
}

//CShellFolder CShellFolder::Clone() const
//{
//	CThreadSafeComPtr<IShellFolder> pParentShellFolder = shell::DesktopBindToShellFolder(m_parentIdl);
//	CThreadSafeComPtr<IShellFolder> pShellFolder = shell::DesktopBindToShellFolder(m_absoluteIdl);
//
//	return CShellFolder(pParentShellFolder, m_parentIdl, m_childIdl);
//}

CShellComPtr<IShellFolder> CShellFolder::GetShellFolderPtr() const
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
		CShellComPtr<IShellFolder> pGrandParentFolder = nullptr;
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
					auto fun = [](const std::shared_ptr<bool>& spCancelThread, CShellComPtr<IShellFolder> pShellFolder, CIDL folderIdl, std::wstring path, int limit, std::function<void()> sizeChanged)
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
					CShellComPtr<IShellFolder> pParentFolder,
					CShellComPtr<IShellFolder> pFolder,
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
	CShellComPtr<IShellFolder> pFolder, const std::wstring& path,
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

					CShellComPtr<IShellFolder> pChidFolder;
					CComPtr<IEnumIDList> childEnumIdl;
					if (!boost::iequals(childExt, ".zip") &&
						SUCCEEDED(pFolder.Call(&IShellFolder::BindToObject, reinterpret_cast<LPCITEMIDLIST>(childIdl.ptr()), reinterpret_cast<IBindCtx*>(nullptr), IID_PPV_ARGS(& pChidFolder))) &&
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
	CShellComPtr<IShellFolder> pParentFolder = shell::DesktopBindToShellFolder(parentIdl);
	CShellComPtr<IShellFolder> pFolder = shell::DesktopBindToShellFolder(parentIdl + relativeIdl);

	return CShellFolder::GetFolderFileTimes(cancel, pParentFolder, pFolder, relativeIdl, path, tp, limit, ignoreFolderTime);
}

std::optional<FileTimes> CShellFolder::GetFolderFileTimes(const std::shared_ptr<bool>& cancel,
	CShellComPtr<IShellFolder> pParentFolder, CShellComPtr<IShellFolder> pFolder, const CIDL& relativeIdl, const std::wstring& path,
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
			std::vector<std::tuple<CShellComPtr<IShellFolder>, CIDL, std::wstring>> folders;
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

					CShellComPtr<IShellFolder> pChidFolder;
					CComPtr<IEnumIDList> childEnumIdl;
					if (!boost::iequals(childExt, ".zip") &&
						SUCCEEDED(pFolder.Call(&IShellFolder::BindToObject, reinterpret_cast<LPCITEMIDLIST>(childIdl.ptr()), reinterpret_cast<IBindCtx*>(nullptr), IID_PPV_ARGS(& pChidFolder))) &&
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

reactive_vector_ptr<std::shared_ptr<CShellFile>>& CShellFolder::GetFiles() const
{
	if (Files->empty()) {
		//Directory watcher
		m_spWatcher = std::make_shared<CDirectoryWatcher>(std::bind(&CShellFolder::OnDirectoryWatch, this, std::placeholders::_1));

		CShellThread::GetInstance()->Run([this]()->void {
			{
				LOG_SCOPED_TIMER_THIS_1("OpenFolder Enumeration");
				try {

					//{
					//	LOG_SCOPED_TIMER_THIS_1("Single Enumeration");
					//	CShellComPtr<IEnumIDList> enumIdl;
					//	if (SUCCEEDED(Folder->GetShellFolderPtr().Call(&IShellFolder::EnumObjects, GetWndPtr()->m_hWnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN | SHCONTF_FASTITEMS, &enumIdl)) && enumIdl) {
					//		CIDL nextIdl;
					//		ULONG ulRet(0);
					//		while (true) {
					//			SUCCEEDED(enumIdl.Call(&IEnumIDList::Next, 1, nextIdl.ptrptr(), &ulRet));
					//			if (!nextIdl) { break; }
					//			auto spFile = spFolder->CreateShExFileFolder(std::forward<CIDL>(nextIdl));
					//			ItemsSource.emplace_back(spFile); 
					//			nextIdl.Clear();
					//		}
					//	}
					//}
					//ItemsSource.clear();

					{
						LOG_SCOPED_TIMER_THIS_1("Multiple Enumeration");
						CShellComPtr<IEnumIDList> enumIdl;
						constexpr ULONG kBatchSize = 64;
						if (SUCCEEDED(GetShellFolderPtr().Call(&IShellFolder::EnumObjects, nullptr, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN | SHCONTF_FASTITEMS, &enumIdl)) && enumIdl) {
							while (true) {
								PIDLIST_RELATIVE pidls[kBatchSize] = {};
								ULONG fetched = 0;
								HRESULT hr = enumIdl.Call(&IEnumIDList::Next, kBatchSize, pidls, &fetched);
								for (ULONG i = 0; i < fetched; ++i)
								{
									CIDL nextIdl(pidls[i]);
									auto spFile = CreateShExFileFolder(std::move(nextIdl));
									Files.emplace_back(spFile);
								}
								if (hr == S_FALSE)
									break;

								if (FAILED(hr))
									break;
							}
						}
					}

				} catch (std::exception&) {
					THROW_FILE_LINE_FUNC;
				}

				//Set up Watcher
				try {
					if (::PathFileExists(GetPath().c_str()) &&
						!boost::iequals(GetDispExt(), L".zip")) {
						m_spWatcher->QuitWatching();
						m_spWatcher->StartWatching(GetPath(), GetAbsoluteIdl());
					} else {
						m_spWatcher->QuitWatching();
					}
				} catch (std::exception& e) {
					THROW_FILE_LINE_FUNC;
				}

			}
		}).get();

	}
	return Files;
}


void CShellFolder::OnDirectoryWatch(const DirectoryWatchEvent& e) const
{
	if (e.Infos.size() > 1) {
		auto a = 1.f;
	}
	for (auto info : e.Infos) {
		switch (info.first) {
		case FILE_ACTION_ADDED:
			LOG_THIS_1("FILE_ACTION_ADDED");
			Added(info.second);
			break;
		case FILE_ACTION_MODIFIED:
			LOG_THIS_1("FILE_ACTION_MODIFIED");
			Modified(info.second);
			break;
		case FILE_ACTION_REMOVED:
			LOG_THIS_1("FILE_ACTION_REMOVED");
			Removed(info.second);
			break;
		case FILE_ACTION_RENAMED_NEW_NAME:
			LOG_THIS_1("FILE_ACTION_RENAMED_NEW_NAME");
			{
				std::list<std::wstring> oldnew;
				boost::split(oldnew, info.second, boost::is_any_of(L"/"));
				Renamed(oldnew.front(), oldnew.back());
			}
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
			LOG_THIS_1("FILE_ACTION_RENAMED_OLD_NAME");
			break;
		default:
			break;
		}
	}

	//SubmitUpdate();
}


void CShellFolder::Added(const std::wstring& fileName) const
{
	LOG_THIS_1("Added " + wstr2str(fileName));
	CIDL idl;
	ULONG chEaten;
	ULONG dwAttributes;
	HRESULT hr = GetShellFolderPtr().Call(&IShellFolder::ParseDisplayName, nullptr, nullptr, const_cast<LPWSTR>(fileName.c_str()), &chEaten, idl.ptrptr(), &dwAttributes);
	if (SUCCEEDED(hr) && idl) {
		Files.push_back(CreateShExFileFolder(std::move(idl)));
		//auto spRow = m_allRows.back();

		//PostUpdate(Updates::ColumnVisible);
		//PostUpdate(Updates::RowVisible);
		//PostUpdate(Updates::Row);
		//PostUpdate(Updates::Scrolls);
		//PostUpdate(Updates::Invalidate);

		//if (m_isNewFile) {
		//	m_spCursorer->OnCursor(Cell(spRow, m_allCols.at(m_frozenColumnCount)));
		//	PostUpdate(Updates::EnsureVisibleFocusedCell);
		//}
		//PostUpdate(Updates::Sort);
		//UpdateFilter();
		//if (m_isNewFile) {
		//	if (auto cols = FindColumns<CFileNameColumn<std::shared_ptr<CShellFile>>>(); !cols.empty()) {
		//		std::static_pointer_cast<CFileNameCell<std::shared_ptr<CShellFile>>>(Cell(spRow, cols.front()))->OnEdit(Event(GetWndPtr()));
		//	}
		//}
		//m_isNewFile = false;
	} else {
		LOG_THIS_1("Added FAILED " + wstr2str(fileName));
	}
}

void CShellFolder::Modified(const std::wstring& fileName) const
{
	LOG_THIS_1("Modified " + wstr2str(fileName));
	auto iter = std::find_if(Files.get_unconst()->begin(), Files.get_unconst()->end(),
		[fileName](const std::shared_ptr<CShellFile>& value)->bool {
			return value->GetPathName() == fileName;
		});
	//	auto iter = FindIfRowIterByFileNameExt(fileName);

	if (iter == Files.get_unconst()->end()) {
		LOG_THIS_1("Modified NoMatch " + wstr2str(fileName));
		return;
	} else {
		//Because ItemIdList includes, size, last write time, etc., it is necessary to get new one.
		ULONG chEaten;
		ULONG dwAttributes;
		CIDL newIdl;
		HRESULT hRes = (*iter)->GetParentShellFolderPtr().Call(
			&IShellFolder::ParseDisplayName, nullptr, nullptr, (LPWSTR)fileName.c_str(), &chEaten, newIdl.ptrptr(), &dwAttributes);

		if (SUCCEEDED(hRes) && newIdl) {
			//std::get<std::shared_ptr<CShellFile>>(*iter) = m_spFolder->CreateShExFileFolder(newIdl);
			Files.replace(iter, CreateShExFileFolder(std::move(newIdl)));
			//m_allRows[iter - ItemsSource->cbegin() + m_frozenRowCount]->SetIsMeasureValid(false);
			//PostUpdate(Updates::ColumnVisible);
			//PostUpdate(Updates::RowVisible);
			//PostUpdate(Updates::Row);
			//PostUpdate(Updates::Scrolls);
			//PostUpdate(Updates::Invalidate);
			//PostUpdate(Updates::Sort);
			//UpdateFilter();
		} else {
			LOG_THIS_1("Modified FAILED " + wstr2str(fileName));
		}
	}
}
void CShellFolder::Removed(const std::wstring& fileName) const
{
	LOG_THIS_1("Removed " + wstr2str(fileName));

	auto iter = std::find_if(Files->cbegin(), Files->cend(),
		[fileName](const std::shared_ptr<CShellFile>& value)->bool {
			return value->GetPathName() == fileName;
		});

	if (iter == Files->cend()) {
		LOG_THIS_1("Removed NoMatch " + wstr2str(fileName));
		return;
	} else {

		Files.erase(iter);

		//m_spCursorer->OnCursorClear(this);
		//m_spCeller->OnClear();

		//PostUpdate(Updates::ColumnVisible);
		//PostUpdate(Updates::RowVisible);
		//PostUpdate(Updates::Row);
		//PostUpdate(Updates::Scrolls);
		//PostUpdate(Updates::Invalidate);
		//PostUpdate(Updates::Sort);

		//UpdateFilter();
	}

}
void CShellFolder::Renamed(const std::wstring& oldName, const std::wstring& newName) const
{
	LOG_THIS_1("Renamed " + wstr2str(oldName) + "=>" + wstr2str(newName));
	auto iter = std::find_if(Files.get_unconst()->begin(), Files.get_unconst()->end(),
		[oldName](const std::shared_ptr<CShellFile>& value)->bool {
			return value->GetPathName() == oldName;
		});

	if (iter == Files.get_unconst()->end())
	{
		LOG_THIS_1("Renamed NoMatch " + wstr2str(oldName) + "=>" + wstr2str(newName));
	} else {
		ULONG chEaten;
		ULONG dwAttributes;
		CIDL newIdl;

		HRESULT hRes = (*iter)->GetParentShellFolderPtr().Call(&IShellFolder::ParseDisplayName, nullptr, nullptr, (LPWSTR)newName.c_str(), &chEaten, newIdl.ptrptr(), &dwAttributes);

		if (SUCCEEDED(hRes) && newIdl) {
			(*iter) = CreateShExFileFolder(std::move(newIdl));
			//m_allRows[iter - Files->cbegin() + m_frozenRowCount]->SetIsMeasureValid(false);
			//PostUpdate(Updates::ColumnVisible);
			//PostUpdate(Updates::RowVisible);
			//PostUpdate(Updates::Row);
			//PostUpdate(Updates::Scrolls);
			//PostUpdate(Updates::Invalidate);
			//PostUpdate(Updates::Sort);
			//UpdateFilter();
		} else {
			LOG_THIS_1("Renamed FAILED " + wstr2str(oldName) + "=>" + wstr2str(newName));
			return Removed(oldName);
		}
	}
}


