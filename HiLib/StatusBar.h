#pragma once
#include "Direct2DWrite.h"
#include "D2DWControl.h"
#include "StatusBarProperty.h"

/**************/
/* CStatusBar */
/**************/
class CStatusBar : public CD2DWControl
{
protected:
	std::shared_ptr<StatusBarProperty> m_spStatusBarProp;
	std::wstring m_text;
public:
	CStatusBar(CD2DWControl* pParentControl, const std::shared_ptr<StatusBarProperty>& spStatusBarProp);
	virtual ~CStatusBar();

	void SetText(const std::wstring& text) { m_text = text; }
	CSizeF GetSize()const { return CSizeF(m_rect.Width(), m_rect.Height()); }
	virtual void OnPaint(const PaintEvent& e);
	virtual CSizeF MeasureSize(const std::wstring& text);
};