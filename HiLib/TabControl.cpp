#include "TabControl.h"

CTabControl::CTabControl(CWnd* pWnd):m_pWnd(pWnd){}
CTabControl::~CTabControl() = default;

/***********/
/* Command */
/***********/
void CTabControl::OnCommandCloneTab(const CommandEvent& e)
{}
void CTabControl::OnCommandCloseTab(const CommandEvent& e)
{}
void CTabControl::OnCommandCloseAllButThisTab(const CommandEvent& e)
{}

void CTabControl::OnCreate(const CreateEvent& e) 
{
	//Show Current view
	if (m_selectedIndex.get() > 0) {
		auto pData = m_itemsSource[m_selectedIndex.get()];
		auto iter = m_itemsContentTemplate.find(typeid(*pData).name());
		if (iter != m_itemsContentTemplate.end()) {
			m_spCurControl = iter->second.operator()(pData);
		}
	}
	//
	GetHeaderRects = [rects = std::vector<d2dw::CRectF>(), this]()mutable->std::vector<d2dw::CRectF>&
	{
		if (rects.empty()) {
			d2dw::FormatF format;
			d2dw::SolidLine line;
			d2dw::CRectF padding(4, 4, 4, 4);
			if (m_selectedIndex.get() > 0) {
				auto rc = GetRectInWnd();
				for (auto pData : m_itemsSource) {
					auto iter = m_itemsHeaderTemplate.find(typeid(*pData).name());
					auto left = rc.left;
					if (iter != m_itemsHeaderTemplate.end()) {
						auto text = iter->second.operator()(pData);
						auto size = GetWndPtr()->GetDirectPtr()->CalcTextSize(format, text);
						rects.emplace_back(
							left + padding.left,
							rc.top + padding.top,
							left + padding.left + size.width + padding.right,
							rc.top + padding.top + size.height + padding.bottom);
					}
				}
			}
		}
		return rects;
	};

	GetContentRect = [rect = d2dw::CRectF(), this]()mutable->d2dw::CRectF
	{
		auto rc = GetRectInWnd();
		return d2dw::CRectF(rc.left, rc.top + GetHeaderRects()[0].Height(), rc.right, rc.bottom);
	}
}

void CTabControl::OnPaint(const PaintEvent& e)
{
	d2dw::FormatF format;
	d2dw::SolidLine line;
	d2dw::CRectF padding(4, 4, 4, 4);


	if (m_selectedIndex.get() > 0) {
		auto rc = GetRectInWnd();
		//Header
		auto headerRects = GetHeaderRects();
		for (auto i = 0; i < m_itemsSource.size(); i++) {
			auto headerRc = headerRects[i];
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(line, d2dw::CPointF(headerRc.left, headerRc.bottom), headerRc.LeftTop());
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(line, headerRc.LeftTop(), d2dw::CPointF(headerRc.right, headerRc.top));
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(line, d2dw::CPointF(headerRc.right, headerRc.top), d2dw::CPointF(headerRc.right, headerRc.bottom));

			auto iter = m_itemsHeaderTemplate.find(typeid(*m_itemsSource[i]).name());
			auto text = iter->second.operator()(m_itemsSource[i]);
			d2dw::CRectF textRc = headerRc; textRc.DeflateRect(padding);
			GetWndPtr()->GetDirectPtr()->DrawTextLayout(format, text, textRc);
		}
	
		//Content
		auto contentRc = GetContentRect();
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(d2dw::SolidFill(), contentRc);
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(line, contentRc.LeftTop(), d2dw::CPointF(contentRc.left, contentRc.bottom));
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(line, d2dw::CPointF(contentRc.left, contentRc.bottom), d2dw::CPointF(contentRc.right, contentRc.bottom));
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(line, d2dw::CPointF(contentRc.right, contentRc.bottom), d2dw::CPointF(contentRc.right, contentRc.top));

		//Control
		d2dw::CRectF controlRc(contentRc); controlRc.DeflateRect(padding);
		m_spCurControl->OnRect(RectEvent(GetWndPtr(), controlRc));
		m_spCurControl->OnPaint(e);
	}

}
void CTabControl::OnCommand(const CommandEvent& e)
{
	switch (e.ID) {
		//TOTODODO
	}
}
void CTabControl::OnRect(const RectEvent& e)
{ 
	m_rect = e.Rect;
	GetHeaderRects().clear();
	GetContentRect().SetRect(0, 0, 0, 0);
}

void CTabControl::OnLButtonDown(const LButtonDownEvent& e)
{
	auto headerRects = GetHeaderRects();
	auto iter = std::find_if(headerRects.begin(), headerRects.end(),
		[&](const d2dw::CRectF& rc)->bool { rc.PtInRect(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

	if (iter != headerRects.end()) {
		m_selectedIndex.notify_set(iter - headerRects.begin());
	} else {
		m_spCurControl->OnLButtonDown(e);
	}


}
	virtual void CTabControl::OnLButtonUp(const LButtonUpEvent& e);
	virtual void CTabControl::OnLButtonClk(const LButtonClkEvent& e) {}
	virtual void CTabControl::OnLButtonSnglClk(const LButtonSnglClkEvent& e) {}
	virtual void CTabControl::OnLButtonDblClk(const LButtonDblClkEvent& e) {}
	virtual void CTabControl::OnLButtonBeginDrag(const LButtonBeginDragEvent& e) {}

	virtual void CTabControl::OnRButtonDown(const RButtonDownEvent& e) {}

	virtual void CTabControl::OnMButtonDown(const MouseEvent& e){}//TODO
	virtual void CTabControl::OnMButtonUp(const MouseEvent& e){}//TODO

	virtual void CTabControl::OnMouseMove(const MouseMoveEvent& e);
	virtual void CTabControl::OnMouseEnter(const MouseEvent& e);//TODO
	virtual void CTabControl::OnMouseLeave(const MouseLeaveEvent& e);
	virtual void CTabControl::OnMouseWheel(const MouseWheelEvent& e){}

	virtual void CTabControl::OnKeyDown(const KeyDownEvent& e) {}
	virtual void CTabControl::OnSysKeyDown(const SysKeyDownEvent& e){}
	virtual void CTabControl::OnChar(const CharEvent& e) {}
	virtual void CTabControl::OnContextMenu(const ContextMenuEvent& e){}
	virtual void CTabControl::OnSetFocus(const SetFocusEvent& e) {}
	virtual void CTabControl::OnSetCursor(const SetCursorEvent& e) {}
	virtual void CTabControl::OnKillFocus(const KillFocusEvent& e) {}




