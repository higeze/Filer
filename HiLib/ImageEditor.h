#pragma once
#include "D2DWControl.h"
#include "Textbox.h"
#include "TextBlock.h"
#include "ReactiveProperty.h"
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

private:
	ReactiveCommand<HWND> m_save;
	ReactiveCommand<HWND> m_open;
	ReactiveCommand<HWND> m_save_as;
	ReactiveCommand<HWND> m_open_as;

	CBinding m_bindScaleText;

public:
	CImageEditor(
		CD2DWControl* pParentControl,
		const std::shared_ptr<ImageEditorProperty>& spTextProp);

	virtual ~CImageEditor() {}

	ReactiveCommand<HWND>& GetOpenCommand() { return m_open; }
	ReactiveCommand<HWND>& GetSaveCommand() { return m_save; }
	ReactiveCommand<HWND>& GetOpenAsCommand() { return m_open_as; }
	ReactiveCommand<HWND>& GetSaveAsCommand() { return m_save_as; }

	std::tuple<CRectF, CRectF, CRectF, CRectF> GetRects() const;

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
