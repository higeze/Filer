#pragma once
#include "D2DWControl.h"
#include "ReactiveProperty.h"
#include "getter_macro.h"
#include "encoding_type.h"

class CPdfView;
class CTextBox;
class CTextBlock;
class CEditorTextBox;
class CStatusBar;
struct PDFEditorProperty;

class CPDFEditor :public CD2DWControl
{
	SHAREDPTR_GETTER(CTextBox, FilterBox)
	SHAREDPTR_GETTER(CTextBox, PageBox)
	SHAREDPTR_GETTER(CTextBlock, TotalPageBlock)
	SHAREDPTR_GETTER(CTextBox, ScaleBox)
	SHAREDPTR_GETTER(CTextBlock, PercentBlock)
	SHAREDPTR_GETTER(CPdfView, PDFView)
	SHAREDPTR_GETTER(CStatusBar, StatusBar)

	SHAREDPTR_GETTER(PDFEditorProperty, Prop)

private:
	ReactiveCommand<HWND> m_save;
	ReactiveCommand<HWND> m_open;
	ReactiveCommand<HWND> m_save_as;
	ReactiveCommand<HWND> m_open_as;

	CBinding m_bindFilterText;

public:
	CPDFEditor(
		CD2DWControl* pParentControl,
		const std::shared_ptr<PDFEditorProperty>& spTextProp);

	virtual ~CPDFEditor() {}

	ReactiveCommand<HWND>& GetOpenCommand() { return m_open; }
	ReactiveCommand<HWND>& GetSaveCommand() { return m_save; }
	ReactiveCommand<HWND>& GetOpenAsCommand() { return m_open_as; }
	ReactiveCommand<HWND>& GetSaveAsCommand() { return m_save_as; }


	std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> GetRects() const;

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
