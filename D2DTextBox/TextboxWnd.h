#pragma once
#include "MyWnd.h"
#include "UIElement.h"
#include "Direct2DWrite.h"
#include "observable.h"

class CTextBox;
struct TextEditorProperty;
class CMouseStateMachine;
class CUIElement;

class CTextboxWnd:public CWnd, public CUIElement
{
public:
	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;
	std::unique_ptr<CMouseStateMachine> m_pMouseMachine;
	std::vector<std::shared_ptr<CUIElement>> m_pControls;
	std::shared_ptr<CUIElement> m_pFocusedControl;

public :
	CTextboxWnd();
	virtual ~CTextboxWnd();

	d2dw::CDirect2DWrite* GetDirectPtr() const override { return  m_pDirect.get(); }
	bool GetIsFocused()const;

	std::shared_ptr<CUIElement>& GetFocusedControlPtr()
	{
		return m_pFocusedControl;
	}
	void ClearControlPtr() { m_pControls.clear(); }
	void AddControlPtr(const std::shared_ptr<CUIElement>& pControl) 
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
	void SendAll(TFunc f, const TEvent& e, bool invalidate = false)
	{
		for (auto& pControl : m_pControls) {
			(pControl.get()->*f)(e);
		}
		if (invalidate) { InvalidateRect(NULL, FALSE); }
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

	void SetFocusControl(const std::shared_ptr<CUIElement>& spControl)
	{
		if (m_pFocusedControl) {
			m_pFocusedControl->OnKillFocus(KillFocusEvent(this, 0, 0, nullptr));
		}
		m_pFocusedControl = spControl;
		m_pFocusedControl->OnSetFocus(SetFocusEvent(this, 0, 0, nullptr));
	}

	virtual void OnCreate(const CreateEvent& e) { SendAll(&CUIElement::OnCreate, e); }
	virtual void OnPaint(const PaintEvent& e) { SendAll(&CUIElement::OnPaint, e, false); }
	virtual void OnClose(const CloseEvent& e) { SendAll(&CUIElement::OnClose, e); }
	virtual void OnCommand(const CommandEvent& e) { SendAll(&CUIElement::OnCommand, e); }
	virtual void OnRect(const RectEvent& e) { SendFocused(&CUIElement::OnRect, e); }

	virtual void OnLButtonDown(const LButtonDownEvent& e) 
	{
		auto iter = std::find_if(m_pControls.begin(), m_pControls.end(),
			[&](const std::shared_ptr<CUIElement>& x) {return x->GetRectInWnd().PtInRect(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

		SetFocusControl(iter != m_pControls.end() ? *iter : m_pControls.front());
		SendFocused(&CUIElement::OnLButtonDown, e);

	}
	virtual void OnLButtonUp(const LButtonUpEvent& e) { SendPtInRect(&CUIElement::OnLButtonUp, e); }
	virtual void OnLButtonClk(const LButtonClkEvent& e) { SendPtInRect(&CUIElement::OnLButtonClk, e); }
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) { SendPtInRect(&CUIElement::OnLButtonSnglClk, e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) { SendPtInRect(&CUIElement::OnLButtonDblClk, e); }
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) { SendPtInRect(&CUIElement::OnLButtonBeginDrag, e); }

	virtual void OnRButtonDown(const RButtonDownEvent& e) { SendPtInRect(&CUIElement::OnRButtonDown, e); }

	virtual void OnMButtonDown(const MouseEvent& e) { SendPtInRect(&CUIElement::OnMButtonDown, e); }
	virtual void OnMButtonUp(const MouseEvent& e) { SendPtInRect(&CUIElement::OnMButtonUp, e); }

	virtual void OnMouseMove(const MouseMoveEvent& e) { SendPtInRect(&CUIElement::OnMouseMove, e); }
	virtual void OnMouseWheel(const MouseWheelEvent& e) { SendPtInRect(&CUIElement::OnMouseWheel, e); }

	virtual void OnContextMenu(const ContextMenuEvent& e) { SendPtInRect(&CUIElement::OnContextMenu, e); }
	virtual void OnSetCursor(const SetCursorEvent& e) { SendPtInRect(&CUIElement::OnSetCursor, e); }

	//Focus
	virtual void OnMouseEnter(const MouseEvent& e) { SendPtInRect(&CUIElement::OnMouseEnter, e); }
	virtual void OnMouseLeave(const MouseLeaveEvent& e) { SendFocused(&CUIElement::OnMouseLeave, e); }
	virtual void OnSetFocus(const SetFocusEvent& e) { SendFocused(&CUIElement::OnSetFocus, e); }
	virtual void OnKillFocus(const KillFocusEvent& e) { SendFocused(&CUIElement::OnKillFocus, e); }
	virtual void OnKeyDown(const KeyDownEvent& e) { SendFocused(&CUIElement::OnKeyDown, e); }
	virtual void OnSysKeyDown(const SysKeyDownEvent& e) { SendFocused(&CUIElement::OnSysKeyDown, e); }
	virtual void OnChar(const CharEvent& e) { SendFocused(&CUIElement::OnChar, e); }



	virtual void OnPropertyChanged(const wchar_t* name) {}

	CWnd* GetWndPtr() const { return nullptr; }
	d2dw::CRectF GetRectInWnd() const { return GetDirectPtr()->Pixels2Dips(GetClientRect()); }

};
