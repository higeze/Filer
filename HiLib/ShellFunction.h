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
	std::wstring GetDisplayNameOf(const CComPtr<IShellFolder>& pParentFolder, const CIDL& childIDL, SHGDNF uFlags);
	std::wstring GetDisplayNameOf(const CComPtr<IShellFolder>& pParentFolder, const LPITEMIDLIST& childIDL, SHGDNF uFlags);


	std::wstring strret2wstring(STRRET& strret, PCUITEMID_CHILD pidl);
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

	void FindIncrementalOne(
		const CIDL& srcParentIDL,
		const CIDL& srcChildIDL,
		const CIDL& destParentIDL,
		const std::function<void()> countup,
		const std::function<void(const CIDL&, const CIDL&)>& find);

	void FindIncrementalOne(
		const CComPtr<IShellFolder>& pSrcFolder,
		const CIDL& srcIDL,
		const CIDL& srcChildIDL,
		const CComPtr<IShellFolder>& pDestFolder,
		const CIDL& destIDL,
		const std::function<void()> countup,
		const std::function<void(const CIDL&, const CIDL&)>& find);


	void CountFileOne(
		const CIDL& parentIDL,
		const CIDL& childIDL,
		const std::function<void()>& countup);
	
	void CountFileOne(
		const CComPtr<IShellFolder>& pParentFolder,
		const CIDL& parentIDL,
		const CIDL& childIDL,
		const std::function<void()>& countup);

	void CountFileInFolder(
		const CComPtr<IShellFolder>& pFolder,
		const CComPtr<IEnumIDList>& pEnumIDL,
		const CIDL& idl,
		const std::function<void()>& countup);

	void CountFileInFolder(
		const CIDL& srcIDL,
		const std::function<void()>& countup);

	void SearchFileInFolder(
		const std::wstring& search,
		const CIDL& srcIDL,
		const std::function<void()>& countup,
		const std::function<void(const CIDL&)> find);

	void SearchOne(
		const std::wstring& search,
		const CComPtr<IShellFolder>& pParentFolder,
		const CIDL& parentIDL,
		const CIDL& childIDL,
		const std::function<void()>& countup,
		const std::function<void(const CIDL&)> find);


//	CComPtr<IShellFolder> GetParentShellFolderByIDL(const CIDL& absIDL);

	bool CopyFiles(const CIDL& destIDL, const std::vector<LPITEMIDLIST>& srcIDLs);
	bool MoveFiles(const CIDL& destIDL, const std::vector<LPITEMIDLIST>& srcIDLs);
	bool DeleteFiles(const std::vector<LPITEMIDLIST>& srcIDLs);

	enum class FileType
	{
		None,
		File,
		Zip,
		Folder,
		Virtual,
		Known,
		Drive
	};


	struct ParsedFileType
	{
		//Constructor/Destructor
		ParsedFileType() = default;
		~ParsedFileType() = default;
		//No Copy
		ParsedFileType(const ParsedFileType&) = delete;
		ParsedFileType& operator=(const ParsedFileType&) = delete;
		//Move
		ParsedFileType(ParsedFileType&&) = default;
		ParsedFileType& operator=(ParsedFileType&&) = default;
		//Field
		FileType FileType;
		std::wstring FilePath;
		std::wstring FileName;
		std::wstring FileExt;
		//CComPtr<IShellFolder> ShellFolderPtr;
		//CComPtr<IEnumIDList> EnumIDLPtr;
	};

	ParsedFileType ParseFileTypeSimple(
		const CComPtr<IShellFolder>& pParentFolder,
		const CIDL& childIDL);

	ParsedFileType ParseFileType(
		const CComPtr<IShellFolder>& pParentFolder,
		const CIDL& childIDL);
	
	template<class Fn>
	void for_each_idl_in_shellfolder(HWND hWnd, const CComPtr<IShellFolder>& pFolder, Fn func)
	{
		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(pFolder->EnumObjects(hWnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN, &enumIdl)) && enumIdl) {
			CIDL nextIdl;
			ULONG ulRet(0);
			while (true) {
				SUCCEEDED(enumIdl->Next(1, nextIdl.ptrptr(), &ulRet));
				if (!nextIdl) { break; }
				func(nextIdl);
				nextIdl.Clear();
			}
		}

	}


};