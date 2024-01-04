#pragma once
#include "Launcher.h"
#include "KnownFolder.h"
#include "reactive_property.h"

class CLauncherProperty
{
public:
	reactive_vector_ptr<std::tuple<std::shared_ptr<CLauncher>>> Launchers;

public:
	CLauncherProperty()
		:Launchers()
	{
		Launchers.push_back(std::make_tuple(std::make_shared<CLauncher>(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT")));
	};
	~CLauncherProperty(){};

	friend void to_json(json& j, const CLauncherProperty& o)
	{
		j = json{
			{"Launchers", o.Launchers}
		};
	}
	friend void from_json(const json& j, CLauncherProperty& o)
	{
		get_to(j, "Launchers", o.Launchers);
	}
};