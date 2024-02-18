#include "ShellFunction.h"
#include "Debug.h"
#include "ThreadSafeKnownFolderManager.h"
#include "ThreadSafeDriveFolderManager.h"

std::wstring shell::ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit)

{
	// TODO depends on locale
	//return boost::lexical_cast<std::wstring>(n);
	bool is_minus = n < 0;
	is_minus ? n *= -1 : 0;

	std::wstringstream ss;
	ss << n;
	std::wstring snum = ss.str();
	std::reverse(snum.begin(), snum.end());
	std::wstringstream  ss_csnum;
	for (int i = 0, len = snum.length(); i <= len;) {
		ss_csnum << snum.substr(i, separate_digit);
		if ((i += separate_digit) >= len)
			break;
		ss_csnum << ',';
	}
	if (is_minus) {
		ss_csnum << '-';
	}

	std::wstring cs_num = ss_csnum.str();
	std::reverse(cs_num.begin(), cs_num.end());
	return cs_num;
}

std::wstring shell::FileTime2String(FILETIME* pFileTime)
{
	FILETIME ft;
	SYSTEMTIME st;

	FileTimeToLocalFileTime(pFileTime, &ft);
	FileTimeToSystemTime(&ft, &st);
	tstring str;
	wsprintf(GetBuffer(str, 16), L"%04d/%02d/%02d %02d:%02d",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	ReleaseBuffer(str);
	return str;

}

std::wstring shell::Size2String(ULONGLONG size)
{
	return ConvertCommaSeparatedNumber(size);
}

std::wstring shell::GetDisplayNameOf(const CComPtr<IShellFolder>& pParentFolder, const CIDL& childIDL, SHGDNF uFlags)
{
	return GetDisplayNameOf(pParentFolder, childIDL.ptr(), uFlags);
}

std::wstring shell::GetDisplayNameOf(const CComPtr<IShellFolder>& pParentFolder, const LPITEMIDLIST& childIDL, SHGDNF uFlags)
{
	STRRET strret{ 0 };
	std::wstring ret;
	if (pParentFolder && SUCCEEDED(pParentFolder->GetDisplayNameOf(childIDL, uFlags, &strret))) {
		return strret2wstring(strret, childIDL);
	}
	return ret;
}


std::wstring shell::strret2wstring(STRRET& strret, PCUITEMID_CHILD pidl)
{
	std::wstring ret;
	::StrRetToBufW(&strret, pidl, ::GetBuffer(ret, 256), 256);
	::ReleaseBuffer(ret);
	return ret;
}

std::tuple<std::wstring, std::wstring, std::wstring> shell::GetPathNameExt(const CComPtr<IShellFolder>& pParentFolder, const LPITEMIDLIST& relativeIDL)
{
	STRRET strret;
	std::wstring path;
	std::wstring name;
	std::wstring ext;
	if (SUCCEEDED(pParentFolder->GetDisplayNameOf(relativeIDL, SHGDN_FORPARSING, &strret))) {
		path = shell::strret2wstring(strret, relativeIDL);
		name = ::PathFindFileName(path.c_str());
		ext = ::PathFindExtension(path.c_str());
	}
	return std::make_tuple(path, name, ext);
}

std::optional<FileTimes> shell::GetFileTimes(const CComPtr<IShellFolder>& pParentFolder, const CIDL& relativeIDL)
{
	WIN32_FIND_DATA wfd = { 0 };
	if (SUCCEEDED(::SHGetDataFromIDList(pParentFolder, relativeIDL.ptr(), SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
		return  FileTimes(wfd.ftCreationTime, wfd.ftLastAccessTime, wfd.ftLastWriteTime);
	} else {
		return std::nullopt;
	}
}

bool shell::GetFileSize(ULARGE_INTEGER& size, const CComPtr<IShellFolder>& pParentShellFolder, const CIDL& childIdl)
{
	WIN32_FIND_DATA wfd = { 0 };
	if (!FAILED(::SHGetDataFromIDList(pParentShellFolder, childIdl.ptr(), SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
		size.LowPart = wfd.nFileSizeLow;
		size.HighPart = wfd.nFileSizeHigh;
		return true;
	} else {
		return false;
	}
}

void shell::FindIncrementalOne(
	const CIDL& srcParentIDL,
	const CIDL& srcChildIDL,
	const CIDL& destParentIDL,
	const std::function<void()> countup,
	const std::function<void(const CIDL&, const CIDL&)>& find)
{
	CComPtr<IShellFolder> pSrcParentFolder = shell::DesktopBindToShellFolder(srcParentIDL);
	CComPtr<IShellFolder> pDestParentFolder = shell::DesktopBindToShellFolder(destParentIDL);
	return FindIncrementalOne(pSrcParentFolder, srcParentIDL, srcChildIDL,
		pDestParentFolder, destParentIDL, countup, find);
}

void shell::FindIncrementalOne(
	const CComPtr<IShellFolder>& pSrcParentFolder,
	const CIDL& srcParentIDL,
	const CIDL& srcChildIDL,
	const CComPtr<IShellFolder>& pDestParentFolder,
	const CIDL& destParentIDL,
	const std::function<void()> countup,
	const std::function<void(const CIDL&, const CIDL&)>& find)
{
	ParsedFileType pft = shell::ParseFileType(pSrcParentFolder, srcChildIDL);

	ULONG chEaten = 0;
	ULONG dwAttributes = 0;
	CIDL destChildIDL;

	if (SUCCEEDED(pDestParentFolder->ParseDisplayName(
		NULL,
		NULL,
		const_cast<LPWSTR>(pft.FileName.c_str()),
		&chEaten,
		destChildIDL.ptrptr(),
		&dwAttributes)) && destChildIDL) {//Exist

		switch (pft.FileType) {
		case FileType::File:
		case FileType::Zip:
		{
			FILETIME srcTime = shell::GetFileTimes(pSrcParentFolder, srcChildIDL).value_or(FileTimes()).LastWriteTime;
			FILETIME destTime = shell::GetFileTimes(pDestParentFolder, destChildIDL).value_or(FileTimes()).LastWriteTime;
			if (::CompareFileTime(&srcTime, &destTime) > 0) {
				find(destParentIDL, (srcParentIDL + srcChildIDL));
			}
		}
		break;
		case FileType::Folder:
		{
			CComPtr<IShellFolder> pSrcChildShellFolder;
			CComPtr<IEnumIDList> pSrcChildEnumIDL;
			if (SUCCEEDED(pSrcParentFolder->BindToObject(srcChildIDL.ptr(), 0, IID_IShellFolder, (void**)&pSrcChildShellFolder)) &&
				SUCCEEDED(pSrcChildShellFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pSrcChildEnumIDL))) {

				CComPtr<IShellFolder> pDestChildFolder;
				if (SUCCEEDED(pDestParentFolder->BindToObject(destChildIDL.ptr(), 0, IID_IShellFolder, (void**)&pDestChildFolder))) {
					CIDL srcGrandchildIDL;
					ULONG ulRet(0);
					while (SUCCEEDED(pSrcChildEnumIDL->Next(1, srcGrandchildIDL.ptrptr(), &ulRet))) {
						if (srcGrandchildIDL) {
							shell::FindIncrementalOne(pSrcChildShellFolder, srcParentIDL + srcChildIDL, srcGrandchildIDL,
								pDestChildFolder, destParentIDL + destChildIDL, countup, find);
						} else {
							break;
						}
					}
				}
			}
		}
			break;
		case FileType::Known:
		case FileType::Drive:
		case FileType::Virtual:
		case FileType::None:
		default:
			break;
		}
		countup();
	} else {
		switch (pft.FileType) {
		case FileType::File:
		case FileType::Zip:
		{
			find(destParentIDL, (srcParentIDL + srcChildIDL));
		}
		break;
		case FileType::Folder:
		{
			CComPtr<IShellFolder> pSrcChildShellFolder;
			CComPtr<IEnumIDList> pSrcChildEnumIDL;
			if (SUCCEEDED(pSrcParentFolder->BindToObject(srcChildIDL.ptr(), 0, IID_IShellFolder, (void**)&pSrcChildShellFolder)) &&
				SUCCEEDED(pSrcChildShellFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pSrcChildEnumIDL))) {

				shell::CountFileInFolder(pSrcChildShellFolder, pSrcChildEnumIDL, srcChildIDL, countup);
				find(destParentIDL, (srcParentIDL + srcChildIDL));
			}
		}
		break;
		case FileType::Known:
		case FileType::Drive:
		case FileType::Virtual:
		case FileType::None:
		default:
			break;
		}
		countup();
	}
}

//void shell::GetIncrementalIDLs(
//	const CComPtr<IShellFolder>& pSrcFolder,
//	const CComPtr<IEnumIDList>& pEnum,
//	const CIDL& srcIDL,
//	const CComPtr<IShellFolder>& pDestFolder,
//	const CIDL& destIDL,
//	const std::function<void(int, const CIDL&, const CIDL&)>& read)
//{
//	CIDL srcChildIDL;
//	ULONG ulRet(0);
//	while (SUCCEEDED(pEnum->Next(1, srcChildIDL.ptrptr(), &ulRet))) {
//		if (!srcChildIDL) { break; }
//		shell::CheckIncrementalIDL(pSrcFolder, srcIDL, srcChildIDL, pDestFolder, destIDL, read);
//	}
//}

//void shell::GetIncrementalIDLs(
//	const CComPtr<IShellFolder>& pSrcFolder,
//	const CIDL& srcIDL,
//	const std::vector<CIDL>& srcChildIDLs,
//	const CComPtr<IShellFolder>& pDestFolder,
//	const CIDL& destIDL,
//	const std::function<void(int, const CIDL&, const CIDL&)>& read)
//{
//	for (const auto& srcChildIDL : srcChildIDLs) {
//		if (!srcChildIDL) { break; }
//		shell::CheckIncrementalIDL(pSrcFolder, srcIDL, srcChildIDL, pDestFolder, destIDL, read);
//	}
//}

CComPtr<IShellFolder> shell::DesktopBindToShellFolder(const CIDL& idl)
{
	CComPtr<IShellFolder> pDesktopFolder;
	if (FAILED(::SHGetDesktopFolder(&pDesktopFolder))) {
		throw std::exception(FILE_LINE_FUNC);
	}

	CComPtr<IShellFolder> pFolder;

	if (FAILED(pDesktopFolder->BindToObject(idl.ptr(), 0, IID_IShellFolder, (void**)&pFolder))) {
		if (CIDL desktopIDL; SUCCEEDED(::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, desktopIDL.ptrptr())) && ::ILIsEqual(idl.ptr(), desktopIDL.ptr())) {
			pFolder = pDesktopFolder;
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}
	return pFolder;
}


//void shell::GetIncrementalIDLs(
//	const CIDL& srcIDL,
//	const std::vector<CIDL>& srcChildIDLs,
//	const CIDL& destIDL,
//	const std::function<void(int, const CIDL&, const CIDL&)>& read)
//{
//	std::vector<std::pair<CIDL, std::vector<CIDL>>> idlPairs;
//	std::vector<CIDL> idls;
//
//	CComPtr<IShellFolder> pSrcFolder = shell::DesktopBindToShellFolder(srcIDL);
//	CComPtr<IShellFolder> pDestFolder = shell::DesktopBindToShellFolder(destIDL);
//
//	return shell::GetIncrementalIDLs(
//		pSrcFolder,
//		srcIDL,
//		srcChildIDLs,
//		pDestFolder,
//		destIDL, read);
//}


//
//int shell::GetEnumCount(
//	const CComPtr<IShellFolder>& pFolder,
//	const CComPtr<IEnumIDList>& pEnum,
//	const std::function<void(int)>& read)
//{
//	int count = 0;
//
//	auto fileFun = [&]()->void {
//		count++;
//		read(1);
//	};
//
//	auto dirFun = [&](const CComPtr<IShellFolder>& pSrcItemFolder, const CComPtr<IEnumIDList>& pSrcItemEnum)->void {
//		count += GetEnumCount(pSrcItemFolder, pSrcItemEnum, read);
//		read(1);
//	};
//
//	auto virFun = [&]()->void {
//		count++;
//		read(1);
//	};
//
//	CIDL childIDL;
//	ULONG ulRet(0);
//	while (SUCCEEDED(pEnum->Next(1, childIDL.ptrptr(), &ulRet))) {
//
//		if (childIDL) {
//			shell::RunFunctionEachFileFolderVirtual<void>(pFolder, childIDL, fileFun, dirFun, virFun);
//		} else {
//			break;
//		}
//	}
//	return count;
//}

//int shell::GetFileCount(
//	const CComPtr<IShellFolder>& pSrcFolder,
//	const CIDL& srcIDL,
//	const CIDL& srcChildIDL,
//	const std::function<void(int)>& read)
//{
//	int count = 0;
//
//	auto fileFun = [&]()->void {
//		count++;
//		read(1);
//	};
//
//	auto dirFun = [&](const CComPtr<IShellFolder>& pSrcItemFolder, const CComPtr<IEnumIDList>& pSrcItemEnum)->void {
//		count += GetEnumCount(pSrcItemFolder, pSrcItemEnum, read);
//		read(1);
//	};
//
//	auto virFun = [&]()->void {
//		count++;
//		read(1);
//	};
//
//	shell::RunFunctionEachFileFolderVirtual<void>(pSrcFolder, srcChildIDL, fileFun, dirFun, virFun);
//
//	return count;
//}




void shell::CountFileOne(
	const CComPtr<IShellFolder>& pParentFolder,
	const CIDL& parentIDL,
	const CIDL& childIDL,
	const std::function<void()>& countup)
{
	ParsedFileType pft = shell::ParseFileType(pParentFolder, childIDL);

	switch (pft.FileType) {
	case FileType::File:
	case FileType::Zip:
		break;
	case FileType::Folder:
	{
		CComPtr<IShellFolder> pChildShellFolder;
		CComPtr<IEnumIDList> pChildEnumIDL;
		if (SUCCEEDED(pParentFolder->BindToObject(childIDL.ptr(), 0, IID_IShellFolder, (void**)&pChildShellFolder)) &&
			SUCCEEDED(pChildShellFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pChildEnumIDL))) {
			shell::CountFileInFolder(pChildShellFolder, pChildEnumIDL, parentIDL + childIDL, countup);
		}
	}
		break;
	case FileType::Known:
	case FileType::Drive:
	case FileType::Virtual:
	case FileType::None:
	default:
		break;
	}
	countup();
}


void shell::CountFileOne(
	const CIDL& parentIDL,
	const CIDL& childIDL,
	const std::function<void()>& countup)
{
	CComPtr<IShellFolder> pParentFolder = shell::DesktopBindToShellFolder(parentIDL);
	return shell::CountFileOne(pParentFolder, parentIDL, childIDL, countup);
}

void shell::CountFileInFolder(
	const CComPtr<IShellFolder>& pFolder,
	const CComPtr<IEnumIDList>& pEnumIDL,
	const CIDL& idl,
	const std::function<void()>& countup)
{
	CIDL childIDL;
	ULONG ulRet(0);
	while (SUCCEEDED(pEnumIDL->Next(1, childIDL.ptrptr(), &ulRet))) {
		if (childIDL) {
			shell::CountFileOne(pFolder, idl, childIDL, countup);
		} else {
			break;
		}
	}
}

void shell::CountFileInFolder(
	const CIDL& srcIDL,
	const std::function<void()>& countup)
{
	CComPtr<IShellFolder> pSrcFolder = shell::DesktopBindToShellFolder(srcIDL);
	CComPtr<IEnumIDList> pEnumIDL;
	if (SUCCEEDED(pSrcFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pEnumIDL))) {
		shell::CountFileInFolder(pSrcFolder, pEnumIDL, srcIDL, countup);
	}

}


//CComPtr<IShellFolder> shell::GetParentShellFolderByIDL(const CIDL& absIDL)
//{
//	CIDL parentIDL = absIDL.CloneParentIDL();
//	return shell::DesktopBindToShellFolder(parentIDL);
//}


bool shell::CopyFiles(const CIDL& destIDL, const std::vector<LPITEMIDLIST>& srcIDLs)
{
	CComPtr<IShellItem2> pDestItem;
	HRESULT hr = ::SHCreateItemFromIDList(destIDL.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestItem));
	if (FAILED(hr)) { return false; }

	CComPtr<IShellItemArray> pSrcItemAry = nullptr;
	hr = ::SHCreateShellItemArrayFromIDLists(srcIDLs.size(), (LPCITEMIDLIST*)(srcIDLs.data()), &pSrcItemAry);
	if (FAILED(hr)) { return false; }

	CComPtr<IFileOperation> pFileOperation;

	hr = pFileOperation.CoCreateInstance(CLSID_FileOperation);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->CopyItems(pSrcItemAry, pDestItem);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->PerformOperations();
	return SUCCEEDED(hr);
}

bool shell::MoveFiles(const CIDL& destIDL, const std::vector<LPITEMIDLIST>& srcIDLs)
{
	CComPtr<IShellItem2> pDestItem;
	HRESULT hr = ::SHCreateItemFromIDList(destIDL.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestItem));
	if (FAILED(hr)) { return false; }

	CComPtr<IShellItemArray> pSrcItemAry = nullptr;
	hr = ::SHCreateShellItemArrayFromIDLists(srcIDLs.size(), (LPCITEMIDLIST*)(srcIDLs.data()), &pSrcItemAry);
	if (FAILED(hr)) { return false; }

	CComPtr<IFileOperation> pFileOperation;

	hr = pFileOperation.CoCreateInstance(CLSID_FileOperation);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->MoveItems(pSrcItemAry, pDestItem);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->PerformOperations();
	return SUCCEEDED(hr);
}

bool shell::DeleteFiles(const std::vector<LPITEMIDLIST>& srcIDLs)
{
	CComPtr<IShellItemArray> pSrcItemAry = nullptr;
	HRESULT hr = ::SHCreateShellItemArrayFromIDLists(srcIDLs.size(), (LPCITEMIDLIST*)(srcIDLs.data()), &pSrcItemAry);
	if (FAILED(hr)) { return false; }

	CComPtr<IFileOperation> pFileOperation;

	hr = pFileOperation.CoCreateInstance(CLSID_FileOperation);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->DeleteItems(pSrcItemAry);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->PerformOperations();
	return SUCCEEDED(hr);
}




void shell::SearchFileInFolder(
	const std::wstring& search,
	const CIDL& srcIDL,
	const std::function<void()>& countup,
	const std::function<void(const CIDL&)> find
)
{
	CComPtr<IShellFolder> pSrcFolder = shell::DesktopBindToShellFolder(srcIDL);
	CComPtr<IEnumIDList> pEnumIDL;
	if(SUCCEEDED(pSrcFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pEnumIDL))){
		CIDL childIDL;
		ULONG ulRet(0);
		while (SUCCEEDED(pEnumIDL->Next(1, childIDL.ptrptr(), &ulRet))) {

			if (childIDL) {
				SearchOne(search, pSrcFolder, srcIDL, childIDL, countup, find);
			} else {
				break;
			}
		}
	}
}

void shell::SearchOne(
	const std::wstring& search,
	const CComPtr<IShellFolder>& pParentFolder,
	const CIDL& parentIDL,
	const CIDL& childIDL,
	const std::function<void()>& countup,
	const std::function<void(const CIDL&)> find
)
{
	ParsedFileType pft = shell::ParseFileType(pParentFolder, childIDL);
	if (!boost::algorithm::ifind_first(pft.FileName, search).empty()) {
		find(parentIDL + childIDL);
	}

	switch (pft.FileType) {
	case FileType::File:
	case FileType::Zip:
		break;
	case FileType::Folder:
	{
		CComPtr<IShellFolder> pChildShellFolder;
		CComPtr<IEnumIDList> pChildEnumIDL;
		if (SUCCEEDED(pParentFolder->BindToObject(childIDL.ptr(), 0, IID_IShellFolder, (void**)&pChildShellFolder)) &&
			SUCCEEDED(pChildShellFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pChildEnumIDL))) {

			CIDL nextIDL;
			ULONG ulRet(0);
			while (SUCCEEDED(pChildEnumIDL->Next(1, nextIDL.ptrptr(), &ulRet))) {
				if (nextIDL) {
					shell::SearchOne(search, pChildShellFolder, parentIDL + childIDL, nextIDL, countup, find);
				} else {
					break;
				}
			}
		}
	}
		break;
	case FileType::Known:
	case FileType::Drive:
	case FileType::Virtual:
	case FileType::None:
	default:
		break;
	}
	countup();

}

//shell::ParsedFileType shell::ParseFileTypeSimple(
//	const CComPtr<IShellFolder>& pParentFolder,
//	const CIDL& childIDL)
//{
//	ParsedFileType ret;
//	ret.FilePath = shell::GetDisplayNameOf(pParentFolder, childIDL);
//	if (ret.FilePath.empty()) {
//		ret.FileType = FileType::None;
//		return ret;
//	} else {
//		ret.FileName = ::PathFindFileName(ret.FilePath.c_str());
//		ret.FileExt = ::PathFindExtension(ret.FilePath.c_str());
//
//		if (ret.FilePath[0] == L':') {
//			//Virtual
//			ret.FileType = FileType::Virtual;
//		} else if (boost::iequals(ret.FileExt, ".zip")) {
//			//Zip
//			ret.FileType = FileType::Zip;
//		} else if (
//
//			CComPtr<IShellFolder> pShellFolder;
//			CComPtr<IEnumIDList> pEnumIDL;
//			SUCCEEDED(pParentFolder->BindToObject(childIDL.ptr(), 0, IID_IShellFolder, (void**)&pShellFolder)) &&
//			SUCCEEDED(pShellFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pEnumIDL))) {
//			//Folder
//			ret.FileType = FileType::Folder;
//		} else {
//			//File
//			ret.FileType = FileType::File;
//		}
//		return ret;
//	}
//}

shell::ParsedFileType shell::ParseFileType(
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





