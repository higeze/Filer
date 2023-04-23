#pragma once
#include "D2DWWindow.h"
#include "D2DWControl.h"
#include "Scroll.h"
#include "ScrollProperty.h"
#include "ReactiveProperty.h"
#include "ImageViewStateMachine.h"
#include "FileIsInUse.h"
#include "getter_macro.h"
#include "PreviewHandlerFrame.h"
#include "getter_macro.h"

struct PreviewControlProperty;

class CPreviewControl : public CD2DWControl
{
protected:
	std::shared_ptr<PreviewControlProperty> m_pProp;
	CComPtr<IFileIsInUse> m_pFileIsInUse;
	CComPtr<IPreviewHandler> m_pPreviewHandler;
	DECLARE_LAZY_GETTER(CComPtr<IPreviewHandlerFrame>, PreviewHandlerFramePtr);

public:
	CPreviewControl(CD2DWControl* pParentControl, const std::shared_ptr<PreviewControlProperty>& pProp);
	virtual ~CPreviewControl();
public:
	//ReactiveProperty
	ReactiveWStringProperty Path;
	
	//CRectF GetRenderRectInWnd();
	//CSizeF GetRenderSize();
	//CSizeF GetRenderContentSize();

	void Open(const std::wstring& path);
	void Open();
	void Close();

public:
	/******************/
	/* Windows Message*/
	/******************/
	virtual void OnCreate(const CreateEvt& e);
	virtual void OnRect(const RectEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;

	//virtual void OnPaint(const PaintEvent& e) { m_pMachine->process_event(e); }
	virtual void OnSetFocus(const SetFocusEvent& e) override;
	//virtual void OnKillFocus(const KillFocusEvent& e) override { m_pMachine->process_event(e); }
	//virtual void OnKeyDown(const KeyDownEvent& e) override { m_pMachine->process_event(e); }
	//virtual void OnLButtonDown(const LButtonDownEvent& e) override { m_pMachine->process_event(e); }
	//virtual void OnLButtonUp(const LButtonUpEvent& e) override { m_pMachine->process_event(e); }
	//virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override { m_pMachine->process_event(e); }
	//virtual void OnMouseMove(const MouseMoveEvent& e) override { m_pMachine->process_event(e); }
	//virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override 
	//{
	//	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	//	m_pMachine->process_event(e); 
	//}
	//virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override 
	//{
	//	e.WndPtr->ReleaseCapturedControlPtr();
	//	m_pMachine->process_event(e);
	//}
	//virtual void OnSetCursor(const SetCursorEvent& e) override { m_pMachine->process_event(e); }
	//virtual void OnContextMenu(const ContextMenuEvent& e) override { m_pMachine->process_event(e); }
	//virtual void OnChar(const CharEvent& e) override { m_pMachine->process_event(e); }

	virtual void OnClose(const CloseEvent& e) override;
	virtual void OnDestroy(const DestroyEvent& e) override;
	virtual void OnEnable(const EnableEvent& e) override;

	virtual void OnWndKillFocus(const KillFocusEvent& e) override;

	/**************/
	/* SM Message */
	/**************/
};
