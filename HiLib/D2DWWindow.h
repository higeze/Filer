#pragma once
#include "MyWnd.h"
#include "UIElement.h"
#include "D2DWControl.h"
#include "Direct2DWrite.h"
#include "ReactiveProperty.h"
#include "MouseStateMachine.h"
#include "Debug.h"

class CD2DWControl;
class CDispatcher;
class CDropTargetManager;

class CD2DWWindow:public CWnd, public CD2DWControl
{
private:
	std::unique_ptr<CDispatcher> m_pDispatcher;
	std::shared_ptr<CDirect2DWrite> m_pDirect;
	std::unique_ptr<CMouseStateMachine> m_pMouseMachine;
	std::unique_ptr<CDropTargetManager> m_pDropTargetManager;

	std::shared_ptr<CD2DWControl> m_pCapturedControl;

public :
	CD2DWWindow();
	virtual ~CD2DWWindow();

	CD2DWWindow* GetWndPtr()const override{ return const_cast<CD2DWWindow*>(this); }
	CDirect2DWrite* GetDirectPtr() const { return  m_pDirect.get(); }
	std::unique_ptr<CDropTargetManager>& GetDropTargetManagerPtr() { return m_pDropTargetManager; }
	std::unique_ptr<CDispatcher>& GetDispatcherPtr() { return m_pDispatcher; }
	CRectF GetRectInWnd() const { return GetDirectPtr()->Pixels2Dips(GetClientRect()); }
	CPointF GetCursorPosInWnd() const;
	bool GetIsFocused()const;
	void Update();

	std::shared_ptr<CD2DWControl>& GetCapturedControlPtr() { return m_pCapturedControl; }
	void SetCapturedControlPtr(const std::shared_ptr<CD2DWControl>& spControl){ m_pCapturedControl = spControl; }
	void ReleaseCapturedControlPtr() { m_pCapturedControl = nullptr; }


public:
	template<typename TEvent>
	std::function<LRESULT(UINT, WPARAM, LPARAM, BOOL&)> Normal_Message(void(CD2DWWindow::*memberfunc)(const TEvent&))
	{
		return [this, memberfunc](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
		{
			(this->*memberfunc)(TEvent(this, wParam, lParam, &bHandled));
			this->InvalidateRect(NULL, FALSE);
			return 0;
		};
	}
	template<typename T>
	LRESULT UserInputMachine_Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pMouseMachine->process_event(T(this, wParam, lParam, &bHandled));
		InvalidateRect(NULL, FALSE);
		return 0;
	}

	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnFinalMessage(HWND hWnd) override {}

	/***************/
	/* Control Msg */
	/***************/

	virtual void OnCreate(const CreateEvt& e) override = 0;
	//virtual void OnRect(const RectEvent& e)
	virtual void OnPaint(const PaintEvent& e) { SendAll(&CUIElement::OnPaint, e, false); }
	virtual void OnClosing(const ClosingEvent& e) { SendCopyAll(&CUIElement::OnClosing, e); }
	virtual void OnClose(const CloseEvent& e) { SendCopyAll(&CUIElement::OnClose, e); }
	virtual void OnCommand(const CommandEvent& e)
	{
		*(e.HandledPtr) = FALSE;
		SendFocused(&CUIElement::OnCommand, e);
	}

	//virtual void OnLButtonDown(const LButtonDownEvent& e)
	virtual void OnLButtonUp(const LButtonUpEvent& e) { SendMouse(&CUIElement::OnLButtonUp, e); }
	virtual void OnLButtonClk(const LButtonClkEvent& e) { SendMouse(&CUIElement::OnLButtonClk, e); }
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) { SendMouse(&CUIElement::OnLButtonSnglClk, e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) { SendMouse(&CUIElement::OnLButtonDblClk, e); }
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) { SendMouse(&CUIElement::OnLButtonBeginDrag, e); }
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) { SendMouse(&CUIElement::OnLButtonEndDrag, e); }

	//virtual void OnRButtonDown(const RButtonDownEvent& e);

	virtual void OnMButtonDown(const MButtonDownEvent& e) { SendMouse(&CUIElement::OnMButtonDown, e); }
	virtual void OnMButtonUp(const MButtonUpEvent& e) { SendMouse(&CUIElement::OnMButtonUp, e); }

	//virtual void OnMouseMove(const MouseMoveEvent& e) { SendMouse(&CUIElement::OnMouseMove, e); }
	virtual void OnMouseWheel(const MouseWheelEvent& e) { SendPtInRect(&CUIElement::OnMouseWheel, e); }
	virtual void OnContextMenu(const ContextMenuEvent& e) { SendMouse(&CUIElement::OnContextMenu, e); }

	virtual void OnSetCursor(const SetCursorEvent& e) 
	{
		*(e.HandledPtr) = FALSE;
		SendPtInRect(&CUIElement::OnSetCursor, e);
	}

	//Focus
	virtual void OnMouseEnter(const MouseEnterEvent& e) { SendPtInRect(&CUIElement::OnMouseEnter, e); }
	virtual void OnMouseLeave(const MouseLeaveEvent& e) { SendFocused(&CUIElement::OnMouseLeave, e); }
	virtual void OnSetFocus(const SetFocusEvent& e) 
	{ 
		SendAll(&CUIElement::OnWndSetFocus, e);
		SendFocused(&CUIElement::OnSetFocus, e);
	}
	virtual void OnKillFocus(const KillFocusEvent& e)
	{ 
		SendAll(&CUIElement::OnWndKillFocus, e);
		SendFocused(&CUIElement::OnKillFocus, e);
	}
	virtual void OnKeyDown(const KeyDownEvent& e);

	virtual void OnKeyUp(const KeyUpEvent& e) 
	{ 
		*(e.HandledPtr) = FALSE;
		SendFocused(&CUIElement::OnKeyUp, e);
	}
	virtual void OnSysKeyDown(const SysKeyDownEvent& e) { SendFocused(&CUIElement::OnSysKeyDown, e); }
	virtual void OnChar(const CharEvent& e) { SendFocused(&CUIElement::OnChar, e); }

	virtual void OnPropertyChanged(const wchar_t* name) {}
};

template<class TControl>
class CD2DWSingleControlWnd : public CD2DWWindow
{
private:
	std::shared_ptr<TControl> m_spChildControl;
public:
	template<typename... TArgs>
	CD2DWSingleControlWnd(TArgs... args)
		:CD2DWWindow(), m_spChildControl(std::make_shared<TControl>(this, args...))
	{
		//RegisterArgs and CreateArgs
		RegisterClassExArgument()
			.lpszClassName(_T("CD2DWSingleControlWindow"))
			.style(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS)
			.hCursor(::LoadCursor(NULL, IDC_ARROW))
			.hbrBackground(NULL);
		CreateWindowExArgument()
			.lpszClassName(_T("CD2DWSingleControlWindow"))
			.lpszWindowName(_T("D2DWSingleConrrolWindow"))
			.dwStyle(WS_OVERLAPPEDWINDOW | WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE)
			.dwExStyle(WS_EX_ACCEPTFILES)
			.hMenu(nullptr);	
	}

	std::shared_ptr<TControl> GetChildControlPtr()const { return m_spChildControl; }

	virtual void OnCreate(const CreateEvt& e) override
	{
		m_spChildControl->OnCreate(CreateEvt(this, this, GetRectInWnd()));
	}

	virtual void OnClose(const CloseEvent& e) override
	{
		CD2DWWindow::OnClose(e);
		DestroyWindow();
	}

	virtual void OnRect(const RectEvent& e) override
	{
		CD2DWWindow::OnRect(e);
		m_spChildControl->OnRect(RectEvent(this, m_rect));
	}

};

