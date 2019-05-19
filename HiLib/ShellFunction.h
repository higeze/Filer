#pragma once

namespace shell
{
	std::wstring STRRET2WSTR(STRRET& strret, LPITEMIDLIST pidl);
	std::tuple<std::wstring, std::wstring, std::wstring> GetPathNameExt(const CComPtr<IShellFolder>& pParentFolder, const LPITEMIDLIST& relativeIDL);
	FILETIME GetLastWriteTime(const CComPtr<IShellFolder>& pParentFolder, const LPITEMIDLIST& relativeIDL);
};