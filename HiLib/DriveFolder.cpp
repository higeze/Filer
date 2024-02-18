#include "DriveFolder.h"
#include "Debug.h"
#include "ThreadSafeDriveFolderManager.h"

CDriveFolder::CDriveFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CKnownDriveBaseFolder(pParentShellFolder, parentIdl, childIdl, pShellFolder)
{
}

const std::wstring& CDriveFolder::GetDispExt() const
{
	if (!m_optDispExt.has_value()) {
		m_optDispExt.emplace(L"drive");
	}
	return m_optDispExt.value();
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CDriveFolder::GetSize(const std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed)
{
	auto [avail, total, free] = GetSizes();
	if (total.QuadPart == 0) {
		return std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable);
	} else {
		ULARGE_INTEGER used = { 0 };
		used.QuadPart = total.QuadPart - free.QuadPart;
		return std::make_pair(used, FileSizeStatus::Available);
	}
}

std::tuple<ULARGE_INTEGER, ULARGE_INTEGER, ULARGE_INTEGER> CDriveFolder::GetSizes()
{
	ULARGE_INTEGER avail = { 0 };
	ULARGE_INTEGER total = { 0 };
	ULARGE_INTEGER free = { 0 };
	if (::GetDiskFreeSpaceExW(GetPath().c_str(), &avail, &total, &free)) {
		return { avail, total, free };
	} else {
		return { {0}, {0}, {0} };
	}
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

