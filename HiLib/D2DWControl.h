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
	CRectF m_rect;
public:

	CD2DWControl(CD2DWControl* pParentControl):m_pParentControl(pParentControl){}
	virtual CD2DWWindow* GetWndPtr()const { return m_pParentControl->GetWndPtr(); }
	virtual CD2DWControl* GetParentControlPtr()const { return m_pParentControl; }
	virtual CRectF GetRectInWnd()const override { return m_rect; }
	virtual void OnCreate(const CreateEvt& e) override { m_rect = e.RectF; }
	virtual void OnCommand(const CommandEvent& e) override;
	virtual void OnRect(const RectEvent& e) override { m_rect = e.Rect; }
	virtual bool GetIsFocused()const;
	virtual bool GetIsFocusable()const { return true; }
};
