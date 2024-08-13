#include "TabControl.h"
#include "TabControlProperty.h"
#include "D2DWWindow.h"
#include "Dispatcher.h"
#include <fmt/format.h>
#include <stack>

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
CRectF CTabHeaderControl::FOCUS_PADDING = CRectF(2.f, 2.f, 2.f, 0.f);

CTabHeaderControl::CTabHeaderControl(CTabControl* pTabControl)
	:CD2DWControl(pTabControl), m_spButton(std::make_shared<CButton>(this))
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
		textSize = GetWndPtr()->GetDirectPtr()->CalcTextSize(GetFormat(), text);
	}

	auto size = CSizeF(
			GetPadding().left + iconSize.width + GetPadding().right
			+ (std::max)(textSize.width, m_minWidth) + GetPadding().right
			+ buttonSize.width + GetPadding().right,
			GetPadding().top + textSize.height + GetPadding().bottom);

	return { iconSize, textSize, buttonSize, size};
}

std::tuple<CSizeF, CSizeF, CSizeF, CSizeF> CTabHeaderControl::GetSizes()
{
	if (!m_isMeasureValid) {
		auto [iconSize, textSize, buttonSize, size] = MeasureSizes();
		m_iconSize = iconSize; m_textSize = textSize; 
		m_buttonSize = buttonSize + CSizeF(m_spButton->GetMargin().left + m_spButton->GetMargin().right, m_spButton->GetMargin().top + m_spButton->GetMargin().bottom);
		m_size = size;
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
	iconRect.MoveToXY(rc.left + GetPadding().left, rc.top + GetPadding().top );
	textRect.MoveToXY(iconRect.right + GetPadding().left, rc.top + GetPadding().top );
	buttonRect.MoveToXY(rc.right - buttonSize.width, rc.top);

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

void CTabHeaderControl::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	m_spButton->Arrange(std::get<2>(GetRects()));
}

void CTabHeaderControl::OnPaint(const PaintEvent& e)
{
	auto pTabControl = static_cast<CTabControl*>(m_pParentControl);
	auto [iconRect, textRect, buttonRect, rect] = GetRects();
	if (GetIsSelected()) {
		rect += FOCUS_PADDING;
	}
	auto bkgndFill = GetIsSelected() ?
		(GetIsFocused() ?
			GetSelectedOverlay() :
			GetUnfocusSelectedOverlay()) :
		GetNormalBackground();

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(bkgndFill, rect);

	if (auto pt = GetWndPtr()->GetDirectPtr()->Pixels2Dips(GetWndPtr()->GetCursorPosInClient()); rect.PtInRect(pt)) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetHotOverlay(), rect);
	}
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(GetNormalBorder(), CPointF(rect.left, rect.bottom), rect.LeftTop());
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(GetNormalBorder(), rect.LeftTop(), CPointF(rect.right, rect.top));
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(GetNormalBorder(), CPointF(rect.right, rect.top), CPointF(rect.right, rect.bottom));

	if (GetIsSelected()) {
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(bkgndFill.Color, GetNormalBorder().Width, CPointF(rect.left, rect.bottom), CPointF(rect.right, rect.bottom));
	} else {
		GetWndPtr()->GetDirectPtr()->DrawSolidLine(GetNormalBorder(), CPointF(rect.left, rect.bottom), CPointF(rect.right, rect.bottom));
	}

	auto iterIcon = pTabControl->m_itemsHeaderIconTemplate.find(typeid(*pTabControl->ItemsSource->at(GetIndex())).name());
	iterIcon->second.operator()(pTabControl->ItemsSource->at(GetIndex()),iconRect);

	auto iterText = pTabControl->m_itemsHeaderTemplate.find(typeid(*pTabControl->ItemsSource->at(GetIndex())).name());
	auto text = iterText->second.operator()(pTabControl->ItemsSource->at(GetIndex()));
	if (!text.empty()) {
		GetWndPtr()->GetDirectPtr()->DrawTextLayout(GetFormat(), text, textRect);
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

CAddTabHeaderControl::CAddTabHeaderControl(CTabControl* pTabControl)
	:CD2DWControl(pTabControl), m_spButton(std::make_shared<CButton>(this))
{
	IsFocusable.set(false);
}

bool CAddTabHeaderControl::GetIsSelected()const
{
	return false;
}

std::tuple<CSizeF, CSizeF> CAddTabHeaderControl::MeasureSizes()
{
	auto buttonSize = CSizeF(16.f, 16.f) + CSizeF(m_spButton->GetMargin().left + m_spButton->GetMargin().right, m_spButton->GetMargin().top + m_spButton->GetMargin().bottom);
	auto size = buttonSize;

	return {buttonSize, size};
}

std::tuple<CSizeF, CSizeF> CAddTabHeaderControl::GetSizes()
{
	if (!m_isMeasureValid) {
		auto [buttonSize, size] = MeasureSizes();
		m_buttonSize = buttonSize;
		m_size = size;
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
	buttonRect.MoveToXY(rc.left, rc.top);

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
				pTabControl->OnCommandNewTab();
			});
	}, shared_from_this());

	m_spButton->Content.set(L"+");
	m_spButton->IsFocusable.set(false);

	m_spButton->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
}

void CAddTabHeaderControl::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	m_spButton->Arrange(std::get<0>(GetRects()));
}

void CAddTabHeaderControl::OnPaint(const PaintEvent& e)
{
	auto pTabControl = static_cast<CTabControl*>(m_pParentControl);
	auto [buttonRect, rect] = GetRects();
	auto bkgndFill = GetIsSelected() ?
		(pTabControl->GetIsFocused() ?
			GetSelectedOverlay() :
			GetUnfocusSelectedOverlay()) :
		GetNormalBackground();

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(bkgndFill, rect);

	if (auto pt = GetWndPtr()->GetDirectPtr()->Pixels2Dips(GetWndPtr()->GetCursorPosInClient()); rect.PtInRect(pt)) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetHotOverlay(), rect);
	}
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(GetNormalBorder(), CPointF(rect.left, rect.bottom), rect.LeftTop());
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(GetNormalBorder(), rect.LeftTop(), CPointF(rect.right, rect.top));
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(GetNormalBorder(), CPointF(rect.right, rect.top), CPointF(rect.right, rect.bottom));
	GetWndPtr()->GetDirectPtr()->DrawSolidLine(GetNormalBorder(), CPointF(rect.left, rect.bottom), CPointF(rect.right, rect.bottom));

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
CRectF CTabControl::MARGIN = CRectF(1.f, 1.f, 1.f, 1.f);

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

CTabControl::CTabControl(CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl), SelectedIndex(-1), 
	m_headers([](std::shared_ptr<CTabHeaderControl>& sp, size_t idx) { sp->SetIndex(idx); }),
	m_addHeader(std::make_shared<CAddTabHeaderControl>(this)),
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
	auto rcInWnd = GetRectInWnd() - MARGIN - CTabHeaderControl::FOCUS_PADDING;
	auto left = rcInWnd.left;
	auto top = rcInWnd.top;

	for (auto& pHeader : m_headers) {
		pHeader->SetMeasureValid(false);//TODO HIGH

		auto hdrSize = pHeader->GetSize();
		if (left + hdrSize.width > rcInWnd.right && left != rcInWnd.left) {
			left = rcInWnd.left;
			top += hdrSize.height;
		}

		pHeader->Arrange(CRectF(left, top, left + hdrSize.width, top + hdrSize.height));
		left = pHeader->GetRectInWnd().right;
	}

	auto hdrSize = m_addHeader->GetSize();
	if (left + hdrSize.width > rcInWnd.right && left != rcInWnd.left) {
		left = rcInWnd.left;
		top += hdrSize.height;
	}

	m_addHeader->Arrange(CRectF(left, top, left + hdrSize.width, top + hdrSize.height));
}

/***********/
/* Command */
/***********/
void CTabControl::OnCommandCloneTab()
{
	//TODOLOW should deep clone
	ItemsSource.push_back(ItemsSource->at(*SelectedIndex));
	SelectedIndex.set(*SelectedIndex - 1);
}
void CTabControl::OnCommandLockTab()
{
	if (m_contextIndex) {
		auto& item = ItemsSource->at(m_contextIndex.value());
		item->Unlock.set(!(*item->Unlock));
	}
}

void CTabControl::OnCommandCloseTab()
{
	if (m_contextIndex) {
		if (ItemsSource.get_unconst()->at(m_contextIndex.value())->AcceptClosing(GetWndPtr(), false)) {
			ItemsSource.erase(ItemsSource->cbegin() + m_contextIndex.value());
		}
	}
}
void CTabControl::OnCommandCloseAllButThisTab()
{
	if (m_contextIndex) {
		std::vector<decltype(ItemsSource)::const_iterator> iterCloses;

		for (auto iter = ItemsSource->cbegin(); iter != ItemsSource->cend(); ++iter) {
			if (m_contextIndex.value() != std::distance(ItemsSource->cbegin(), iter) &&
				(*iter)->AcceptClosing(GetWndPtr(), false)) {
				iterCloses.push_back(iter);
			}
		}

		for (auto iter = iterCloses.crbegin(); iter != iterCloses.crend(); ++iter) {
			ItemsSource.erase(*iter);
		}

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
					auto header = std::make_shared<CTabHeaderControl>(this);
					m_headers.idx_push_back(header);
					header->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
					UpdateHeaderRects();

					SelectedIndex.force_notify_set((std::min)((size_t)notify.new_starting_index, ItemsSource->size() - 1));
					break;
				}
				case notify_container_changed_action::insert:
				{
					auto header = std::make_shared<CTabHeaderControl>(this);
					m_headers.idx_insert(m_headers.cbegin() + notify.new_starting_index, header);
					header->OnCreate(CreateEvt(GetWndPtr(), this, CRectF()));
					UpdateHeaderRects();

					SelectedIndex.force_notify_set((std::min)((size_t)notify.new_starting_index, ItemsSource->size() - 1));
					break;
				}
				case notify_container_changed_action::erase:
				{
					for (size_t i = notify.old_starting_index; i < notify.old_starting_index + notify.old_items.size(); i++) {
						m_headers[i]->OnClose(CloseEvent(GetWndPtr(), NULL, NULL));
					}
					m_headers.idx_erase(
						m_headers.cbegin() + notify.old_starting_index,
						m_headers.cbegin() + notify.old_starting_index + notify.old_items.size());

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
					auto header = std::make_shared<CTabHeaderControl>(this);
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
							auto header = std::make_shared<CTabHeaderControl>(this);
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
		auto header = std::make_shared<CTabHeaderControl>(this);
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
		rect = RenderRect();
		if (!m_headers.empty()) {
			rect.top = m_headers.back()->GetRectInWnd().bottom;
		}
		return rect;
	};

	GetControlRect = [rect = CRectF(), this]()mutable->CRectF&
	{
		rect = GetContentRect(); rect.DeflateRect(GetPadding());
		return rect;
	};
}

void CTabControl::OnPaint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	if (*SelectedIndex >= 0) {
		//Content
		const auto& contentRc = GetContentRect();
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(GetNormalBorder(), contentRc);

		//Header, Paint selected header on last
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
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PopAxisAlignedClip();
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

void CTabControl::Arrange(const CRectF& rc)
{
	CD2DWControl::Arrange(rc);
	UpdateHeaderRects();
	if (m_spCurControl) { m_spCurControl->Arrange(GetControlRect()); }
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
	bool ctrl = ::IsKeyDown(VK_CONTROL);
	switch (e.Char)
	{
	case 'T':
		if (ctrl) {
			OnCommandNewTab();
			*(e.HandledPtr) = TRUE;
		}
		break;
	case 'W':
		if (ctrl) {
			OnCommandCloseTab();
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






