#pragma once
#include "notify_property_changed.h"
#include "MyFriendSerializer.h"
#include <future>
#include <mutex>
#include "reactive_property.h"
#include "reactive_vector.h"
#include "reactive_string.h"
#include "JsonSerializer.h"

class CShellFile;

class CFavorite/*: public notify_property_changed*/
{
public:
	reactive_wstring_ptr Path;
	reactive_wstring_ptr ShortName;

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
		:CFavorite(*other.Path, *other.ShortName){}
	CFavorite& operator=(const CFavorite& other)
	{
		Path.set(*other.Path);
		ShortName.set(*other.ShortName);
		return *this;
	}
	bool operator==(const CFavorite& other) const
	{
		return *Path == *other.Path && *ShortName == *ShortName;
	}
	bool operator!=(const CFavorite& other) const
	{
		return !(operator==(other));
	}

	CFavorite Clone() const
	{
		CFavorite clone;
		clone.Path.set(*Path);
		clone.ShortName.set(*ShortName);
		return clone;
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
		json_safe_to(j, "Path", o.Path);
		json_safe_to(j, "ShortName", o.ShortName);
	}
	friend void from_json(const json& j, CFavorite& o)
	{
		json_safe_from(j, "Path", o.Path);
		json_safe_from(j, "ShortName", o.ShortName);
	}

};

template<>
struct adl_vector_item<CFavorite>
{
	static CFavorite clone(const CFavorite& item)
	{
		return item.Clone();
	}

	static void bind(CFavorite& src, CFavorite& dst)
	{
		src.Path.binding(dst.Path);
		src.ShortName.binding(dst.ShortName);
	}
};

