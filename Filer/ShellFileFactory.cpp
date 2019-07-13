#include "ShellFileFactory.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "ShellZipFolder.h"
#include "KnownFolder.h"
#include "DriveFolder.h"
#include <chrono>
#include <functional>

std::shared_ptr<CShellFile> CShellFileFactory::CreateShellFilePtr(const CComPtr<IShellFolder>& pParentFolder, const CIDL& parentIdl, const CIDL& childIdl)
{

	CComPtr<IShellFolder> pFolder;
	CComPtr<IEnumIDList> enumIdl;
	STRRET strret;
	std::wstring path;
	std::wstring ext;
	if (SUCCEEDED(pParentFolder->GetDisplayNameOf(childIdl.ptr(), SHGDN_FORPARSING, &strret)))
	{
		path = childIdl.STRRET2WSTR(strret);
		ext = ::PathFindExtension(path.c_str());
	}

	if (path.empty()) {
		spdlog::info("CShellFileFactory::CreateShellFilePtr No Path");
		return nullptr;
	}

	if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - s_cacheTime).count() > 600) {
		spdlog::info("CShellFileFactory::CreateShellFilePtr Cache count:" + s_fileCache.size());
		s_cacheTime = std::chrono::system_clock::now();
		if (s_fileCache.size() > 10000) {
			s_fileCache.clear();
		}
	}

	if (auto iter = s_fileCache.find(std::make_tuple(path, pParentFolder, childIdl)); iter != s_fileCache.end()) {
		iter->second->Reset();
		return iter->second;
	} else {
		std::shared_ptr<CShellFile> pFile;
		if (!childIdl) {
			pFile = CKnownFolderManager::GetInstance()->GetDesktopFolder();
		} else if (auto spKnownFolder = CKnownFolderManager::GetInstance()->GetKnownFolderByPath(path)) {
			pFile = spKnownFolder;
		} else if (path[0] == L':') {
			pFile = std::make_shared<CShellFile>(pParentFolder, parentIdl, childIdl);
		} else if (auto spDriveFolder = CDriveManager::GetInstance()->GetDriveFolderByPath(path)) {
			pFile = spDriveFolder;
		} else if (boost::iequals(ext, ".zip")) {
			pFile = std::make_shared<CShellZipFolder>(pParentFolder, parentIdl, childIdl);
		} else if (
			//Do not use ::PathIsDirectory(path.c_str()), because it's slower
			(SUCCEEDED(pParentFolder->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pFolder)) &&
				SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)))) {
			pFile = std::make_shared<CShellFolder>(pParentFolder, parentIdl, childIdl, pFolder);
		} else {
			pFile = std::make_shared<CShellFile>(pParentFolder, parentIdl, childIdl);
		}
		s_fileCache.emplace(std::make_tuple(path, pParentFolder, childIdl), pFile);
		return pFile;
	}
}



//static
std::shared_ptr<CShellFile> CShellFileFactory::CreateShellFilePtr(const std::wstring& path)
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
