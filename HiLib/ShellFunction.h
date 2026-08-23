#pragma once
#include "IDL.h"
#include <tuple>
#include <ShlObj.h>
#include <boost/algorithm/string.hpp>
#include "MyString.h"
#include <optional>

#include "ThreadSafeComPtr.h"
#include "Debug.h"


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
		inline std::size_t operator()(const std::tuple<std::wstring, CThreadSafeComPtr<IShellFolder>, CIDL>& key)const
		{
			return std::hash<std::wstring>()(std::get<0>(key));
		}
	};

	struct ShellFileEqual
	{
		inline std::size_t operator()(const std::tuple<std::wstring, CThreadSafeComPtr<IShellFolder>, CIDL>& left, const std::tuple<std::wstring, CThreadSafeComPtr<IShellFolder>, CIDL>& right)const
		{
			CThreadSafeComPtr<IShellFolder> pFolder = std::get<1>(left);
			CIDL childIDL1 = std::get<2>(left);
			CIDL childIDL2 = std::get<2>(right);
			return pFolder.Call(&IShellFolder::CompareIDs, SHCIDS_ALLFIELDS, childIDL1.ptr(), childIDL2.ptr()) == 0;
		}
	};

	std::wstring FileTime2String(FILETIME *pFileTime);
	std::wstring Size2String(ULONGLONG size);
	std::wstring ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit = 3);

	bool GetFileSize(ULARGE_INTEGER& size, CThreadSafeComPtr<IShellFolder> pParentShellFolder, const CIDL& childIdl);
	std::wstring GetDisplayNameOf(CThreadSafeComPtr<IShellFolder> pParentFolder, const CIDL& childIDL, SHGDNF uFlags);
	std::wstring GetDisplayNameOf(CThreadSafeComPtr<IShellFolder> pParentFolder, const LPITEMIDLIST& childIDL, SHGDNF uFlags);


	std::wstring strret2wstring(STRRET& strret, PCUITEMID_CHILD pidl);
	std::tuple<std::wstring, std::wstring, std::wstring> GetPathNameExt(CThreadSafeComPtr<IShellFolder> pParentFolder, const LPITEMIDLIST& relativeIDL);
	std::optional<FileTimes> GetFileTimes(CThreadSafeComPtr<IShellFolder> pParentFolder, const CIDL& relativeIDL);

	template<typename TRect>
	auto RunFunctionEachFileFolderVirtual(
		CThreadSafeComPtr<IShellFolder> pFolder,
		const CIDL& childIDL,
		const std::function<TRect()>& f,
		const std::function<TRect(CThreadSafeComPtr<IShellFolder>, const CComPtr<IEnumIDList>&)>& d,
		const std::function<TRect()>& v)->TRect
	{
		CThreadSafeComPtr<IShellFolder> pItemFolder;
		CComPtr<IEnumIDList> pItemEnumIDL;

		auto pne = GetPathNameExt(pFolder, childIDL.ptr());
		if (std::get<0>(pne)[0] == L':'){
			//Virtual
			return v();
		} else if (boost::iequals(std::get<2>(pne), ".zip")) {
			//File
			return f();
		} else if (
			SUCCEEDED(pFolder.Call(&IShellFolder::BindToObject, childIDL.ptr(), reinterpret_cast<IBindCtx*>(nullptr), IID_IShellFolder, reinterpret_cast<void**>(&pItemFolder))) &&
			SUCCEEDED(pItemFolder.Call(&IShellFolder::EnumObjects, reinterpret_cast<HWND>(NULL), SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &pItemEnumIDL))) {
			//Folder
			return d(pItemFolder, pItemEnumIDL);
		} else {
			//File
			return f();
		}
	}

	CThreadSafeComPtr<IShellFolder> DesktopBindToShellFolder(const CIDL& idl);

	void FindIncrementalOne(
		const CIDL& srcParentIDL,
		const CIDL& srcChildIDL,
		const CIDL& destParentIDL,
		const std::function<void()> countup,
		const std::function<void(const CIDL&, const CIDL&)>& find);

	void FindIncrementalOne(
		const CThreadSafeComPtr<IShellFolder>& pSrcFolder,
		const CIDL& srcIDL,
		const CIDL& srcChildIDL,
		const CThreadSafeComPtr<IShellFolder>& pDestFolder,
		const CIDL& destIDL,
		const std::function<void()> countup,
		const std::function<void(const CIDL&, const CIDL&)>& find);


	void CountFileOne(
		const CIDL& parentIDL,
		const CIDL& childIDL,
		const std::function<void()>& countup);
	
	void CountFileOne(
		const CThreadSafeComPtr<IShellFolder>& pParentFolder,
		const CIDL& parentIDL,
		const CIDL& childIDL,
		const std::function<void()>& countup);

	void CountFileInFolder(
		const CThreadSafeComPtr<IShellFolder>& pFolder,
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
		const CThreadSafeComPtr<IShellFolder>& pParentFolder,
		const CIDL& parentIDL,
		const CIDL& childIDL,
		const std::function<void()>& countup,
		const std::function<void(const CIDL&)> find);


//	CThreadSafeComPtr<IShellFolder> GetParentShellFolderByIDL(const CIDL& absIDL);

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
		Drive,
		Desktop
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
		CThreadSafeComPtr<IShellFolder> pParentFolder,
		const CIDL& childIDL);

	ParsedFileType ParseFileType(
		CThreadSafeComPtr<IShellFolder> pParentFolder,
		const CIDL& childIDL);
	
	template<class Fn>
	void for_each_idl_in_shellfolder(HWND hWnd, CThreadSafeComPtr<IShellFolder> pFolder, Fn func)
	{
		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(pFolder.Call(&IShellFolder::EnumObjects, hWnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN, &enumIdl)) && enumIdl) {
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