#pragma once
#include "MyWnd.h"
#include "UIElement.h"
#include "D2DWControl.h"
#include "Direct2DWrite.h"
#include "MouseStateMachine.h"
#include "TSFManager.h"
#include "Debug.h"

class CD2DWControl;
class CDispatcher;
class CDropTargetManager;

//class CD2DWWindow:public CWnd, public CD2DWControl
//{
//public:
//	static LRESULT CALLBACK StaticHostWndSubProc(HWND hWnd, UINT uMsg, WPARAM wParam,
//		LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
//
//protected:
//	std::unordered_map<HWND, std::tuple<std::shared_ptr<CD2DWControl>, WNDPROC>> m_host_map;
//	std::unique_ptr<CDispatcher> m_pDispatcher;
//	std::shared_ptr<CDirect2DWrite> m_pDirect;
//	std::unique_ptr<CMouseStateMachine> m_pMouseMachine;
//	std::unique_ptr<CDropTargetManager> m_pDropTargetManager;
//
//	std::shared_ptr<CD2DWControl> m_pCapturedControl;
//
//public :
//	CD2DWWindow();
//	virtual ~CD2DWWindow();
//
//	std::shared_ptr<CD2DWHostWndControl> FindHostWndControlPtr(HWND hWnd);
//	LRESULT HostWndSubProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
//
//	CD2DWWindow* GetWndPtr()const override{ return const_cast<CD2DWWindow*>(this); }
//	CDirect2DWrite* GetDirectPtr() const { return  m_pDirect.get(); }
//	std::unique_ptr<CDropTargetManager>& GetDropTargetManagerPtr() { return m_pDropTargetManager; }
//	std::unique_ptr<CDispatcher>& GetDispatcherPtr() { return m_pDispatcher; }
//	CRectF GetRectInWnd() const { return GetDirectPtr()->Pixels2Dips(GetClientRect()); }
//	CPointF GetCursorPosInWnd() const;
//	bool GetIsFocused()const;
//	void Update();
//
//	std::shared_ptr<CD2DWControl>& GetCapturedControlPtr() { return m_pCapturedControl; }
//	void SetCapturedControlPtr(const std::shared_ptr<CD2DWControl>& spControl){ m_pCapturedControl = spControl; }
//	void ReleaseCapturedControlPtr() { m_pCapturedControl = nullptr; }
//
//
//public:
//	template<typename TEvent>
//	std::function<LRESULT(UINT, WPARAM, LPARAM, BOOL&)> Normal_Message(void(CD2DWWindow::*memberfunc)(const TEvent&))
//	{
//		return [this, memberfunc](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
//		{
//			(this->*memberfunc)(TEvent(this, wParam, lParam, &bHandled));
//			this->InvalidateRect(NULL, FALSE);
//			return 0;
//		};
//	}
//	template<typename TRect>
//	LRESULT UserInputMachine_Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	{
//		m_pMouseMachine->process_event(TRect(this, wParam, lParam, &bHandled));
//		InvalidateRect(NULL, FALSE);
//		return 0;
//	}
//
//	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
//	virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	void OnFinalMessage(HWND hWnd) override {}
//
//	/***************/
//	/* Control Msg */
//	/***************/
//
//	virtual void OnCreate(const CreateEvt& e) override = 0;
//	//virtual void OnRect(const RectEvent& e)
//	virtual void OnPaint(const PaintEvent& e) { SendAll(&CUIElement::OnPaint, e, false); }
//	virtual void OnClosing(const ClosingEvent& e) { SendCopyAllReverse(&CUIElement::OnClosing, e); }
//	virtual void OnClose(const CloseEvent& e) { SendCopyAll(&CUIElement::OnClose, e); }
//	virtual void OnCommand(const CommandEvent& e)
//	{
//		*(e.HandledPtr) = FALSE;
//		SendFocused(&CUIElement::OnCommand, e);
//	}
//
//	//virtual void OnLButtonDown(const LButtonDownEvent& e)
//	virtual void OnLButtonUp(const LButtonUpEvent& e) { SendMouseReverse(&CUIElement::OnLButtonUp, e); }
//	virtual void OnLButtonClk(const LButtonClkEvent& e) { SendMouseReverse(&CUIElement::OnLButtonClk, e); }
//	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) { SendMouseReverse(&CUIElement::OnLButtonSnglClk, e); }
//	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) { SendMouseReverse(&CUIElement::OnLButtonDblClk, e); }
//	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) { SendMouseReverse(&CUIElement::OnLButtonBeginDrag, e); }
//	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) { SendMouseReverse(&CUIElement::OnLButtonEndDrag, e); }
//	//virtual void OnRButtonDown(const RButtonDownEvent& e);
//
//	virtual void OnMButtonDown(const MButtonDownEvent& e) { SendMouseReverse(&CUIElement::OnMButtonDown, e); }
//	virtual void OnMButtonUp(const MButtonUpEvent& e) { SendMouseReverse(&CUIElement::OnMButtonUp, e); }
//
//	//virtual void OnMouseMove(const MouseMoveEvent& e) { SendMouse(&CUIElement::OnMouseMove, e); }
//	virtual void OnMouseWheel(const MouseWheelEvent& e) { SendPtInRectReverse(&CUIElement::OnMouseWheel, e); }
//	virtual void OnContextMenu(const ContextMenuEvent& e) { SendMouseReverse(&CUIElement::OnContextMenu, e); }
//
//	virtual void OnSetCursor(const SetCursorEvent& e) 
//	{
//		*(e.HandledPtr) = FALSE;
//		SendPtInRectReverse(&CUIElement::OnSetCursor, e);
//	}
//
//	//Focus
//	virtual void OnMouseEnter(const MouseEnterEvent& e) { SendPtInRectReverse(&CUIElement::OnMouseEnter, e); }
//	virtual void OnMouseLeave(const MouseLeaveEvent& e) { SendFocused(&CUIElement::OnMouseLeave, e); }
//	virtual void OnSetFocus(const SetFocusEvent& e) 
//	{ 
//		SendAllReverse(&CUIElement::OnSetFocus, e);
//		SendFocused(&CUIElement::OnSetFocus, e);
//	}
//	virtual void OnKillFocus(const KillFocusEvent& e)
//	{ 
//		SendAllReverse(&CUIElement::OnKillFocus, e);
//		SendFocused(&CUIElement::OnKillFocus, e);
//	}
//	virtual void OnKeyDown(const KeyDownEvent& e);
//
//	virtual void OnKeyUp(const KeyUpEvent& e) 
//	{ 
//		*(e.HandledPtr) = FALSE;
//		SendFocused(&CUIElement::OnKeyUp, e);
//	}
//	virtual void OnSysKeyDown(const SysKeyDownEvent& e) { SendFocused(&CUIElement::OnSysKeyDown, e); }
//	virtual void OnChar(const CharEvent& e) { SendFocused(&CUIElement::OnChar, e); }
//	virtual void OnImeStartComposition(const ImeStartCompositionEvent& e) { SendFocused(&CUIElement::OnImeStartComposition, e); }
//	virtual void OnKeyTraceDown(const KeyTraceDownEvent& e){ SendFocused(&CUIElement::OnKeyTraceDown, e); }
//	virtual void OnKeyTraceUp(const KeyTraceUpEvent& e){ SendFocused(&CUIElement::OnKeyTraceUp, e); }
//
//	virtual void OnPropertyChanged(const wchar_t* name) {}
//};

class CD2DWWindow:public CWnd, public CD2DWControl
{
public:
	static LRESULT CALLBACK StaticHostWndSubProc(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

protected:
	std::unordered_map<HWND, std::tuple<std::shared_ptr<CD2DWControl>, WNDPROC>> m_host_map;
	std::unique_ptr<CDispatcher> m_pDispatcher;
	std::shared_ptr<CDirect2DWrite> m_pDirect;
	std::unique_ptr<CMouseStateMachine> m_pMouseMachine;
	std::unique_ptr<CDropTargetManager> m_pDropTargetManager;

	std::shared_ptr<CD2DWControl> m_pCapturedControl;

public :
	CD2DWWindow();
	virtual ~CD2DWWindow();

	std::shared_ptr<CD2DWHostWndControl> FindHostWndControlPtr(HWND hWnd);
	LRESULT HostWndSubProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	CD2DWWindow* GetWndPtr()const override{ return const_cast<CD2DWWindow*>(this); }
	CDirect2DWrite* GetDirectPtr() const { return  m_pDirect.get(); }
	std::unique_ptr<CDropTargetManager>& GetDropTargetManagerPtr() { return m_pDropTargetManager; }
	std::unique_ptr<CDispatcher>& GetDispatcherPtr() { return m_pDispatcher; }
	CRectF GetRectInWnd() const { return GetDirectPtr()->Pixels2Dips(GetClientRect()); }
	CPointF GetCursorPosInWnd() const;
	bool IsFocused()const;
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
	template<typename TRect>
	LRESULT UserInputMachine_Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pMouseMachine->process_event(TRect(this, wParam, lParam, &bHandled));
		InvalidateRect(NULL, FALSE);
		return 0;
	}

	virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnFinalMessage(HWND hWnd) override {}

	/********************/
	/* Message Tempalte */
	/********************/
	template<typename _Bubble, typename _Event>
	void BubbleMouseMessage(_Bubble&& bubble, _Event&& e)
	{
		if (m_pCapturedControl) {
			(m_pCapturedControl.get()->*bubble)(e);
			InvalidateRect(NULL, FALSE);
		} else {
			std::vector<std::shared_ptr<CD2DWControl>> tunnelControls;
			std::shared_ptr<CD2DWControl> pParentControl = std::dynamic_pointer_cast<CD2DWControl>(shared_from_this());
			while (1) {
				std::vector<std::shared_ptr<CD2DWControl>> childControls = pParentControl->m_childControls;
				auto iter = std::find_if(childControls.crbegin(), childControls.crend(),
					[&](const std::shared_ptr<CD2DWControl>& pChildControl) {
						return *pChildControl->IsEnabled && pChildControl->GetRectInWnd().PtInRect(e.PointInWnd);
					});
				if (iter != childControls.crend()) {
					tunnelControls.push_back(*iter);
					pParentControl = *iter;
				} else {
					break;
				}
			}

			for (auto iter = tunnelControls.rbegin(); iter != tunnelControls.rend(); iter++) {
				(iter->get()->*bubble)(e);
				if (*e.HandledPtr) { break; }
			}
		}
	}

	template<typename _Bubble, typename _Event>
	void BubbleMouseMessageAndFocus(_Bubble&& bubble, _Event&& e)
	{
		if (m_pCapturedControl) {
			(m_pCapturedControl.get()->*bubble)(e);
			InvalidateRect(NULL, FALSE);
		} else {
			std::vector<std::shared_ptr<CD2DWControl>> tunnelControls;
			std::shared_ptr<CD2DWControl> pParentControl = std::dynamic_pointer_cast<CD2DWControl>(shared_from_this());
			while (1) {
				std::vector<std::shared_ptr<CD2DWControl>> childControls = pParentControl->m_childControls;
				auto iter = std::find_if(childControls.crbegin(), childControls.crend(),
					[&](const std::shared_ptr<CD2DWControl>& pChildControl) {
						return *pChildControl->IsEnabled && pChildControl->GetRectInWnd().PtInRect(e.PointInWnd);
					});
				if (iter != childControls.crend()) {
					tunnelControls.push_back(*iter);
					pParentControl = *iter;
				} else {
					break;
				}
			}

			for (auto iter = tunnelControls.rbegin(); iter != tunnelControls.rend(); iter++) {
				(*iter)->m_pParentControl->SetFocusedControlPtr(*iter);
			}

			for (auto iter = tunnelControls.rbegin(); iter != tunnelControls.rend(); iter++) {
				(iter->get()->*bubble)(e);
				if (*e.HandledPtr) { break; }
			}
		}
	}

	template<typename _Bubble, typename _Event>
	void BubbleFocusMessage(_Bubble&& bubble, _Event&& e)
	{
		std::vector<std::shared_ptr<CD2DWControl>> tunnelControls;
		std::shared_ptr<CD2DWControl> pParentControl = std::dynamic_pointer_cast<CD2DWControl>(shared_from_this());
		while (1) {
			if (pParentControl->m_pFocusedControl) {
				tunnelControls.push_back(pParentControl->m_pFocusedControl);
				pParentControl = pParentControl->m_pFocusedControl;
			} else {
				break;
			}
		}

		for (auto iter = tunnelControls.rbegin(); iter != tunnelControls.rend(); iter++) {
			(iter->get()->*bubble)(e);
			if (*e.HandledPtr) { break; }
		}
	}
	/***************/
	/* Control Msg */
	/***************/
	//Create & Destroy
	virtual void OnCreate(const CreateEvt& e) override { ProcessMessageToAllReverse(&CD2DWControl::OnCreate, e); }
	virtual void OnDestroy(const DestroyEvent& e) override { ProcessMessageToAllReverse(&CD2DWControl::OnDestroy, e); }
	//virtual void OnRect(const RectEvent& e)
	virtual void OnPaint(const PaintEvent& e) override { ProcessMessageToAllReverse(&CD2DWControl::OnPaint, e); }
	virtual void OnClosing(const ClosingEvent& e) override { ProcessMessageToAllReverse(&CUIElement::OnClosing, e); }
	virtual void OnClose(const CloseEvent& e) override { ProcessMessageToAllReverse(&CUIElement::OnClose, e); }

	//Mouse Message
	virtual void OnLButtonDown(const LButtonDownEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnLButtonDown, e); }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnLButtonUp, e); }
	virtual void OnLButtonClk(const LButtonClkEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonClk, e); }
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonSnglClk, e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonDblClk, e); }

	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonBeginDrag, e); }
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonEndDrag, e); }
	
	virtual void OnRButtonDown(const RButtonDownEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnRButtonDown, e); }
	virtual void OnRButtonUp(const RButtonUpEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnRButtonUp, e); }
	virtual void OnContextMenu(const ContextMenuEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnContextMenu, e); }

	virtual void OnMButtonDown(const MButtonDownEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnMButtonDown, e); }
	virtual void OnMButtonUp(const MButtonUpEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnMButtonUp, e); }

	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	void ProcessMouseEntryLeave(const MouseMoveEvent& e);
	void ProcessMouseLeaveRecursive(const std::shared_ptr<CD2DWControl> pLeave, const MouseMoveEvent& e);
	void ProcessMouseEntryRecursive(const std::shared_ptr<CD2DWControl> pEntry, const MouseMoveEvent& e);
	void ProcessMouseInRecursive(const std::shared_ptr<CD2DWControl> pEntry, const MouseMoveEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnMouseWheel, e); }

	virtual void OnSetCursor(const SetCursorEvent& e)  { BubbleMouseMessage(&CD2DWControl::OnSetCursor, e); }

	//Keyboard Message
	virtual void OnKeyDown(const KeyDownEvent& e) override;
	virtual void OnKeyUp(const KeyUpEvent& e) override { BubbleFocusMessage(&CD2DWControl::OnKeyUp, e); }
	virtual void OnChar(const CharEvent& e) override { BubbleFocusMessage(&CD2DWControl::OnChar, e); }
	virtual void OnKeyTraceDown(const KeyTraceDownEvent& e) override { BubbleFocusMessage(&CD2DWControl::OnKeyTraceDown, e); }
	virtual void OnKeyTraceUp(const KeyTraceUpEvent& e)  override { BubbleFocusMessage(&CD2DWControl::OnKeyTraceUp, e); }
	virtual void OnImeStartComposition(const ImeStartCompositionEvent& e) override { BubbleFocusMessage(&CD2DWControl::OnImeStartComposition, e); }
	virtual void OnSysKeyDown(const SysKeyDownEvent& e) override { BubbleFocusMessage(&CD2DWControl::OnSysKeyDown, e); }

	//Focus
	virtual void OnCommand(const CommandEvent& e) override{ BubbleFocusMessage(&CD2DWControl::OnCommand, e); }
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override { BubbleFocusMessage(&CD2DWControl::OnMouseLeave, e); }
	virtual void OnSetFocus(const SetFocusEvent& e) override { BubbleFocusMessage(&CD2DWControl::OnSetFocus, e); }
	virtual void OnKillFocus(const KillFocusEvent& e) override { BubbleFocusMessage(&CD2DWControl::OnKillFocus, e); }

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

