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
			m_pIconThread.reset(new std::thread([this]()->void {
				try {
					CCoInitializer coinit(COINIT_APARTMENTTHREADED);
					SHFILEINFO sfi = { 0 };
					::SHGetFileInfo((LPCTSTR)GetAbsoluteIdl().ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
					SetLockIcon(std::make_pair(std::make_shared<CIcon>(sfi.hIcon), FileIconStatus::Available));
					SignalFileIconChanged(this);
				} catch (...) {
					BOOST_LOG_TRIVIAL(error) << L"CKnownDriveBaseFolder::GetIcon " + GetFileNameWithoutExt() + L" Icon thread exception";
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
			//LPWSTR pPath;
			//pFolder->GetPath(KF_FLAG_DEFAULT, &pPath);
			KNOWNFOLDERID id;
			pFolder->GetId(&id);
			if (id == FOLDERID_Desktop) {
				CComPtr<IShellFolder> pDesktopFolder;
				::SHGetDesktopFolder(&pDesktopFolder);
				CIDL desktopIDL;
				::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, desktopIDL.ptrptr());
				auto knownFolder = std::make_shared<CKnownFolder>(pDesktopFolder, CIDL(), desktopIDL, pFolder, pDesktopFolder);
				m_knownFolderMap.insert(std::make_pair(knownFolder->GetPath(), knownFolder));
			} else if (idl) {
				auto parentIdl = idl.CloneParentIDL();
				auto childIdl = idl.CloneLastID();
				
				CComPtr<IShellFolder> pDesktopFolder;
				CComPtr<IShellFolder> pShellFolder;
				CComPtr<IShellFolder> pParentShellFolder;

				if (SUCCEEDED(::SHGetDesktopFolder(&pDesktopFolder)) &&
					SUCCEEDED(pDesktopFolder->BindToObject(idl.ptr(), 0, IID_IShellFolder, (void**)&pShellFolder)) &&
					pShellFolder &&
					((parentIdl && parentIdl.m_pIDL->mkid.cb && SUCCEEDED(pDesktopFolder->BindToObject(parentIdl.ptr(), 0, IID_IShellFolder, (void**)&pParentShellFolder))) ||
					((!parentIdl || !parentIdl.m_pIDL->mkid.cb) && SUCCEEDED(::SHGetDesktopFolder(&pParentShellFolder)))) &&
					pParentShellFolder)
				{
					auto knownFolder = std::make_shared<CKnownFolder>(pParentShellFolder, parentIdl, childIdl, pFolder, pShellFolder);
					m_knownFolderMap.insert(std::make_pair(knownFolder->GetPath(), knownFolder));
				} else {
					BOOST_LOG_TRIVIAL(trace) << L"CKnownFolder::CKnownFolder Non enumerable "
						<< (boost::format("%1$08x-%2$04x") % kfid.Data1 % kfid.Data2).str();
				}
			} else {
				BOOST_LOG_TRIVIAL(trace) <<L"CKnownFolder::CKnownFolder IDL is null "
					<< (boost::format("%1$08x-%2$04x") % kfid.Data1 % kfid.Data2).str();
			}
		}
	}

}

//bool CKnownFolderManager::IsKnownFolder(CIDL& idl)
//{
//	return std::find_if(m_knownFolderMap.begin(), m_knownFolderMap.end(), [idl](const auto& folder)->bool {return folder->GetAbsoluteIdl() == idl; }) != m_knownFolderMap.end();
//}
//
//std::shared_ptr<CKnownFolder> CKnownFolderManager::GetKnownFolderByIDL(CIDL& idl)
//{
//	auto iter = std::find_if(m_knownFolderMap.begin(), m_knownFolderMap.end(), [idl](const auto& folder)->bool {return folder->GetAbsoluteIdl() == idl; });
//	if (iter == m_knownFolderMap.end()) {
//		return nullptr;
//	}
//	else {
//		return *iter;
//	}
//}
//
std::shared_ptr<CKnownFolder> CKnownFolderManager::GetKnownFolderByPath(const std::wstring& path)
{
	auto iter = m_knownFolderMap.find(path);
	if (iter == m_knownFolderMap.end()) {
		return nullptr;
	} else {
		return iter->second;
	}
}

std::shared_ptr<CKnownFolder> CKnownFolderManager::GetKnownFolderById(const KNOWNFOLDERID& id)
{
	auto iter = std::find_if(m_knownFolderMap.begin(), m_knownFolderMap.end(), [id](const auto& pair)->bool {return pair.second->GetId() == id; });
	if (iter == m_knownFolderMap.end()) {
		return nullptr;
	} else {
		return iter->second;
	}
}

std::shared_ptr<CKnownFolder> CKnownFolderManager::GetDesktopFolder()
{
	if (!m_desktopFolder) {
		m_desktopFolder = GetKnownFolderById(FOLDERID_Desktop);
	}
	return m_desktopFolder;
}
