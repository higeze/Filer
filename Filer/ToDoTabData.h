#pragma once
#include "TabControl.h"
#include "ToDoDoc.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"

/***************/
/* ToDoTabData */
/***************/
struct ToDoTabData:public TabData
{
private:
	std::shared_ptr<int> Dummy;
public:
	reactive_property_ptr<CToDoDoc> Doc;
	reactive_command_ptr<std::wstring> OpenCommand;
	reactive_command_ptr<std::wstring> SaveCommand;

	ToDoTabData(const std::wstring& path = std::wstring())
		:TabData(),
		Dummy(std::make_shared<int>(0))
	{
		OpenCommand.subscribe([this](const std::wstring& path) { Doc.get_unconst()->Open(path); }, Dummy);
		SaveCommand.subscribe([this](const std::wstring& path) { Doc.get_unconst()->Save(path); }, Dummy);
	};

	virtual ~ToDoTabData() = default;

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override;

	friend void to_json(json& j, const ToDoTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.Doc->Path;
	}
	friend void from_json(const json& j, ToDoTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Path").get_to(o.Doc.get_unconst()->Path);
		o.Doc.get_unconst()->Open(*o.Doc->Path);
	}
};
