#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"
#include "reactive_property.h"

class CD2DWWindow;

enum class DockEnum
{
	None,
	Left,
	Top,
	Right,
	Bottom,
	Fill,
	//Vertical,
	//Horizontal,
	//LeftFix,
	//TopFix,
	//RightFix,
	//BottomFix,
};



class CD2DWControl: public virtual CUIElement
{
	friend class CD2DWWindow;
public:
	static UINT WM_CONTROLMESSAGE;
public:

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
	reactive_property_ptr<DockEnum> Dock;
	//reactive_property_ptr<size_t> GridRow;
	//reactive_property_ptr<size_t> GridColumn;



public:

	CD2DWControl(CD2DWControl* pParentControl = nullptr):
		m_pParentControl(pParentControl), IsEnabled(true), IsFocusable(true){}
	virtual ~CD2DWControl() = default;

	virtual CD2DWWindow* GetWndPtr()const { return m_pParentControl->GetWndPtr(); }
	virtual CD2DWControl* GetParentControlPtr()const { return m_pParentControl; }
	virtual CRectF GetRectInWnd()const override { return m_rect; }

	std::shared_ptr<CD2DWControl>& GetFocusedControlPtr(){ return m_pFocusedControl;}
	//void SetFocusedControlPtr(const std::shared_ptr<CD2DWControl>& spControl);
	std::shared_ptr<CD2DWControl>& GetMouseControlPtr(){ return m_pMouseControl;}
	void SetMouseControlPtr(const std::shared_ptr<CD2DWControl>& spControl) { m_pMouseControl = spControl; }
	virtual bool GetIsTabStop()const { return m_isTabStop; }
	virtual void SetIsTabStop(bool value) { m_isTabStop = value; }

	std::vector<std::shared_ptr<CD2DWControl>>& GetChildControlPtrs() { return m_childControls; }
	void AddChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl);
	void EraseChildControlPtr(const std::shared_ptr<CD2DWControl>& pControl);

	/***************/
	/* Control Msg */
	/***************/
	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnDestroy(const DestroyEvent& e) override;
	virtual void OnClose(const CloseEvent& e) override;
	virtual void OnPaint(const PaintEvent& e) override { ProcessMessageToAll(&CD2DWControl::OnPaint, e); }
	virtual void OnEnable(const EnableEvent& e) override;

	//Mouse Message
	virtual void OnLButtonDown(const LButtonDownEvent& e) override {}
	virtual void OnLButtonUp(const LButtonUpEvent& e) override {}
	virtual void OnLButtonClk(const LButtonClkEvent& e) override {}
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override {}
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override {}

	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override {}
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override {}

	virtual void OnRButtonDown(const RButtonDownEvent& e) override {}
	virtual void OnRButtonUp(const RButtonUpEvent& e) override {}
	virtual void OnContextMenu(const ContextMenuEvent& e) override {}

	virtual void OnMButtonDown(const MButtonDownEvent& e) override {}
	virtual void OnMButtonUp(const MButtonUpEvent& e) override {}

	virtual void OnMouseMove(const MouseMoveEvent& e) override {}
	virtual void OnMouseWheel(const MouseWheelEvent& e) override {}

	virtual void OnSetCursor(const SetCursorEvent& e) override {}

	//Keyboard Message
	virtual void OnKeyDown(const KeyDownEvent& e) override;
	virtual void OnKeyUp(const KeyUpEvent& e) override {}
	virtual void OnChar(const CharEvent& e) override {}
	virtual void OnImeStartComposition(const ImeStartCompositionEvent& e) override {}
	virtual void OnKeyTraceDown(const KeyTraceDownEvent& e) override {}
	virtual void OnKeyTraceUp(const KeyTraceUpEvent& e) override {}
	virtual void OnSysKeyDown(const SysKeyDownEvent& e) override {}


	virtual void OnCommand(const CommandEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override {}
	virtual void OnSetFocus(const SetFocusEvent& e) override {}
	virtual void OnKillFocus(const KillFocusEvent& e) override {}

	virtual void OnRect(const RectEvent& e) override { m_rect = e.Rect; }

	//DesiredSize includes Margin
	//RenderSize doesnt include Margin
	//RenderRect doesnt include Margin
	//Arrange includes Margin
	//GetRectInWnd includes Margin

	CSizeF m_size;
	virtual void Measure(const CSizeF& availableSize) { m_size = availableSize; }
	virtual CSizeF DesiredSize() const { return m_size; }
	virtual CSizeF RenderSize() const { return CSizeF(m_size.width - GetMargin().Width(), m_size.height - GetMargin().Height()); }

	virtual CRectF RenderRect() const { return GetRectInWnd().DeflateRectCopy(GetMargin()); }
	virtual void Arrange(const CRectF& rc) { m_rect = rc; }
	virtual CRectF ArrangedRect() const { return m_rect; }

	virtual bool IsFocused()const;
	virtual bool GetIsFocused()const;

	CRectF CalcCenterRectF(const CSizeF& size) const;

	/*************/
	/* templates */
	/*************/
	template<typename _Bubble, typename _Event>
	void ProcessMessageToAll(_Bubble bubble, const _Event& e)
	{
		std::vector<std::shared_ptr<CD2DWControl>> childControls = m_childControls;
		for (auto iter = childControls.cbegin(); iter != childControls.cend(); ++iter) {
			(iter->get()->*bubble)(e);
		}
	}

	template<typename _Bubble, typename _Event>
	void ProcessMessageToAllReverse(_Bubble bubble, const _Event& e)
	{
		std::vector<std::shared_ptr<CD2DWControl>> childControls = m_childControls;
		for (auto iter = childControls.crbegin(); iter != childControls.crend(); ++iter) {
			(iter->get()->*bubble)(e);
		}
	}

	friend void to_json(json& j, const CD2DWControl& o) 
	{
		j = json{
			{"Dock", o.Dock}
		};
	}

	friend void from_json(const json& j, CD2DWControl& o)
	{
		get_to(j, "Dock", o.Dock);
	}

	template<typename T>
	std::vector<std::shared_ptr<T>> FindChildren(const std::shared_ptr<CD2DWControl>& that)
	{
		std::vector<std::shared_ptr<T>> ret;

		for (auto& child : that->m_childControls) {
			if (auto p = std::dynamic_pointer_cast<T>(child)) {
				ret.push_back(p);
			}
			std::vector<std::shared_ptr<T>> child_ret = FindChildren<T>(child);
			std::copy(child_ret.cbegin(), child_ret.cend(), std::back_inserter(ret));
		}

		return ret;
	}


	//template<typename TFunc, typename TEvent>
	//void SendAll(TFunc f, const TEvent& e, bool invalidate = true)
	//{
	//	for (auto iter = m_childControls.cbegin(); iter != m_childControls.cend(); ++iter) {
	//		(iter->get()->*f)(e);
	//	}
	//	if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	//}

	//template<typename TFunc, typename TEvent>
	//void SendAllReverse(TFunc f, const TEvent& e, bool invalidate = true)
	//{
	//	for (auto iter = m_childControls.crbegin(); iter != m_childControls.crend(); ++iter) {
	//		(iter->get()->*f)(e);
	//	}
	//	if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	//}

	//template<typename TFunc, typename TEvent>
	//void SendCopyAll(TFunc f, const TEvent& e, bool invalidate = true)
	//{
	//	auto controls = m_childControls;
	//	for (auto iter = controls.cbegin(); iter != controls.cend(); ++iter) {
	//		(iter->get()->*f)(e);
	//	}
	//	if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	//}

	//template<typename TFunc, typename TEvent>
	//void SendCopyAllReverse(TFunc f, const TEvent& e, bool invalidate = true)
	//{
	//	auto controls = m_childControls;
	//	for (auto iter = controls.crbegin(); iter != controls.crend(); ++iter) {
	//		(iter->get()->*f)(e);
	//	}
	//	if (invalidate) { GetWndPtr()->InvalidateRect(NULL, FALSE); }
	//}

	//template<typename TFunc, typename TEvent>
	//void SendMouseReverse(TFunc f, const TEvent& e)
	//{
	//	if (GetWndPtr()->GetCapturedControlPtr()) {
	//		(GetWndPtr()->GetCapturedControlPtr().get()->*f)(e);
	//		GetWndPtr()->InvalidateRect(NULL, FALSE);
	//	} else {
	//		SendPtInRectReverse(f, e);
	//	}
	//}

	//template<typename _Tunnel, typename _Bubble, typename _Self, typename _Event>
	//void ProcessMessage(_Tunnel&& tunnel, _Bubble&& bubble, _Self&& self, _Event&& e)
	//{
	//	if (!*e.HandledPtr){ return; }
	//	if (!*IsEnabled) { return; }
	//	for (auto iter = m_childControls.begin(); iter != m_childControls.end(); iter++) {
	//		if (!*e.HandledPtr) { break; }
	//		if (!*(*iter->IsEnabled)) { continue; }
	//		
	//	}

	//}

	//template<typename TFunc, typename TEvent>
	//void SendPtInRectReverse(TFunc f, const TEvent& e, bool setFocus = false)
	//{
	//	auto iter = std::find_if(m_childControls.crbegin(), m_childControls.crend(),
	//		[&](const std::shared_ptr<CD2DWControl>& x) {
	//			return *x->IsEnabled && x->GetRectInWnd().PtInRect(e.PointInWnd);
	//		});

	//	if (iter != m_childControls.crend()) {
	//		(iter->get()->*f)(e);
	//		if (setFocus) {
	//			SetFocusedControlPtr(*iter);
	//		}
	//	}
	//	GetWndPtr()->InvalidateRect(NULL, FALSE);
	//}

	//template<typename TFunc, typename TEvent>
	//void SendCapturePtInRectReverse(TFunc f, const TEvent& e)
	//{
	//	if (GetWndPtr()->GetCapturedControlPtr()) {
	//		(GetWndPtr()->GetCapturedControlPtr().get()->*f)(e);
	//		GetWndPtr()->InvalidateRect(NULL, FALSE);
	//		*e.HandledPtr = TRUE;
	//	} else {
	//		SendPtInRectReverse(f, e);
	//	}
	//}

	//template<typename TFunc, typename TEvent>
	//void SendFocused(TFunc f, const TEvent& e)
	//{
	//	if (m_pFocusedControl) { (m_pFocusedControl.get()->*f)(e); }
	//	GetWndPtr()->InvalidateRect(NULL, FALSE);
	//}
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
