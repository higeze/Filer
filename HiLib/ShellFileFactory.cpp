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
#include "Debug.h"

shell::ParsedFileType CShellFileFactory::ParseFileType(
	const CComPtr<IShellFolder>& pParentFolder,
	const CIDL& childIDL)
{
	shell::ParsedFileType ret;
	ret.FilePath = shell::GetDisplayNameOf(pParentFolder, childIDL, SHGDN_FORPARSING);

	if (ret.FilePath.empty()) {
		ret.FileType = shell::FileType::None;
	} else {
		ret.FileName = ::PathFindFileName(ret.FilePath.c_str());
		ret.FileExt = ::PathFindExtension(ret.FilePath.c_str());
		//{
		//	::OutputDebugString(std::format(L"name:{}\r\n", ret.FileName.c_str()).c_str());

		//	ULONG sfgao = SFGAO_CAPABILITYMASK | SFGAO_GHOSTED | SFGAO_LINK | SFGAO_SHARE | SFGAO_FOLDER | SFGAO_FILESYSTEM;
		//	pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)(childIDL.ptrptr()), &sfgao);
		//	::OutputDebugString(std::format(L"sfgao:{}\r\n", sfgao).c_str());

		//	WIN32_FIND_DATA wfd = { 0 };
		//	::SHGetDataFromIDList(pParentFolder, childIDL.ptr(), SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA));
		//	::OutputDebugString(std::format(L"attr:{}\r\n", wfd.dwFileAttributes).c_str());
		//}

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
	if (path.empty() ||
		path[0] != L':' && !::PathFileExists(path.c_str()))
	{
		return std::make_shared<CShellInvalidFile>();
	} else {
		//{
		//	LOG_SCOPED_TIMER_THIS_1("PathFileExists");
		//	::PathFileExists(path.c_str());
		//}
		//{
		//	LOG_SCOPED_TIMER_THIS_1("GetFileAttributes");
		//	::GetFileAttributes(path.c_str());
		//}
		//{
		//	LOG_SCOPED_TIMER_THIS_1("PathFileExists");
		//	::ILCreateFromPath(path.c_str());
		//}
		{
			LOG_SCOPED_TIMER_THIS_1("ParseDisplayName");

			auto desktop(CKnownFolderManager::GetInstance()->GetDesktopFolder());
			CIDL absIdl;

			ULONG         chEaten;
			ULONG         dwAttributes;
			HRESULT hr = desktop->GetShellFolderPtr()->ParseDisplayName(
				NULL,
				NULL,
				const_cast<LPWSTR>(path.c_str()),
				&chEaten,
				absIdl.ptrptr(),
				&dwAttributes);

			if (FAILED(hr)) {//Not Exist
				return std::make_shared<CShellInvalidFile>();
			} else {
				CIDL parentIDL = absIdl.CloneParentIDL();
				CComPtr<IShellFolder>  pParentFolder = shell::DesktopBindToShellFolder(parentIDL);

				return CreateShellFilePtr(pParentFolder, parentIDL, absIdl.CloneLastID());
			}
		}
	}
}
