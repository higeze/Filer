#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"
#include "ReactiveProperty.h"

class CD2DWWindow;

class CD2DWControl: public virtual CUIElement
{
public:
	static UINT WM_CONTROLMESSAGE;
protected:
	std::unordered_map<WORD,std::function<void(const CommandEvent&)>> m_commandMap;
	CD2DWControl* m_pParentControl;
	std::vector<std::shared_ptr<CD2DWControl>> m_childControls;
	std::shared_ptr<CD2DWControl> m_pFocusedControl;
	std::shared_ptr<CD2DWControl> m_pMouseControl;

	CRectF m_rect;
	ReactiveProperty<bool> m_isEnabled = true;
	ReactiveProperty<bool> m_isFocusable = true;

public:

	CD2DWControl(CD2DWControl* pParentControl):m_pParentControl(pParentControl){}
	virtual CD2DWWindow* GetWndPtr()const { return m_pParentControl->GetWndPtr(); }
	virtual CD2DWControl* GetParentControlPtr()const { return m_pParentControl; }
	virtual CRectF GetRectInWnd()const override { return m_rect; }

	std::shared_ptr<CD2DWControl>& GetFocusedControlPtr(){ return m_pFocusedControl;}
	void SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl);
	std::shared_ptr<CD2DWControl>& GetMouseControlPtr(){ return m_pMouseControl;}
	void SetMouseControlPtr(const std::shared_ptr<CD2DWControl>& spControl) { m_pMouseControl = spControl; }

	std::vector<std::shared_ptr<CD2DWControl>>& GetChildControlPtrs() { return m_childControls; }
	void AddChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl);
	void EraseChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl);

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnDestroy(const DestroyEvent& e) override;
	virtual void OnClose(const CloseEvent& e) override;

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

	virtual void OnCommand(const CommandEvent& e) override;

	virtual void OnKeyDown(const KeyDownEvent& e) override;
	virtual void OnSysKeyDown(const SysKeyDownEvent& e) override;
	virtual void OnChar(const CharEvent& e) override;



	virtual void OnRect(const RectEvent& e) override { m_rect = e.Rect; }
	virtual bool GetIsFocused()const;

	ReactiveProperty<bool>& GetIsEnabled() { return m_isEnabled; }
	ReactiveProperty<bool>& GetIsFocusable() { return m_isFocusable; }


	/*************/
	/* templates */
	/*************/

	template<typename TFunc, typename TEvent>
	void SendAll(TFunc f, const TEvent& e, bool invalidate = true)
	{
		for (auto& pControl : m_childControls) {
			(pControl.get()->*f)(e);
		}
		if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	}

	template<typename TFunc, typename TEvent>
	void SendCopyAll(TFunc f, const TEvent& e, bool invalidate = true)
	{
		auto controls = m_childControls;
		for (auto& pControl : controls) {
			(pControl.get()->*f)(e);
		}
		if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	}

	template<typename TFunc, typename TEvent>
	void SendMouse(TFunc f, const TEvent& e)
	{
		if (GetWndPtr()->GetCapturedControlPtr()) {
			(GetWndPtr()->GetCapturedControlPtr().get()->*f)(e);
			GetWndPtr()->InvalidateRect(NULL, FALSE);
		} else {
			SendPtInRect(f, e);
		}
	}

	template<typename TFunc, typename TEvent>
	void SendPtInRect(TFunc f, const TEvent& e, bool setFocus = false)
	{
		auto iter = std::find_if(m_childControls.cbegin(), m_childControls.cend(),
			[&](const std::shared_ptr<CD2DWControl>& x) {
				return x->GetIsEnabled() && x->GetRectInWnd().PtInRect(e.PointInWnd);
			});

		if (iter != m_childControls.cend()) {
			(iter->get()->*f)(e);
			if (setFocus) {
				SetFocusedControlPtr(*iter);
			}
		}
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}

	template<typename TFunc, typename TEvent>
	void SendCapturePtInRect(TFunc f, const TEvent& e)
	{
		if (GetWndPtr()->GetCapturedControlPtr()) {
			(GetWndPtr()->GetCapturedControlPtr().get()->*f)(e);
			GetWndPtr()->InvalidateRect(NULL, FALSE);
		} else {
			SendPtInRect(f, e);
		}
	}

	template<typename TFunc, typename TEvent>
	void SendFocused(TFunc f, const TEvent& e)
	{
		if (m_pFocusedControl) { (m_pFocusedControl.get()->*f)(e); }
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}
};
