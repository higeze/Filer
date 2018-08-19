#include "KnownFolder.h"
#include "MyCom.h"
#include "MyIcon.h"

//CKnownFolder::CKnownFolder(CComPtr<IKnownFolder>& pKnownFolder, CIDL& IDL)
//	:CShellFolder(IDL), m_pKnownFolder(pKnownFolder){}


CKnownFolder::CKnownFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder, CComPtr<IKnownFolder>& pKnownFolder)
	:CShellFolder(pParentShellFolder, parentIdl, childIdl,pShellFolder), m_pKnownFolder(pKnownFolder){}

//CIDL CKnownFolder::GetIDL()
//{
//	if (!m_idl) {
//		m_pKnownFolder->GetIDList(NULL, m_idl.ptrptr());
//	}
//	return m_idl;
//}

//std::wstring CKnownFolder::GetPath()
//{ 
//	if (m_path.empty()) {
//		CComPtr<IShellFolder> pDesktop;
//		::SHGetDesktopFolder(&pDesktop);
//		STRRET strret;
//		std::wstring path = STRRET2WSTR(strret, GetIDL().ptr());
//	}
//	return m_path;
//}

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

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CKnownFolder::GetIcon()
{
	if (GetLockIcon().second == FileIconStatus::None) {
		SHFILEINFO sfi = { 0 };
		::SHGetFileInfo((LPCTSTR)GetAbsoluteIdl().ptr(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
		SetLockIcon(std::make_pair(std::make_shared<CIcon>(sfi.hIcon), FileIconStatus::Available));
	}
	return m_icon;
}

std::wstring CKnownFolder::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
		m_wstrFileNameWithoutExt = m_childIdl.STRRET2WSTR(strret);
	}
	return m_wstrFileNameWithoutExt;
}

std::wstring CKnownFolder::GetFileName()
{
	if (m_wstrFileName.empty()) {
		STRRET strret;
		m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
		m_wstrFileName = m_childIdl.STRRET2WSTR(strret);
	}
	return m_wstrFileName;
}

std::wstring CKnownFolder::GetExt()
{
	if (m_wstrExt.empty()) {
		m_wstrExt = L"kf";
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
			if (idl) {
				auto parentIdl = idl.CloneParentIDL();
				auto childIdl = idl.CloneLastID();
				
				CShellFolder desktop;
				CComPtr<IShellFolder> pShellFolder;
				CComPtr<IShellFolder> pParentShellFolder;

				if (SUCCEEDED(desktop.GetShellFolderPtr()->BindToObject(idl.ptr(), 0, IID_IShellFolder, (void**)&pShellFolder)) &&
					((parentIdl && SUCCEEDED(desktop.GetShellFolderPtr()->BindToObject(parentIdl.ptr(), 0, IID_IShellFolder, (void**)&pParentShellFolder))) ||
					(!parentIdl && SUCCEEDED(::SHGetDesktopFolder(&pParentShellFolder)))))
				{
					m_knownFolders.push_back(std::make_shared<CKnownFolder>(pParentShellFolder,parentIdl, childIdl, pShellFolder, pFolder));
				} else {
					std::wcout <<L"nonenum/"<<kfid.Data1 << kfid.Data2 << std::endl;
				}
			} else {
				std::wcout <<L"idlnull/" << kfid.Data1 << kfid.Data2  << std::endl;
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


