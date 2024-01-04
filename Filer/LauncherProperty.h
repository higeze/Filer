#pragma once
#include "Launcher.h"
#include "KnownFolder.h"
#include "reactive_property.h"

class CLauncherProperty
{
public:
	reactive_vector_ptr<any_tuple> Launchers;

public:
	CLauncherProperty()
		:Launchers()
	{
		Launchers.push_back(std::make_shared<CLauncher>(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT"));
	};
	~CLauncherProperty(){};

	friend void to_json(json& j, const CLauncherProperty& o)
	{
		std::vector<std::shared_ptr<CLauncher>> values;
		std::transform(o.Launchers->cbegin(), o.Launchers->cend(), std::back_inserter(values),
			[](const any_tuple& value) { return value.get<std::shared_ptr<CLauncher>>(); });

		j = json{
			{"Launchers", values}
		};
	}
	friend void from_json(const json& j, CLauncherProperty& o)
	{
		//std::vector<std::shared_ptr<CLauncher>> values;
		//get_to(j, "Launchers", values);

		//o.Launchers.clear();
		//std::transform(values.begin(), values.end(), std::back_inserter(*o.Launchers.get_unconst()),
		//	[](std::shared_ptr<CLauncher>& value) { return any_tuple(value); });
	}
};