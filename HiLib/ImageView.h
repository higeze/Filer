#pragma once
#include "D2DWWindow.h"
#include "D2DWControl.h"
#include "Scroll.h"
#include "ScrollProperty.h"

#include "ImageViewProperty.h"
#include <msctf.h>

#include <cstdlib>
#include <utility>
 
#include <windows.h>
#include <shcore.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <d2d1_2.h>
#include <wincodec.h>

#include "ImageViewStateMachine.h"
#include "FileIsInUse.h"

#include "getter_macro.h"

#include <future>
#include <mutex>
#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"

class CVScroll;
class CHScroll;
class CImageDrawer;
#include "D2DImage.h"

enum class ImageScaleMode
{
	None,
	MinWidthHeight,
	Width,
	Height
};

class CImageView : public CD2DWControl
{
public:
	/* Scroll */
	SHAREDPTR_GETTER(CVScroll, VScroll)
	SHAREDPTR_GETTER(CHScroll, HScroll)

	/**********/
	/* Static */
	/**********/
public:
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<FLOAT> Scale;
	reactive_property_ptr<CD2DImage> Image;
	reactive_command_ptr<void> OpenCommand;
	reactive_wstring_ptr Find;
	reactive_property_ptr<D2D1_BITMAPSOURCE_ORIENTATION> Rotate;

	/*********/
	/* Field */
	/*********/
protected:

	FLOAT m_minScale = 0.1f;
	FLOAT m_maxScale = 8.f;

	CRectF m_rect;
	std::shared_ptr<ImageViewProperty> m_pProp;

	FLOAT m_prevScale;

	CComPtr<IFileIsInUse> m_pFileIsInUse;

	std::unique_ptr<CImageViewStateMachine> m_pMachine;

	ImageScaleMode m_initialScaleMode;
	std::unique_ptr<CImageDrawer> m_imgDrawer;


public:
	CImageView(CD2DWControl* pParentControl, const std::shared_ptr<ImageViewProperty>& pProp);
	virtual ~CImageView();
public:
	// Getter
	FLOAT GetMinScale() const { return m_minScale; }
	FLOAT GetMaxScale() const { return m_maxScale; }
	
	CRectF GetRenderRectInWnd();
	CSizeF GetRenderSize();
	CSizeF GetRenderContentSize();

	std::tuple<CRectF, CRectF> GetRects() const;

	void Open(const std::wstring& path);
	void Open();
	void Close();

public:
	/******************/
	/* Windows Message*/
	/******************/
	virtual void OnCreate(const CreateEvt& e);
	virtual void OnPaint(const PaintEvent& e) { m_pMachine->process_event(e); }
	virtual void OnSetFocus(const SetFocusEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnKillFocus(const KillFocusEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnKeyDown(const KeyDownEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnLButtonDown(const LButtonDownEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnMouseMove(const MouseMoveEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override 
	{
		e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
		m_pMachine->process_event(e); 
	}
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override 
	{
		e.WndPtr->ReleaseCapturedControlPtr();
		m_pMachine->process_event(e);
	}
	virtual void OnSetCursor(const SetCursorEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnContextMenu(const ContextMenuEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnChar(const CharEvent& e) override { m_pMachine->process_event(e); }

	virtual void OnMouseWheel(const MouseWheelEvent& e) override;
	virtual void OnClose(const CloseEvent& e) override;
	virtual void OnDestroy(const DestroyEvent& e) override;
	virtual void OnEnable(const EnableEvent& e) override;

	virtual void OnRect(const RectEvent& e) override;

	/**************/
	/* SM Message */
	/**************/
	virtual void Normal_Paint(const PaintEvent& e);
	virtual void Normal_LButtonDown(const LButtonDownEvent& e);
	virtual void Normal_LButtonUp(const LButtonUpEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonClk(const LButtonClkEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonSnglClk(const LButtonSnglClkEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonDblClk(const LButtonDblClkEvent& e);
	virtual void Normal_RButtonDown(const RButtonDownEvent& e) { /*Do nothing*/ }
	virtual void Normal_MouseMove(const MouseMoveEvent& e) { /*Do nothing*/ }
	virtual void Normal_MouseLeave(const MouseLeaveEvent& e) { /*Do nothing*/ }
	virtual void Normal_ContextMenu(const ContextMenuEvent& e);
	virtual void Normal_KeyDown(const KeyDownEvent& e);
	virtual void Normal_Char(const CharEvent& e) { /*Do nothing*/ }
	virtual void Normal_SetFocus(const SetFocusEvent& e) { /*Do nothing*/ }
	virtual void Normal_KillFocus(const KillFocusEvent& e);

	virtual void NormalPan_LButtonDown(const LButtonDownEvent& e) {}
	virtual void NormalPan_SetCursor(const SetCursorEvent& e);

	//virtual void NormalText_LButtonDown(const LButtonDownEvent& e);
	//virtual void NormalText_SetCursor(const SetCursorEvent& e);

	virtual void VScrlDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual void VScrlDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void VScrlDrag_MouseMove(const MouseMoveEvent& e);
	virtual void VScrlDrag_SetCursor(const SetCursorEvent& e);
	virtual bool VScrl_Guard_SetCursor(const SetCursorEvent& e);
	virtual bool VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);

	virtual void HScrlDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual void HScrlDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void HScrlDrag_MouseMove(const MouseMoveEvent& e);
	virtual void HScrlDrag_SetCursor(const SetCursorEvent& e);
	virtual bool HScrl_Guard_SetCursor(const SetCursorEvent& e);
	virtual bool HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);

	virtual void Panning_OnEntry(const LButtonBeginDragEvent& e);
	virtual void Panning_OnExit(const LButtonEndDragEvent& e);
	virtual void Panning_MouseMove(const MouseMoveEvent& e);
	virtual void Panning_SetCursor(const SetCursorEvent& e);
	virtual bool Panning_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);

	//virtual void TextDrag_OnEntry(const LButtonBeginDragEvent& e);
	//virtual void TextDrag_OnExit(const LButtonEndDragEvent& e);
	//virtual void TextDrag_MouseMove(const MouseMoveEvent& e);
	//virtual void TextDrag_SetCursor(const SetCursorEvent& e);
	//virtual bool TextDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);

	virtual void Error_StdException(const std::exception& e);

	void UpdateScroll();

	void Update();

};
