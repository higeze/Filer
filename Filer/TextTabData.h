#pragma once
#include "TabControl.h"
#include "TextDoc.h"
#include "encoding_type.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"

/***************/
/* TextTabData */
/***************/
struct TextTabData :public TabData
{
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<CTextDoc> Doc;
	reactive_property_ptr<CTextCaret> Caret;
	reactive_command_ptr<HWND> OpenCommand;
	reactive_command_ptr<HWND> SaveCommand;
	reactive_command_ptr<HWND> OpenAsCommand;
	reactive_command_ptr<HWND> SaveAsCommand;

	TextTabData(const std::wstring& path = std::wstring())
		:TabData(), Dummy(std::make_shared<int>(0)), Doc(path)
	{
		OpenCommand.subscribe([this](HWND hWnd) { Open(hWnd); }, Dummy);
		SaveCommand.subscribe([this](HWND hWnd) { Save(hWnd); }, Dummy);
		OpenAsCommand.subscribe([this](HWND hWnd) { OpenAs(hWnd); }, Dummy);
		SaveAsCommand.subscribe([this](HWND hWnd) { SaveAs(hWnd); }, Dummy);
	}

	virtual ~TextTabData() = default;

	void Open(HWND hWnd);
	void OpenAs(HWND hWnd);

	void Save(HWND hWnd);
	void SaveAs(HWND hWnd);

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override;

	friend void to_json(json& j, const TextTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Doc"] = o.Doc;
	}
	friend void from_json(const json& j, TextTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		get_to(j, "Doc", o.Doc);
	}
};
