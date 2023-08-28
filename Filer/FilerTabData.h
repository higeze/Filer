#pragma once
#include "TabControl.h"
#include "ShellFolder.h"
#include "ShellFileFactory.h"
#include "KnownFolder.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"

/***************/
/* FilerTabData */
/***************/
struct FilerTabData:public TabData
{
	std::wstring Path;
	std::shared_ptr<CShellFolder> FolderPtr;

	FilerTabData()
		:TabData(){ }

	FilerTabData(const std::wstring& path);
	FilerTabData(const std::shared_ptr<CShellFolder>& spFolder)
		:TabData(), FolderPtr(spFolder), Path(spFolder->GetPath()){}

	virtual ~FilerTabData() = default;

	FilerTabData(const FilerTabData& data)
	{
		Path = data.Path;
		FolderPtr = data.FolderPtr->Clone();
	}

	friend void to_json(json& j, const FilerTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.FolderPtr->GetPath();
	}
	friend void from_json(const json& j, FilerTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Path").get_to(o.Path);
		if (!o.Path.empty()) {
			auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(o.Path);
			if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
				o.FolderPtr = sp;
			} else {
				o.FolderPtr = CKnownFolderManager::GetInstance()->GetDesktopFolder();
				o.Path = o.FolderPtr->GetPath();
			}
		}
	}
};


