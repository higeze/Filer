#pragma once
#include "ShellFile.h"
#include <boost/timer.hpp>
#include <chrono>

struct ShellFileHash
{
	inline std::size_t operator()(const std::pair<std::wstring, CIDL>& key)const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(key.first)>()(key.first));
		return seed;
	}
};

struct ShellFileEqual
{
	inline std::size_t operator()(const std::pair<std::wstring, CIDL>& left, const std::pair<std::wstring, CIDL>& right)const
	{
		return left.first == right.first && ::ILIsEqual(left.second.ptr(), right.second.ptr());
	}
};



class CShellFolder :public CShellFile
{
private:
	CComPtr<IShellFolder> m_pShellFolder;

	std::shared_ptr<bool> m_spCancelThread = std::make_shared<bool>(false);

	std::future<std::pair<ULARGE_INTEGER, FileSizeStatus>> m_futureSize;
	std::future<std::pair<FILETIME, FileTimeStatus>> m_futureTime;

	std::mutex m_mtxSize;
	std::mutex m_mtxTime;

	//static std::unordered_map<std::pair<std::wstring, CIDL>, std::shared_ptr<CShellFile>, ShellFileHash, ShellFileEqual> s_fileCache;
	//static std::chrono::system_clock::time_point s_cacheTime;

public:
//	CShellFolder();
	CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder = nullptr);
	virtual ~CShellFolder();


//	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction) override;

	virtual std::wstring GetFileNameWithoutExt()override;
	virtual std::wstring GetFileName()override;
	virtual std::wstring GetExt()override;

	virtual void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt) override;
	virtual void SetExt(const std::wstring& wstrExt) override;


	CComPtr<IShellFolder> GetShellFolderPtr();
	std::shared_ptr<CShellFolder> GetParent();
	std::shared_ptr<CShellFolder> Clone()const;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed = nullptr)override;
	virtual std::pair<FILETIME, FileTimeStatus> GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed = nullptr)override;
	std::shared_ptr<CShellFile> CreateShExFileFolder(CIDL& relativeIdl);
	static std::shared_ptr<CShellFile> CreateShExFileFolder(const CComPtr<IShellFolder>& pShellFolder, const CIDL& parentIdl, const CIDL& childIdl);
	static std::shared_ptr<CShellFile> CreateShExFileFolder(const std::wstring& path);
	static bool GetFolderLastWriteTime(FILETIME& time, const std::shared_ptr<bool>& cancel,
		const CComPtr<IShellFolder>& pParentFolder, const CComPtr<IShellFolder>& pFolder, const CIDL& relativeIdl, const std::wstring& path,
		boost::timer& tim, int limit, bool ignoreFolderTime);
	static bool GetFolderSize(ULARGE_INTEGER& size, const std::shared_ptr<bool>& cancel,
		const CComPtr<IShellFolder>& pFolder, const std::wstring& path,
		const boost::timer& tim, const int limit);
private:
	std::pair<FILETIME, FileTimeStatus> GetLockLastWriteTime();
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetLockSize();

protected:
	void SetLockSize(std::pair<ULARGE_INTEGER, FileSizeStatus>& size);
	void SetLockLastWriteTime(std::pair<FILETIME, FileTimeStatus>& time);

};
