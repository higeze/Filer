#include "ToolTip.h"
#include "D2DWWindow.h"

CSizeF CToolTip::MeasureOverride(const CSizeF& availableSize)
{
	return GetWndPtr()->GetDirectPtr()->CalcTextSize(m_spProp->Format, *Content)
		+ CSizeF(m_spProp->Line.Width * 2.f, m_spProp->Line.Width * 2.f)
		+ CSizeF(m_spProp->Radius * 2.f, m_spProp->Radius * 2.f)
		+ CSizeF(m_spProp->Padding.left + m_spProp->Padding.right, m_spProp->Padding.top + m_spProp->Padding.bottom);
}

void CToolTip::OnCreate(const CreateEvt& e)
{
	m_pParentControl = e.ParentPtr;
	m_rect =  e.RectF;
	auto p = shared_from_this();
	GetWndPtr()->SetToolTipControlPtr(std::dynamic_pointer_cast<CToolTip>(shared_from_this()));
}

void CToolTip::OnDestroy(const DestroyEvent& e)
{
	GetWndPtr()->SetToolTipControlPtr(nullptr);
}

void CToolTip::OnPaint(const PaintEvent& e)
{
	if (Content->empty()) { return; }
	//GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	CRectF renderRect = RenderRect();
	GetWndPtr()->GetDirectPtr()->FillSolidRoundedRectangle(m_spProp->BackgroundFill, renderRect, m_spProp->Radius, m_spProp->Radius);
	GetWndPtr()->GetDirectPtr()->DrawSolidRoundedRectangle(m_spProp->Line, renderRect, m_spProp->Radius, m_spProp->Radius);

	CRectF contentRect = renderRect
		- CRectF(m_spProp->Line.Width, m_spProp->Line.Width, m_spProp->Line.Width, m_spProp->Line.Width)
		- CRectF(m_spProp->Radius, m_spProp->Radius, m_spProp->Radius, m_spProp->Radius)
		- m_spProp->Padding;
	GetWndPtr()->GetDirectPtr()->DrawTextInRect(m_spProp->Format, *Content, contentRect);

	//GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PopAxisAlignedClip();
}
