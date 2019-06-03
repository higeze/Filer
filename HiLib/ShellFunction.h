#pragma once
#include "IDL.h"
#include <tuple>
#include <ShlObj.h>
#include <boost/algorithm/string.hpp>
#include "MyString.h"
#include <optional>


struct FileTimes
{
	FileTimes(){}
	FileTimes(const FILETIME& creationTime, const FILETIME& lastAccessTime, const FILETIME& lastWriteTime)
		:CreationTime(creationTime), LastAccessTime(lastAccessTime), LastWriteTime(lastWriteTime){}
	FILETIME CreationTime = { 0 };
	FILETIME LastAccessTime = { 0 };
	FILETIME LastWriteTime = { 0 };
};

namespace shell
{

	struct IdlHash
	{
		inline std::size_t operator()(const CIDL& key)const
		{
			std::wstring path;
			::SHGetPathFromIDListW(key.ptr(), ::GetBuffer(path, MAX_PATH));
			::ReleaseBuffer(path);
			return std::hash<std::wstring>()(path);
		}
	};

	struct IdlEqual
	{
		inline std::size_t operator()(const CIDL& left, const CIDL& right)const
		{
			return ::ILIsEqual(left.ptr(), right.ptr());
		}
	};


	struct ShellFileHash
	{
		inline std::size_t operator()(const std::tuple<std::wstring, CComPtr<IShellFolder>, CIDL>& key)const
		{
			return std::hash<std::wstring>()(std::get<0>(key));
		}
	};

	struct ShellFileEqual
	{
		inline std::size_t operator()(const std::tuple<std::wstring, CComPtr<IShellFolder>, CIDL>& left, const std::tuple<std::wstring, CComPtr<IShellFolder>, CIDL>& right)const
		{
			return HRESULT_CODE(std::get<1>(left)->CompareIDs(SHCIDS_ALLFIELDS, std::get<2>(left).ptr(), std::get<2>(right).ptr())) == 0;
		}
	};

	std::wstring FileTime2String(FILETIME *pFileTime);
	std::wstring Size2String(ULONGLONG size);
	std::wstring ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit = 3);

	bool GetFileSize(ULARGE_INTEGER& size, const CComPtr<IShellFolder>& pParentShellFolder, const CIDL& childIdl);




	std::wstring STRRET2WSTR(STRRET& strret, LPITEMIDLIST pidl);
	std::tuple<std::wstring, std::wstring, std::wstring> GetPathNameExt(const CComPtr<IShellFolder>& pParentFolder, const LPITEMIDLIST& relativeIDL);
	std::optional<FileTimes> GetFileTimes(const CComPtr<IShellFolder>& pParentFolder, const CIDL& relativeIDL);

	template<typename T>
	auto RunFunctionEachFileFolderVirtual(
		const CComPtr<IShellFolder>& pFolder,
		const CIDL& childIDL,
		const std::function<T()>& f,
		const std::function<T(const CComPtr<IShellFolder>&, const CComPtr<IEnumIDList>&)>& d,
		const std::function<T()>& v)->T
	{
		CComPtr<IShellFolder> pItemFolder;
		CComPtr<IEnumIDList> pItemEnumIDL;

		auto pne = GetPathNameExt(pFolder, childIDL.ptr());
		if (std::get<0>(pne)[0] == L':'){
			//Virtual
			return v();
		} else if (boost::iequals(std::get<2>(pne), ".zip")) {
			//File
			return f();
		} else if (
			SUCCEEDED(pFolder->BindToObject(childIDL.ptr(), 0, IID_IShellFolder, (void**)&pItemFolder)) &&
			SUCCEEDED(pItemFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pItemEnumIDL))) {
			//Folder
			return d(pItemFolder, pItemEnumIDL);
		} else {
			//File
			return f();
		}
	}

	CComPtr<IShellFolder> DesktopBindToShellFolder(const CIDL& idl);

	void CheckIncrementalIDL(
		const CComPtr<IShellFolder>& pSrcFolder,
		const CIDL& srcIDL,
		const CIDL& srcChildIDL,
		const CComPtr<IShellFolder>& pDestFolder,
		const CIDL& destIDL,
		const std::function<void(int, const CIDL&, const CIDL&)>& read);

	void GetIncrementalIDLs(
		const CIDL& srcIDL,
		const std::vector<CIDL>& srcChildIDLs,
		const CIDL& destIDL,
		const std::function<void(int, const CIDL&, const CIDL&)>& read);

	void GetIncrementalIDLs(
		const CComPtr<IShellFolder>& pSrcFolder,
		const CIDL& srcIDL,
		const std::vector<CIDL>& srcChildIDLs,
		const CComPtr<IShellFolder>& pDestFolder,
		const CIDL& destIDL,
		const std::function<void(int, const CIDL&, const CIDL&)>& read);

	void GetIncrementalIDLs(
		const CComPtr<IShellFolder>& pSrcFolder,
		const CComPtr<IEnumIDList>& pEnum,
		const CIDL& srcIDL,
		const CComPtr<IShellFolder>& pDestFolder,
		const CIDL& destIDL,
		const std::function<void(int, const CIDL&, const CIDL&)>& read);

	int GetFileCount(
		const CIDL& srcIDL,
		const CIDL& srcChildIDL,
		const std::function<void(int)>& read);

	int GetFileCount(
		const CComPtr<IShellFolder>& pSrcFolder,
		const CIDL& srcIDL,
		const CIDL& srcChildIDL,
		const std::function<void(int)>& read);

	int GetEnumCount(
		const CComPtr<IShellFolder>& pFolder,
		const CComPtr<IEnumIDList>& pEnum,
		const std::function<void(int)>& read);

	//void SearchFolder(
	//	const CIDL& srcIDL,
	//	const std::function<void()>& searchCount,
	//	const std::function<void(const CIDL&)> find
	//);

//	CComPtr<IShellFolder> GetParentShellFolderByIDL(const CIDL& absIDL);

	bool CopyFiles(const CIDL& destIDL, const std::vector<LPITEMIDLIST>& srcIDLs);
	bool MoveFiles(const CIDL& destIDL, const std::vector<LPITEMIDLIST>& srcIDLs);




};