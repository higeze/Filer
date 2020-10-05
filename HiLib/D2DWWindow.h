#pragma once
#include "MyWnd.h"
#include "UIElement.h"
#include "D2DWControl.h"
#include "Direct2DWrite.h"
#include "ReactiveProperty.h"

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
	std::vector<std::shared_ptr<CD2DWControl>> m_pControls;
	std::unique_ptr<CDropTargetManager> m_pDropTargetManager;

	std::shared_ptr<CD2DWControl> m_pFocusedControl;
	std::shared_ptr<CD2DWControl> m_pCapturedControl;


public :
	CD2DWWindow();
	virtual ~CD2DWWindow();

	CD2DWWindow* GetWndPtr()const override{ return const_cast<CD2DWWindow*>(this); }
	CDirect2DWrite* GetDirectPtr() const { return  m_pDirect.get(); }
	std::unique_ptr<CDropTargetManager>& GetDropTargetManagerPtr() { return m_pDropTargetManager; }
	CPointF GetCursorPosInWnd() const
	{ 
		CPoint pt;
		::GetCursorPos(&pt);
		::ScreenToClient(m_hWnd, &pt);
		return GetDirectPtr()->Pixels2Dips(pt);
	}
	std::unique_ptr<CDispatcher>& GetDispatcherPtr() { return m_pDispatcher; }
	bool GetIsFocused()const;

	std::shared_ptr<CD2DWControl>& GetFocusedControlPtr()
	{
		return m_pFocusedControl;
	}
	void SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl);
	std::shared_ptr<CD2DWControl>& GetCapturedControlPtr() { return m_pCapturedControl; }
	void SetCapturedControlPtr(const std::shared_ptr<CD2DWControl>& spControl)
	{
		::OutputDebugString(L"SetCapturedControl\r\n");
		m_pCapturedControl = spControl;
	}
	void ReleaseCapturedControlPtr()
	{
		::OutputDebugString(L"ReleaseCapturedControl\r\n");
		m_pCapturedControl = nullptr;
	}
	std::vector<std::shared_ptr<CD2DWControl>>& GetControls() { return m_pControls; }
	void ClearControlPtr() { m_pControls.clear(); }
	void AddControlPtr(const std::shared_ptr<CD2DWControl>& pControl) 
	{ 
		m_pControls.push_back(pControl);
		m_pFocusedControl = pControl;
	}
	void Update();

public:
	template<typename T>
	LRESULT Normal_Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pMouseMachine->process_event(T(this, wParam, lParam, &bHandled));
		InvalidateRect(NULL, FALSE);
		return 0;
	}
	template<typename T>
	LRESULT UserInputMachine_Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pMouseMachine->process_event(T(this, wParam, lParam, &bHandled));
		InvalidateRect(NULL, FALSE);
		return 0;
	}

	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

	virtual LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void OnFinalMessage(HWND hWnd) override;
	
	template<typename TFunc, typename TEvent>
	void SendAll(TFunc f, const TEvent& e, bool invalidate = true)
	{
		for (auto& pControl : m_pControls) {
			(pControl.get()->*f)(e);
		}
		//if (invalidate) { InvalidateRect(NULL, FALSE); }
	}

	template<typename TFunc, typename TEvent>
	void SendMouse(TFunc f, const TEvent& e)
	{
		if (GetCapturedControlPtr()) {
			(GetCapturedControlPtr().get()->*f)(e);
		} else {
			for (auto& pControl : m_pControls) {
				if (pControl->GetRectInWnd().PtInRect(GetDirectPtr()->Pixels2Dips(e.PointInClient))) {
					(pControl.get()->*f)(e);
					break;
				}
			}
		}
		InvalidateRect(NULL, FALSE);
	}

	template<typename TFunc, typename TEvent>
	void SendPtInRect(TFunc f, const TEvent& e)
	{
		auto pt = GetDirectPtr()->Pixels2Dips(GetCursorPosInClient());
		for (auto& pControl : m_pControls) {
			if (pControl->GetRectInWnd().PtInRect(pt)) {
				(pControl.get()->*f)(e);
				break;
			}
		}
		InvalidateRect(NULL, FALSE);
	}

	template<typename TFunc, typename TEvent>
	void SendFocused(TFunc f, const TEvent& e)
	{
		if (m_pFocusedControl) { (m_pFocusedControl.get()->*f)(e); }
		InvalidateRect(NULL, FALSE);
	}


	/***************/
	/* Control Msg */
	/***************/

	virtual void OnCreate(const CreateEvt& e) = 0;
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
	virtual void OnRect(const RectEvent& e) {}


	virtual void OnLButtonDown(const LButtonDownEvent& e) 
	{
		auto iter = std::find_if(m_pControls.begin(), m_pControls.end(),
			[&](const std::shared_ptr<CUIElement>& x) {return x->GetRectInWnd().PtInRect(GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

		SetFocusedControlPtr(iter != m_pControls.end() ? *iter : m_pControls.front());
		SendMouse(&CUIElement::OnLButtonDown, e);

	}
	virtual void OnLButtonUp(const LButtonUpEvent& e) { SendMouse(&CUIElement::OnLButtonUp, e); }
	virtual void OnLButtonClk(const LButtonClkEvent& e) { SendMouse(&CUIElement::OnLButtonClk, e); }
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) { SendMouse(&CUIElement::OnLButtonSnglClk, e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) { SendMouse(&CUIElement::OnLButtonDblClk, e); }
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) { SendMouse(&CUIElement::OnLButtonBeginDrag, e); }
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) { SendMouse(&CUIElement::OnLButtonEndDrag, e); }

	virtual void OnRButtonDown(const RButtonDownEvent& e) { SendMouse(&CUIElement::OnRButtonDown, e); }

	virtual void OnMButtonDown(const MouseEvent& e) { SendMouse(&CUIElement::OnMButtonDown, e); }
	virtual void OnMButtonUp(const MouseEvent& e) { SendMouse(&CUIElement::OnMButtonUp, e); }

	virtual void OnMouseMove(const MouseMoveEvent& e) { SendMouse(&CUIElement::OnMouseMove, e); }

	virtual void OnMouseWheel(const MouseWheelEvent& e) { SendPtInRect(&CUIElement::OnMouseWheel, e); }

	//virtual void OnCaptureChanged(const CaptureChangedEvent& e)
	//{
	//	if (e.HWnd == m_hWnd) {
	//		auto pt = GetDirectPtr()->Pixels2Dips(GetCursorPosInClient());
	//		auto iter = std::find_if(m_pControls.begin(), m_pControls.end(),
	//			[&](const std::shared_ptr<CUIElement>& x) {return x->GetRectInWnd().PtInRect(pt); });
	//		SetCapturedControl(*iter);
	//	} else {
	//		ReleaseCapturedControl();
	//	}
	//}

	virtual void OnContextMenu(const ContextMenuEvent& e) { SendMouse(&CUIElement::OnContextMenu, e); }

	virtual void OnSetCursor(const SetCursorEvent& e) 
	{
		*(e.HandledPtr) = FALSE;
		SendPtInRect(&CUIElement::OnSetCursor, e);
	}

	//Focus
	virtual void OnMouseEnter(const MouseEvent& e) { SendPtInRect(&CUIElement::OnMouseEnter, e); }
	virtual void OnMouseLeave(const MouseLeaveEvent& e) { SendFocused(&CUIElement::OnMouseLeave, e); }
	virtual void OnSetFocus(const SetFocusEvent& e) { SendFocused(&CUIElement::OnSetFocus, e); }
	virtual void OnKillFocus(const KillFocusEvent& e) { SendFocused(&CUIElement::OnKillFocus, e); }
	virtual void OnKeyDown(const KeyDownEvent& e) { SendFocused(&CUIElement::OnKeyDown, e); }
	virtual void OnSysKeyDown(const SysKeyDownEvent& e) { SendFocused(&CUIElement::OnSysKeyDown, e); }
	virtual void OnChar(const CharEvent& e) { SendFocused(&CUIElement::OnChar, e); }
	virtual void OnPropertyChanged(const wchar_t* name) {}

	CRectF GetRectInWnd() const { return GetDirectPtr()->Pixels2Dips(GetClientRect()); }
};

