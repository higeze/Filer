#pragma once
#include "TabControl.h"
#include "TextCaret.h"
#include "FileStatus.h"
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

	reactive_wstring_ptr Path;	
	reactive_wstring_ptr Text;
	reactive_property_ptr<encoding_type> Encoding;
	reactive_property_ptr<CTextCaret> Caret;
	reactive_property_ptr<FileStatus> Status;

	reactive_command_ptr<HWND> OpenCommand;
	reactive_command_ptr<HWND> SaveCommand;
	reactive_command_ptr<HWND> OpenAsCommand;
	reactive_command_ptr<HWND> SaveAsCommand;

	TextTabData(const std::wstring& path = std::wstring())
		:TabData(),
		Path(path),
		Text(L""),
		Encoding(encoding_type::UNKNOWN),
		Status(FileStatus::None),
		Caret(0, 0, 0, 0, 0),
		Dummy(std::make_shared<int>(0))
	{
		OpenCommand.subscribe([this](HWND hWnd) { Open(hWnd); }, Dummy);
		SaveCommand.subscribe([this](HWND hWnd) { Save(hWnd); }, Dummy);
		OpenAsCommand.subscribe([this](HWND hWnd) { OpenAs(hWnd); }, Dummy);
		SaveAsCommand.subscribe([this](HWND hWnd) { SaveAs(hWnd); }, Dummy);

		//CloseCommand.Subscribe([this]() { Close(); });
		Text.subscribe([this](auto)
		{
			Status.set(FileStatus::Dirty);
		}, Dummy);
	}

	virtual ~TextTabData() = default;

	void Open(HWND hWnd);
	void OpenAs(HWND hWnd);
	void Open(const std::wstring& path, const encoding_type& enc);

	void Save(HWND hWnd);
	void SaveAs(HWND hWnd);
	void Save(const std::wstring& path, const encoding_type& enc);

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override;

	friend void to_json(json& j, const TextTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.Path;
	}
	friend void from_json(const json& j, TextTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Path").get_to(o.Path);
	}
};
