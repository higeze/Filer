#include "ShellFunction.h"
#include "Debug.h"

std::wstring shell::STRRET2WSTR(STRRET& strret, LPITEMIDLIST pidl)
{
	int nLength;
	LPSTR lpmstr;
	WCHAR wcRet[MAX_PATH] = { 0 };
	switch (strret.uType) {
	case STRRET_WSTR:
		return std::wstring(strret.pOleStr);
		break;
	case STRRET_OFFSET:
		lpmstr = (LPSTR)(((char*)pidl) + strret.uOffset);
		nLength = ::MultiByteToWideChar(CP_THREAD_ACP, 0, lpmstr, -1, NULL, 0);
		::MultiByteToWideChar(CP_THREAD_ACP, 0, lpmstr, -1, wcRet, nLength);
		break;
	case STRRET_CSTR:
		lpmstr = strret.cStr;
		nLength = ::MultiByteToWideChar(CP_THREAD_ACP, 0, lpmstr, -1, NULL, 0);
		::MultiByteToWideChar(CP_THREAD_ACP, 0, lpmstr, -1, wcRet, nLength);
		//::wcscpy_s(wcRet,wcslen(wcRet),(LPCWSTR)strret.cStr);
		break;
	default:
		break;
	}
	return std::wstring(wcRet);
}

std::tuple<std::wstring, std::wstring, std::wstring> shell::GetPathNameExt(const CComPtr<IShellFolder>& pParentFolder, const LPITEMIDLIST& relativeIDL)
{
	STRRET strret;
	std::wstring path;
	std::wstring name;
	std::wstring ext;
	if (SUCCEEDED(pParentFolder->GetDisplayNameOf(relativeIDL, SHGDN_FORPARSING, &strret))) {
		path = shell::STRRET2WSTR(strret, relativeIDL);
		name = ::PathFindFileName(path.c_str());
		ext = ::PathFindExtension(path.c_str());
	}
	return std::make_tuple(path, name, ext);
}

FILETIME shell::GetLastWriteTime(const CComPtr<IShellFolder>& pParentFolder, const LPITEMIDLIST& relativeIDL)
{
	WIN32_FIND_DATA wfd = { 0 };
	if (!FAILED(::SHGetDataFromIDList(pParentFolder, relativeIDL, SHGDFIL_FINDDATA, &wfd, sizeof(WIN32_FIND_DATA)))) {
		return  wfd.ftLastWriteTime;
	} else {
		return FILETIME{ 0 };
	}
}

void shell::CheckIncrementalIDL(
	const CComPtr<IShellFolder>& pSrcFolder,
	const CIDL& srcIDL,
	const CIDL& srcChildIDL,
	const CComPtr<IShellFolder>& pDestFolder,
	const CIDL& destIDL,
	const std::function<void(int, const CIDL&, const CIDL&)>& read)
{
	ULONG chEaten = 0;
	ULONG dwAttributes = 0;
	CIDL destChildIDL;
	auto srcTuple = shell::GetPathNameExt(pSrcFolder, srcChildIDL.ptr());

	if (SUCCEEDED(pDestFolder->ParseDisplayName(
		NULL,
		NULL,
		const_cast<LPWSTR>(std::get<1>(srcTuple).c_str()),
		&chEaten,
		destChildIDL.ptrptr(),
		&dwAttributes)) && destChildIDL) {//Exist

		auto fileFun = [&]()->void {
			FILETIME srcTime = shell::GetLastWriteTime(pSrcFolder, srcChildIDL.ptr());
			FILETIME destTime = shell::GetLastWriteTime(pDestFolder, destChildIDL.ptr());
			ULARGE_INTEGER srcUli = { srcTime.dwLowDateTime, srcTime.dwHighDateTime };
			ULARGE_INTEGER destUli = { destTime.dwLowDateTime, destTime.dwHighDateTime };
			if (srcUli.QuadPart > destUli.QuadPart) {
				read(1, destIDL, (srcIDL + srcChildIDL));
			} else {
				read(1, CIDL(), CIDL());
			}
		};

		auto dirFun = [&](const CComPtr<IShellFolder>& pSrcItemFolder, const CComPtr<IEnumIDList>& pSrcItemEnum)->void {
			CComPtr<IShellFolder> pDestItemFolder;
			if (SUCCEEDED(pDestFolder->BindToObject(destChildIDL.ptr(), 0, IID_IShellFolder, (void**)&pDestItemFolder))) {
				shell::GetIncrementalIDLs(
					pSrcItemFolder,
					pSrcItemEnum,
					srcIDL + srcChildIDL,
					pDestItemFolder,
					destIDL + destChildIDL,
					read);
			}
			read(1, CIDL(), CIDL());
		};

		auto virFun = [&]()->void {
			read(1, CIDL(), CIDL());
		};

		shell::RunFunctionEachFileFolderVirtual<void>(pSrcFolder, srcChildIDL, fileFun, dirFun, virFun);
	} else {
		auto fileFun = [&]()->void {
			read(1, destIDL, srcIDL + srcChildIDL);
		};
		auto dirFun = [&](const CComPtr<IShellFolder>& pSrcItemFolder, const CComPtr<IEnumIDList>& pSrcItemEnum)->void {
			shell::GetEnumCount(pSrcItemFolder, pSrcItemEnum, [read](int i)->void{read(i, CIDL(), CIDL()); });
			read(1, destIDL, srcIDL + srcChildIDL);
		};
		auto virFun = [&]()->void {
			read(1, CIDL(), CIDL());
		};

		shell::RunFunctionEachFileFolderVirtual<void>(pSrcFolder, srcChildIDL, fileFun, dirFun, virFun);
	}
}


void shell::GetIncrementalIDLs(
	const CComPtr<IShellFolder>& pSrcFolder,
	const CComPtr<IEnumIDList>& pEnum,
	const CIDL& srcIDL,
	const CComPtr<IShellFolder>& pDestFolder,
	const CIDL& destIDL,
	const std::function<void(int, const CIDL&, const CIDL&)>& read)
{
	CIDL srcChildIDL;
	ULONG ulRet(0);
	while (SUCCEEDED(pEnum->Next(1, srcChildIDL.ptrptr(), &ulRet))) {
		if (!srcChildIDL) { break; }
		shell::CheckIncrementalIDL(pSrcFolder, srcIDL, srcChildIDL, pDestFolder, destIDL, read);
	}
}

void shell::GetIncrementalIDLs(
	const CComPtr<IShellFolder>& pSrcFolder,
	const CIDL& srcIDL,
	const std::vector<CIDL>& srcChildIDLs,
	const CComPtr<IShellFolder>& pDestFolder,
	const CIDL& destIDL,
	const std::function<void(int, const CIDL&, const CIDL&)>& read)
{
	for (const auto& srcChildIDL : srcChildIDLs) {
		if (!srcChildIDL) { break; }
		shell::CheckIncrementalIDL(pSrcFolder, srcIDL, srcChildIDL, pDestFolder, destIDL, read);
	}
}

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


void shell::GetIncrementalIDLs(
	const CIDL& srcIDL,
	const std::vector<CIDL>& srcChildIDLs,
	const CIDL& destIDL,
	const std::function<void(int, const CIDL&, const CIDL&)>& read)
{
	std::vector<std::pair<CIDL, std::vector<CIDL>>> idlPairs;
	std::vector<CIDL> idls;

	CComPtr<IShellFolder> pSrcFolder = shell::DesktopBindToShellFolder(srcIDL);
	CComPtr<IShellFolder> pDestFolder = shell::DesktopBindToShellFolder(destIDL);

	return shell::GetIncrementalIDLs(
		pSrcFolder, 
		srcIDL, 
		srcChildIDLs,
		pDestFolder,
		destIDL, read);
}

int shell::GetEnumCount(
	const CComPtr<IShellFolder>& pFolder,
	const CComPtr<IEnumIDList>& pEnum,
	const std::function<void(int)>& read)
{
	int count = 0;

	auto fileFun = [&]()->void {
		count++;
		read(1);
	};

	auto dirFun = [&](const CComPtr<IShellFolder>& pSrcItemFolder, const CComPtr<IEnumIDList>& pSrcItemEnum)->void {
		count += GetEnumCount(pSrcItemFolder, pSrcItemEnum, read);
		read(1);
	};

	auto virFun = [&]()->void {
		count++;
		read(1);
	};

	CIDL childIDL;
	ULONG ulRet(0);
	while (SUCCEEDED(pEnum->Next(1, childIDL.ptrptr(), &ulRet))) {

		if (childIDL) {
			shell::RunFunctionEachFileFolderVirtual<void>(pFolder, childIDL, fileFun, dirFun, virFun);
		} else {
			break;
		}
	}
	return count;
}

int shell::GetFileCount(
	const CComPtr<IShellFolder>& pSrcFolder,
	const CIDL& srcIDL,
	const CIDL& srcChildIDL,
	const std::function<void(int)>& read)
{
	int count = 0;

	auto fileFun = [&]()->void {
		count++;
		read(1);
	};

	auto dirFun = [&](const CComPtr<IShellFolder>& pSrcItemFolder, const CComPtr<IEnumIDList>& pSrcItemEnum)->void {
		count += GetEnumCount(pSrcItemFolder, pSrcItemEnum, read);
		read(1);
	};

	auto virFun = [&]()->void {
		count++;
		read(1);
	};

	shell::RunFunctionEachFileFolderVirtual<void>(pSrcFolder, srcChildIDL, fileFun, dirFun, virFun);

	return count;
}


int shell::GetFileCount(
	const CIDL& srcIDL,
	const CIDL& srcChildIDL,
	const std::function<void(int)>& read)
{
	CComPtr<IShellFolder> pDesktopFolder;
	if (FAILED(::SHGetDesktopFolder(&pDesktopFolder))) {
		throw std::exception(FILE_LINE_FUNC);
	}

	CComPtr<IShellFolder> pSrcFolder;
	if (FAILED(pDesktopFolder->BindToObject(srcIDL.ptr(), 0, IID_IShellFolder, (void**)&pSrcFolder))) {
		if (CIDL desktopIDL; FAILED(::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, desktopIDL.ptrptr())) || !::ILIsEqual(srcIDL.ptr(), desktopIDL.ptr())) {
			return 0;
		} else {
			pSrcFolder = pDesktopFolder;
		}
	}

	return shell::GetFileCount(pSrcFolder, srcIDL, srcChildIDL, read);
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




