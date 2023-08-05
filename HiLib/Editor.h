#pragma once
#include "D2DWControl.h"
#include "ReactiveProperty.h"
#include "reactive_string.h"
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

	ReactiveProperty<encoding_type> m_encoding;
	ReactiveCommand<HWND> m_save;
	ReactiveCommand<HWND> m_open;
	ReactiveCommand<HWND> m_save_as;
	ReactiveCommand<HWND> m_open_as;

public:
	CEditor(
		CD2DWControl* pParentControl,
		const std::shared_ptr<EditorProperty>& spTextProp);

	virtual ~CEditor() {}

	reactive_wstring_ptr Path;
	ReactiveProperty<encoding_type>& GetEncoding() { return m_encoding; }
	ReactiveCommand<HWND>& GetOpenCommand() { return m_open; }
	ReactiveCommand<HWND>& GetSaveCommand() { return m_save; }
	ReactiveCommand<HWND>& GetOpenAsCommand() { return m_open_as; }
	ReactiveCommand<HWND>& GetSaveAsCommand() { return m_save_as; }

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
