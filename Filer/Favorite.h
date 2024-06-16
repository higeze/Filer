#pragma once
#include "notify_property_changed.h"
#include "MyFriendSerializer.h"
#include <future>
#include <mutex>
#include "JsonSerializer.h"

class CShellFile;

class CFavorite: public notify_property_changed
{
	NOTIFIABLE_PROPERTY(std::wstring, Path)
	NOTIFIABLE_PROPERTY(std::wstring, ShortName)

private:
	std::shared_ptr<CShellFile> m_spFile;

	std::shared_ptr<bool> m_spCancel;
	std::future<std::shared_ptr<CShellFile>> m_futureFile;
	std::mutex m_mtxFile;

public:
	CFavorite(void);
	CFavorite(std::wstring path, std::wstring shortName);
	virtual ~CFavorite(void);
	CFavorite(const CFavorite& other)
		:CFavorite(other.GetPath(), other.GetShortName()){}
	CFavorite& operator=(const CFavorite& other)
	{
		m_Path = other.m_Path;
		m_ShortName = other.m_ShortName;
		return *this;
	}
	bool operator==(const CFavorite& other) const
	{
		return GetPath() == other.GetPath() && GetShortName() == other.GetShortName();
	}
	bool operator!=(const CFavorite& other) const
	{
		return !(operator==(other));
	}

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
public:
	friend void to_json(json& j, const CFavorite& o)
	{
		json_safe_to(j, "Path", o.m_Path);
		json_safe_to(j, "ShortName", o.m_ShortName);
	}
	friend void from_json(const json& j, CFavorite& o)
	{
		json_safe_from(j, "Path", o.m_Path);
		json_safe_from(j, "ShortName", o.m_ShortName);
	}

};

