#include "TabControl.h"
#include "D2DWWindow.h"
#include "Dispatcher.h"
#include <fmt/format.h>

CTabHeaderControl::CTabHeaderControl(CTabControl* pTabControl, const std::shared_ptr<TabHeaderControlProperty>& spProp)
	:CD2DWControl(pTabControl),m_spProp(spProp), m_spButton(std::make_shared<CButton>(this, spProp->ButtonProp))
{
	GetIsFocusable().set(false);
	m_spButton->GetCommand().Subscribe([this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke(
			[pTabControl = static_cast<CTabControl*>(m_pParentControl), index = GetIndex()]()->void
			{
				if (pTabControl->m_itemsSource[index]->ClosingFunction()) {
					pTabControl->m_itemsSource.erase(pTabControl->m_itemsSource.cbegin() + index);
				}
			}
		);
	});

	m_spButton->GetContent().set(L"x");
	m_spButton->GetIsFocusable().set(false);
}


bool CTabHeaderControl::GetIsSelected()const
{
	return GetIndex() == static_cast<CTabControl*>(m_pParentControl)->m_selectedIndex;
}

std::tuple<CSizeF, CSizeF, CSizeF, CSizeF> CTabHeaderControl::MeasureSizes()
{
	auto iconSize = CSizeF(16.f, 16.f);
	auto textSize = CSizeF();
	auto buttonSize = CSizeF(16.f, 16.f);

	auto pTabControl = static_cast<CTabControl*>(m_pParentControl);
	auto pItem = pTabControl->m_itemsSource[GetIndex()];
	auto iterHeader = pTabControl->m_itemsHeaderTemplate.find(typeid(*pItem).name());
	if (iterHeader != pTabControl->m_itemsHeaderTemplate.end() && GetWndPtr()->GetDirectPtr()) {
		auto text = iterHeader->second.operator()(pItem);
		textSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_spProp->Format), text);
	}

	auto size = CSizeF(
			m_spProp->Padding->left + iconSize.width + m_spProp->Padding->right
			+ (std::max)(textSize.width, m_minWidth) + m_spProp->Padding->right
			+ buttonSize.width + m_spProp->Padding->right,
			m_spProp->Padding->top + textSize.height + m_spProp->Padding->bottom);

	return { iconSize, textSize, buttonSize, size};
}

std::tuple<CSizeF, CSizeF, CSizeF, CSizeF> CTabHeaderControl::GetSizes()
{
	if (!m_isMeasureValid) {
		auto [iconSize, textSize, buttonSize, size] = MeasureSizes();
		m_iconSize = iconSize; m_textSize = textSize; m_buttonSize = buttonSize; m_size = size;
		if (m_textSize.width && m_textSize.height ) {
			m_isMeasureValid = true;
		}
	}
	return { m_iconSize, m_textSize, m_buttonSize, m_size };
}

CSizeF CTabHeaderControl::GetSize()
{
	return std::get<3>(GetSizes());
}

std::tuple<CRectF, CRectF, CRectF, CRectF> CTabHeaderControl::GetRects()
{
	auto rc = GetRectInWnd();
	auto [iconSize, textSize, buttonSize, size] = GetSizes();
	auto iconRect = CRectF(iconSize);
	auto textRect = CRectF(textSize);
	auto buttonRect = CRectF(buttonSize);
	iconRect.MoveToXY(rc.left + m_spProp->Padding->left, rc.top + m_spProp->Padding->top );
	textRect.MoveToXY(iconRect.right + m_spProp->Padding->left, rc.top + m_spProp->Padding->top );
	buttonRect.MoveToXY(rc.right - m_spProp->Padding->right - buttonSize.width, rc.top + m_spProp->Padding->top );

	return { iconRect, textRect, buttonRect, rc };
}

void CTabHeaderControl::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	m_spButton->OnCreate(CreateEvt(GetWndPtr(), CRectF()));
}

void CTabHeaderControl::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	m_spButton->OnRect(RectEvent(GetWndPtr(), std::get<2>(GetRects())));
}

void CTabHeaderControl::OnPaint(const PaintEvent& e)
{
	auto pTabControl = static_cast<CTabControl*>(m_pParentControl);
	auto [iconRect, textRect, buttonRect, rect] = GetRects();
	auto bkgndFill = GetIsSelected() ?
		(pTabControl->GetIsFocused() ?
			*(pTabControl->m_spProp->SelectedFill) :
			*(pTabControl->m_spProp->UnfocusSelectedFill)) :
		*(pTabControl->m_spProp->NormalFill);

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(bkgndFill, rect);

	if (auto pt = GetWndPtr()->GetDirectPtr()->Pixels2Dips(GetWndPtr()->GetCursorPosInClient()); rect.PtInRect(pt)) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(pTabControl->m_spProp->HotFill), rect);
	}
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(pTabControl->m_spProp->Line), CPointF(rect.left, rect.bottom), rect.LeftTop());
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(pTabControl->m_spProp->Line), rect.LeftTop(), CPointF(rect.right, rect.top));
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(pTabControl->m_spProp->Line), CPointF(rect.right, rect.top), CPointF(rect.right, rect.bottom));

	if (GetIndex() == pTabControl->GetSelectedIndex()) {
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(bkgndFill.Color, pTabControl->m_spProp->Line->Width, CPointF(rect.left, rect.bottom), CPointF(rect.right, rect.bottom));
	} else {
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(pTabControl->m_spProp->Line), CPointF(rect.left, rect.bottom), CPointF(rect.right, rect.bottom));
	}

	auto iterIcon = pTabControl->m_itemsHeaderIconTemplate.find(typeid(*pTabControl->m_itemsSource[GetIndex()]).name());
	GetWndPtr()->GetDirectPtr()->DrawBitmap(iterIcon->second.operator()(pTabControl->m_itemsSource[GetIndex()]),iconRect);

	auto iterText = pTabControl->m_itemsHeaderTemplate.find(typeid(*pTabControl->m_itemsSource[GetIndex()]).name());
	auto text = iterText->second.operator()(pTabControl->m_itemsSource[GetIndex()]);
	if (!text.empty()) {
		GetWndPtr()->GetDirectPtr()->DrawTextLayout(*(m_spProp->Format), text, textRect);
	}

	m_spButton->OnPaint(e);
}

void CTabHeaderControl::OnLButtonDown(const LButtonDownEvent& e)
{
	if (m_spButton->GetRectInWnd().PtInRect(e.PointInWnd)) {
		m_spButton->OnLButtonDown(e);
	} else {
		auto pTabControl = static_cast<CTabControl*>(m_pParentControl);
		pTabControl->m_selectedIndex.set(GetIndex());
	}
}

CTabControl::CTabControl(CD2DWControl* pParentControl, const std::shared_ptr<TabControlProperty>& spProp)
	:CD2DWControl(pParentControl), m_spProp(spProp), m_selectedIndex(-1), 
	m_headers([](std::shared_ptr<CTabHeaderControl>& sp, size_t idx) { sp->SetIndex(idx); })
{
	//ItemsSource
	m_itemsSource.SubscribeDetail(
		[this](const NotifyVectorChangedEventArgs<std::shared_ptr<TabData>>& e)->void
		{
			switch (e.Action) {
				case NotifyVectorChangedAction::Add:
				{
					auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
					m_headers.idx_push_back(header);
					header->OnCreate(CreateEvt(GetWndPtr(), CRectF()));
					UpdateHeaderRects();

					m_selectedIndex.force_notify_set((std::min)((size_t)e.NewStartingIndex, m_itemsSource.size() - 1));
					break;
				}
				case NotifyVectorChangedAction::Insert:
				{
					auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
					m_headers.idx_insert(m_headers.cbegin() + e.NewStartingIndex, header);
					header->OnCreate(CreateEvt(GetWndPtr(), CRectF()));
					UpdateHeaderRects();

					m_selectedIndex.force_notify_set((std::min)((size_t)e.NewStartingIndex, m_itemsSource.size() - 1));
					break;
				}
				case NotifyVectorChangedAction::Remove:
				{
					m_headers[e.OldStartingIndex]->OnDestroy(DestroyEvent(GetWndPtr()));
					m_headers.idx_erase(m_headers.cbegin() + e.OldStartingIndex);
					UpdateHeaderRects();

					if (!m_itemsSource.empty()) {
						m_selectedIndex.force_notify_set((std::min)((size_t)e.OldStartingIndex, m_itemsSource.size() - 1));
					}
					break;
				}
				case NotifyVectorChangedAction::Replace:
				{
					m_headers[e.NewStartingIndex]->OnDestroy(DestroyEvent(GetWndPtr()));
					m_headers.idx_erase(m_headers.cbegin() + e.NewStartingIndex);
					auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
					m_headers.idx_insert(m_headers.cbegin() + e.NewStartingIndex, header);
					header->OnCreate(CreateEvt(GetWndPtr(), CRectF()));
					UpdateHeaderRects();

					m_selectedIndex.force_notify_set((std::min)((size_t)e.NewStartingIndex, m_itemsSource.size() - 1));
					break;
				}
				case NotifyVectorChangedAction::Reset:
				{
					if (!m_itemsSource.empty()) {
						m_headers.clear();
						for (auto& item : e.NewItems) {
							auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
							m_headers.idx_push_back(header);
							header->OnCreate(CreateEvt(GetWndPtr(), CRectF()));
						}
						UpdateHeaderRects();
						m_selectedIndex.force_notify_set((std::min)((size_t)m_selectedIndex.get(), m_itemsSource.size() - 1));
					}
					break;
				}
				default:
					break;
			}
		});
	//SelectedIndex
	m_selectedIndex.Subscribe( 
		[this](const int& value)->void {
			if (GetWndPtr()->m_hWnd) {
				auto spData = m_itemsSource[value];
				auto spControl = m_itemsControlTemplate[typeid(*spData).name()](spData);
				if (m_spCurControl != spControl) {
					if (m_spCurControl) {
						m_spCurControl->GetIsEnabled().set(false);
					}
					m_spCurControl = spControl;
					m_spCurControl->GetIsEnabled().set(true);
					SetFocusedControlPtr(spControl);
				}
			}
			//for (auto p : m_childControls) {
			//	if (std::string(typeid(*p).name()).find("CFilerGridView") != std::string::npos) {
			//		::OutputDebugStringA(fmt::format("{}:{}", typeid(*p).name(), p->GetIsEnabled().get()).c_str());
			//	} else if (std::string(typeid(*p).name()).find("CToDoGridView") != std::string::npos) {
			//		::OutputDebugStringA(fmt::format("{}:{}", typeid(*p).name(), p->GetIsEnabled().get()).c_str());
			//	} else if (std::string(typeid(*p).name()).find("CTextEditor") != std::string::npos){
			//		::OutputDebugStringA(fmt::format("{}:{}", typeid(*p).name(), p->GetIsEnabled().get()).c_str());
			//	}
			//}
		});


	GetContentRect = [rect = CRectF(), this]()mutable->CRectF&
	{
		rect = GetRectInWnd();
		if (!m_headers.empty()) {
			rect.top = m_headers.back()->GetRectInWnd().bottom;
		}
		return rect;
	};

	GetControlRect = [rect = CRectF(), this]()mutable->CRectF&
	{
		rect = GetContentRect(); rect.DeflateRect(*(m_spProp->Padding));
		return rect;
	};

}
CTabControl::~CTabControl() = default;

std::optional<size_t> CTabControl::GetPtInHeaderRectIndex(const CPointF& pt)const
{
	auto iter = std::find_if(m_headers.cbegin(), m_headers.cend(),
		[&](const auto& spHeader)->bool { return spHeader->GetRectInWnd().PtInRect(pt); });
	if (iter != m_headers.cend()) {
		return iter - m_headers.begin();
	} else {
		return std::nullopt;
	}
}

void CTabControl::UpdateHeaderRects()
{
	auto rcInWnd = GetRectInWnd();
	auto left = rcInWnd.left;
	auto top = rcInWnd.top;

	for (auto& pHeader : m_headers) {
		pHeader->SetMeasureValid(false);//TODO HIGH

		auto hdrSize = pHeader->GetSize();
		if (left + hdrSize.width > rcInWnd.right && left != rcInWnd.left) {
			left = rcInWnd.left;
			top += hdrSize.height;
		}

		pHeader->OnRect(RectEvent(GetWndPtr(),
			CRectF(left, top, left + hdrSize.width, top + hdrSize.height)));
		left = pHeader->GetRectInWnd().right;
	}
}

/***********/
/* Command */
/***********/
void CTabControl::OnCommandCloneTab(const CommandEvent& e)
{
	//TODOLOW should deep clone
	m_itemsSource.push_back(m_itemsSource[m_selectedIndex.get()]);
	m_selectedIndex.set(m_selectedIndex - 1);
}

void CTabControl::OnCommandCloseTab(const CommandEvent& e)
{
	if (m_contextIndex) {
		if (m_itemsSource[m_contextIndex.value()]->ClosingFunction()) {
			m_itemsSource.erase(m_itemsSource.cbegin() + m_contextIndex.value());
		}
	}
}
void CTabControl::OnCommandCloseAllButThisTab(const CommandEvent& e)
{
	if (m_contextIndex) {
		//Erase larger tab
		for (auto iter = m_itemsSource.cbegin() + (m_contextIndex.value() + 1), end = m_itemsSource.cend(); iter != end; ++iter) {
			if (!((*iter)->ClosingFunction())) {
				return;
			}
		}
		m_itemsSource.erase(m_itemsSource.cbegin() + (m_contextIndex.value() + 1), m_itemsSource.cend());
		
		//Erase smaller tab
		for (auto iter = m_itemsSource.cbegin(), end = m_itemsSource.cbegin() + m_contextIndex.value(); iter != end; ++iter) {
			if (!((*iter)->ClosingFunction())) {
				return;
			}
		}
		m_itemsSource.erase(m_itemsSource.cbegin(), m_itemsSource.cbegin() + m_contextIndex.value());

		m_selectedIndex.force_notify_set(0);
	}
}

/**************/
/* UI Message */
/**************/

void CTabControl::OnCreate(const CreateEvt& e) 
{
	CD2DWControl::OnCreate(e);
}

void CTabControl::OnPaint(const PaintEvent& e)
{
	if (m_selectedIndex.get() >= 0) {
		auto rc = GetRectInWnd();
		//Content
		const auto& contentRc = GetContentRect();
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetIsFocused()?*(m_spProp->SelectedFill):*(m_spProp->UnfocusSelectedFill), contentRc);
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangle(*(m_spProp->Line), contentRc);
		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), contentRc.LeftTop(), CPointF(contentRc.left, contentRc.bottom));
		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), CPointF(contentRc.left, contentRc.bottom), CPointF(contentRc.right, contentRc.bottom));
		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), CPointF(contentRc.right, contentRc.bottom), CPointF(contentRc.right, contentRc.top));

		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), 
		//	CPointF(m_headers.front()->GetRectInWnd().left, m_headers.front()->GetRectInWnd().bottom),
		//	CPointF(m_headers[m_selectedIndex.get()]->GetRectInWnd().left, m_headers[m_selectedIndex]->GetRectInWnd().bottom));
		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), 
		//	CPointF(m_headers[m_selectedIndex.get()]->GetRectInWnd().right, m_headers[m_selectedIndex]->GetRectInWnd().bottom),
		//	CPointF(rc.right, m_headers.back()->GetRectInWnd().bottom) );

		//Header
		for (const auto& pHeader : m_headers) {
			pHeader->OnPaint(e);
		}

		//Control
		m_spCurControl->OnPaint(e);
	}

}

void CTabControl::OnClosing(const ClosingEvent& e)
{ 
	for (auto& item : m_itemsSource) 		{
		if (!(item->ClosingFunction())) {
			*(e.CancelPtr) = true;
			return;
		}
	}

	CD2DWControl::OnClosing(e);
}

void CTabControl::OnRect(const RectEvent& e)
{ 
	CD2DWControl::OnRect(e);
	UpdateHeaderRects();
	GetContentRect().SetRect(0, 0, 0, 0);
	GetControlRect().SetRect(0, 0, 0, 0);
	if (m_spCurControl) { m_spCurControl->OnRect(RectEvent(GetWndPtr(), GetControlRect())); }
}

void CTabControl::OnContextMenu(const ContextMenuEvent& e)
{
	m_contextIndex = GetPtInHeaderRectIndex(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}

void CTabControl::OnSetCursor(const SetCursorEvent& e)
{
	CD2DWControl::OnSetCursor(e);
	if (!*e.HandledPtr) {
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		*(e.HandledPtr) = true;
	}
}



