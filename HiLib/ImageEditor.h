#pragma once
#include "D2DWControl.h"
#include "Textbox.h"
#include "TextBlock.h"
#include "reactive_command.h"
#include "getter_macro.h"

class CImageView;
class CStatusBar;
struct ImageEditorProperty;

class CImageEditor :public CD2DWControl
{
	SHAREDPTR_GETTER(CTextBox, ScaleBox)
	SHAREDPTR_GETTER(CTextBlock, PercentBlock)
	SHAREDPTR_GETTER(CImageView, ImageView)
	SHAREDPTR_GETTER(CStatusBar, StatusBar)
	SHAREDPTR_GETTER(ImageEditorProperty, Prop)

public:
	reactive_command_ptr<HWND> SaveCommand;
	reactive_command_ptr<HWND> OpenCommand;
	reactive_command_ptr<HWND> SaveAsCommand;
	reactive_command_ptr<HWND> OpenAsCommand;

public:
	CImageEditor(CD2DWControl* pParentControl);

	virtual ~CImageEditor() {}

	std::tuple<CRectF, CRectF, CRectF, CRectF> GetRects() const;
	virtual void ArrangeOverride(const CRectF& rc) override;

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;

	void Open();
	void OpenAs();
	void Save();
	void SaveAs();
	void Update();
};
