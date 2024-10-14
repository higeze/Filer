#include "Textbox.h"
//#include "TextBoxProperty.h"
#include "D2DWWindow.h"
#include "TextEditSink.h"
#include "TextStoreACP.h"
#include "MyClipboard.h"
#include "Direct2DWrite.h"
#include "Debug.h"
#include "UIElement.h"
#include "GridView.h"
#include "Scroll.h"
//#include "ScrollProperty.h"//ScrollProperty
#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "ResourceIDFactory.h"

#include <nameof/nameof.hpp>
#include "IDoCommand.h"
#include "TSFManager.h"



#define TAB_WIDTH_4CHAR 4


//HRESULT InitDisplayAttrbute();
//HRESULT UninitDisplayAttrbute();




/***************/
/* constructor */
/***************/
CTextBox::CTextBox(
	CD2DWControl* pParentControl,
	std::unique_ptr<CVScroll>&& pVScroll,
	std::unique_ptr<CHScroll>&& pHScroll,
	const std::wstring& text)
	:CD2DWControl(pParentControl),
	m_initText(text),
	Text(text),
	EnterText(text),
	Caret(0, text.size(), 0, 0, text.size()),
	m_pTextMachine(std::make_unique<CTextBoxStateMachine>(this)),
	m_pVScroll(std::forward<std::unique_ptr<CVScroll>>(pVScroll)),
	m_pHScroll(std::forward<std::unique_ptr<CHScroll>>(pHScroll))
{
	m_pVScroll->ScrollChanged.connect([this]() { ClearActualRects(); });
	m_pHScroll->ScrollChanged.connect([this]() { ClearActualRects(); });
}

CTextBox::CTextBox(
	CD2DWControl* pParentControl,
	const std::wstring& text)
	: CTextBox(pParentControl,
		std::make_unique<CVScroll>(this),
		std::make_unique<CHScroll>(this),
		text)
{}
/**************/
/* destructor */
/**************/
CTextBox::~CTextBox() = default;

void CTextBox::InitTSF()
{
	FAILED_THROW(GetDocumentMgrPtr()->Push(GetContextPtr()));
	FAILED_THROW(GetTextEditSinkPtr()->_Advise(GetContextPtr()));
}
void CTextBox::UninitTSF()
{
	FAILED_THROW(GetDocumentMgrPtr()->Pop(TF_POPF_ALL));
	FAILED_THROW(GetTextEditSinkPtr()->_Unadvise());
}

CSizeF CTextBox::MeasureOverride(const CSizeF& availableSize)
{
	return MeasureSize(Text->empty() ? L"A" : *Text);
}

CSizeF CTextBox::MeasureSize(const std::wstring& text)
{
	CSizeF size = GetWndPtr()->GetDirectPtr()->CalcTextSize(GetFormat(), text);
	size.width += GetPadding().left
		+ GetPadding().right
		+ GetNormalBorder().Width;
	size.height += GetPadding().top
		+ GetPadding().bottom
		+ GetNormalBorder().Width;

	return size;
}

/***************/
/* Lazy Getter */
/***************/

const CComPtr<ITfDocumentMgr>& CTextBox::GetDocumentMgrPtr() const
{
	if (!m_pDocumentMgr) {
		FAILED_THROW(CTSFManager::GetInstance()->GetThreadMgrPtr()->CreateDocumentMgr(&m_pDocumentMgr))
	}
	return m_pDocumentMgr;
}

const CComPtr<ITfContext>& CTextBox::GetContextPtr() const
{
	if (!m_pContext) {
		FAILED_THROW(GetDocumentMgrPtr()->CreateContext(CTSFManager::GetInstance()->GetID(), 0, static_cast<ITextStoreACP*>(GetTextStorePtr().p), &m_pContext, &m_editCookie));
	}
	return m_pContext;
}

const CComPtr<CTextStore>& CTextBox::GetTextStorePtr() const
{
	if (!m_pTextStore) {
		m_pTextStore.Attach(new CTextStore(const_cast<CTextBox*>(this)));
	}
	return m_pTextStore;
}

const CComPtr<CTextEditSink>& CTextBox::GetTextEditSinkPtr() const
{
	if (!m_pTextEditSink) {
		m_pTextEditSink.Attach(new CTextEditSink(const_cast<CTextBox*>(this)));
	}
	return m_pTextEditSink;
}

void CTextBox::LoadTextLayoutPtr()
{
	auto pageRect = GetPageRect();
	auto pDirect = GetWndPtr()->GetDirectPtr();
	auto pRender = pDirect->GetD2DDeviceContext();
	auto pFactory = pDirect->GetDWriteFactory();
	auto size = CSizeF(GetIsWrap() ? (std::max)(0.f, pageRect.Width()) : FLT_MAX, FLT_MAX);

	CComPtr<IDWriteTextLayout> pTextLayout0(nullptr);
	const IID* piid = &__uuidof(IDWriteTextLayout1);
	if (FAILED(pFactory->CreateTextLayout(Text->c_str(), Text->size(), pDirect->GetTextFormat(GetFormat()), size.width, size.height, &pTextLayout0)) ||
		FAILED(pTextLayout0->QueryInterface(*piid, (void**)&m_pTextLayout))) {
		throw std::exception(FILE_LINE_FUNC);
	} else {
		//Default set up
		CComPtr<IDWriteTypography> typo;
		pFactory->CreateTypography(&typo);

		DWRITE_FONT_FEATURE feature;
		feature.nameTag = DWRITE_FONT_FEATURE_TAG_STANDARD_LIGATURES;
		feature.parameter = 0;
		typo->AddFontFeature(feature);
		DWRITE_TEXT_RANGE range;
		range.startPosition = 0;
		range.length = Text->size();
		m_pTextLayout->SetTypography(typo, range);

		m_pTextLayout->SetCharacterSpacing(0.0f, 0.0f, 0.0f, DWRITE_TEXT_RANGE{ 0, Text->size() });
		m_pTextLayout->SetPairKerning(FALSE, DWRITE_TEXT_RANGE{ 0, Text->size() });
	}
}


void CTextBox::LoadOriginCharRects()
{
	CRectF pageRect(GetPageRect());
	CSizeF size(GetIsWrap() ? pageRect.Width() : FLT_MAX, FLT_MAX);

	m_optOriginCharRects = CDirect2DWrite::CalcCharRects(GetTextLayoutPtr(), Text->size());
	if (Text->empty()) {
		auto size = GetWndPtr()->GetDirectPtr()->CalcTextSize(GetFormat(), L"");
		m_optOriginCharRects->emplace_back(
			0.f, 0.f,
			size.width, size.height);
	} else if (Text->back() == L'\n') {
		m_optOriginCharRects->emplace_back(
			0.f, m_optOriginCharRects->back().bottom,
			0.f, m_optOriginCharRects->back().bottom + m_optOriginCharRects->back().Height());
	} else {
		m_optOriginCharRects->emplace_back(
			m_optOriginCharRects->back().right, m_optOriginCharRects->back().top,
			m_optOriginCharRects->back().right, m_optOriginCharRects->back().bottom);
	}
}

void CTextBox::LoadOriginCursorCharRects()
{
	CRectF pageRect(GetPageRect());
	m_optOriginCursorCharRects = GetOriginCharRects();
	//Max Right
	auto maxRight = (std::max)(pageRect.Width(), std::max_element(m_optOriginCursorCharRects->begin(), m_optOriginCursorCharRects->end(),
		[](const auto& lhs, const auto& rhs) {return lhs.right < rhs.right; })->right);

	//Case of IsWrap
	/*example-1*/
	/*abcd\n   */
	/*efghijklm*/
	/*no\n     */
	/*\n       */
	/*pqrstu\n */
	/*vwxzy    */

	/*example-2*/
	/*abcd\n   */
	/*efghijklm*/
	/*no\n     */
	/*\n       */
	/*pqrstu\n */
	/*vwxzy\n  */
	/*         */

	//Case of Not IsWrap
	/*example-1    */
	/*abcd\n       */
	/*efghijklmno\n*/
	/*\n           */
	/*pqrstu\n     */
	/*vwxzy        */

	/*example-2    */
	/*abcd\n       */
	/*efghijklmno\n*/
	/*\n           */
	/*pqrstu\n     */
	/*vwxzy\n      */
	/*             */

	for (std::size_t i = 0; i < m_optOriginCursorCharRects->size(); i++) {
		if (i == Text->size() || Text->at(i) == L'\n') {
			m_optOriginCursorCharRects->at(i).right = maxRight;
		}
	}
}

void CTextBox::LoadOriginCaptureCharRects()
{
	auto charRects = GetOriginCharRects();
	m_optOriginCaptureCharRects = charRects;
	for (std::size_t i = 0; i < m_optOriginCaptureCharRects->size(); i++) {
		//top
		if (charRects[i].top == charRects.front().top) {
			m_optOriginCaptureCharRects->at(i).top = -FLT_MAX;
		}
		//left
		if (i == 0 || charRects[i].top > charRects[i - 1].top) {
			m_optOriginCaptureCharRects->at(i).left = -FLT_MAX;
		}
		//right
		if (i == Text->size() || Text->at(i) == L'\n') {
			m_optOriginCaptureCharRects->at(i).right = FLT_MAX;
		}
		//bottom
		if (charRects[i].bottom == charRects.back().bottom) {
			m_optOriginCaptureCharRects->at(i).bottom = FLT_MAX;
		}
	}
}

void CTextBox::LoadActualCursorCharRects()
{
	m_optActualCursorCharRects = GetOriginCursorCharRects();
	CPointF offset(GetPageRect().LeftTop());
	offset.Offset(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());
	std::for_each(m_optActualCursorCharRects->begin(), m_optActualCursorCharRects->end(),
		[offset](auto& rc) { rc.OffsetRect(offset); });
}
void CTextBox::LoadActualCaptureCharRects()
{
	m_optActualCaptureCharRects = GetOriginCaptureCharRects();
	CPointF offset(GetPageRect().LeftTop());
	offset.Offset(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());
	std::for_each(m_optActualCaptureCharRects->begin(), m_optActualCaptureCharRects->end(),
		[offset](auto& rc) {rc.OffsetRect(offset); });
}

void CTextBox::LoadActualCharRects()
{
	m_optActualCharRects = GetOriginCharRects();
	CPointF offset(GetPageRect().LeftTop());
	offset.Offset(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());
	std::for_each(m_optActualCharRects->begin(), m_optActualCharRects->end(),
		[offset](auto& rc) { rc.OffsetRect(offset); });
}
void CTextBox::LoadActualSelectionCharRects()
{
	m_optActualSelectionCharRects = GetActualCharRects();
	auto size = GetWndPtr()->GetDirectPtr()->CalcTextSize(GetFormat(), L"a");
	for (size_t i = 0; i < m_optActualSelectionCharRects->size(); i++) {
		if (i == Text->size() || Text->at(i) == L'\n') {
			m_optActualSelectionCharRects->at(i).right += size.width;
		}
	}
}
void CTextBox::LoadOriginContentRect()
{
	auto rcPage(GetPageRect());
	auto charRects(GetOriginCharRects());
	if (!charRects.empty()) {
		m_optOriginContentRect = CRectF(
			charRects.front().left,
			charRects.front().top,
			(std::max)(rcPage.Width(), std::max_element(charRects.begin(), charRects.end(), [](const auto& lhs, const auto& rhs) { return lhs.right < rhs.right; })->right),
			charRects.back().bottom);
	}
}

void CTextBox::LoadActualContentRect()
{
	auto rcPage(GetPageRect());
	auto charRects(GetActualCharRects());
	if (!charRects.empty()) {
		m_optActualContentRect = CRectF(
			charRects.front().left,
			charRects.front().top,
			(std::max)(rcPage.right, std::max_element(charRects.begin(), charRects.end(), [](const auto& lhs, const auto& rhs) { return lhs.right < rhs.right; })->right),
			charRects.back().bottom);
	}
}


void CTextBox::Clear()
{
	Caret.get_unconst()->Clear();
	Text.clear();

	m_pVScroll->SetScrollPage(GetPageRect().Height());
	m_pVScroll->SetScrollPos(0.f);
	m_pVScroll->SetScrollRange(0.f, GetOriginContentRect().Height());

	m_pHScroll->SetScrollPage(GetPageRect().Width());
	m_pHScroll->SetScrollPos(0.f);
	m_pHScroll->SetScrollRange(0.f, GetOriginContentRect().Width());
}

void CTextBox::MoveCaret(const int& index, const CPointF& point)
{
	Caret.get_unconst()->Move(index, point);
	if (GetIsFocused()) {
		StartCaretBlink();
	}
}

void CTextBox::MoveCaretWithShift(const int& index, const CPointF& point)
{
	Caret.get_unconst()->MoveWithShift(index, point);
	if (GetIsFocused()) {
		StartCaretBlink();
	}
}

void CTextBox::MoveSelection(const int& selectedBegin, const int& selectedEnd)
{
	Caret.get_unconst()->Select(selectedBegin, selectedEnd, GetOriginCharRects()[Text->size()].CenterPoint());
	if (GetIsFocused()) {
		StartCaretBlink();
	}
}

void CTextBox::EnsureVisibleCaret()
{
	if (m_isScrollable) {
		auto pageRect = GetPageRect();
		auto contentRect = GetOriginContentRect();
		auto charRects = GetOriginCharRects();
		if (!charRects.empty()) {
			//Range
			m_pVScroll->SetScrollRange(0, contentRect.Height());
			m_pHScroll->SetScrollRange(0, contentRect.Width());
			//Pos
			auto curRect = charRects[*Caret->Current];
			auto orgRect = charRects[0];

			if ((curRect.top - m_pVScroll->GetScrollPos() + pageRect.top) < pageRect.top) {
				m_pVScroll->SetScrollPos(curRect.top - orgRect.top);
			} else if ((curRect.bottom - m_pVScroll->GetScrollPos() +pageRect.top) > pageRect.bottom) {
				m_pVScroll->SetScrollPos(curRect.bottom - orgRect.top - pageRect.Height());
			}

			if ((curRect.left - m_pHScroll->GetScrollPos() + pageRect.left) < pageRect.left) {
				m_pHScroll->SetScrollPos(curRect.left - orgRect.left);
			} else if ((curRect.right - m_pHScroll->GetScrollPos() + pageRect.left) > pageRect.right) {
				m_pHScroll->SetScrollPos(curRect.right - orgRect.left - pageRect.Width());
			}
		}
	}
}


/*******************/
/* Windows Message */
/*******************/

void CTextBox::OnCreate(const CreateEvt& e)
{

	CD2DWControl::OnCreate(e);

	Caret.get_unconst()->Point.set(CPointF(0, GetLineHeight() * 0.5f));
	 
	Text.subscribe([this](auto e)
	{
		UpdateAll();
		switch (e.action) {
			case notify_container_changed_action::insert://new,null,idx,-1
				GetTextStorePtr()->OnTextChange(e.new_starting_index, e.new_starting_index, e.new_starting_index + e.new_items.size());
				MoveCaret(e.new_starting_index + e.new_items.size(), GetOriginCharRects()[e.new_starting_index + e.new_items.size()].CenterPoint());
				break;
			case notify_container_changed_action::erase://null,old,-1, idx
				GetTextStorePtr()->OnTextChange(e.old_starting_index, e.old_starting_index + e.old_items.size(), e.old_starting_index);
				MoveCaret(e.old_starting_index, GetOriginCharRects()[e.old_starting_index].CenterPoint());
				break;
			case notify_container_changed_action::replace://new,old,idx,idx
				GetTextStorePtr()->OnTextChange(e.new_starting_index, e.old_starting_index + e.old_items.size(), e.new_starting_index + e.new_items.size());
				MoveCaret(e.new_starting_index + e.new_items.size(), GetOriginCharRects()[e.new_starting_index + e.new_items.size()].CenterPoint());
				break;
			case notify_container_changed_action::reset://new,old,0,0
				GetTextStorePtr()->OnTextChange(0, 0, 0);
				MoveCaret(0, CPointF(0, GetLineHeight() * 0.5f));
				break;
			default:
				break;
		}
	}
	, shared_from_this());

	Caret.get_unconst()->Current.subscribe([this](auto) { EnsureVisibleCaret(); }, shared_from_this());
	//m_carets.Subscribe(
	//	[this](const std::tuple<int, int, int, int, int>& value)->void {
	//		EnsureVisibleCaret();
	//	}
	//);

	if (GetIsFocused()) {
		m_isFirstDrawCaret = true;
	}

	InitTSF();
}

void CTextBox::OnDestroy(const DestroyEvent& e)
{
	StopCaretBlink();
	UninitTSF();

	CD2DWControl::OnDestroy(e);
}

void CTextBox::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);
}

void CTextBox::ArrangeOverride(const CRectF& finalRect)
{
	CD2DWControl::ArrangeOverride(finalRect);
	UpdateAll();
}

void CTextBox::OnMouseWheel(const MouseWheelEvent& e)
{
	m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetDeltaScroll() * e.Delta / WHEEL_DELTA);
}

/*****************/
/* State Machine */
/*****************/


void CTextBox::Normal_Paint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	//PaintBackground
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetRectInWnd());
	//PaintLine

	//Paint Focused Line
	CRectF rcBorder(GetRectInWnd());
	rcBorder.DeflateRect(1.0f, 1.0f);
	if (m_hasBorder) {
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(GetNormalBorder(), rcBorder);
	} else {
		// Do nothing
	}
	
	//PaintContent
	if (!Text->empty()) {
		PaintText(e);
		PaintSelection(e);
		PaintHighlite(e);
		PaintCaret(e);
		PaintCompositionLine(e);
	} else {
		PaintCaret(e);
	}
	//PaintScroll
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CTextBox::Normal_SetFocus(const SetFocusEvent& e)
{
	FAILED_THROW(CTSFManager::GetInstance()->GetThreadMgrPtr()->SetFocus(GetDocumentMgrPtr()));
	m_isFirstDrawCaret = true;
}


void CTextBox::Normal_KillFocus(const KillFocusEvent& e)
{
	if (m_isEnterText) {
		EnterText.set(*Text);
	}

	StopCaretBlink();
	TerminateCompositionString();
}

void CTextBox::Normal_KeyUp(const KeyUpEvent& e)
{
	switch (e.Char) {
		case VK_CONTROL:
		{
			BOOL dummy = FALSE;
			Normal_SetCursor(SetCursorEvent(GetWndPtr(), HTCLIENT, &dummy));
			break;

		}
		default:
			break;
	}
}

void CTextBox::Normal_KeyDown(const KeyDownEvent& e)
{
	bool shift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;
	bool alt = (::GetKeyState(VK_MENU) & 0x80) != 0;

	switch (e.Char) {
	case VK_CONTROL:
	{
		BOOL dummy = FALSE;
		Normal_SetCursor(SetCursorEvent(GetWndPtr(), HTCLIENT, &dummy));
		*e.HandledPtr = TRUE;
		break;

	}
	case VK_LEFT:
	{
		auto position = std::clamp(*Caret->Current - 1, 0, (int)Text->size());
		auto point = GetOriginCharRects()[position].CenterPoint();

		if (shift) {
			MoveCaretWithShift(position, point);
			*e.HandledPtr = TRUE;
		} else {
			MoveCaret(position, point);
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case VK_RIGHT:
	{
		auto position = std::clamp(*Caret->Current + 1, 0, (int)Text->size());
		auto point = GetOriginCharRects()[position].CenterPoint();

		if (shift) {
			MoveCaretWithShift(position, point);
			*e.HandledPtr = TRUE;
		} else {
			MoveCaret(position, point);
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case VK_UP:
	{
		auto curCharRect = GetOriginCharRects()[*Caret->Current];
		auto caret_point = *Caret->Point;
		auto point = CPointF(caret_point.x,
			(std::max)(GetOriginCharRects().front().CenterPoint().y, caret_point.y - curCharRect.Height()));
		if (auto position = GetOriginCharPosFromPoint(point)) {
			if (shift) {
				MoveCaretWithShift(position.value(), point);
				*e.HandledPtr = TRUE;
			} else {
				MoveCaret(position.value(), point);
				*e.HandledPtr = TRUE;
			}
		}
		break;
	}
	case VK_DOWN:
	{
		auto curCharRect = GetOriginCharRects()[*Caret->Current];
		auto caret_point = *Caret->Point;
		auto point = CPointF(caret_point.x,
			(std::min)(GetOriginCharRects().back().CenterPoint().y, caret_point.y + curCharRect.Height()));
		if (auto newPos = GetOriginCharPosFromPoint(point)) {
			if (shift) {
				MoveCaretWithShift(newPos.value(), point);
				*e.HandledPtr = TRUE;
			} else {
				MoveCaret(newPos.value(), point);
				*e.HandledPtr = TRUE;
			}
		}
		break;
	}
	case VK_HOME:
	{
		if (auto index = GetFirstCharPosInLine(*Caret->Current)){
			auto point = GetOriginCharRects()[index.value()].CenterPoint();
			if (shift) {
				MoveCaretWithShift(index.value(), point);
				*e.HandledPtr = TRUE;
			} else {
				MoveCaret(index.value(), point);
				*e.HandledPtr = TRUE;
			}
		}
		break;
	}
	case VK_END:
	{
		if (auto index = GetLastCharPosInLine(*Caret->Current)) {
			auto point = GetOriginCharRects()[index.value()].CenterPoint();
			if (shift) {
				MoveCaretWithShift(index.value(), point);
				*e.HandledPtr = TRUE;
			} else {
				MoveCaret(index.value(), point);
				*e.HandledPtr = TRUE;
			}
		}
		break;
	}
	case VK_DELETE:
	{
		if (Caret->IsNotSelected()) {
			if (*Caret->Current < (int)Text->size()) {
				DeleteOne();
				*e.HandledPtr = TRUE;
			}
		} else {
			DeleteSelection();
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case VK_BACK:
	{
		if (Caret->IsNotSelected()) {
			if (*Caret->Current > 0) {
				BackspaceOne();
				*e.HandledPtr = TRUE;
			}

		} else {
			DeleteSelection();
			*e.HandledPtr = TRUE;
		}

		break;
	}
	//case VK_ESCAPE:
	//	break;
	case 'Z':
	{
		if (ctrl) {
			m_doMgr.UnDo();
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case 'Y':
	{
		if (ctrl) {
			m_doMgr.ReDo();
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case 'A':
	{
		if (ctrl) {
			Caret.get_unconst()->Select(0, Text->size(), GetOriginCharRects()[Text->size()].CenterPoint());
			StartCaretBlink();
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case 'C':
	{
		if (ctrl) {
			CopySelectionToClipboard();
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case 'X':
	{
		if (ctrl) {
			CopySelectionToClipboard();
			DeleteSelection();
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case 'V':
	case VK_INSERT:
	{
		if (ctrl) {
			PasteFromClipboard();
			*e.HandledPtr = TRUE;
		}
		break;
	}
	case VK_RETURN:
		if (m_isEnterText && !alt) {
			EnterText.set(*Text);//TODOHIGH Cannot call *e.HandledPtr = TRUE, since this Enter message is used at GridView Edit_Guard_KeyDown
		} else {
			ReplaceSelection(L"\n");
			*e.HandledPtr = TRUE;
		}
		break;
	case VK_TAB:
		if (m_acceptsTab) {
			ReplaceSelection(L"\t");
			*e.HandledPtr = TRUE;
		}
		break;

	default:
		break;
	}
}

void CTextBox::Normal_LButtonDown(const LButtonDownEvent& e)
{
	auto newPoint = GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);

	if (auto index = GetActualCharPosFromPoint(newPoint)) {
		auto point = GetOriginCharRects()[index.value()].CenterPoint();
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			MoveCaretWithShift(index.value(), point);
			return;
		} else {
			MoveCaret(index.value(), point);
		}
	}
}

void CTextBox::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
	auto newPoint = GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);
	std::vector<wchar_t> delimiters{ L' ', L'\t', L'\n' };

	if (auto index = GetActualCharPosFromPoint(newPoint); index && index.value() > 0 && index.value() < static_cast<int>(Text->size())) {
		if (std::find(delimiters.begin(), delimiters.end(), Text->at(index.value())) == delimiters.end()) {
			size_t selBegin = index.value() - 1;
			for (; selBegin > 0; --selBegin) {
				if (std::find(delimiters.begin(), delimiters.end(), Text->at(selBegin)) != delimiters.end()) {
					selBegin++;
					break;
				}
			}
			size_t selEnd = index.value() + 1;
			for (; selEnd < Text->size(); ++selEnd) {
				if (std::find(delimiters.begin(), delimiters.end(), Text->at(selEnd)) != delimiters.end()) {
					break;
				}
			}
			MoveSelection(selBegin, selEnd);
		}
	}
}

void CTextBox::TextDrag_OnEntry(const LButtonBeginDragEvent& e)
{
}

void CTextBox::TextDrag_OnExit(const LButtonEndDragEvent& e)
{
}

void CTextBox::TextDrag_MouseMove(const MouseMoveEvent& e)
{
	if (e.Flags & MK_LBUTTON) {
		CPointF newPoint(e.PointInWnd);

		if (auto index = GetActualCaptureCharPosFromPoint(newPoint)) {
			auto point = GetOriginCharRects()[index.value()].CenterPoint();
			MoveCaretWithShift(index.value(), point);
		}
	}
}

void CTextBox::Normal_MouseMove(const MouseMoveEvent& e){}

void CTextBox::Normal_Char(const CharEvent& e)
{
	if (::IsKeyDown(VK_CONTROL)) {
		return;
	}

	// normal character input. not TSF.
	if (e.Char >= L' ' && e.Char < 256) {
		WCHAR wc[] = { static_cast<WCHAR>(e.Char), '\0' };
		ReplaceSelection(wc);
		*e.HandledPtr = TRUE;
	}
}

void CTextBox::Normal_SetCursor(const SetCursorEvent& e)
{
	CPointF pt = GetWndPtr()->GetCursorPosInWnd();
	if (GetRectInWnd().PtInRect(pt)) {
		if (m_pVScroll->GetIsVisible() && m_pVScroll->GetRectInWnd().PtInRect(pt) ||
			m_pHScroll->GetIsVisible() && m_pHScroll->GetRectInWnd().PtInRect(pt)) {
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			*(e.HandledPtr) = TRUE;
			return;
		} else {
		}
		::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
		*(e.HandledPtr) = TRUE;
	}
}

void CTextBox::Normal_ContextMenu(const ContextMenuEvent& e)
{
	//CreateMenu
	CMenu menu(::CreatePopupMenu());
	//Add Row
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	MENUITEMINFO mii_separator = { 0 };
	mii_separator.cbSize = sizeof(MENUITEMINFO);
	mii_separator.fMask = MIIM_FTYPE;
	mii_separator.fType = MFT_SEPARATOR;

	//Copy
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Copy");
	mii.dwTypeData = const_cast<LPWSTR>(L"Copy\tCtrl+C");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
	//Paste
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Paste");
	mii.dwTypeData = const_cast<LPWSTR>(L"Paste\tCtrl+V");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
	////Separator
	//menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii_separator);
	////Execute
	//mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Execute");
	//mii.dwTypeData = const_cast<LPWSTR>(L"Execute");
	//menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	//Show Menu
	::SetForegroundWindow(GetWndPtr()->m_hWnd);
	int idCmd = menu.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		e.PointInScreen.x,
		e.PointInScreen.y,
		GetWndPtr()->m_hWnd);

	//Command
	if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Copy")) {
		CopySelectionToClipboard();
	} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Paste")) {
		PasteFromClipboard();
	}
	//else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Execute")) {
	//	auto exe = m_text.substr(std::get<caret::SelBegin>(m_carets.get()), std::get<caret::SelEnd>(m_carets.get()) - std::get<caret::SelBegin>(m_carets.get()));
	//	exe = ((exe.front() == L'\"') ? L"" : L"\"") + boost::algorithm::trim_copy(exe) + ((exe.back() == L'\"') ? L"" : L"\"");
	//	SHELLEXECUTEINFO execInfo = {};
	//	execInfo.cbSize = sizeof(execInfo);
	//	execInfo.hwnd = GetWndPtr()->m_hWnd;
	//	execInfo.lpVerb = nullptr;
	//	execInfo.lpFile = exe.c_str();
	//	execInfo.nShow = SW_SHOWDEFAULT;
	//	::ShellExecuteEx(&execInfo);
	//}
	*e.HandledPtr = TRUE;
}



/***************/
/* VScrollDrag */
/***************/
void CTextBox::VScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_pVScroll->SetState(UIElementState::Dragged);
	m_pVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}
void CTextBox::VScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_pVScroll->SetState(UIElementState::Normal);
	m_pVScroll->SetStartDrag(0.f);
}
bool CTextBox::VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pVScroll->GetIsVisible() && m_pVScroll->GetThumbRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}
void CTextBox::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pVScroll->SetScrollPos(
		m_pVScroll->GetScrollPos() +
		(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y) - m_pVScroll->GetStartDrag()) *
		m_pVScroll->GetScrollDistance() /
		m_pVScroll->GetRectInWnd().Height());
	m_pVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}


/***************/
/* HScrollDrag */
/***************/
void CTextBox::HScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_pHScroll->SetState(UIElementState::Dragged);
	m_pHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CTextBox::HScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_pHScroll->SetState(UIElementState::Normal);
	m_pHScroll->SetStartDrag(0.f);
}

bool CTextBox::HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pHScroll->GetIsVisible() && m_pHScroll->GetThumbRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}

void CTextBox::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pHScroll->SetScrollPos(
		m_pHScroll->GetScrollPos() +
		(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x) - m_pHScroll->GetStartDrag()) *
		m_pHScroll->GetScrollDistance() /
		m_pHScroll->GetRectInWnd().Width());
	m_pHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CTextBox::Error_StdException(const std::exception& e)
{
	::OutputDebugStringA(e.what());

	std::string msg = fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n",
		e.what(), GetLastErrorString());

	MessageBoxA(GetWndPtr()->m_hWnd, msg.c_str(), "Exception in StateMachine", MB_ICONWARNING);
	Clear();
}









bool CTextBox::CopySelectionToClipboard()
{
	int selLen = *Caret->SelectedEnd - *Caret->SelectedBegin;
	if (selLen > 0) {
		std::wstring strCopy = Text->substr(*Caret->SelectedBegin, selLen);
		boost::algorithm::replace_all(strCopy, L"\n", L"\r\n");

		HGLOBAL hGlobal = ::GlobalAlloc(GHND | GMEM_SHARE, (strCopy.size() + 1) * sizeof(wchar_t));
		wchar_t* strMem = (wchar_t*)::GlobalLock(hGlobal);
		::GlobalUnlock(hGlobal);

		if (strMem != NULL) {
			::wcscpy_s(strMem, strCopy.size() + 1, strCopy.c_str());
			if (CClipboard clipboard; clipboard.Open(GetWndPtr()->m_hWnd) != 0) {
				clipboard.Empty();
				clipboard.SetData(CF_UNICODETEXT, hGlobal);
			}
		}
	} else {
		if (CClipboard clipboard; clipboard.Open(GetWndPtr()->m_hWnd) != 0) {
			clipboard.Empty();
			clipboard.SetData(CF_UNICODETEXT, NULL);
			clipboard.Close();
		}
	}
	return true;
}

bool CTextBox::PasteFromClipboard()
{
	if (CClipboard clipboard; clipboard.Open(GetWndPtr()->m_hWnd) != 0) {
		HANDLE hGlobal = clipboard.GetData(CF_UNICODETEXT);
		if (hGlobal) {
			std::wstring str((LPWSTR)GlobalLock(hGlobal));
//			boost::algorithm::replace_all(str, L"\r\n", L"\n");
			boost::algorithm::replace_all(str, L"\r", L"");
			//str = FilterInputString(str);
			ReplaceSelection(str);

			GlobalUnlock(hGlobal);
		}
	}
	return true;
}

CRectF CTextBox::GetRectInWnd() const
{
	return CD2DWControl::GetRectInWnd();
}

CRectF CTextBox::GetPageRect() const
{
	CRectF rcPage(GetRectInWnd());
	rcPage.DeflateRect(GetNormalBorder().Width * 0.5f);
	rcPage.DeflateRect(GetPadding());
	return rcPage;
}

void CTextBox::StartCaretBlink()
{
	m_bCaret = true;
	m_timer.run([this]()->void
		{
			m_bCaret = !m_bCaret;
			GetWndPtr()->InvalidateRect(NULL, FALSE);
		}, 
		std::chrono::milliseconds(::GetCaretBlinkTime()));
	GetWndPtr()->InvalidateRect(NULL, FALSE);
}

void CTextBox::StopCaretBlink()
{
	m_bCaret = false;
	m_timer.stop();
}

void CTextBox::CancelEdit()
{
	Text.assign(m_initText);
	Caret.get_unconst()->Move(Text->size(), GetOriginCharRects()[Text->size()].CenterPoint());
	//m_carets.set(std::get<caret::CurCaret>(m_carets.get()), Text->size(), Text->size(), Text->size(), Text->size());
}

void CTextBox::Insert(const std::wstring& text)
{
	auto cur = *Caret->Current;
	m_doMgr.Do(std::make_shared<CDoCommand>(
		[this, index = cur, value=text]()
		{ 
			Text.insert(index, value);
			//MoveCaret(index + value.size(), GetOriginCharRects()[index].CenterPoint());
		},
		[this, index = cur, count = text.size(), car = Caret->Clone()]()
		{
			Text.erase(index, count);
			Caret.get_unconst()->CopyValueOnly(car);
		}
		));
}
void CTextBox::ReplaceSelection(const std::wstring& text)
{
	auto selBeg = *Caret->SelectedBegin;
	auto selEnd = *Caret->SelectedEnd;
	m_doMgr.Do(std::make_shared<CDoCommand>(
		[this, index = selBeg, count = selEnd-selBeg, value=text]()
		{ 
			Text.replace(index, count, value);
			//MoveCaret(index + value.size(), GetOriginCharRects()[index].CenterPoint());
		},
		[this, index = selBeg, count = text.size(), value = Text->substr(selBeg, selEnd-selBeg), car = Caret->Clone()]()
		{
			Text.replace(index, count, value);
			Caret.get_unconst()->CopyValueOnly(car);
		}
		));
}

void CTextBox::Replace(const std::size_t& index, const std::size_t& count, const std::wstring& text)
{
	m_doMgr.Do(std::make_shared<CDoCommand>(
		[this, index = index, count = count, value=text]()
		{ 
			Text.replace(index, count, value);
			//MoveCaret(index + value.size(), GetOriginCharRects()[index].CenterPoint());
		},
		[this, index = index, count = text.size(), value = Text->substr(index, count), car = Caret->Clone()]()
		{
			Text.replace(index, count, value);
			Caret.get_unconst()->CopyValueOnly(car);
		}
		));
}

void CTextBox::DeleteOne()
{
	auto cur = *Caret->Current;
	m_doMgr.Do(std::make_shared<CDoCommand>(
		[this, index = cur, count = 1]()
		{ 
			Text.erase(index, count);
			//MoveCaret(index, GetOriginCharRects()[index].CenterPoint());
		},
		[this, index = cur, value = Text->substr(cur, 1), car = Caret->Clone()]()
		{
			Text.insert(index, value);
			Caret.get_unconst()->CopyValueOnly(car);
		}
		));
}
void CTextBox::BackspaceOne()
{
	auto cur = *Caret->Current;
	m_doMgr.Do(std::make_shared<CDoCommand>(
		[this, index = cur-1, count = 1]()
		{ 
			Text.erase(index, count);
			//MoveCaret(index, GetOriginCharRects()[index].CenterPoint());
		},
		[this, index = cur-1, value = Text->substr(cur-1, 1), car = Caret->Clone()]()
		{
			Text.insert(index, value);
			Caret.get_unconst()->CopyValueOnly(car);
		}
		));
}

void CTextBox::DeleteSelection()
{
	auto selBeg = *Caret->SelectedBegin;
	auto selEnd = *Caret->SelectedEnd;
	m_doMgr.Do(std::make_shared<CDoCommand>(
		[this, index = selBeg , count = selEnd-selBeg]
		{
			Text.erase(index, count);
			//MoveCaret(index, GetOriginCharRects()[index].CenterPoint());
		},
		[this, index = selBeg , value = Text->substr(selBeg, selEnd-selBeg), car = Caret->Clone()]
		{
			Text.insert(index, value);
			Caret.get_unconst()->CopyValueOnly(car);
		}
		));
}

void CTextBox::SelectAll()
{
	Caret.get_unconst()->Select(0, Text->size(), GetOriginCharRects()[Text->size()].CenterPoint());
}

void CTextBox::ClearText()
{
	Text.clear();
	//auto index = 0;
	//auto point = CPointF(0, GetLineHeight() * 0.5f);
	//MoveCaret(index, point);
}

bool CTextBox::GetIsVisible()const
{
	return true;
}

void CTextBox::PaintText(const PaintEvent& e)
{
	CRectF pageRect(GetPageRect());

	//const std::vector<CRectF>& charRects = GetActualCharRects();
	const std::vector<CRectF>& selCharRects = GetActualSelectionCharRects();

	//Draw Text
	//auto rcWnd = GetRectInWnd();
	auto rect = GetActualContentRect();
	auto origin = rect.LeftTop();

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->DrawTextLayout(
		origin, GetTextLayoutPtr(), GetWndPtr()->GetDirectPtr()->GetColorBrush(GetFormat().Color), D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);
	for (size_t i = 0; i < Text->size(); i++) {
		switch (Text->at(i)) {
			case L'\r':
				break;
			case L'\n':
				GetWndPtr()->GetDirectPtr()->DrawLineFeed(GetBlankLine(), selCharRects[i]);
				break;
			case L'\t':
				GetWndPtr()->GetDirectPtr()->DrawTab(GetBlankLine(), selCharRects[i]);
				break;
			case L' ':
				GetWndPtr()->GetDirectPtr()->DrawHalfSpace(GetBlankLine(), selCharRects[i]);
				break;
			case L'　':
				GetWndPtr()->GetDirectPtr()->DrawFullSpace(GetBlankLine(), selCharRects[i]);
				break;
			default:
				break;
		}
	}

	//auto low_iter = std::partition_point(charRects.begin(), charRects.end(), [rcWnd](const auto& x)->bool { return x.bottom < rcWnd.top; });
	//auto upp_iter = std::partition_point(charRects.begin(), charRects.end(), [rcWnd](const auto& x)->bool { return x.top < rcWnd.bottom; });

	//for (size_t i = low_iter - charRects.begin(), end = upp_iter - charRects.begin(); i < end; i++) {
	//	switch (Text->at(i)) {
	//		case L'\r':
	//			break;
	//		case L'\n':
	//			GetWndPtr()->GetDirectPtr()->DrawLineFeed(*(m_pProp->BlankLine), selCharRects[i]);
	//			break;
	//		case L'\t':
	//			GetWndPtr()->GetDirectPtr()->DrawTab(*(m_pProp->BlankLine), selCharRects[i]);
	//			break;
	//		case L' ':
	//			GetWndPtr()->GetDirectPtr()->DrawHalfSpace(*(m_pProp->BlankLine), selCharRects[i]);
	//		case L'　':
	//			GetWndPtr()->GetDirectPtr()->DrawFullSpace(*(m_pProp->BlankLine), selCharRects[i]);
	//		default:
	//			break;
	//	}
	//}
}

void CTextBox::PaintSelection(const PaintEvent& e)
{
	const std::vector<CRectF>& selCharRects = GetActualSelectionCharRects();
	for (auto n = *Caret->SelectedBegin; n < *Caret->SelectedEnd; n++) {
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(
			SolidFill(CColorF(0, 140.f / 255, 255.f / 255, 100.f / 255)),
			selCharRects[n]);
	}
}



void CTextBox::PaintCaret(const PaintEvent& e)
{
	//Draw Caret
	const std::vector<CRectF>& charRects = GetActualCharRects();
	CRectF caretRect = charRects[*Caret->Current];
	caretRect.right = caretRect.left + 1;
	if (m_isFirstDrawCaret) {
		m_isFirstDrawCaret = false;
		StartCaretBlink();
	}
	if (m_bCaret) {
		GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetFormat().Color, caretRect);
		GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}
}

void CTextBox::PaintCompositionLine(const PaintEvent& e)
{
	//Draw Composition line
	const std::vector<CRectF>& charRects = GetActualCharRects();
	for (const auto& compositionInfo : m_compositionInfos) {
		if (compositionInfo.DisplayAttribute.lsStyle != TF_LS_NONE) {
			CPointF ptStart, ptEnd;
			for (auto n = compositionInfo.Start; n < compositionInfo.End; n++) {
				if (n == compositionInfo.Start || (charRects[n - 1].bottom + charRects[n - 1].Height() / 2.f) < charRects[n].bottom) {
					ptStart.SetPoint(charRects[n].left, charRects[n].bottom);
				}
				if (n == (compositionInfo.End - 1) || (charRects[n].bottom + charRects[n].Height() / 2.f) < charRects[n + 1].bottom) {
					ptEnd.SetPoint(charRects[n].right, charRects[n].bottom);
					GetWndPtr()->GetDirectPtr()->DrawSolidTriangleWave(GetCompositionLine(), ptStart, ptEnd, 4.f, 8.f);
				}
			}
		}
	}
}

FLOAT CTextBox::GetLineHeight()
{
	return GetWndPtr()->GetDirectPtr()->CalcTextSize(GetFormat(), L"").height;
}

void CTextBox::ClearCompositionRenderInfo()
{
	m_compositionInfos.clear();
}

BOOL CTextBox::AddCompositionRenderInfo(int start, int end, TF_DISPLAYATTRIBUTE* pDisplayAttribute)
{
	m_compositionInfos.emplace_back(start, end, *pDisplayAttribute);
	return TRUE;
}

std::optional<int> CTextBox::GetOriginCharPosFromPoint(const CPointF& pt)
{
	if (Text->size() == 0) {
		return std::nullopt;
	}
	std::vector<CRectF> charRects(GetOriginCursorCharRects());

	auto iter = std::find_if(charRects.begin(), charRects.end(),
		[pt](const CRectF& rc)->bool {
			return rc.left <= pt.x && pt.x < rc.right &&
				rc.top <= pt.y && pt.y < rc.bottom;
		});
	if (iter == charRects.end()) {
		return std::nullopt;
	} else {
		return std::make_optional(std::distance(charRects.begin(), iter));
	}
}

std::optional<int> CTextBox::GetActualCharPosFromPoint(const CPointF& pt)
{
	if (Text->size() == 0) {
		return std::nullopt;
	}
	std::vector<CRectF> charRects(GetActualCursorCharRects());

	auto iter = std::find_if(charRects.begin(), charRects.end(),
		[pt](const CRectF& rc)->bool {
			return rc.left <= pt.x && pt.x < rc.right&&
				rc.top <= pt.y && pt.y < rc.bottom;
		});
	if (iter == charRects.end()) {
		return std::nullopt;
	} else {
		return std::make_optional(std::distance(charRects.begin(), iter));
	}
}

std::optional<int> CTextBox::GetActualCaptureCharPosFromPoint(const CPointF& pt)
{
	if (Text->size() == 0) {
		return std::nullopt;
	}
	std::vector<CRectF> charRects(GetActualCaptureCharRects());

	auto iter = std::find_if(charRects.begin(), charRects.end(),
		[pt](const CRectF& rc)->bool {
			return rc.left <= pt.x && pt.x < rc.right&&
				rc.top <= pt.y && pt.y < rc.bottom;
		});
	if (iter == charRects.end()) {
		return std::nullopt;
	} else {
		//auto ch = GetActualCharRects();
		//::OutputDebugStringA("char");
		//for (auto rc : ch) {
		//	::OutputDebugStringA(fmt::format("{}\t{}\t{}\t{}\r\n", rc.left, rc.top, rc.right, rc.bottom).c_str());
		//}

		//::OutputDebugStringA("cap");
		//for (auto rc : charRects) {
		//	::OutputDebugStringA(fmt::format("{}\t{}\t{}\t{}\r\n", rc.left, rc.top, rc.right, rc.bottom).c_str());
		//}

		return std::make_optional(std::distance(charRects.begin(), iter));
	}
}

std::optional<int> CTextBox::GetFirstCharPosInLine(const int& pos)
{
	std::vector<CRectF> charRects(GetOriginCharRects());
	CRectF curRect(charRects[pos]);
	for (auto i = pos - 1; i > 0; i--) {
		if (charRects[i].top != curRect.top) {
			return std::make_optional(i + 1);
		}
	}
	return std::make_optional(0);
}

std::optional<int> CTextBox::GetLastCharPosInLine(const int& pos)
{
	std::vector<CRectF> charRects(GetOriginCharRects());
	CRectF curRect(charRects[pos]);
	for (auto i = pos + 1; i < (int)charRects.size(); i++) {
		if (charRects[i].top != curRect.top) {
			return std::make_optional(i);
		}
	}
	return std::make_optional((int)charRects.size());

}

void CTextBox::ClearOriginRects()
{
	ClearOriginCharRects();
	ClearOriginCursorCharRects();
	ClearOriginCaptureCharRects();
	ClearOriginContentRect();
}

void CTextBox::ClearActualRects()
{
	ClearActualCharRects();
	ClearActualCursorCharRects();
	ClearActualCaptureCharRects();
	ClearActualSelectionCharRects();
	ClearActualContentRect();
}

void CTextBox::UpdateScroll()
{
	//VScroll
	//Page
	m_pVScroll->SetScrollPage(GetPageRect().Height());
	//Range
	m_pVScroll->SetScrollRange(0, GetOriginContentRect().Height());

	//HScroll
	//Page
	m_pHScroll->SetScrollPage(GetPageRect().Width());
	//Range
	m_pHScroll->SetScrollRange(0, GetOriginContentRect().Width());

	//VScroll
	//Position
	CRectF rcClient(GetRectInWnd());
	CRectF rcVertical;
	FLOAT lineHalfWidth = GetNormalBorder().Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_pHScroll->GetIsVisible() ? (m_pHScroll->GetBandWidth() + lineHalfWidth) : lineHalfWidth);
	//rcVertical.bottom = rcClient.bottom - lineHalfWidth;

	m_pVScroll->ArrangeCore(rcVertical);

	//HScroll
	//Position
	CRectF rcHorizontal;
	rcHorizontal.left = rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom - ::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_pVScroll->GetIsVisible() ? (m_pVScroll->GetBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;
	m_pHScroll->ArrangeCore(rcHorizontal);
}

void CTextBox::UpdateAll()
{
	ClearTextLayoutPtr();
	ClearOriginRects();
	UpdateScroll();
	ClearActualRects();
}

void CTextBox::TerminateCompositionString()
{
    if (GetTextStorePtr()->GetCurrentCompositionView())
    {
        CComPtr<ITfContextOwnerCompositionServices> pCompositionServices;
        if (GetContextPtr()->QueryInterface(IID_ITfContextOwnerCompositionServices, (void **)&pCompositionServices) == S_OK)
        {
            pCompositionServices->TerminateComposition(GetTextStorePtr()->GetCurrentCompositionView());
        }
    }
}
