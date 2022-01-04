#pragma once
#include "Textbox.h"
#include "getter_macro.h"

struct EditorTextBoxProperty;

struct ExecutableInfo
{
	std::wstring Link = L"";
	UINT32 StartPosition = 0;
	UINT32 Length = 0;
};

class CEditorTextBox:public CTextBox
{
	LAZY_GETTER_NO_CLEAR_IMPL(std::vector<CRectF>, HighliteRects)

protected:
	std::vector<ExecutableInfo> m_executableInfos;
public:
	CEditorTextBox(
		CD2DWControl* pParentControl,
		const std::shared_ptr<EditorTextBoxProperty>& pProp,
		const std::wstring& text);
	virtual ~CEditorTextBox() = default;
	void LoadTextLayoutPtr() override;
	void Normal_LButtonDown(const LButtonDownEvent& e) override;
	void Normal_SetCursor(const SetCursorEvent& e) override;
	void PaintHighlite(const PaintEvent& e) override;

	void UpdateAll() override;
};
