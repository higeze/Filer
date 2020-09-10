#include "TabControl.h"

CTabControl::CTabControl(CWnd* pWnd, const std::shared_ptr<TabControlProperty>& spProp)
	:m_pWnd(pWnd), m_spProp(spProp)
{
	//SelectedIndex
	m_selectedIndex.Changed.connect(
		[this](const NotifyChangedEventArgs<int>& e)->void {
			auto spData = m_itemsSource[m_selectedIndex.get()];
			m_spCurControl = m_itemsControlTemplate[typeid(*spData).name()](spData);
		});

}
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
	GetHeaderRects = [rects = std::vector<d2dw::CRectF>(), this]()mutable->std::vector<d2dw::CRectF>&
	{
		if (rects.empty()) {
			FLOAT minWidth = 30.f;
			if (m_selectedIndex.get() >= 0) {
				auto rcInWnd = GetRectInWnd();
				auto left = rcInWnd.left;
				for (auto pData : m_itemsSource) {
					auto iter = m_itemsHeaderTemplate.find(typeid(*pData).name());
					if (iter != m_itemsHeaderTemplate.end()) {
						auto text = iter->second.operator()(pData);
						auto size = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_spProp->Format), text);
						auto rect = d2dw::CRectF(
							left,
							rcInWnd.top,
							left + m_spProp->Padding->left + (std::max)(size.width, minWidth) + m_spProp->Padding->right,
							rcInWnd.top + m_spProp->Padding->top + size.height + m_spProp->Padding->bottom);
						left += rect.Width();
						rects.push_back(rect);
					}
				}
			}
		}
		return rects;
	};

	GetContentRect = [rect = d2dw::CRectF(), this]()mutable->d2dw::CRectF&
	{
		rect = GetRectInWnd();
		auto rects = GetHeaderRects();
		auto iter = std::max_element(rects.begin(), rects.end(), [](const d2dw::CRectF& left, const d2dw::CRectF& right)->bool
		{return left.Height() > right.Height(); });
		rect.top += (iter != rects.end()) ? iter->Height() : 0;
		return rect;
	};

	GetControlRect = [rect = d2dw::CRectF(), this]()mutable->d2dw::CRectF&
	{
		rect = GetContentRect(); rect.DeflateRect(*(m_spProp->Padding));
		return rect;
	};

	//Show Current view
	if (m_selectedIndex.get() > 0) {
		auto pData = m_itemsSource[m_selectedIndex.get()];
		auto iter = m_itemsControlTemplate.find(typeid(*pData).name());
		if (iter != m_itemsControlTemplate.end()) {
			m_spCurControl = iter->second.operator()(pData);
		}
	}
	//
}

void CTabControl::OnPaint(const PaintEvent& e)
{
	if (m_selectedIndex.get() >= 0) {
		auto rc = GetRectInWnd();
		//Header
		auto headerRects = GetHeaderRects();
		for (size_t i = 0; i < m_itemsSource.size(); i++) {
			auto headerRc = headerRects[i];
			GetWndPtr()->GetDirectPtr()->FillSolidRectangle(i == m_selectedIndex.get() ? *(m_spProp->SelectedFill) : *(m_spProp->NormalFill), headerRc);
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), d2dw::CPointF(headerRc.left, headerRc.bottom), headerRc.LeftTop());
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), headerRc.LeftTop(), d2dw::CPointF(headerRc.right, headerRc.top));
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), d2dw::CPointF(headerRc.right, headerRc.top), d2dw::CPointF(headerRc.right, headerRc.bottom));

			auto iter = m_itemsHeaderTemplate.find(typeid(*m_itemsSource[i]).name());
			auto text = iter->second.operator()(m_itemsSource[i]);
			if (!text.empty()) {
				d2dw::CRectF textRc = headerRc; textRc.DeflateRect(*(m_spProp->Padding));
				GetWndPtr()->GetDirectPtr()->DrawTextLayout(*(m_spProp->Format), text, textRc);
			}
		}
	
		//Content
		auto contentRc = GetContentRect();
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetIsFocused()?*(m_spProp->SelectedFill):*(m_spProp->UnfocusSelectedFill), contentRc);
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), contentRc.LeftTop(), d2dw::CPointF(contentRc.left, contentRc.bottom));
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), d2dw::CPointF(contentRc.left, contentRc.bottom), d2dw::CPointF(contentRc.right, contentRc.bottom));
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), d2dw::CPointF(contentRc.right, contentRc.bottom), d2dw::CPointF(contentRc.right, contentRc.top));

		//Control
		auto controlRc = GetControlRect();
		m_spCurControl->OnRect(RectEvent(GetWndPtr(), controlRc));
		m_spCurControl->OnPaint(e);
	}

}
void CTabControl::OnCommand(const CommandEvent& e)
{
	//switch (e.ID) {
	//	//TOTODODO
	//}
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
		[&](const d2dw::CRectF& rc)->bool { return rc.PtInRect(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

	if (iter != headerRects.end()) {
		m_selectedIndex.notify_set(iter - headerRects.begin());
	} else {
		m_spCurControl->OnLButtonDown(e);
	}
}

void CTabControl::OnLButtonUp(const LButtonUpEvent& e)
{
	m_spCurControl->OnLButtonUp(e);
}
void CTabControl::OnLButtonClk(const LButtonClkEvent& e)
{
	m_spCurControl->OnLButtonClk(e);
}
void CTabControl::OnLButtonSnglClk(const LButtonSnglClkEvent& e)
{
	m_spCurControl->OnLButtonSnglClk(e);
}
void CTabControl::OnLButtonDblClk(const LButtonDblClkEvent& e) 
{
	m_spCurControl->OnLButtonDblClk(e);
}
void CTabControl::OnLButtonBeginDrag(const LButtonBeginDragEvent& e) 
{
	m_spCurControl->OnLButtonBeginDrag(e);
}
void CTabControl::OnRButtonDown(const RButtonDownEvent& e)
{
	m_spCurControl->OnRButtonDown(e);
}
void CTabControl::OnMButtonDown(const MouseEvent& e)
{
	m_spCurControl->OnMButtonDown(e);
}
void CTabControl::OnMButtonUp(const MouseEvent& e)
{
	m_spCurControl->OnMButtonUp(e);
}
void CTabControl::OnMouseMove(const MouseMoveEvent& e)
{
	m_spCurControl->OnMouseMove(e);
}
void CTabControl::OnMouseEnter(const MouseEvent& e)
{
	m_spCurControl->OnMouseEnter(e);
}
void CTabControl::OnMouseLeave(const MouseLeaveEvent& e)
{
	m_spCurControl->OnMouseLeave(e);
}
void CTabControl::OnMouseWheel(const MouseWheelEvent& e)
{
	m_spCurControl->OnMouseWheel(e);
}
void CTabControl::OnKeyDown(const KeyDownEvent& e)
{
	m_spCurControl->OnKeyDown(e);
}
void CTabControl::OnSysKeyDown(const SysKeyDownEvent& e)
{
	m_spCurControl->OnSysKeyDown(e);
}
void CTabControl::OnChar(const CharEvent& e)
{
	m_spCurControl->OnChar(e);
}
void CTabControl::OnContextMenu(const ContextMenuEvent& e)
{
	m_spCurControl->OnContextMenu(e);
}
void CTabControl::OnSetFocus(const SetFocusEvent& e)
{
	m_spCurControl->OnSetFocus(e);
}
void CTabControl::OnSetCursor(const SetCursorEvent& e)
{
	m_spCurControl->OnSetCursor(e);
}
void CTabControl::OnKillFocus(const KillFocusEvent& e)
{
	m_spCurControl->OnKillFocus(e);
}




