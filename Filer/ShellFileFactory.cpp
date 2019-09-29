#include "ShellFileFactory.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "ShellZipFolder.h"
#include "KnownFolder.h"
#include "DriveFolder.h"
#include <chrono>
#include <functional>
#include "ShellFunction.h"
#include "ThreadSafeKnownFolderManager.h"
#include "ThreadSafeDriveFolderManager.h"

shell::ParsedFileType CShellFileFactory::ParseFileType(
	const CComPtr<IShellFolder>& pParentFolder,
	const CIDL& childIDL)
{
	shell::ParsedFileType ret;
	ret.FilePath = shell::GetDisplayNameOf(pParentFolder, childIDL);
	if (ret.FilePath.empty()) {
		ret.FileType = shell::FileType::None;
	} else {
		ret.FileName = ::PathFindFileName(ret.FilePath.c_str());
		ret.FileExt = ::PathFindExtension(ret.FilePath.c_str());
		SFGAOF sfgao = SFGAO_FOLDER;
		if (shell::CThreadSafeKnownFolderManager::GetInstance()->IsExist(ret.FilePath)) {
			ret.FileType = shell::FileType::Known;
		} else if (ret.FilePath[0] == L':') {
			ret.FileType = shell::FileType::Virtual;
		} else if (shell::CThreadSafeDriveFolderManager::GetInstance()->IsExist(ret.FilePath)) {
			ret.FileType = shell::FileType::Drive;
		} else if (boost::iequals(ret.FileExt, ".zip")) {
			ret.FileType = shell::FileType::Zip;
		} else if (pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)(childIDL.ptrptr()), &sfgao), (sfgao & SFGAO_FOLDER) == SFGAO_FOLDER) {
			ret.FileType = shell::FileType::Folder;
		} else {
			ret.FileType = shell::FileType::File;
		}
	}
	return ret;
}


std::shared_ptr<CShellFile> CShellFileFactory::CreateShellFilePtr(const CComPtr<IShellFolder>& pParentFolder, const CIDL& parentIdl, const CIDL& childIdl)
{
	auto parsed = ParseFileType(pParentFolder, childIdl);
	switch (parsed.FileType) {
	case shell::FileType::Drive:
		return CDriveFolderManager::GetInstance()->GetDriveFolderByPath(parsed.FilePath);
	case shell::FileType::Known:
		return CKnownFolderManager::GetInstance()->GetKnownFolderByPath(parsed.FilePath);
	case shell::FileType::Folder:
		return std::make_shared<CShellFolder>(pParentFolder, parentIdl, childIdl);
	case shell::FileType::Zip:
		return std::make_shared<CShellZipFolder>(pParentFolder, parentIdl, childIdl);
	case shell::FileType::Virtual:
	case shell::FileType::File:
		return std::make_shared<CShellFile>(pParentFolder, parentIdl, childIdl);
	case shell::FileType::None:
	default:
		return std::make_shared<CShellInvalidFile>();
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
