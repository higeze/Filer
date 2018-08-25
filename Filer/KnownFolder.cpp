#include "KnownFolder.h"
#include "FileIconCache.h"
#include "MyCom.h"
#include "MyIcon.h"

CKnownDriveBaseFolder::CKnownDriveBaseFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CShellFolder(pParentShellFolder, parentIdl, childIdl, pShellFolder){}


std::pair<std::shared_ptr<CIcon>, FileIconStatus> CKnownDriveBaseFolder::GetIcon()
{
	switch (GetLockIcon().second) {
	case FileIconStatus::None:
	{
		SetLockIcon(std::make_pair(CFileIconCache::GetInstance()->GetDefaultIcon(), FileIconStatus::Loading));
		if (!m_pIconThread) {
			//m_iconFuture = m_iconPromise.get_future();
			m_pIconThread.reset(new std::thread([this]()->void {
				SHFILEINFO sfi = { 0 };
				::SHGetFileInfo((LPCTSTR)GetAbsoluteIdl().ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
				SetLockIcon(std::make_pair(std::make_shared<CIcon>(sfi.hIcon), FileIconStatus::Available));
				SignalFileIconChanged(this);
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

std::wstring CKnownDriveBaseFolder::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
		m_wstrFileNameWithoutExt = m_childIdl.STRRET2WSTR(strret);
	}
	return m_wstrFileNameWithoutExt;
}

std::wstring CKnownDriveBaseFolder::GetFileName()
{
	if (m_wstrFileName.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
		m_wstrFileName = m_childIdl.STRRET2WSTR(strret);
	}
	return m_wstrFileName;
}







CKnownFolder::CKnownFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IKnownFolder>& pKnownFolder, CComPtr<IShellFolder> pShellFolder)
	:CKnownDriveBaseFolder(pParentShellFolder, parentIdl, childIdl, pShellFolder), m_pKnownFolder(pKnownFolder){}

KF_CATEGORY CKnownFolder::GetCategory()
{
	KF_CATEGORY category;
	if (SUCCEEDED(m_pKnownFolder->GetCategory(&category))) {
		return category;
	}
	else {
		throw std::exception("CKnownFolder::GetCategory");
	}
}

KNOWNFOLDERID CKnownFolder::GetId()
{
	KNOWNFOLDERID id;
	if (SUCCEEDED(m_pKnownFolder->GetId(&id))) {
		return id;
	} else {
		throw std::exception("CKnownFolder::GetId");
	}

}

std::pair<ULARGE_INTEGER, FileSizeStatus> CKnownFolder::GetSize()
{
	if (m_size.second == FileSizeStatus::None) {
		if (GetCategory() == KF_CATEGORY_VIRTUAL) {
			SetLockSize(std::make_pair(ULARGE_INTEGER(), FileSizeStatus::Unavailable));
		}else {
			return CShellFolder::GetSize();
		}
	}
	return m_size;
}

std::wstring CKnownFolder::GetExt()
{
	if (m_wstrExt.empty()) {
		m_wstrExt = L"known";
	}
	return m_wstrExt;
}






CKnownFolderManager::CKnownFolderManager()
{
	CCoInitializer coinit;
	CComPtr<IKnownFolderManager> pMgr;
	if (FAILED(pMgr.CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER))) { return; }
	KNOWNFOLDERID* pknownid;
	UINT count(0);
	if (FAILED(pMgr->GetFolderIds(&pknownid, &count))) { return; }
	for (UINT i = 0; i < count; i++) {
		CComPtr<IKnownFolder> pFolder;
		if (SUCCEEDED(pMgr->GetFolder(pknownid[i], &pFolder))) {
			CIDL idl;
			pFolder->GetIDList(KF_FLAG_DEFAULT, idl.ptrptr());
			KNOWNFOLDERID kfid;
			pFolder->GetId(&kfid);
			LPWSTR pPath;
			pFolder->GetPath(KF_FLAG_DEFAULT, &pPath);
			KNOWNFOLDERID id;
			pFolder->GetId(&id);
			if (id == FOLDERID_Desktop) {
				CComPtr<IShellFolder> pDesktopFolder;
				::SHGetDesktopFolder(&pDesktopFolder);
				CIDL desktopIDL;
				::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, desktopIDL.ptrptr());

				m_knownFolders.push_back(std::make_shared<CKnownFolder>(pDesktopFolder, CIDL(), desktopIDL, pFolder, pDesktopFolder));
			} else if (idl) {
				auto parentIdl = idl.CloneParentIDL();
				auto childIdl = idl.CloneLastID();
				
				CComPtr<IShellFolder> pDesktopFolder;
				CComPtr<IShellFolder> pShellFolder;
				CComPtr<IShellFolder> pParentShellFolder;

				if (SUCCEEDED(::SHGetDesktopFolder(&pDesktopFolder)) &&
					SUCCEEDED(pDesktopFolder->BindToObject(idl.ptr(), 0, IID_IShellFolder, (void**)&pShellFolder)) &&
					((parentIdl && SUCCEEDED(pDesktopFolder->BindToObject(parentIdl.ptr(), 0, IID_IShellFolder, (void**)&pParentShellFolder))) ||
					(!parentIdl && SUCCEEDED(::SHGetDesktopFolder(&pParentShellFolder)))))
				{
					m_knownFolders.push_back(std::make_shared<CKnownFolder>(pParentShellFolder,parentIdl, childIdl, pFolder, pShellFolder));
				} else {
					BOOST_LOG_TRIVIAL(trace) <<L"nonenum/"<<kfid.Data1 << kfid.Data2;
				}
			} else {
				BOOST_LOG_TRIVIAL(trace) <<L"idlnull/" << kfid.Data1 << kfid.Data2;
			}
			//::ILFree(pidl);
			//This way couldn't get desktop, computer, etc path as CLSID
			//LPWSTR pPath;
			//pFolder->GetPath(NULL, &pPath);
			//if (pPath) {
			//	m_knownIconMap.emplace(std::make_pair(std::wstring(pPath), std::shared_ptr<CIcon>(nullptr)));
			//} else {

			//}
			//::CoTaskMemFree(pPath);
		}
	}

}

bool CKnownFolderManager::IsKnownFolder(CIDL& idl)
{
	return std::find_if(m_knownFolders.begin(), m_knownFolders.end(), [idl](const auto& folder)->bool {return folder->GetAbsoluteIdl() == idl; }) != m_knownFolders.end();
}

std::shared_ptr<CKnownFolder> CKnownFolderManager::GetKnownFolderByIDL(CIDL& idl)
{
	auto iter = std::find_if(m_knownFolders.begin(), m_knownFolders.end(), [idl](const auto& folder)->bool {return folder->GetAbsoluteIdl() == idl; });
	if (iter == m_knownFolders.end()) {
		return nullptr;
	}
	else {
		return *iter;
	}
}

std::shared_ptr<CKnownFolder> CKnownFolderManager::GetKnownFolderByPath(const std::wstring& path)
{
	auto iter = std::find_if(m_knownFolders.begin(), m_knownFolders.end(), [path](const auto& folder)->bool {return boost::iequals(folder->GetPath(), path); });
	if (iter == m_knownFolders.end()) {
		return nullptr;
	} else {
		return *iter;
	}
}

std::shared_ptr<CKnownFolder> CKnownFolderManager::GetKnownFolderById(const KNOWNFOLDERID& id)
{
	auto iter = std::find_if(m_knownFolders.begin(), m_knownFolders.end(), [id](const auto& folder)->bool {return folder->GetId() == id; });
	if (iter == m_knownFolders.end()) {
		return nullptr;
	} else {
		return *iter;
	}
}




CDriveFolder::CDriveFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CKnownDriveBaseFolder(pParentShellFolder, parentIdl, childIdl, pShellFolder){}

std::wstring CDriveFolder::GetExt()
{
	if (m_wstrExt.empty()) {
		m_wstrExt = L"drive";
	}
	return m_wstrExt;
}

CDriveManager::CDriveManager()
{
	std::array<wchar_t, 64> logicalDrives;

	if (::GetLogicalDriveStrings(64, logicalDrives.data())) {
		auto desktop(CKnownFolderManager::GetInstance()->GetKnownFolderById(FOLDERID_Desktop));
		wchar_t* p;
		for (p = logicalDrives.data(); *p != L'\0'; p += lstrlen(p) + 1) {
			CIDL relativeIdl;

			ULONG         chEaten;
			ULONG         dwAttributes;
			HRESULT hr = desktop->GetShellFolderPtr()->ParseDisplayName(
				NULL,
				NULL,
				const_cast<LPWSTR>(p),
				&chEaten,
				relativeIdl.ptrptr(),
				&dwAttributes);
			if (FAILED(hr)) { throw std::exception("CDriveFolderManager::CDriveFolderManager"); }
			
			auto idl = desktop->GetAbsoluteIdl() + relativeIdl;
			auto parentIdl = idl.CloneParentIDL();
			auto childIdl = idl.CloneLastID();

			CComPtr<IShellFolder> pShellFolder;
			CComPtr<IShellFolder> pParentShellFolder;

			if (SUCCEEDED(desktop->GetShellFolderPtr()->BindToObject(idl.ptr(), 0, IID_IShellFolder, (void**)&pShellFolder)) &&
				((parentIdl && SUCCEEDED(desktop->GetShellFolderPtr()->BindToObject(parentIdl.ptr(), 0, IID_IShellFolder, (void**)&pParentShellFolder))) ||
				(!parentIdl && SUCCEEDED(::SHGetDesktopFolder(&pParentShellFolder))))) {
				m_driveFolders.push_back(std::make_shared<CDriveFolder>(pParentShellFolder, parentIdl, childIdl, pShellFolder));
			} else {
				BOOST_LOG_TRIVIAL(trace) << L"nonenum";
			}
		}
	}
}

std::shared_ptr<CDriveFolder> CDriveManager::GetDriveFolderByIDL(CIDL& idl)
{
	auto iter = std::find_if(m_driveFolders.begin(), m_driveFolders.end(), [idl](const auto& folder)->bool {return folder->GetAbsoluteIdl() == idl; });
	if (iter == m_driveFolders.end()) {
		return nullptr;
	} else {
		return *iter;
	}
}

std::shared_ptr<CDriveFolder> CDriveManager::GetDriveFolderByPath(const std::wstring& path)
{
	auto iter = std::find_if(m_driveFolders.begin(), m_driveFolders.end(), [path](const auto& folder)->bool {return boost::iequals(folder->GetPath(), path); });
	if (iter == m_driveFolders.end()) {
		return nullptr;
	} else {
		return *iter;
	}
}

