#pragma once
#include "notify_property_changed.h"
#include "Favorite.h"

class CLauncher : public CFavorite
{
NOTIFIABLE_PROPERTY(bool, RunAs)

public:
	CLauncher(void) :CFavorite() {}
	CLauncher(std::wstring path, std::wstring shortName, bool runas = false)
		:CFavorite(path, shortName), m_RunAs(runas){}

	CLauncher(const CLauncher& other)
		:CLauncher(other.GetPath(), other.GetShortName(), other.GetRunAs()){}
	CLauncher& operator=(const CLauncher& other)
	{
		CFavorite::operator=(other);
		m_RunAs = other.m_RunAs;
		return *this;
	}
	bool operator==(const CLauncher& other) const
	{
		return CFavorite::operator==(other) && GetRunAs() != other.GetRunAs();
	}
	bool operator!=(const CLauncher& other) const
	{
		return !(operator==(other));
	}

	virtual ~CLauncher(void) = default;

public:
	friend void to_json(json& j, const CLauncher& o)
	{
		to_json(j, static_cast<const CFavorite&>(o));
		json_safe_to(j, "RunAs", o.m_RunAs);
	}
	friend void from_json(const json& j, CLauncher& o)
	{
		from_json(j, static_cast<CFavorite&>(o));
		json_safe_from(j, "RunAs", o.m_RunAs);
	}
};

