#pragma once
#include "D2DWWindow.h"
#include "D2DWControl.h"
#include "reactive_property.h"
#include "getter_macro.h"
//#include "FilerHook.h"
//#pragma comment(lib, "FilerHook.lib")


class CPreviewWnd;
struct PreviewControlProperty;

//class CPreviewWindowsHooker
//{
//public:
//	CPreviewWindowsHooker(HWND hWndOwner, void* pControl, HWND hWndPreview)
//	{
//		::SetPreviewWindowHook(hWndOwner, pControl, hWndPreview);
//	}
//
//	~CPreviewWindowsHooker()
//	{
//		::ResetPreviewWindowHook();
//	}
//
//};

class CPreviewControl : public CD2DWHostWndControl
{
protected:
	std::unique_ptr<CPreviewWnd> m_pWnd;
	//std::unique_ptr<CPreviewWindowsHooker> m_pHooker;
	std::shared_ptr<PreviewControlProperty> m_pProp;
	FARPROC m_previewProc;

public:
	CPreviewControl(CD2DWControl* pParentControl, const std::shared_ptr<PreviewControlProperty>& pProp);
	virtual ~CPreviewControl();
public:
	reactive_wstring_ptr Path;

	void Open(const std::wstring& path);
	void Open();
	void Close();
	std::tuple<CRect> GetRects() const;

public:
	/******************/
	/* Windows Message*/
	/******************/
	virtual void OnCreate(const CreateEvt& e);
	virtual void OnRect(const RectEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;

	virtual void OnPaint(const PaintEvent& e);
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

	virtual void OnKillFocus(const KillFocusEvent& e) override;
};
