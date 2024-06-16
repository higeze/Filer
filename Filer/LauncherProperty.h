#pragma once
#include "Launcher.h"
#include "KnownFolder.h"
#include "reactive_property.h"

class CLauncherProperty
{
public:
	reactive_vector_ptr<CLauncher> Launchers;

public:
	CLauncherProperty()
		:Launchers()
	{
		Launchers.push_back(CLauncher(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT"));
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
		json_safe_from(j, "Launchers", o.Launchers);
	}
};