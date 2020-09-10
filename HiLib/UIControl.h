#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"

namespace d2dw
{

class CWindow;

class CUIControl: public CUIElement
{
protected:
	CWindow* m_pWnd;
	CRectF m_rect;
public:

	CUIControl(CWindow* pWnd):m_pWnd(pWnd){}
	virtual CWindow* GetWndPtr()const { return m_pWnd; }
	virtual CRectF GetRectInWnd()const override { return m_rect; }

	virtual bool GetIsFocused()const;
};

}
