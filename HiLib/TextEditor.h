#pragma once
#include "TextBox.h"

struct TextEditorProperty;

struct ExecutableInfo
{
	std::wstring Link = L"";
	UINT32 StartPosition = 0;
	UINT32 Length = 0;
};

class CEditorTextBox:public CTextBox
{
protected:
	std::vector<ExecutableInfo> m_executableInfos;
	ReactiveWStringProperty m_filter;
public:
	CEditorTextBox(
		CD2DWControl* pParentControl,
		const std::shared_ptr<TextEditorProperty> pProp,
		const std::wstring& text);
	virtual ~CEditorTextBox() = default;
	const CComPtr<IDWriteTextLayout1>& GetTextLayoutPtr() override;
	void Normal_LButtonDown(const LButtonDownEvent& e) override;
	void Normal_SetCursor(const SetCursorEvent& e) override;
};

class CTextEditor :public CD2DWControl
{
private:
	std::shared_ptr<CEditorTextBox> m_spTextBox;
	std::shared_ptr<CStatusBar>  m_spStatusBar;
	std::shared_ptr<TextEditorProperty> m_pProp;

	ReactiveWStringProperty m_path;
	ReactiveProperty<encoding_type> m_encoding;
	ReactiveCommand<HWND> m_save;
	ReactiveCommand<HWND> m_open;
	ReactiveCommand<HWND> m_save_as;
	ReactiveCommand<HWND> m_open_as;

public:
	CTextEditor(
		CD2DWControl* pParentControl,
		const std::shared_ptr<TextEditorProperty>& spTextProp);

	virtual ~CTextEditor() {}

	std::shared_ptr<CTextBox> GetTextBoxPtr() const { return m_spTextBox; }
	std::shared_ptr<CStatusBar>  GetStatusBarPtr() const { m_spStatusBar; }

	ReactiveWStringProperty& GetPath() { return m_path; }
	ReactiveProperty<encoding_type>& GetEncoding() { return m_encoding; }
	ReactiveCommand<HWND>& GetOpenCommand() { return m_open; }
	ReactiveCommand<HWND>& GetSaveCommand() { return m_save; }
	ReactiveCommand<HWND>& GetOpenAsCommand() { return m_open_as; }
	ReactiveCommand<HWND>& GetSaveAsCommand() { return m_save_as; }

	std::tuple<CRectF, CRectF> GetRects() const;

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
