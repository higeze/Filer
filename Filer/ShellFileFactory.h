#pragma once
#include "IDL.h"
#include <chrono>
#include <tuple>
#include <functional>


class CShellFile;

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


class CShellFileFactory
{
private:

	std::unordered_map<std::tuple<std::wstring, CComPtr<IShellFolder>, CIDL>, std::shared_ptr<CShellFile>, ShellFileHash, ShellFileEqual> s_fileCache;
	std::chrono::system_clock::time_point s_cacheTime;

public:
	static CShellFileFactory* GetInstance()
	{
		static CShellFileFactory factory;
		return &factory;
	}

	std::shared_ptr<CShellFile> CreateShellFilePtr(const CComPtr<IShellFolder>& pShellFolder, const CIDL& parentIdl, const CIDL& childIdl);
	std::shared_ptr<CShellFile> CreateShellFilePtr(const std::wstring& path);
};
