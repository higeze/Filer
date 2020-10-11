#pragma once
#include "MyWnd.h"
#include "UIElement.h"
#include "D2DWControl.h"
#include "Direct2DWrite.h"
#include "ReactiveProperty.h"
#include "Debug.h"

class CMouseStateMachine;
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
	virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnFinalMessage(HWND hWnd) override {}
	


	/***************/
	/* Control Msg */
	/***************/

	virtual void OnCreate(const CreateEvt& e) override = 0;
	//virtual void OnRect(const RectEvent& e)
	virtual void OnPaint(const PaintEvent& e) { SendAll(&CUIElement::OnPaint, e, false); }
	virtual void OnClose(const CloseEvent& e) { SendAll(&CUIElement::OnClose, e); }
	virtual void OnCommand(const CommandEvent& e) 
	{
		*(e.HandledPtr) = FALSE;
		SendFocused(&CUIElement::OnCommand, e);
		if (!*e.HandledPtr) {
			CD2DWControl::OnCommand(e);
		}
	}

	//virtual void OnLButtonDown(const LButtonDownEvent& e)
	virtual void OnLButtonUp(const LButtonUpEvent& e) { SendMouse(&CUIElement::OnLButtonUp, e); }
	virtual void OnLButtonClk(const LButtonClkEvent& e) { SendMouse(&CUIElement::OnLButtonClk, e); }
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) { SendMouse(&CUIElement::OnLButtonSnglClk, e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) { SendMouse(&CUIElement::OnLButtonDblClk, e); }
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) { SendMouse(&CUIElement::OnLButtonBeginDrag, e); }
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) { SendMouse(&CUIElement::OnLButtonEndDrag, e); }

	virtual void OnRButtonDown(const RButtonDownEvent& e) { SendMouse(&CUIElement::OnRButtonDown, e); }

	virtual void OnMButtonDown(const MouseEvent& e) { SendMouse(&CUIElement::OnMButtonDown, e); }
	virtual void OnMButtonUp(const MouseEvent& e) { SendMouse(&CUIElement::OnMButtonUp, e); }

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
	virtual void OnSetFocus(const SetFocusEvent& e) { SendFocused(&CUIElement::OnSetFocus, e); }
	virtual void OnKillFocus(const KillFocusEvent& e) { SendFocused(&CUIElement::OnKillFocus, e); }
	virtual void OnKeyDown(const KeyDownEvent& e) { SendFocused(&CUIElement::OnKeyDown, e); }
	virtual void OnSysKeyDown(const SysKeyDownEvent& e) { SendFocused(&CUIElement::OnSysKeyDown, e); }
	virtual void OnChar(const CharEvent& e) { SendFocused(&CUIElement::OnChar, e); }

	virtual void OnPropertyChanged(const wchar_t* name) {}
};

