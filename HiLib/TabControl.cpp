#include "TabControl.h"
#include "TabControlProperty.h"
#include "D2DWWindow.h"
#include "Dispatcher.h"
#include <fmt/format.h>

namespace sml = boost::sml;

/***********/
/* TabData */
/***********/
bool TabData::AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing)
{
	if (!isWndClosing && !*Unlock) {
		int ync = pWnd->MessageBox(
		L"This tab is locked",
		L"Locked",
		MB_OK);
		return false;
	} else {
		return true;
	}
}

/*********************/
/* CTabHeaderControl */
/*********************/
CTabHeaderControl::CTabHeaderControl(CTabControl* pTabControl, const std::shared_ptr<TabHeaderControlProperty>& spProp)
	:CD2DWControl(pTabControl),m_spProp(spProp), m_spButton(std::make_shared<CButton>(this, spProp->ButtonProp))
{
	IsFocusable.set(false);
}

bool CTabHeaderControl::GetIsSelected()const
{
	return GetIndex() == *static_cast<CTabControl*>(m_pParentControl)->SelectedIndex;
}

std::tuple<CSizeF, CSizeF, CSizeF, CSizeF> CTabHeaderControl::MeasureSizes()
{
	auto iconSize = CSizeF(16.f, 16.f);
	auto textSize = CSizeF();
	auto buttonSize = CSizeF(16.f, 16.f);

	auto pTabControl = static_cast<CTabControl*>(m_pParentControl);
	auto pItem = pTabControl->ItemsSource->at(GetIndex());
	auto iterHeader = pTabControl->m_itemsHeaderTemplate.find(typeid(*pItem).name());
	if (iterHeader != pTabControl->m_itemsHeaderTemplate.end() && GetWndPtr()->GetDirectPtr()) {
		auto text = iterHeader->second.operator()(pItem);
		textSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(m_spProp->Format, text);
	}

	auto size = CSizeF(
			m_spProp->Padding.left + iconSize.width + m_spProp->Padding.right
			+ (std::max)(textSize.width, m_minWidth) + m_spProp->Padding.right
			+ buttonSize.width + m_spProp->Padding.right,
			m_spProp->Padding.top + textSize.height + m_spProp->Padding.bottom);

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
	iconRect.MoveToXY(rc.left + m_spProp->Padding.left, rc.top + m_spProp->Padding.top );
	textRect.MoveToXY(iconRect.right + m_spProp->Padding.left, rc.top + m_spProp->Padding.top );
	buttonRect.MoveToXY(rc.right - m_spProp->Padding.right - buttonSize.width, rc.top + m_spProp->Padding.top );

	return { iconRect, textRect, buttonRect, rc };
}

void CTabHeaderControl::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	
	//Set up Button
	m_spButton->Command.subscribe([this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke(
			[pTabControl = static_cast<CTabControl*>(m_pParentControl), index = GetIndex()]()->void
			{
				if (pTabControl->ItemsSource.get_unconst()->at(index)->AcceptClosing(pTabControl->GetWndPtr(), false)) {
					pTabControl->ItemsSource.erase(pTabControl->ItemsSource->cbegin() + index);
				}
			}
		);
	}, shared_from_this());

	m_spButton->Content.set(L"x");
	m_spButton->DisableContent.set(L"l");
	m_spButton->IsFocusable.set(false);
	static_cast<CTabControl*>(m_pParentControl)->ItemsSource.get_unconst()->at(GetIndex())->Unlock.binding(m_spButton->IsEnabled);

	m_spButton->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
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
	if (GetIsSelected()) {
		rect += CRectF(2.f, 2.f, 2.f, 0.f);
	}
	auto bkgndFill = GetIsSelected() ?
		(pTabControl->GetIsFocused() ?
			pTabControl->m_spProp->SelectedFill :
			pTabControl->m_spProp->UnfocusSelectedFill) :
		pTabControl->m_spProp->NormalFill;

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(bkgndFill, rect);

	if (auto pt = GetWndPtr()->GetDirectPtr()->Pixels2Dips(GetWndPtr()->GetCursorPosInClient()); rect.PtInRect(pt)) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(pTabControl->m_spProp->HotFill, rect);
	}
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(pTabControl->m_spProp->Line, CPointF(rect.left, rect.bottom), rect.LeftTop());
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(pTabControl->m_spProp->Line, rect.LeftTop(), CPointF(rect.right, rect.top));
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(pTabControl->m_spProp->Line, CPointF(rect.right, rect.top), CPointF(rect.right, rect.bottom));

	if (GetIndex() == *pTabControl->SelectedIndex) {
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(bkgndFill.Color, pTabControl->m_spProp->Line.Width, CPointF(rect.left, rect.bottom), CPointF(rect.right, rect.bottom));
	} else {
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(pTabControl->m_spProp->Line, CPointF(rect.left, rect.bottom), CPointF(rect.right, rect.bottom));
	}

	auto iterIcon = pTabControl->m_itemsHeaderIconTemplate.find(typeid(*pTabControl->ItemsSource->at(GetIndex())).name());
	iterIcon->second.operator()(pTabControl->ItemsSource->at(GetIndex()),iconRect);

	auto iterText = pTabControl->m_itemsHeaderTemplate.find(typeid(*pTabControl->ItemsSource->at(GetIndex())).name());
	auto text = iterText->second.operator()(pTabControl->ItemsSource->at(GetIndex()));
	if (!text.empty()) {
		GetWndPtr()->GetDirectPtr()->DrawTextLayout(m_spProp->Format, text, textRect);
	}

	m_spButton->OnPaint(e);
}

void CTabHeaderControl::OnLButtonDown(const LButtonDownEvent& e)
{
	if (m_spButton->GetRectInWnd().PtInRect(e.PointInWnd)) {
		m_spButton->OnLButtonDown(e);
	} else {
		auto pTabControl = static_cast<CTabControl*>(m_pParentControl);
		pTabControl->SelectedIndex.set(GetIndex());
	}
}

/************************/
/* CAddTabHeaderControl */
/************************/

CAddTabHeaderControl::CAddTabHeaderControl(CTabControl* pTabControl, const std::shared_ptr<TabHeaderControlProperty>& spProp)
	:CD2DWControl(pTabControl),m_spProp(spProp), m_spButton(std::make_shared<CButton>(this, spProp->ButtonProp))
{
	IsFocusable.set(false);
}

bool CAddTabHeaderControl::GetIsSelected()const
{
	return false;
}

std::tuple<CSizeF, CSizeF> CAddTabHeaderControl::MeasureSizes()
{
	auto buttonSize = CSizeF(16.f, 16.f);

	auto size = CSizeF(
			m_spProp->Padding.left + buttonSize.width + m_spProp->Padding.right,
			m_spProp->Padding.top + buttonSize.height + m_spProp->Padding.bottom);

	return {buttonSize, size};
}

std::tuple<CSizeF, CSizeF> CAddTabHeaderControl::GetSizes()
{
	if (!m_isMeasureValid) {
		auto [buttonSize, size] = MeasureSizes();
		m_buttonSize = buttonSize; m_size = size;
		m_isMeasureValid = true;
	}
	return { m_buttonSize, m_size };
}

CSizeF CAddTabHeaderControl::GetSize()
{
	return std::get<1>(GetSizes());
}

std::tuple<CRectF, CRectF> CAddTabHeaderControl::GetRects()
{
	auto rc = GetRectInWnd();
	auto [buttonSize, size] = GetSizes();
	auto buttonRect = CRectF(buttonSize);
	buttonRect.MoveToXY(rc.left + m_spProp->Padding.left, rc.top + m_spProp->Padding.top);

	return { buttonRect, rc };
}

void CAddTabHeaderControl::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);

	//Set up Button
	m_spButton->Command.subscribe([this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke(
			[pTabControl = static_cast<CTabControl*>(m_pParentControl)]()->void
			{
				pTabControl->OnCommandNewTab(CommandEvent(pTabControl->GetWndPtr(), 0,0));
			});
	}, shared_from_this());

	m_spButton->Content.set(L"+");
	m_spButton->IsFocusable.set(false);

	m_spButton->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
}

void CAddTabHeaderControl::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	m_spButton->OnRect(RectEvent(GetWndPtr(), std::get<0>(GetRects())));
}

void CAddTabHeaderControl::OnPaint(const PaintEvent& e)
{
	auto pTabControl = static_cast<CTabControl*>(m_pParentControl);
	auto [buttonRect, rect] = GetRects();
	auto bkgndFill = GetIsSelected() ?
		(pTabControl->GetIsFocused() ?
			pTabControl->m_spProp->SelectedFill :
			pTabControl->m_spProp->UnfocusSelectedFill) :
		pTabControl->m_spProp->NormalFill;

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(bkgndFill, rect);

	if (auto pt = GetWndPtr()->GetDirectPtr()->Pixels2Dips(GetWndPtr()->GetCursorPosInClient()); rect.PtInRect(pt)) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(pTabControl->m_spProp->HotFill, rect);
	}
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(pTabControl->m_spProp->Line, CPointF(rect.left, rect.bottom), rect.LeftTop());
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(pTabControl->m_spProp->Line, rect.LeftTop(), CPointF(rect.right, rect.top));
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(pTabControl->m_spProp->Line, CPointF(rect.right, rect.top), CPointF(rect.right, rect.bottom));
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(pTabControl->m_spProp->Line, CPointF(rect.left, rect.bottom), CPointF(rect.right, rect.bottom));

	m_spButton->OnPaint(e);
}

void CAddTabHeaderControl::OnLButtonDown(const LButtonDownEvent& e)
{
	if (m_spButton->GetRectInWnd().PtInRect(e.PointInWnd)) {
		m_spButton->OnLButtonDown(e);
	}
}

/***************/
/* CTabControl */
/***************/

/***********/
/* Machine */
/***********/

struct CTabControl::Machine
{
	class Normal {};
	class Dragging {};
	class Error {};

	template<class TRect, class R, class... Ts>
	auto call(R(TRect::* f)(Ts...))const
	{
		return [f](TRect* self, Ts... args) { return (self->*f)(args...); };
	}

	auto operator()() const noexcept
	{
		using namespace sml;
		return make_transition_table(
			*state<Normal> +event<LButtonBeginDragEvent>[call(&CTabControl::Guard_LButtonBeginDrag_Normal_To_Dragging)] = state<Dragging>,
			state<Normal> +event<LButtonBeginDragEvent> / call(&CTabControl::Normal_LButtonBeginDrag),

			state<Normal> +event<MouseMoveEvent> / call(&CTabControl::Normal_MouseMove),
			
			//Dragging
			state<Dragging> +on_entry<LButtonBeginDragEvent> / call(&CTabControl::Dragging_OnEntry),
			state<Dragging> +on_exit<LButtonEndDragEvent> / call(&CTabControl::Dragging_OnExit),
			state<Dragging> +event<LButtonEndDragEvent> = state<Normal>,
			state<Dragging> +event<MouseMoveEvent> / call(&CTabControl::Dragging_MouseMove),
			//Error handler
			*state<Error> +exception<std::exception> / call(&CTabControl::Error_StdException) = state<Normal>
		);
	}
};

/***************/
/* Constructor */
/***************/
//This should be called after Machine declared.

CTabControl::CTabControl(CD2DWControl* pParentControl, const std::shared_ptr<TabControlProperty>& spProp)
	:CD2DWControl(pParentControl), m_spProp(spProp), SelectedIndex(-1), 
	m_headers([](std::shared_ptr<CTabHeaderControl>& sp, size_t idx) { sp->SetIndex(idx); }),
	m_addHeader(std::make_shared<CAddTabHeaderControl>(this, spProp->HeaderProperty)),
	m_pMachine(new boost::sml::sm<Machine>{ this }),
	m_dragFrom(-1), m_dragTo(-1)
{}

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

	auto hdrSize = m_addHeader->GetSize();
	if (left + hdrSize.width > rcInWnd.right && left != rcInWnd.left) {
		left = rcInWnd.left;
		top += hdrSize.height;
	}

	m_addHeader->OnRect(RectEvent(GetWndPtr(),
		CRectF(left, top, left + hdrSize.width, top + hdrSize.height)));
}

/***********/
/* Command */
/***********/
void CTabControl::OnCommandCloneTab(const CommandEvent& e)
{
	//TODOLOW should deep clone
	ItemsSource.push_back(ItemsSource->at(*SelectedIndex));
	SelectedIndex.set(*SelectedIndex - 1);
}
void CTabControl::OnCommandLockTab(const CommandEvent& e)
{
	if (m_contextIndex) {
		auto& item = ItemsSource->at(m_contextIndex.value());
		item->Unlock.set(!(*item->Unlock));
	}
}

void CTabControl::OnCommandCloseTab(const CommandEvent& e)
{
	if (m_contextIndex) {
		if (ItemsSource.get_unconst()->at(m_contextIndex.value())->AcceptClosing(GetWndPtr(), false)) {
			ItemsSource.erase(ItemsSource->cbegin() + m_contextIndex.value());
		}
	}
}
void CTabControl::OnCommandCloseAllButThisTab(const CommandEvent& e)
{
	if (m_contextIndex) {
		//Erase larger tab
		for (auto iter = ItemsSource->cbegin() + (m_contextIndex.value() + 1), end = ItemsSource->cend(); iter != end; ++iter) {
			if (!((*iter)->AcceptClosing(GetWndPtr(), false))) {
				return;
			}
		}
		//TODO HIGH
		ItemsSource.erase(ItemsSource->cbegin() + (m_contextIndex.value() + 1), ItemsSource->cend());
		
		//Erase smaller tab
		for (auto iter = ItemsSource->cbegin(), end = ItemsSource->cbegin() + m_contextIndex.value(); iter != end; ++iter) {
			if (!((*iter)->AcceptClosing(GetWndPtr(), false))) {
				return;
			}
		}
		//TODO HIGH
		ItemsSource.erase(ItemsSource->cbegin(), ItemsSource->cbegin() + m_contextIndex.value());

		SelectedIndex.force_notify_set(0);
	}
}

/**************/
/* UI Message */
/**************/

void CTabControl::OnCreate(const CreateEvt& e) 
{
	CD2DWControl::OnCreate(e);

	//ItemsSource
	ItemsSource.subscribe(
		[this](auto notify)->void
		{
			switch (notify.action) {
				case notify_container_changed_action::push_back:
				{
					auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
					m_headers.idx_push_back(header);
					header->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
					UpdateHeaderRects();

					SelectedIndex.force_notify_set((std::min)((size_t)notify.new_starting_index, ItemsSource->size() - 1));
					break;
				}
				case notify_container_changed_action::insert:
				{
					auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
					m_headers.idx_insert(m_headers.cbegin() + notify.new_starting_index, header);
					header->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
					UpdateHeaderRects();

					SelectedIndex.force_notify_set((std::min)((size_t)notify.new_starting_index, ItemsSource->size() - 1));
					break;
				}
				case notify_container_changed_action::erase:
				{
					m_headers[notify.old_starting_index]->OnClose(CloseEvent(GetWndPtr(), NULL, NULL));
					m_headers.idx_erase(m_headers.cbegin() + notify.old_starting_index);
					UpdateHeaderRects();

					if (!ItemsSource->empty()) {
						SelectedIndex.force_notify_set((std::min)((size_t)notify.old_starting_index, ItemsSource->size() - 1));
					}
					break;
				}
				case notify_container_changed_action::replace:
				{
					m_headers[notify.new_starting_index]->OnClose(CloseEvent(GetWndPtr(), NULL, NULL));
					m_headers.idx_erase(m_headers.cbegin() + notify.new_starting_index);
					auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
					m_headers.idx_insert(m_headers.cbegin() + notify.new_starting_index, header);
					header->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
					UpdateHeaderRects();

					SelectedIndex.force_notify_set((std::min)((size_t)notify.new_starting_index, ItemsSource->size() - 1));
					break;
				}
				case notify_container_changed_action::reset:
				{
					if (!ItemsSource->empty()) {
						for (auto header : m_headers) {
							header->OnClose(CloseEvent(GetWndPtr(), NULL, NULL));
						}
						m_headers.clear();
						for (auto& item : notify.new_items) {
							auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
							m_headers.idx_push_back(header);
							header->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
						}
						UpdateHeaderRects();
						SelectedIndex.force_notify_set((std::min)((size_t)(*SelectedIndex), ItemsSource->size() - 1));
					}
					break;
				}
				default:
					break;
			}
		}, shared_from_this());

	/***********/
	/* Headers */
	/***********/
	for (auto& item : *ItemsSource) {
		auto header = std::make_shared<CTabHeaderControl>(this, m_spProp->HeaderProperty);
		m_headers.idx_push_back(header);
		header->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	}
	m_addHeader->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
	UpdateHeaderRects();

	//SelectedIndex
	SelectedIndex.subscribe( 
		[this](const int& value)->void {
			if (GetWndPtr()->m_hWnd) {
				auto spData = ItemsSource->at(value);
				auto spControl = m_itemsControlTemplate[typeid(*spData).name()](spData);
				if (m_spCurControl != spControl) {
					if (m_spCurControl) {
						m_spCurControl->OnEnable(EnableEvent(GetWndPtr(), false));
						m_spCurControl->IsEnabled.set(false);
					}
					m_spCurControl = spControl;
					m_spCurControl->OnEnable(EnableEvent(GetWndPtr(), true));
					m_spCurControl->IsEnabled.set(true);
				}
				GetWndPtr()->SetFocusToControl(spControl);
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
		},shared_from_this());


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
		rect = GetContentRect(); rect.DeflateRect(m_spProp->Padding);
		return rect;
	};
}

void CTabControl::OnPaint(const PaintEvent& e)
{
	if (*SelectedIndex >= 0) {
		auto rc = GetRectInWnd();
		//Content
		const auto& contentRc = GetContentRect();
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetIsFocused()?m_spProp->SelectedFill:m_spProp->UnfocusSelectedFill, contentRc);
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(m_spProp->Line, contentRc);
		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), contentRc.LeftTop(), CPointF(contentRc.left, contentRc.bottom));
		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), CPointF(contentRc.left, contentRc.bottom), CPointF(contentRc.right, contentRc.bottom));
		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), CPointF(contentRc.right, contentRc.bottom), CPointF(contentRc.right, contentRc.top));

		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), 
		//	CPointF(m_headers.front()->GetRectInWnd().left, m_headers.front()->GetRectInWnd().bottom),
		//	CPointF(m_headers[SelectedIndex.get()]->GetRectInWnd().left, m_headers[SelectedIndex]->GetRectInWnd().bottom));
		//GetWndPtr()->GetDirectPtr()->DrawSolidLine(*(m_spProp->Line), 
		//	CPointF(m_headers[SelectedIndex.get()]->GetRectInWnd().right, m_headers[SelectedIndex]->GetRectInWnd().bottom),
		//	CPointF(rc.right, m_headers.back()->GetRectInWnd().bottom) );

		//Header, Paint selected cell at last
		for (const auto& pHeader : m_headers) {
			if (pHeader->GetIndex() != *SelectedIndex) {
				pHeader->OnPaint(e);
			}
		}
		m_addHeader->OnPaint(e);
		m_headers[*SelectedIndex]->OnPaint(e);

		//Move line
		if (m_dragTo >= 0 && (size_t)m_dragTo < m_headers.size()) {
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(SolidLine(1.f, 0.f, 0.f, 1.f, 2.f), 
				m_headers[m_dragTo]->GetRectInWnd().LeftTop(), 
				CPointF(m_headers[m_dragTo]->GetRectInWnd().left, m_headers[m_dragTo]->GetRectInWnd().bottom));
		} else if (m_dragTo == m_headers.size()) {
			GetWndPtr()->GetDirectPtr()->DrawSolidLine(SolidLine(1.f, 0.f, 0.f, 1.f, 2.f), 
				CPointF(m_headers.back()->GetRectInWnd().right, m_headers.back()->GetRectInWnd().top),
				CPointF(m_headers.back()->GetRectInWnd().right, m_headers.back()->GetRectInWnd().bottom));
		}

		//Control
		m_spCurControl->OnPaint(e);
	}

}

void CTabControl::OnClosing(const ClosingEvent& e)
{ 
	for (auto& item : *ItemsSource) 		{
		if (!(item->AcceptClosing(GetWndPtr(), true))) {
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
		*(e.HandledPtr) = TRUE;
	}
}


void CTabControl::OnKeyDown(const KeyDownEvent& e)
{
	switch (e.Char)
	{
	case 'T':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			OnCommandNewTab(CommandEvent(e.WndPtr, 0, 0));
			*(e.HandledPtr) = TRUE;
		}
		break;
	case 'W':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			OnCommandCloseTab(CommandEvent(e.WndPtr, 0, 0));
			*(e.HandledPtr) = TRUE;
		}
		break;
	default:
		break;
	}
}

//


void CTabControl::OnLButtonBeginDrag(const LButtonBeginDragEvent& e) { m_pMachine->process_event(e); }
void CTabControl::OnLButtonEndDrag(const LButtonEndDragEvent& e) { m_pMachine->process_event(e); }
void CTabControl::OnMouseMove(const MouseMoveEvent& e) { m_pMachine->process_event(e); }

bool CTabControl::Guard_LButtonBeginDrag_Normal_To_Dragging(const LButtonBeginDragEvent& e)
{
	auto iter = std::find_if(m_headers.cbegin(), m_headers.cend(),
		[&](const std::shared_ptr<CTabHeaderControl>& x) {
			return *x->IsEnabled && x->GetRectInWnd().PtInRect(e.PointInWnd);
		});

	return iter != m_headers.cend();
}

void CTabControl::Normal_LButtonBeginDrag(const LButtonBeginDragEvent& e){ CD2DWControl::OnLButtonBeginDrag(e); }
void CTabControl::Normal_LButtonEndDrag(const LButtonEndDragEvent& e){ CD2DWControl::OnLButtonEndDrag(e); }
void CTabControl::Normal_MouseMove(const MouseMoveEvent& e) { CD2DWControl::OnMouseMove(e); }
	
void CTabControl::Dragging_OnEntry(const LButtonBeginDragEvent& e)
{
	auto iter = std::find_if(m_headers.cbegin(), m_headers.cend(),
		[&](const std::shared_ptr<CTabHeaderControl>& x) {
			return *x->IsEnabled && x->GetRectInWnd().PtInRect(e.PointInWnd);
		});

	if (iter != m_headers.cend()) {
		m_dragFrom = std::distance(m_headers.cbegin(), iter);
	} else {
		m_dragFrom = -1;
	}
	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
}

void CTabControl::Dragging_OnExit(const LButtonEndDragEvent& e)
{
	auto iter = FindPtInDraggingHeaderRect(e.PointInWnd);
	m_dragTo = std::distance(m_headers.cbegin(), iter);

	if (m_dragFrom != m_dragTo) {
		auto temp = ItemsSource->at(m_dragFrom);
		ItemsSource.erase(ItemsSource->cbegin() + m_dragFrom);
		m_dragTo = m_dragTo > m_dragFrom ? m_dragTo - 1 : m_dragTo;
		ItemsSource.insert(ItemsSource->cbegin() + m_dragTo, temp);
		UpdateHeaderRects();
	}
	m_dragFrom = -1;
	m_dragTo = -1;
	GetWndPtr()->ReleaseCapturedControlPtr();
}

void CTabControl::Dragging_MouseMove(const MouseMoveEvent& e)
{
	auto iter = FindPtInDraggingHeaderRect(e.PointInWnd);
	m_dragTo = std::distance(m_headers.cbegin(), iter);
}

void CTabControl::Error_StdException(const std::exception& e){}

index_vector<std::shared_ptr<CTabHeaderControl>>::const_iterator CTabControl::FindPtInDraggingHeaderRect(const CPointF& pt)
{
	auto isTop = [&](const std::shared_ptr<CTabHeaderControl>& x)->bool
	{
		return x->GetRectInWnd().top == m_headers.front()->GetRectInWnd().top;
	};
	auto isBottom =  [&](const std::shared_ptr<CTabHeaderControl>& x)->bool
	{
		return x->GetRectInWnd().bottom == m_headers.back()->GetRectInWnd().bottom;
	};
	auto isLeft = [&](const std::shared_ptr<CTabHeaderControl>& x)->bool
	{
		return x->GetRectInWnd().left == m_headers.front()->GetRectInWnd().left;
	};
	auto isRight = [&](const std::shared_ptr<CTabHeaderControl>& x)->bool
	{
		return x != m_headers.back() && x->GetRectInWnd().top < m_headers[x->GetIndex() + 1]->GetRectInWnd().top;
	};

	return std::find_if(m_headers.cbegin(), m_headers.cend(),
		[&](const std::shared_ptr<CTabHeaderControl>& x) {
			auto rc = x->GetRectInWnd();
			rc.left = isLeft(x) ? - FLT_MAX : rc.left;
			rc.top = isTop(x) ? -FLT_MAX : rc.top;
			rc.right = isRight(x) ? FLT_MAX : rc.right;
			rc.bottom = isBottom(x) ? FLT_MAX : rc.bottom;

			return *x->IsEnabled && rc.PtInRect(pt);
		});
}






