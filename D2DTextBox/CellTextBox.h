#pragma once

#include "Textbox.h"
#include "EditorTextBox.h"
#include "CellProperty.h"

class CTextCell;

class CCellTextBox :public CTextBox
{
protected:
	CTextCell* m_pCell;
	std::function<void(const std::wstring&)> m_changed;
	std::function<void(const std::wstring&)> m_final;
public:
	CCellTextBox(
		CD2DWControl* pParentControl,
		const std::shared_ptr<TextBoxProperty> pProp,
		const std::wstring& text,
		CTextCell* pCell,
		std::function<void(const std::wstring&)> changed,
		std::function<void(const std::wstring&)> final);

	CTextCell* GetCellPtr() { return m_pCell; }

	void OnClose(const CloseEvent& e);
	bool GetIsVisible()const;
	CRectF GetRectInWnd() const;
};

class CEditorCellTextBox :public CEditorTextBox
{
protected:
	CTextCell* m_pCell;
	std::function<void(const std::wstring&)> m_changed;
	std::function<void(const std::wstring&)> m_final;
public:
	CEditorCellTextBox(
		CD2DWControl* pParentControl,
		const std::shared_ptr<EditorTextBoxProperty> pProp,
		const std::wstring& text,
		CTextCell* pCell,
		std::function<void(const std::wstring&)> changed,
		std::function<void(const std::wstring&)> final);

	CTextCell* GetCellPtr() { return m_pCell; }

	void OnClose(const CloseEvent& e);
	bool GetIsVisible()const;
	CRectF GetRectInWnd() const;
};
