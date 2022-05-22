#pragma once
#include "Launcher.h"
#include "KnownFolder.h"
#include "ReactiveProperty.h"

class CLauncherProperty
{
private:
	std::shared_ptr<ReactiveVectorProperty<std::tuple<std::shared_ptr<CLauncher>>>> m_spLauncher;

public:
	CLauncherProperty():m_spLauncher(std::make_shared<ReactiveVectorProperty<std::tuple<std::shared_ptr<CLauncher>>>>())
	{
		m_spLauncher->push_back(std::make_tuple(std::make_shared<CLauncher>(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT")));
	};
	~CLauncherProperty(){};

	std::shared_ptr<ReactiveVectorProperty<std::tuple<std::shared_ptr<CLauncher>>>>& GetLaunchersPtr() { return m_spLauncher; }
	ReactiveVectorProperty<std::tuple<std::shared_ptr<CLauncher>>>& GetLaunchers(){return *m_spLauncher;}


	friend void to_json(json& j, const CLauncherProperty& o)
	{
		j = json{
			{"Launchers", o.m_spLauncher}
		};
	}
	friend void from_json(const json& j, CLauncherProperty& o)
	{
		get_to_nothrow(j, "Launchers", o.m_spLauncher);
	}
};