#pragma once
#include "Favorite.h"

class CLauncher : public CFavorite
{
public:
	bool RunAs = false;
public:
	CLauncher(void) :CFavorite() {}
	CLauncher(std::wstring path, std::wstring shortName, bool runas = false)
		:CFavorite(path, shortName), RunAs(runas){}

	virtual ~CLauncher(void) = default;

	friend void to_json(json& j, const CLauncher& o)
	{
		to_json(j, static_cast<const CFavorite&>(o));
		j["RunAs"] = o.RunAs;
	}
	friend void from_json(const json& j, CLauncher& o)
	{
		from_json(j, static_cast<CFavorite&>(o));
		get_to(j, "RunAs", o.RunAs);
	}
};

