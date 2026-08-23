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

	std::unordered_map<std::tuple<std::wstring, CThreadSafeComPtr<IShellFolder>, CIDL>,
		std::shared_ptr<CShellFile>, shell::ShellFileHash, shell::ShellFileEqual> s_fileCache;
	std::chrono::system_clock::time_point s_cacheTime;

public:
	static CShellFileFactory* GetInstance()
	{
		static CShellFileFactory factory;
		return &factory;
	}
	shell::ParsedFileType ParseFileType(CThreadSafeComPtr<IShellFolder> pParentFolder, const CIDL& parentIDL, const CIDL& childIDL);
	std::shared_ptr<CShellFile> CreateShellFilePtr(CThreadSafeComPtr<IShellFolder> pShellFolder, const CIDL& parentIdl, CIDL&& childIdl);
	std::shared_ptr<CShellFile> CreateShellFilePtr(const CIDL& parentIdl, CIDL&& childIdl);
	std::shared_ptr<CShellFile> CreateShellFilePtr(const std::wstring& path);
};
