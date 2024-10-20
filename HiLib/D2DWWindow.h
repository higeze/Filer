#pragma once
#include <mutex>
#include <shared_mutex>
#include "MyWnd.h"
#include "UIElement.h"
#include "D2DWControl.h"
#include "Direct2DWrite.h"
#include "MouseStateMachine.h"
#include "TSFManager.h"
#include "Debug.h"
#include "DeadlineTimer.h"

class CDispatcher;
class CDropTargetManager;

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
	std::shared_ptr<CD2DWControl> m_pToolTip;
	CDeadlineTimer m_toolTipDeadlineTimer;

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

private:
	std::shared_mutex m_toolTipMtx;
public:

	std::shared_ptr<CD2DWControl>& GetToolTipControlPtr() 
	{
		std::shared_lock<std::shared_mutex>(m_toolTipMtx);
		return m_pToolTip;
	}
	void SetToolTipControlPtr(const std::shared_ptr<CD2DWControl>& spControl) 
	{
		std::unique_lock<std::shared_mutex> lock(m_toolTipMtx);
		m_pToolTip = spControl;
	}

	CDeadlineTimer& GetToolTipDeadlineTimer() { return m_toolTipDeadlineTimer; }

	//template<class _Control>
	//void CloseChildControl()
	//{
	//	for(std::shared_ptr<CD2DWControl>& spControl : m_childControls) {
	//		if (auto p = std::dynamic_pointer_cast<_Control>(spControl)) {
	//			spControl->OnClose(CloseEvent(this, NULL, NULL));
	//		}
	//	}
	//}


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
		InvalidateRect(NULL, FALSE);
	}

	void SetFocusToControl(const std::shared_ptr<CD2DWControl>& pControl)
	{
		std::shared_ptr<CD2DWControl> pFocusedControl = pControl;
		while(pFocusedControl->m_pParentControl){
			if (pFocusedControl->m_pParentControl->m_pFocusedControl != pFocusedControl) {
				if (pFocusedControl->m_pParentControl->m_pFocusedControl) {
					pFocusedControl->m_pParentControl->m_pFocusedControl->OnKillFocus(KillFocusEvent(GetWndPtr(), 0, 0, nullptr));
				}
				pFocusedControl->m_pParentControl->m_pFocusedControl = pControl;
				pFocusedControl->m_pParentControl->m_pFocusedControl->OnSetFocus(SetFocusEvent(GetWndPtr(), 0, 0, nullptr));
			}
			pFocusedControl = std::dynamic_pointer_cast<CD2DWControl>(pFocusedControl->m_pParentControl->shared_from_this());
		};


		////Cur Focused
		//std::vector<std::shared_ptr<CD2DWControl>> tunnelCurControls;
		//std::shared_ptr<CD2DWControl> pParentControl = std::dynamic_pointer_cast<CD2DWControl>(shared_from_this());
		//while (1) {
		//	if (pParentControl->m_pFocusedControl) {
		//		tunnelCurControls.push_back(pParentControl->m_pFocusedControl);
		//		pParentControl = pParentControl->m_pFocusedControl;
		//	} else {
		//		break;
		//	}
		//}
		////New Focused
		//std::vector<std::shared_ptr<CD2DWControl>> bubbleNewControls;
		//std::shared_ptr<CD2DWControl> pChildControl = pControl;
		//do {
		//	if (pChildControl->m_pParentControl) {
		//		bubbleNewControls.push_back(pChildControl);
		//		pChildControl = std::dynamic_pointer_cast<CD2DWControl>(pChildControl->m_pParentControl->shared_from_this());
		//	} else {
		//		break;
		//	}
		//} while (true);

		////Kill Focus
		//for (auto iter = tunnelCurControls.rbegin(); iter != tunnelCurControls.rend(); iter++) {
		//	if (std::find(bubbleNewControls.begin(), bubbleNewControls.end(), *iter) == bubbleNewControls.end()) {
		//		(*iter)->m_pParentControl->m_pFocusedControl = nullptr;
		//		(*iter)->OnKillFocus(KillFocusEvent(GetWndPtr(), 0, 0, nullptr));
		//	}
		//}
		////Focus
		//for (auto iter = bubbleNewControls.begin(); iter != bubbleNewControls.end(); iter++) {
		//	if (std::find(tunnelCurControls.rbegin(), tunnelCurControls.rend(), *iter) == tunnelCurControls.rend()) {
		//		(*iter)->m_pParentControl->m_pFocusedControl = *iter;
		//		(*iter)->OnSetFocus(SetFocusEvent(GetWndPtr(), 0, 0, nullptr));
		//	}
		//}
	}

	template<typename _Bubble, typename _Event>
	void BubbleMouseMessageAndFocus(_Bubble&& bubble, _Event&& e)
	{
		if (m_pCapturedControl) {
			(m_pCapturedControl.get()->*bubble)(e);
		} else {
			//Cur Focused
			std::vector<std::shared_ptr<CD2DWControl>> tunnelCurControls;
			std::shared_ptr<CD2DWControl> pParentControl = std::dynamic_pointer_cast<CD2DWControl>(shared_from_this());
			while (1) {
				if (pParentControl->m_pFocusedControl) {
					tunnelCurControls.push_back(pParentControl->m_pFocusedControl);
					pParentControl = pParentControl->m_pFocusedControl;
				} else {
					break;
				}
			}
			//New Focused
			std::vector<std::shared_ptr<CD2DWControl>> tunnelNewControls;
			pParentControl = std::dynamic_pointer_cast<CD2DWControl>(shared_from_this());
			while (1) {
				std::vector<std::shared_ptr<CD2DWControl>> childControls = pParentControl->m_childControls;
				auto iter = std::find_if(childControls.crbegin(), childControls.crend(),
					[&](const std::shared_ptr<CD2DWControl>& pChildControl) {
						return *pChildControl->IsEnabled && pChildControl->GetRectInWnd().PtInRect(e.PointInWnd);
					});
				if (iter != childControls.crend()) {
					tunnelNewControls.push_back(*iter);
					pParentControl = *iter;
				} else {
					break;
				}
			}

			//Kill Focus
			for (auto iter = tunnelCurControls.rbegin(); iter != tunnelCurControls.rend(); iter++) {
				if (std::find(tunnelNewControls.rbegin(), tunnelNewControls.rend(), *iter) == tunnelNewControls.rend()) {
					(*iter)->m_pParentControl->m_pFocusedControl = nullptr;
					(*iter)->OnKillFocus(KillFocusEvent(GetWndPtr(), 0, 0, nullptr));
				}
			}
			//Focus
			for (auto iter = tunnelNewControls.rbegin(); iter != tunnelNewControls.rend(); iter++) {
				if (std::find(tunnelCurControls.rbegin(), tunnelCurControls.rend(), *iter) == tunnelCurControls.rend()) {
					(*iter)->m_pParentControl->m_pFocusedControl = *iter;
					(*iter)->OnSetFocus(SetFocusEvent(GetWndPtr(), 0, 0, nullptr));
				}
			}
			//Event
			for (auto iter = tunnelNewControls.rbegin(); iter != tunnelNewControls.rend(); iter++) {
				(iter->get()->*bubble)(e);
				if (*e.HandledPtr) { break; }
			}
		}
		InvalidateRect(NULL, FALSE);
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
	virtual void OnSize(const SizeEvent& e);
	virtual void OnPaint(const PaintEvent& e) override;/* { ProcessMessageToAll(&CD2DWControl::OnPaint, e); }*/
	virtual void OnClosing(const ClosingEvent& e) override { ProcessMessageToAllReverse(&CUIElement::OnClosing, e); }
	virtual void OnClose(const CloseEvent& e) override { ProcessMessageToAllReverse(&CUIElement::OnClose, e); }

	//Mouse Message
	virtual void OnLButtonDown(const LButtonDownEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnLButtonDown, e); }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonUp, e); }
	virtual void OnLButtonClk(const LButtonClkEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonClk, e); }
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonSnglClk, e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonDblClk, e); }

	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonBeginDrag, e); }
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnLButtonEndDrag, e); }
	
	virtual void OnRButtonDown(const RButtonDownEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnRButtonDown, e); }
	virtual void OnRButtonUp(const RButtonUpEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnRButtonUp, e); }
	virtual void OnContextMenu(const ContextMenuEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnContextMenu, e); }

	virtual void OnMButtonDown(const MButtonDownEvent& e) override { BubbleMouseMessageAndFocus(&CD2DWControl::OnMButtonDown, e); }
	virtual void OnMButtonUp(const MButtonUpEvent& e) override { BubbleMouseMessage(&CD2DWControl::OnMButtonUp, e); }

	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
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
};

