#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"

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
	std::shared_ptr<CD2DWControl> m_pCapturedControl;
	std::shared_ptr<CD2DWControl> m_pMouseControl;

	CRectF m_rect;
public:

	CD2DWControl(CD2DWControl* pParentControl):m_pParentControl(pParentControl){}
	virtual CD2DWWindow* GetWndPtr()const { return m_pParentControl->GetWndPtr(); }
	virtual CD2DWControl* GetParentControlPtr()const { return m_pParentControl; }
	virtual CRectF GetRectInWnd()const override { return m_rect; }

	std::shared_ptr<CD2DWControl>& GetFocusedControlPtr(){ return m_pFocusedControl;}
	void SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl);
	std::shared_ptr<CD2DWControl>& GetMouseControlPtr(){ return m_pMouseControl;}
	void SetMouseControlPtr(const std::shared_ptr<CD2DWControl>& spControl) { m_pMouseControl = spControl; }
	std::shared_ptr<CD2DWControl>& GetCapturedControlPtr() { return m_pCapturedControl; }
	void SetCapturedControlPtr(const std::shared_ptr<CD2DWControl>& spControl){ m_pCapturedControl = spControl; }
	void ReleaseCapturedControlPtr() { m_pCapturedControl = nullptr; }

	std::vector<std::shared_ptr<CD2DWControl>>& GetChildControlPtrs() { return m_childControls; }
	void AddChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl);
	void EraseChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl);

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnClose(const CloseEvent& e) override;
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnRButtonDown(const RButtonDownEvent& e) override;
	virtual void OnCommand(const CommandEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnRect(const RectEvent& e) override { m_rect = e.Rect; }
	virtual bool GetIsFocused()const;
	virtual bool GetIsFocusable()const { return true; }

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
		if (GetCapturedControlPtr()) {
			(GetCapturedControlPtr().get()->*f)(e);
		} else {
			for (auto& pControl : m_childControls) {
				if (pControl->GetRectInWnd().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient))) {
					(pControl.get()->*f)(e);
					break;
				}
			}
		}
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}

	template<typename TFunc, typename TEvent>
	void SendPtInRect(TFunc f, const TEvent& e)
	{
		auto pt = GetWndPtr()->GetDirectPtr()->Pixels2Dips(GetWndPtr()->GetCursorPosInClient());
		for (auto& pControl : m_childControls) {
			if (pControl->GetRectInWnd().PtInRect(pt)) {
				(pControl.get()->*f)(e);
				break;
			}
		}
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}

	template<typename TFunc, typename TEvent>
	void SendFocused(TFunc f, const TEvent& e)
	{
		if (m_pFocusedControl) { (m_pFocusedControl.get()->*f)(e); }
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}

	template<typename TFunc, typename TEvent>
	void SetFocusSendPtInRect(TFunc f, const TEvent& e) 
	{
		auto iter = std::find_if(m_childControls.cbegin(), m_childControls.cend(),
			[&](const std::shared_ptr<CUIElement>& x) {return x->GetRectInWnd().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

		if (iter != m_childControls.cend()) {
			SetFocusedControlPtr(*iter);
			(iter->get()->*f)(e);
		} else {

		}
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}


};
