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
	//std::wstring Path;
	reactive_property_ptr<std::shared_ptr<CShellFolder>> Folder;

	FilerTabData()
		:TabData(){ }

	FilerTabData(const std::wstring& path);
	FilerTabData(const std::shared_ptr<CShellFolder>& spFolder)
		:TabData(), Folder(spFolder){}
	FilerTabData(const FilerTabData& other)
		:FilerTabData(other.Folder->GetPath()){}

	virtual ~FilerTabData() = default;

	virtual std::shared_ptr<TabData> ClonePtr() const override { return std::make_shared<FilerTabData>(*this); }

	//FilerTabData(const FilerTabData& data)
	//{
	//	//Path = data.Path;
	//	Folder.set(data.Folder->Clone());
	//}
	template<class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::base_class<TabData>(this));
		archive(cereal::make_nvp("Path", Folder->GetPath()));
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(cereal::base_class<TabData>(this));
		std::wstring path;
		archive(cereal::make_nvp("Path", path));
		if (!path.empty()) {
			auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
			if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
				Folder.set(sp);
			} else {
				Folder.set(CKnownFolderManager::GetInstance()->GetDesktopFolder());
			}
		}
	}

	friend void to_json(json& j, const FilerTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.Folder->GetPath();
	}
	friend void from_json(const json& j, FilerTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		std::wstring path;

		j.at("Path").get_to(path);

		if (!path.empty()) {
			auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
			if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
				o.Folder.set(sp);
			} else {
				o.Folder.set(CKnownFolderManager::GetInstance()->GetDesktopFolder());
			}
		}
	}
};


