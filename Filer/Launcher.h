#pragma once
#include "notify_property_changed.h"
#include "Favorite.h"

class CLauncher : public CFavorite
{
public:
	reactive_property_ptr<bool> RunAs;

public:
	CLauncher(void) :CFavorite() {}
	CLauncher(std::wstring path, std::wstring shortName, bool runas = false)
		:CFavorite(path, shortName), RunAs(runas){}

	CLauncher(const CLauncher& other)
		:CLauncher(*other.Path, *other.ShortName, *other.RunAs){}
	CLauncher& operator=(const CLauncher& other)
	{
		CFavorite::operator=(other);
		RunAs.set(*other.RunAs);
		return *this;
	}
	bool operator==(const CLauncher& other) const
	{
		return CFavorite::operator==(other) && *RunAs == *other.RunAs;
	}
	bool operator!=(const CLauncher& other) const
	{
		return !(operator==(other));
	}

	virtual ~CLauncher(void) = default;

	CLauncher Clone() const
	{
		CLauncher clone;
		clone.Path.set(*Path);
		clone.ShortName.set(*ShortName);
		clone.RunAs.set(*RunAs);
		return clone;
	}


public:
	friend void to_json(json& j, const CLauncher& o)
	{
		to_json(j, static_cast<const CFavorite&>(o));
		json_safe_to(j, "RunAs", o.RunAs);
	}
	friend void from_json(const json& j, CLauncher& o)
	{
		from_json(j, static_cast<CFavorite&>(o));
		json_safe_from(j, "RunAs", o.RunAs);
	}
};

template<>
struct adl_vector_item<CLauncher>
{
	static CLauncher clone(const CLauncher& item)
	{
		return item.Clone();
	}

	static void bind(CLauncher& src, CLauncher& dst)
	{
		src.Path.binding(dst.Path);
		src.ShortName.binding(dst.ShortName);
		src.RunAs.binding(dst.RunAs);
	}
};

