#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"
#include "reactive_property.h"

class CD2DWWindow;

class CD2DWControl: public virtual CUIElement
{
public:
	static UINT WM_CONTROLMESSAGE;

protected:
	std::unordered_map<WORD,std::function<void(const CommandEvent&)>> m_commandMap;
	CD2DWControl* m_pParentControl;
	// Child control
	// Z-order
	//   end <- begin
	// Paint-order
	//   begin -> end
	// Message order
	//   end -> begin
	// Tab order
	//   begin -> end
	std::vector<std::shared_ptr<CD2DWControl>> m_childControls;
	std::shared_ptr<CD2DWControl> m_pFocusedControl;
	std::shared_ptr<CD2DWControl> m_pMouseControl;

	CRectF m_rect;

	bool m_isTabStop = false;
public:
	reactive_property_ptr<bool> IsEnabled;
	reactive_property_ptr<bool> IsFocusable;

public:

	CD2DWControl(CD2DWControl* pParentControl = nullptr):
		m_pParentControl(pParentControl), IsEnabled(true), IsFocusable(true){}
	virtual ~CD2DWControl(){}

	virtual CD2DWWindow* GetWndPtr()const { return m_pParentControl->GetWndPtr(); }
	virtual CD2DWControl* GetParentControlPtr()const { return m_pParentControl; }
	virtual CRectF GetRectInWnd()const override { return m_rect; }

	std::shared_ptr<CD2DWControl>& GetFocusedControlPtr(){ return m_pFocusedControl;}
	void SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl);
	std::shared_ptr<CD2DWControl>& GetMouseControlPtr(){ return m_pMouseControl;}
	void SetMouseControlPtr(const std::shared_ptr<CD2DWControl>& spControl) { m_pMouseControl = spControl; }
	virtual bool GetIsTabStop()const { return m_isTabStop; }
	virtual void SetIsTabStop(bool value) { m_isTabStop = value; }

	std::vector<std::shared_ptr<CD2DWControl>>& GetChildControlPtrs() { return m_childControls; }
	void AddChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl);
	void EraseChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl);

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnDestroy(const DestroyEvent& e) override;
	virtual void OnClose(const CloseEvent& e) override;
	virtual void OnEnable(const EnableEvent& e) override;

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override;
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;


	virtual void OnRButtonDown(const RButtonDownEvent& e) override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;

	virtual void OnMButtonDown(const MButtonDownEvent& e) override;
	virtual void OnMButtonUp(const MButtonUpEvent& e) override;


	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;

	virtual void OnMouseWheel(const MouseWheelEvent& e) override;

	virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnKillFocus(const KillFocusEvent& e) override;

	virtual void OnWndSetFocus(const SetFocusEvent& e) override;
	virtual void OnWndKillFocus(const KillFocusEvent& e) override;

	virtual void OnCommand(const CommandEvent& e) override;

	virtual void OnKeyDown(const KeyDownEvent& e) override;
	virtual void OnKeyUp(const KeyUpEvent& e) override;
	virtual void OnSysKeyDown(const SysKeyDownEvent& e) override;
	virtual void OnChar(const CharEvent& e) override;
	virtual void OnImeStartComposition(const ImeStartCompositionEvent& e) override;




	virtual void OnRect(const RectEvent& e) override { m_rect = e.Rect; }
	virtual bool GetIsFocused()const;

	CRectF CalcCenterRectF(const CSizeF& size);


	/*************/
	/* templates */
	/*************/
	template<typename TFunc, typename TEvent>
	void SendAll(TFunc f, const TEvent& e, bool invalidate = true)
	{
		for (auto iter = m_childControls.cbegin(); iter != m_childControls.cend(); ++iter) {
			(iter->get()->*f)(e);
		}
		if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	}

	template<typename TFunc, typename TEvent>
	void SendAllReverse(TFunc f, const TEvent& e, bool invalidate = true)
	{
		for (auto iter = m_childControls.crbegin(); iter != m_childControls.crend(); ++iter) {
			(iter->get()->*f)(e);
		}
		if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	}

	template<typename TFunc, typename TEvent>
	void SendCopyAll(TFunc f, const TEvent& e, bool invalidate = true)
	{
		auto controls = m_childControls;
		for (auto iter = controls.cbegin(); iter != controls.cend(); ++iter) {
			(iter->get()->*f)(e);
		}
		if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	}

	template<typename TFunc, typename TEvent>
	void SendCopyAllReverse(TFunc f, const TEvent& e, bool invalidate = true)
	{
		auto controls = m_childControls;
		for (auto iter = controls.crbegin(); iter != controls.crend(); ++iter) {
			(iter->get()->*f)(e);
		}
		if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	}

	template<typename TFunc, typename TEvent>
	void SendMouseReverse(TFunc f, const TEvent& e)
	{
		if (GetWndPtr()->GetCapturedControlPtr()) {
			(GetWndPtr()->GetCapturedControlPtr().get()->*f)(e);
			GetWndPtr()->InvalidateRect(NULL, FALSE);
		} else {
			SendPtInRectReverse(f, e);
		}
	}

	template<typename TFunc, typename TEvent>
	void SendPtInRectReverse(TFunc f, const TEvent& e, bool setFocus = false)
	{
		auto iter = std::find_if(m_childControls.crbegin(), m_childControls.crend(),
			[&](const std::shared_ptr<CD2DWControl>& x) {
				return *x->IsEnabled && x->GetRectInWnd().PtInRect(e.PointInWnd);
			});

		if (iter != m_childControls.crend()) {
			(iter->get()->*f)(e);
			if (setFocus) {
				SetFocusedControlPtr(*iter);
			}
		}
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}

	template<typename TFunc, typename TEvent>
	void SendCapturePtInRectReverse(TFunc f, const TEvent& e)
	{
		if (GetWndPtr()->GetCapturedControlPtr()) {
			(GetWndPtr()->GetCapturedControlPtr().get()->*f)(e);
			GetWndPtr()->InvalidateRect(NULL, FALSE);
		} else {
			SendPtInRectReverse(f, e);
		}
	}

	template<typename TFunc, typename TEvent>
	void SendFocused(TFunc f, const TEvent& e)
	{
		if (m_pFocusedControl) { (m_pFocusedControl.get()->*f)(e); }
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}
};

class CD2DWHostWndControl : public CD2DWControl
{
protected:
	HWND m_hostHWnd;
	WNDPROC m_hostProc;
public:
	CD2DWHostWndControl(CD2DWControl* pParentControl = nullptr)
		:CD2DWControl(pParentControl){}
	virtual ~CD2DWHostWndControl(){}
	HWND GetHostHWnd() const { return m_hostHWnd; }
	WNDPROC GetHostOrgProc() const { return m_hostProc; }
};
