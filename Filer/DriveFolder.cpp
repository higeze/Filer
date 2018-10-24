#include "DriveFolder.h"

CDriveFolder::CDriveFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CKnownDriveBaseFolder(pParentShellFolder, parentIdl, childIdl, pShellFolder)
{
}

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
		auto desktop(CKnownFolderManager::GetInstance()->GetDesktopFolder());
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

