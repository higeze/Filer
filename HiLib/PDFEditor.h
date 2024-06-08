#pragma once
#include "D2DWControl.h"
#include "getter_macro.h"
#include "encoding_type.h"
#include "reactive_string.h"
#include "reactive_command.h"

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

public:
	reactive_command_ptr<HWND> SaveCommand;
	reactive_command_ptr<HWND> OpenCommand;
	reactive_command_ptr<HWND> SaveAsCommand;
	reactive_command_ptr<HWND> OpenAsCommand;

public:
	CPDFEditor(CD2DWControl* pParentControl);

	virtual ~CPDFEditor() {}

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
