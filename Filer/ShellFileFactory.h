#pragma once
#include "IDL.h"
#include <chrono>
#include <tuple>
#include <functional>
#include "ShellFunction.h"

class CShellFile;


class CShellFileFactory
{
private:

	std::unordered_map<std::tuple<std::wstring, CComPtr<IShellFolder>, CIDL>,
		std::shared_ptr<CShellFile>, shell::ShellFileHash, shell::ShellFileEqual> s_fileCache;
	std::chrono::system_clock::time_point s_cacheTime;

public:
	static CShellFileFactory* GetInstance()
	{
		static CShellFileFactory factory;
		return &factory;
	}
	shell::ParsedFileType CShellFileFactory::ParseFileType(const CComPtr<IShellFolder>& pParentFolder, const CIDL& childIDL);
	std::shared_ptr<CShellFile> CreateShellFilePtr(const CComPtr<IShellFolder>& pShellFolder, const CIDL& parentIdl, const CIDL& childIdl);
	std::shared_ptr<CShellFile> CreateShellFilePtr(const std::wstring& path);
};
