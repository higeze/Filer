#pragma once
#include "MyWnd.h"
#include "UIElement.h"
#include "Direct2DWrite.h"
#include "observable.h"

class D2DTextbox;
struct TextEditorProperty;
class CMouseStateMachine;
class CUIElement;

class CTextboxWnd:public CWnd, public CUIElement
{
public:
	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;
	std::unique_ptr<CMouseStateMachine> m_pMouseMachine;
	std::shared_ptr<CUIElement> m_pControl;
	std::shared_ptr<TextEditorProperty> m_spProp;

public :
	CTextboxWnd(std::shared_ptr<TextEditorProperty> spProp);
	virtual ~CTextboxWnd();

	d2dw::CDirect2DWrite* GetDirectPtr() override { return  m_pDirect.get(); }
	bool GetIsFocused()const;
	std::shared_ptr<CUIElement>& GetControlPtr()
	{
		return m_pControl;
	}

	void Update();

private:
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


	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void OnFinalMessage(HWND hWnd) override;
};
