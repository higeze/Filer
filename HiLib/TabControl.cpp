#include "TabControl.h"
#include "D2DWWindow.h"

CTabControl::CTabControl(CD2DWControl* pParentControl, const std::shared_ptr<TabControlProperty>& spProp)
	:CD2DWControl(pParentControl), m_spProp(spProp)
{
	//ItemsSource
	m_itemsSource.VectorChanged =
		[this](const NotifyVectorChangedEventArgs<std::shared_ptr<TabData>>& e)->void
	{
		switch (e.Action) {
			case NotifyVectorChangedAction::Add:
			case NotifyVectorChangedAction::Insert:
				GetHeaderRects().clear();
				m_selectedIndex.force_notify_set((std::min)((size_t)e.NewStartingIndex, m_itemsSource.size()));
				break;
			case NotifyVectorChangedAction::Remove:
				GetHeaderRects().clear();
				m_selectedIndex.force_notify_set((std::min)((size_t)e.OldStartingIndex, m_itemsSource.size()));
				break;
			case NotifyVectorChangedAction::Replace:
				GetHeaderRects().clear();
				m_selectedIndex.force_notify_set((std::min)((size_t)e.NewStartingIndex, m_itemsSource.size()));
				break;
			case NotifyVectorChangedAction::Reset:
				GetHeaderRects().clear();
				m_selectedIndex.force_notify_set((std::min)((size_t)m_selectedIndex.get(), m_itemsSource.size()));
			default:
				break;
		}
	};
	//SelectedIndex
	m_selectedIndex.Changed = 
		[this](const NotifyChangedEventArgs<int>& e)->void {
			auto spData = m_itemsSource[m_selectedIndex.get()];
			m_spCurControl = m_itemsControlTemplate[typeid(*spData).name()](spData);
		};
	GetHeaderRects = [rects = std::vector<CRectF>(), this]()mutable->std::vector<CRectF>&
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
						auto rect = CRectF(
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

	GetContentRect = [rect = CRectF(), this]()mutable->CRectF&
	{
		rect = GetRectInWnd();
		auto rects = GetHeaderRects();
		auto iter = std::max_element(rects.begin(), rects.end(), [](const CRectF& left, const CRectF& right)->bool
		{return left.Height() > right.Height(); });
		rect.top += (iter != rects.end()) ? iter->Height() : 0;
		return rect;
	};

	GetControlRect = [rect = CRectF(), this]()mutable->CRectF&
	{
		rect = GetContentRect(); rect.DeflateRect(*(m_spProp->Padding));
		return rect;
	};

}
CTabControl::~CTabControl() = default;

/***********/
/* Command */
/***********/
void CTabControl::OnCommandCloneTab(const CommandEvent& e)
{
	//TODOLOW should deep clone
	m_itemsSource.notify_push_back(m_itemsSource[m_selectedIndex.get()]);
	m_selectedIndex.notify_set(m_selectedIndex - 1);
}
void CTabControl::OnCommandCloseTab(const CommandEvent& e)
{
	if(m_selectedIndex.get()>1){
		m_itemsSource.notify_erase(m_itemsSource.begin() + m_selectedIndex.get());
		m_selectedIndex.forth_notify_set((std::min)(m_selectedIndex.get() - 1, m_selectedIndex.get()));
	}
}
void CTabControl::OnCommandCloseAllButThisTab(const CommandEvent& e)
{
	if (m_selectedIndex.get()>1) {
		//Erase larger tab
		m_itemsSource.notify_erase(m_itemsSource.begin() + (m_selectedIndex.get() + 1), m_itemsSource.end());
		//Erase smaller tab
		m_itemsSource.notify_erase(m_itemsSource.begin(), m_itemsSource.begin() + m_selectedIndex.get());

		m_selectedIndex.forth_notify_set(0);
	}
}

/**************/
/* UI Message */
/**************/

void CTabControl::OnCreate(const CreateEvent& e) 
{
	CD2DWControl::OnCreate(e);
}

void CTabControl::OnPaint(const PaintEvent& e)
{
	if (m_selectedIndex.get() >= 0) {
		auto rc = GetRectInWnd();
		//Header
		auto headerRects = GetHeaderRects();
		for (size_t i = 0; i < m_itemsSource.size(); i++) {
			auto headerRc = headerRects[i];

			GetWndPtr()->GetDirectPtr()->FillSolidRectangle(i == m_selectedIndex.get() ? 
				(GetIsFocused()?*(m_spProp->SelectedFill):*(m_spProp->UnfocusSelectedFill)) :
				*(m_spProp->NormalFill), headerRc);
			if (auto pt = GetWndPtr()->GetDirectPtr()->Pixels2Dips(GetWndPtr()->GetCursorPosInClient()); headerRc.PtInRect(pt)) {
				GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_spProp->HotFill), headerRc);
			}
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), CPointF(headerRc.left, headerRc.bottom), headerRc.LeftTop());
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), headerRc.LeftTop(), CPointF(headerRc.right, headerRc.top));
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), CPointF(headerRc.right, headerRc.top), CPointF(headerRc.right, headerRc.bottom));

			auto iter = m_itemsHeaderTemplate.find(typeid(*m_itemsSource[i]).name());
			auto text = iter->second.operator()(m_itemsSource[i]);
			if (!text.empty()) {
				CRectF textRc = headerRc; textRc.DeflateRect(*(m_spProp->Padding));
				GetWndPtr()->GetDirectPtr()->DrawTextLayout(*(m_spProp->Format), text, textRc);
			}
		}
	
		//Content
		auto contentRc = GetContentRect();
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetIsFocused()?*(m_spProp->SelectedFill):*(m_spProp->UnfocusSelectedFill), contentRc);
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), contentRc.LeftTop(), CPointF(contentRc.left, contentRc.bottom));
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), CPointF(contentRc.left, contentRc.bottom), CPointF(contentRc.right, contentRc.bottom));
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), CPointF(contentRc.right, contentRc.bottom), CPointF(contentRc.right, contentRc.top));

		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), 
			CPointF(headerRects.front().left, headerRects.front().bottom), CPointF(headerRects[m_selectedIndex.get()].left, headerRects[m_selectedIndex].bottom));
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), 
			CPointF(headerRects[m_selectedIndex.get()].right, headerRects[m_selectedIndex].bottom), CPointF(rc.right,headerRects.back().bottom) );

		//Control
		m_spCurControl->OnPaint(e);
	}

}

void CTabControl::OnRect(const RectEvent& e)
{ 
	CD2DWControl::OnRect(e);
	GetHeaderRects().clear();
	GetContentRect().SetRect(0, 0, 0, 0);
	GetControlRect().SetRect(0, 0, 0, 0);
	if (m_spCurControl) { m_spCurControl->OnRect(RectEvent(GetWndPtr(), GetControlRect())); }
}

void CTabControl::OnLButtonDown(const LButtonDownEvent& e)
{
	auto headerRects = GetHeaderRects();
	auto iter = std::find_if(headerRects.begin(), headerRects.end(),
		[&](const CRectF& rc)->bool { return rc.PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient)); });

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
	if (GetControlRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient))) {
		m_spCurControl->OnMouseEnter(e);
	} else {
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		*(e.HandledPtr)= true;
	}
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
	CPointF pt = GetWndPtr()->GetDirectPtr()->Pixels2Dips((GetWndPtr()->GetCursorPosInClient()));
	if (GetRectInWnd().PtInRect(pt)) {
		if (GetControlRect().PtInRect(pt)) {
			m_spCurControl->OnSetCursor(e);
		} else {
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			*(e.HandledPtr) = true;
		}
	}
}
void CTabControl::OnKillFocus(const KillFocusEvent& e)
{
	m_spCurControl->OnKillFocus(e);
}




