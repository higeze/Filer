#pragma once
#include "MyFriendSerializer.h"
#include <future>
#include <mutex>

class CShellFile;

class CFavorite
{
private:
	std::wstring m_path;
	std::wstring m_shortName;
	std::shared_ptr<CShellFile> m_spFile;

	std::shared_ptr<bool> m_spCancel;
	std::future<std::shared_ptr<CShellFile>> m_futureFile;
	std::mutex m_mtxFile;

public:
	CFavorite(void);
	CFavorite(std::wstring path, std::wstring shortName);
	virtual ~CFavorite(void);

	std::wstring GetPath()const{return m_path;}
	std::wstring GetShortName()const{return m_shortName;}
	std::shared_ptr<CShellFile>& GetShellFile(const std::function<void()>& changed);

	std::shared_ptr<CShellFile>& GeLockShellFile()
	{
		std::lock_guard<std::mutex> lock(m_mtxFile);
		return m_spFile;
	}
	void SetLockShellFile(const std::shared_ptr<CShellFile> spFile)
	{ 
		std::lock_guard<std::mutex> lock(m_mtxFile);
		m_spFile = spFile;
	}

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Path", m_path);
		ar("ShortName",m_shortName);
    }
};

