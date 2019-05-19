#include "ShellFunction.h"

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
