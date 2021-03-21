#include "DriveFolder.h"
#include "Debug.h"
#include "ThreadSafeDriveFolderManager.h"

CDriveFolder::CDriveFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CKnownDriveBaseFolder(pParentShellFolder, parentIdl, childIdl, pShellFolder)
{
}

std::wstring CDriveFolder::GetDispExt()
{
	if (m_wstrExt.empty()) {
		m_wstrExt = L"drive";
	}
	return m_wstrExt;
}

CDriveFolderManager::CDriveFolderManager()
{
	Update();
}

void CDriveFolderManager::Update()
{
	shell::CThreadSafeDriveFolderManager::GetInstance()->Update();

	m_driveFolders.clear();
	
	CComPtr<IShellFolder> pDesktopFolder;
	::SHGetDesktopFolder(&pDesktopFolder);

	auto& map = shell::CThreadSafeDriveFolderManager::GetInstance()->GetPathIdlMap();
	for(auto& pair : map){
		auto parentIdl = pair.second.CloneParentIDL();
		auto childIdl = pair.second.CloneLastID();

		CComPtr<IShellFolder> pShellFolder;
		CComPtr<IShellFolder> pParentShellFolder;

		if (SUCCEEDED(pDesktopFolder->BindToObject(pair.second.ptr(), 0, IID_IShellFolder, (void**)&pShellFolder)) &&
			((parentIdl && SUCCEEDED(pDesktopFolder->BindToObject(parentIdl.ptr(), 0, IID_IShellFolder, (void**)&pParentShellFolder))) ||
			(!parentIdl && SUCCEEDED(::SHGetDesktopFolder(&pParentShellFolder))))) {
			m_driveFolders.push_back(std::make_shared<CDriveFolder>(pParentShellFolder, parentIdl, childIdl, pShellFolder));
		} else {
			LOG_THIS_1("nonenum");
		}
	}

}

bool CDriveFolderManager::IsExist(const std::wstring& path)
{
	auto iter = std::find_if(m_driveFolders.begin(), m_driveFolders.end(), [path](const auto& folder)->bool {return boost::iequals(folder->GetPath(), path); });
	return iter != m_driveFolders.end();
}

std::shared_ptr<CDriveFolder> CDriveFolderManager::GetDriveFolderByIDL(CIDL& idl)
{
	auto iter = std::find_if(m_driveFolders.begin(), m_driveFolders.end(), [idl](const auto& folder)->bool {return folder->GetAbsoluteIdl() == idl; });
	if (iter == m_driveFolders.end()) {
		return nullptr;
	} else {
		return *iter;
	}
}

std::shared_ptr<CDriveFolder> CDriveFolderManager::GetDriveFolderByPath(const std::wstring& path)
{
	auto iter = std::find_if(m_driveFolders.begin(), m_driveFolders.end(), [path](const auto& folder)->bool {return boost::iequals(folder->GetPath(), path); });
	if (iter == m_driveFolders.end()) {
		return nullptr;
	} else {
		return *iter;
	}
}

