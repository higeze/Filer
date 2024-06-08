#pragma once
#include "D2DWControl.h"
#include "TextDoc.h"
#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"
#include "getter_macro.h"
#include "encoding_type.h"

class CTextBox;
class CEditorTextBox;
class CStatusBar;
struct EditorProperty;

class CEditor :public CD2DWControl
{
	SHAREDPTR_GETTER(CTextBox, FilterBox)
	SHAREDPTR_GETTER(CEditorTextBox, TextBox)
	SHAREDPTR_GETTER(CStatusBar, StatusBar)
	SHAREDPTR_GETTER(EditorProperty, Prop)
public:
	CEditor(CD2DWControl* pParentControl);

	virtual ~CEditor() {}

	reactive_wstring_ptr Path;	
	reactive_property_ptr<encoding_type> Encoding;
	reactive_property_ptr<FileStatus> Status;

	reactive_command_ptr<HWND> SaveCommand;
	reactive_command_ptr<HWND> OpenCommand;
	reactive_command_ptr<HWND> SaveAsCommand;
	reactive_command_ptr<HWND> OpenAsCommand;

	std::tuple<CRectF, CRectF, CRectF> GetRects() const;

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnRect(const RectEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;

	void Open();
	void OpenAs();
	void Save();
	void SaveAs();
	void Update();
};
