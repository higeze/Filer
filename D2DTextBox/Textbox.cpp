#include "Textbox.h"
#include "D2DWWindow.h"
#include "TextEditSink.h"
#include "TextStoreACP.h"
#include "MyClipboard.h"
#include "CellProperty.h"
#include "Direct2DWrite.h"
#include "Debug.h"
#include "UIElement.h"
#include "GridView.h"
#include "TextCell.h"
#include "Scroll.h"
#include "ScrollProperty.h"//ScrollProperty
#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "ResourceIDFactory.h"

#include <regex>
#include <nameof/nameof.hpp>

#define TAB_WIDTH_4CHAR 4


//HRESULT InitDisplayAttrbute();
//HRESULT UninitDisplayAttrbute();

/*****************/
/* static member */
/*****************/

#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
std::function<CComPtr<ITfThreadMgr2>& ()> CTextBox::GetThreadMgr = [p = CComPtr<ITfThreadMgr2>()]() mutable->CComPtr<ITfThreadMgr2>&
{
	if (!p) {
		FAILED_THROW(::CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr2, (void**)&p));
	}
	return p;
};
#else
std::function<CComPtr<ITfThreadMgr>& ()> CTextBox::GetThreadMgr = [p = CComPtr<ITfThreadMgr>()]() mutable->CComPtr<ITfThreadMgr>&
{
	if (!p) {
		FAILED_THROW(::CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&p));
	}
	return p;
};
#endif

std::function<CComPtr<ITfKeystrokeMgr>& ()> CTextBox::GetKeystrokeMgr = [p = CComPtr<ITfKeystrokeMgr>()]() mutable->CComPtr<ITfKeystrokeMgr>&
{
	if (!p) {
		FAILED_THROW(GetThreadMgr()->QueryInterface(IID_ITfKeystrokeMgr, (void**)&p));
	}
	return p;
};

std::function<CComPtr<ITfDisplayAttributeMgr>& ()> CTextBox::GetDisplayAttributeMgr = [p = CComPtr<ITfDisplayAttributeMgr>()]() mutable->CComPtr<ITfDisplayAttributeMgr>&
{
	if (!p) {
		FAILED_THROW(::CoCreateInstance(
			CLSID_TF_DisplayAttributeMgr,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITfDisplayAttributeMgr,
			(void**)&p));
	}
	return p;
};

std::function<CComPtr<ITfCategoryMgr>& ()> CTextBox::GetCategoryMgr = [p = CComPtr<ITfCategoryMgr>()]() mutable->CComPtr<ITfCategoryMgr>&
{
	if (!p) {
		FAILED_THROW(::CoCreateInstance(
			CLSID_TF_CategoryMgr,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITfCategoryMgr,
			(void**)&p));
	}
	return p;
};


TfClientId CTextBox::s_tfClientId = TF_CLIENTID_NULL;

/*******************/
/* static function */
/*******************/
void CTextBox::AppTSFInit()
{
	FAILED_THROW(GetThreadMgr()->Activate(&s_tfClientId));
	//FAILED_THROW(InitDisplayAttrbute());
}
void CTextBox::AppTSFExit()
{
	//UninitDisplayAttrbute();

	if (GetThreadMgr()) {
		FAILED_THROW(GetThreadMgr()->Deactivate());
		GetThreadMgr().Attach(nullptr);
	}

	if (GetKeystrokeMgr()) {
		GetKeystrokeMgr().Attach(nullptr);
	}
}


CDispAttrProps* CTextBox::GetDispAttrProps()
{
    CComPtr<IEnumGUID> pEnumProp;
    CDispAttrProps *pProps = NULL;
	FAILED_THROW(CTextBox::GetCategoryMgr()->EnumItemsInCategory(GUID_TFCAT_DISPLAYATTRIBUTEPROPERTY, &pEnumProp));

    // Make a database for Display Attribute Properties.
    GUID guidProp;
    pProps = new CDispAttrProps;

    // Add System Display Attribute first.
    // So no other Display Attribute property overwrite it.
    pProps->Add(GUID_PROP_ATTRIBUTE);
	// SUCCEEDED:>=0
	// FAILED:<0
	// S_OK:0
	// S_FALSE:1
    while(pEnumProp->Next(1, &guidProp, NULL) == S_OK)
    {
		if (!::IsEqualGUID(guidProp, GUID_PROP_ATTRIBUTE)) {
			pProps->Add(guidProp);
		}
    }

	return pProps;
}

HRESULT CTextBox::GetDisplayAttributeTrackPropertyRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfReadOnlyProperty **ppProp, CDispAttrProps *pDispAttrProps)
{
    ITfReadOnlyProperty *pProp = NULL;
    GUID  *pguidProp = NULL;
    const GUID **ppguidProp;
    ULONG ulNumProp = 0;
    ULONG i;
 
    pguidProp = pDispAttrProps->GetPropTable();

    ulNumProp = pDispAttrProps->Count();

    // TrackProperties wants an array of GUID *'s
    //if ((ppguidProp = (const GUID **)LocalAlloc(LMEM_ZEROINIT, sizeof(GUID *)*ulNumProp)) == NULL)
	if ((ppguidProp = (const GUID **)new byte[sizeof(GUID* )*ulNumProp]) == NULL)
        return E_OUTOFMEMORY;

    for (i=0; i<ulNumProp; i++)
    {
        ppguidProp[i] = pguidProp++;
    }
    
    if (SUCCEEDED(pic->TrackProperties(ppguidProp, ulNumProp, 0, NULL, &pProp))){
        *ppProp = pProp;
    }

	delete [] (byte*)ppguidProp;
    //LocalFree(ppguidProp);
	return S_OK;
}

//+---------------------------------------------------------------------------
//
//  GetDisplayAttributeData
//
//----------------------------------------------------------------------------

HRESULT CTextBox::GetDisplayAttributeData(TfEditCookie ec, ITfReadOnlyProperty *pProp, ITfRange *pRange, TF_DISPLAYATTRIBUTE *pda, TfGuidAtom *pguid)
{
    VARIANT var;
    IEnumTfPropertyValue *pEnumPropertyVal;
    TF_PROPERTYVAL tfPropVal;
    GUID guid;
    TfGuidAtom gaVal;
    ITfDisplayAttributeInfo *pDAI;

    HRESULT hr = E_FAIL;

    hr = S_FALSE;
    if (SUCCEEDED(pProp->GetValue(ec, pRange, &var)))
    {
        if (SUCCEEDED(var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, 
                                                  (void **)&pEnumPropertyVal)))
        {
            while (pEnumPropertyVal->Next(1, &tfPropVal, NULL) == S_OK)
            {
                if (tfPropVal.varValue.vt == VT_EMPTY)
                    continue; // prop has no value over this span

                gaVal = (TfGuidAtom)tfPropVal.varValue.lVal;

                GetCategoryMgr()->GetGUID(gaVal, &guid);

                if (SUCCEEDED(GetDisplayAttributeMgr()->GetDisplayAttributeInfo(guid, &pDAI, NULL)))
                {
                    //
                    // Issue: for simple apps.
                    // 
                    // Small apps can not show multi underline. So
                    // this helper function returns only one 
                    // DISPLAYATTRIBUTE structure.
                    //
                    if (pda)
                    {
                        pDAI->GetAttributeInfo(pda);
                    }

                    if (pguid)
                    {
                        *pguid = gaVal;
                    }

                    pDAI->Release();
                    hr = S_OK;
                    break;
                }
            }
            pEnumPropertyVal->Release();
        }
        VariantClear(&var);
    }

    return hr;
}


/***************/
/* constructor */
/***************/
CTextBox::CTextBox(
	CD2DWControl* pParentControl,
	CTextCell* pCell,
	const std::shared_ptr<TextboxProperty> pProp,
	const std::wstring& text,
	std::function<void(const std::wstring&)> changed,
	std::function<void(const std::wstring&)> final)
	:CD2DWControl(pParentControl), m_pCell(pCell), m_pProp(pProp),
	m_changed(changed), m_final(final),
	m_initText(text),
	m_text(text),
	m_carets(0, text.size(), 0, 0, text.size()),
	m_pTextMachine(std::make_unique<CTextBoxStateMachine>(this)),
	m_pVScroll(std::make_unique<CVScroll>(this, pProp->VScrollPropPtr, [this](const wchar_t* name) { ResetActualRects(); })),
	m_pHScroll(std::make_unique<CHScroll>(this, pProp->HScrollPropPtr, [this](const wchar_t* name) { ResetActualRects(); }))
{
	GetTextStore = [p = CComPtr<CTextStore>(), this]() mutable->CComPtr<CTextStore>&
	{
		if (!p) {
			p.Attach(new CTextStore(this));
		}
		return p;
	};

	GetDocumentMgr = [p = CComPtr<ITfDocumentMgr>(), this]() mutable->CComPtr<ITfDocumentMgr>&
	{
		if (!p) {
			FAILED_THROW(GetThreadMgr()->CreateDocumentMgr(&p))
		}
		return p;
	};

	GetContext = [p = CComPtr<ITfContext>(), this]() mutable->CComPtr<ITfContext>&
	{
		if (!p) {
			FAILED_THROW(GetDocumentMgr()->CreateContext(s_tfClientId, 0, GetTextStore(), &p, &m_editCookie));
		}
		return p;
	};

	GetTextEditSink = [p = CComPtr<CTextEditSink>(), this]() mutable->CComPtr<CTextEditSink>&
	{
		if (!p) {
			p.Attach(new CTextEditSink(this));
		}
		return p;
	};
}
/**************/
/* destructor */
/**************/
CTextBox::~CTextBox(){}

void CTextBox::InitTSF()
{

	FAILED_THROW(GetDocumentMgr()->Push(GetContext()));


#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	FAILED_THROW(GetThreadMgr()->SetFocus(GetDocumentMgr()));
#else
	CComPtr<ITfDocumentMgr> pDocumentMgrPrev = NULL;
	FAILED_THROW(GetThreadMgr()->AssociateFocus(GetWndPtr()->m_hWnd, GetDocumentMgr(), &pDocumentMgrPrev));
#endif

	FAILED_THROW(GetTextEditSink()->_Advise(GetContext()));
}

void CTextBox::UninitTSF()
{
	if (GetTextEditSink()) {
		FAILED_THROW(GetTextEditSink()->_Unadvise());
	}

	if (GetDocumentMgr()) {
		FAILED_THROW(GetDocumentMgr()->Pop(TF_POPF_ALL));
	}
}

void CTextBox::Clear()
{
	m_carets.set(0, 0, 0, 0, 0);
	m_text.clear();

	m_pVScroll->SetScrollPage(GetPageRect().Height());
	m_pVScroll->SetScrollPos(0.f);
	m_pVScroll->SetScrollRange(0.f, OriginContentRect().Height());

	m_pHScroll->SetScrollPage(GetPageRect().Width());
	m_pHScroll->SetScrollPos(0.f);
	m_pHScroll->SetScrollRange(0.f, OriginContentRect().Width());
}

void CTextBox::MoveCaret(const int& index, const CPointF& point)
{
//	auto modPos = (newPos == m_text.size() - 1 && m_text[m_text.size() - 1] == L'\n' && std::get<caret::CurCaret>(m_carets) != newPos +1) ? newPos + 1 : newPos;
	m_carets.set(
		std::get<caret::CurCaret>(m_carets.get()), 
		index,
		index,
		index,
		index);

	m_caretPoint.set(point);

	ResetCaret();
}

void CTextBox::MoveCaretWithShift(const int& index, const CPointF& point)
{
	m_carets.set(
		std::get<caret::CurCaret>(m_carets.get()),
		index,
		std::get<caret::AncCaret>(m_carets.get()),
		(std::min)(std::get<caret::AncCaret>(m_carets.get()), index),
		(std::max)(std::get<caret::AncCaret>(m_carets.get()), index));

	m_caretPoint.set(point);

	ResetCaret();
}

void CTextBox::MoveSelection(const int& selBegin, const int& selEnd)
{
	m_carets.set(
		std::get<caret::CurCaret>(m_carets.get()),
		selEnd,
		selBegin,
		selBegin,
		selEnd);

	ResetCaret();
}

void CTextBox::EnsureVisibleCaret()
{
	if (m_isScrollable) {
		auto pageRect = GetPageRect();
		auto contentRect = OriginContentRect();
		auto charRects = OriginCharRects();
		if (!charRects.empty()) {
			//Range
			m_pVScroll->SetScrollRange(0, contentRect.Height());
			m_pHScroll->SetScrollRange(0, contentRect.Width());
			//Pos
			auto curRect = charRects[std::get<caret::CurCaret>(m_carets.get())];
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

	m_caretPoint.set(CPointF(0, GetLineHeight() * 0.5f));
	//m_caretPoint.set(CPointF(0, pProp->Format->Font.Size * 0.5f));
	TextLayoutPtr(
	[&val = TextLayoutPtr.value, this]()
	{
		if (!val) {
			auto pageRect = GetPageRect();
			auto pDirect = GetWndPtr()->GetDirectPtr();
			auto pRender = pDirect->GetD2DDeviceContext();
			auto pFactory = pDirect->GetDWriteFactory();
			auto size = CSizeF(m_pProp->IsWrap ? (std::max)(0.f, pageRect.Width()) : FLT_MAX, FLT_MAX);

			CComPtr<IDWriteTextLayout> pTextLayout0(nullptr);
			const IID* piid = &__uuidof(IDWriteTextLayout1);
			if (FAILED(pFactory->CreateTextLayout(m_text.c_str(), m_text.size(), pDirect->GetTextFormat(*m_pProp->Format), size.width, size.height, &pTextLayout0)) ||
				FAILED(pTextLayout0->QueryInterface(*piid, (void**)&val))) {
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
				range.length = m_text.size();
				val->SetTypography(typo, range);

				val->SetCharacterSpacing(0.0f, 0.0f, 0.0f, DWRITE_TEXT_RANGE{ 0, m_text.size() });
				val->SetPairKerning(FALSE, DWRITE_TEXT_RANGE{ 0, m_text.size() });

				//Syntax
				if (auto pTextEditorProp = std::dynamic_pointer_cast<TextEditorProperty>(m_pProp)) {
					for (const auto& tuple : pTextEditorProp->SyntaxAppearances.get()) {
						auto appearance = std::get<0>(tuple);
						if (!appearance.Regex.empty()) {
							auto brush = pDirect->GetColorBrush(appearance.SyntaxFormat.Color);

							std::wsmatch match;
							auto begin = m_text.cbegin();
							auto re = std::wregex(appearance.Regex);//L"/\\*.*?\\*/"
							UINT32 beginPos = 0;
							while (std::regex_search(begin, m_text.cend(), match, re)) {
								DWRITE_TEXT_RANGE range{ beginPos + (UINT32)match.position(), (UINT32)match.length() };
								val->SetDrawingEffect(brush, range);
								if (appearance.SyntaxFormat.IsBold) {
									val->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
								}

								begin = match[0].second;
								beginPos = std::distance(m_text.cbegin(), begin);
							}
						}
					}
				}
				//https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+
				//(?:[a-zA-Z]:|\\\\[a-zA-Z0-9_.$●-]+\\[a-zA-Z0-9_.$●-]+)\\(?:[^\\/:*?"<>|\r\n]+\\)*[^\\/:*?"<>|\r\n]* 
				// 
				


				//Executable
				//auto exePatterns = std::vector<std::wstring>{
				//	LR"((?:[a-zA-Z]:|\\\\[a-zA-Z0-9_.$●-]+\\[a-zA-Z0-9_.$●-]+)\\(?:[^\\/:*?"<>|\r\n]+\\)*[^\\/:*?"<>|\r\n]*)",
				//	LR"(https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+)"
				//};
				if (auto pTextEditorProp = std::dynamic_pointer_cast<TextEditorProperty>(m_pProp)) {
					m_executableInfos.clear();
					for (const auto& apr : pTextEditorProp->ExecutableAppearances) {
						auto brush = pDirect->GetColorBrush(apr.SyntaxFormat.Color);
						std::wsmatch match;
						auto begin = m_text.cbegin();
						auto re = std::wregex(apr.Regex);
						UINT32 beginPos = 0;
						while (std::regex_search(begin, m_text.cend(), match, re)) {
							DWRITE_TEXT_RANGE range{ beginPos + (UINT32)match.position(), (UINT32)match.length() };
							val->SetDrawingEffect(brush, range);
						if (apr.SyntaxFormat.IsBold) {
							val->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
						}
							val->SetUnderline(apr.SyntaxFormat.IsUnderline, range);


							m_executableInfos.push_back(ExecutableInfo{ match.str(), range.startPosition, range.length });
							begin = match[0].second;
							beginPos = std::distance(m_text.cbegin(), begin);
						}
					}
				}
			}
		}
		return val;

	}, nullptr);
	
	OriginCharRects([&val = OriginCharRects.value, this]()->std::vector<CRectF>&{
		if (val.empty()) {
			CRectF pageRect(GetPageRect());
			CSizeF size(m_pProp->IsWrap ? pageRect.Width() : FLT_MAX, FLT_MAX);

			val = CDirect2DWrite::CalcCharRects(TextLayoutPtr(), m_text.size());
			if (m_text.empty()) {
				auto size = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), L"");
				val.emplace_back(
					0.f, 0.f,
					size.width, size.height);
			} else if (m_text.back() == L'\n') {
				val.emplace_back(
					0.f, val.back().bottom,
					0.f, val.back().bottom + val.back().Height());
			} else {
				val.emplace_back(
					val.back().right, val.back().top,
					val.back().right, val.back().bottom);
			}
		}
		return val;
	}, nullptr);

	OriginCursorCharRects(
		[&val = OriginCursorCharRects.value, this]() {
		if (val.empty()) {
			CRectF pageRect(GetPageRect());
			val = OriginCharRects();
			//Max Right
			auto maxRight = (std::max)(pageRect.Width(), std::max_element(val.begin(), val.end(),
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

			for (std::size_t i = 0; i < val.size(); i++) {
				if (m_text[i] == L'\n' || i == m_text.size()) {
					val[i].right = maxRight;
				}
			}
		}
		return val;
	}, nullptr);

	OriginCaptureCharRects(
		[&val = OriginCaptureCharRects.value, this]() {
		if (val.empty()) {
			auto charRects = OriginCharRects();
			val = charRects;
			for (std::size_t i = 0; i < val.size(); i++) {
				//top
				if (charRects[i].top == charRects.front().top) {
					val[i].top = -FLT_MAX;
				}
				//left
				if (i == 0 || charRects[i].top > charRects[i - 1].top) {
					val[i].left = -FLT_MAX;
				}
				//right
				if (m_text[i] == L'\n' || i == m_text.size()) {
					val[i].right = FLT_MAX;
				}
				//bottom
				if (charRects[i].bottom == charRects.back().bottom) {
					val[i].bottom = FLT_MAX;
				}
			}
		};
		return val;
	}, nullptr);


	ActualCursorCharRects(
		[&val = ActualCursorCharRects.value, this]() {
		if (val.empty()) {
			val = OriginCursorCharRects();
			CPointF offset(GetPageRect().LeftTop());
			offset.Offset(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());
			std::for_each(val.begin(), val.end(),
				[offset](auto& rc) {rc.OffsetRect(offset); });
		}
		return val;
	}, nullptr);

	ActualCaptureCharRects(
		[&val = ActualCaptureCharRects.value, this](void)mutable->std::vector<CRectF>& {
		if (val.empty()) {
			val = OriginCaptureCharRects();
			CPointF offset(GetPageRect().LeftTop());
			offset.Offset(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());
			std::for_each(val.begin(), val.end(),
				[offset](auto& rc) {rc.OffsetRect(offset); });
		}
		return val;

	}, nullptr);

	ActualCharRects(
		[&val = ActualCharRects.value, this]() {
		if (val.empty())
		{
			val = OriginCharRects();
			CPointF offset(GetPageRect().LeftTop());
			offset.Offset(-m_pHScroll->GetScrollPos(), -m_pVScroll->GetScrollPos());
			std::for_each(val.begin(), val.end(),
				[offset](auto& rc) {rc.OffsetRect(offset); });
		}
		return val;
	}, nullptr);

	ActualSelectionCharRects(
		[&val = ActualSelectionCharRects.value, this]() {
		if (val.empty())
		{
			val = ActualCharRects();
			auto size = GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), L"a");
			for (size_t i = 0; i < m_text.size(); i++) {
				if (m_text[i] == L'\r' || m_text[i] == L'\n') {
					val[i].right += size.width;
				}
			}
		}
		return val;
	}, nullptr);

	OriginContentRect(
		[&val = OriginContentRect.value, this]()
	{
		auto rcPage(GetPageRect());
		auto charRects(OriginCharRects());
		if (!charRects.empty()) {
			val = CRectF(
				charRects.front().left,
				charRects.front().top,
				(std::max)(rcPage.Width(), std::max_element(charRects.begin(), charRects.end(), [](const auto& lhs, const auto& rhs) {return lhs.right < rhs.right; })->right),
				charRects.back().bottom);
		}
		return val;
	}, nullptr);

	ActualContentRect(
		[&val = ActualContentRect.value, this]()
	{
		auto rcPage(GetPageRect());
		auto charRects(ActualCharRects());
		if (!charRects.empty()) {
			val = CRectF(
				charRects.front().left,
				charRects.front().top,
				(std::max)(rcPage.right, std::max_element(charRects.begin(), charRects.end(), [](const auto& lhs, const auto& rhs) {return lhs.right < rhs.right; })->right),
				charRects.back().bottom);
		}
		return val;
	}, nullptr);

	FAILED_THROW(GetThreadMgr()->SetFocus(GetDocumentMgr()));
	
	m_text.Subscribe(
		[this](const NotifyStringChangedEventArgs<wchar_t>& e)->void {
			if (e.Action == NotifyStringChangedAction::Assign) {
				GetTextStore()->OnTextChange(0, 0, 0);
			} else {
				GetTextStore()->OnTextChange(e.StartIndex, e.OldEndIndex, e.NewEndIndex);
			}
			if (m_changed) { m_changed(e.NewString); }
			UpdateAll();
		}
	);
	m_carets.Subscribe(
		[this](const std::tuple<int, int, int, int, int>& value)->void {
			EnsureVisibleCaret();
		}
	);

	if (GetIsFocused()) {
		m_isFirstDrawCaret = true;
	}

	InitTSF();

	ResetCaret();
}

void CTextBox::OnDestroy(const DestroyEvent& e)
{
	m_timer.stop();
	UninitTSF();

	CD2DWControl::OnDestroy(e);
}

void CTextBox::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);

	if (!m_isClosing) {
		m_isClosing = true;
		if (m_final) { m_final(m_text); }
	}
}

void CTextBox::OnWndKillFocus(const KillFocusEvent& e)
{
	m_timer.stop();
}

void CTextBox::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	UpdateAll();
}

void CTextBox::OnMouseWheel(const MouseWheelEvent& e)
{
	m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollDelta() * e.Delta / WHEEL_DELTA);
}

/*****************/
/* State Machine */
/*****************/


void CTextBox::Normal_Paint(const PaintEvent& e)
{
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	//PaintBackground
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_pProp->NormalFill), GetRectInWnd());
	//PaintLine

	//Paint Focused Line
	CRectF rcBorder(GetRectInWnd());
	rcBorder.DeflateRect(1.0f, 1.0f);
	if (GetIsFocused()) {
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangle(*(m_pProp->FocusedLine), rcBorder);
	} else if (m_hasBorder) {
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangle(*(m_pProp->Line), rcBorder);
	} else {
		// Do nothing
	}
	
	//PaintContent
	Render();
	//PaintScroll
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CTextBox::Normal_SetFocus(const SetFocusEvent& e)
{
	FAILED_THROW(GetThreadMgr()->SetFocus(GetDocumentMgr()));
	m_isFirstDrawCaret = true;
}


void CTextBox::Normal_KillFocus(const KillFocusEvent& e)
{
	m_timer.stop();
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

	switch (e.Char) {
	case VK_CONTROL:
	{
		BOOL dummy = FALSE;
		Normal_SetCursor(SetCursorEvent(GetWndPtr(), HTCLIENT, &dummy));
		break;

	}
	case VK_LEFT:
	{
		auto position = std::clamp(std::get<caret::CurCaret>(m_carets.get()) - 1, 0, (int)m_text.size());
		auto point = OriginCharRects()[position].CenterPoint();

		if (shift) {
			MoveCaretWithShift(position, point);
		} else {
			MoveCaret(position, point);
		}
		break;
	}
	case VK_RIGHT:
	{
		auto position = std::clamp(std::get<caret::CurCaret>(m_carets.get()) + 1, 0, (int)m_text.size());
		auto point = OriginCharRects()[position].CenterPoint();

		if (shift) {
			MoveCaretWithShift(position, point);
		} else {
			MoveCaret(position, point);
		}
		break;
	}
	case VK_UP:
	{
		auto curCharRect = OriginCharRects()[std::get<caret::CurCaret>(m_carets.get())];
		auto point = CPointF(m_caretPoint.get().x,
			(std::max)(OriginCharRects().front().CenterPoint().y, m_caretPoint.get().y - curCharRect.Height()));
		if (auto position = GetOriginCharPosFromPoint(point)) {
			if (shift) {
				MoveCaretWithShift(position.value(), point);
			} else {
				MoveCaret(position.value(), point);
			}
		}
		break;
	}
	case VK_DOWN:
	{
		auto curCharRect = OriginCharRects()[std::get<caret::CurCaret>(m_carets.get())];
		auto point = CPointF(m_caretPoint.get().x,
			(std::min)(OriginCharRects().back().CenterPoint().y, m_caretPoint.get().y + curCharRect.Height()));
		if (auto newPos = GetOriginCharPosFromPoint(point)) {
			if (shift) {
				MoveCaretWithShift(newPos.value(), point);
			} else {
				MoveCaret(newPos.value(), point);
			}
		}
		break;
	}
	case VK_HOME:
	{
		if (auto index = GetFirstCharPosInLine(std::get<caret::CurCaret>(m_carets.get()))){
			auto point = OriginCharRects()[index.value()].CenterPoint();
			if (shift) {
				MoveCaretWithShift(index.value(), point);
			} else {
				MoveCaret(index.value(), point);
			}
		}
		break;
	}
	case VK_END:
	{
		if (auto index = GetLastCharPosInLine(std::get<caret::CurCaret>(m_carets.get()))) {
			auto point = OriginCharRects()[index.value()].CenterPoint();
			if (shift) {
				MoveCaretWithShift(index.value(), point);
			} else {
				MoveCaret(index.value(), point);
			}
		}
		break;
	}
	case VK_DELETE:
	{
		if (std::get<caret::SelBegin>(m_carets.get()) == std::get<caret::SelEnd>(m_carets.get())) {
			if (std::get<caret::CurCaret>(m_carets.get()) < (int)m_text.size()) {
				m_text.erase(std::get<caret::CurCaret>(m_carets.get()), 1);
			}
		} else {
			m_text.erase(std::get<caret::SelBegin>(m_carets.get()), std::get<caret::SelEnd>(m_carets.get()) - std::get<caret::SelBegin>(m_carets.get()));
			auto index = std::get<caret::SelBegin>(m_carets.get());
			auto point = OriginCharRects()[index].CenterPoint();
			MoveCaret(index, point);
		}
		break;
	}
	case VK_BACK:
	{
		if (std::get<caret::SelBegin>(m_carets.get()) == std::get<caret::SelEnd>(m_carets.get())) {
			if (std::get<caret::CurCaret>(m_carets.get()) > 0) {
				m_text.erase(std::get<caret::CurCaret>(m_carets.get()) - 1, 1);
				auto index = std::get<caret::CurCaret>(m_carets.get()) - 1;
				auto point = OriginCharRects()[index].CenterPoint();
				MoveCaret(index, point);
			}

		} else {
			m_text.erase(std::get<caret::SelBegin>(m_carets.get()), std::get<caret::SelEnd>(m_carets.get()) - std::get<caret::SelBegin>(m_carets.get()));
			auto index = std::get<caret::SelBegin>(m_carets.get());
			auto point = OriginCharRects()[index].CenterPoint();
			MoveCaret(index, point);
		}

		break;
	}
	//case VK_ESCAPE:
	//	break;
	case 'A':
	{
		if (ctrl) {
			m_carets.set(std::get<caret::CurCaret>(m_carets.get()), m_text.size(), 0, 0, m_text.size());
			m_caretPoint.set(OriginCharRects()[m_text.size()].CenterPoint());
			ResetCaret();
		}
	}
	case 'C':
	{
		if (ctrl) {
			CopySelectionToClipboard();
		}
		break;
	}
	case 'X':
	{
		if (ctrl) {
			CopySelectionToClipboard();
			m_text.erase(std::get<caret::SelBegin>(m_carets.get()), std::get<caret::SelEnd>(m_carets.get()) - std::get<caret::SelBegin>(m_carets.get()));
			auto index = std::get<caret::SelBegin>(m_carets.get());
			auto point = OriginCharRects()[index].CenterPoint();
			MoveCaret(index, point);
		}
		break;
	}
	case 'V':
	case VK_INSERT:
	{
		if (ctrl) {
			PasteFromClipboard();
		}
		break;
	}
	case VK_RETURN:
		InsertAtSelection(L"\n");
		break;
	case VK_TAB:
		InsertAtSelection(L"\t");
		break;

	default:
		break;

	}
}

void CTextBox::Normal_LButtonDown(const LButtonDownEvent& e)
{
	auto newPoint = GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);

	if (auto index = GetActualCharPosFromPoint(newPoint)) {
		auto point = OriginCharRects()[index.value()].CenterPoint();
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			MoveCaretWithShift(index.value(), point);
			return;
		} else {
			if (::GetAsyncKeyState(VK_CONTROL)) {
				if (auto pos = GetActualCharPosFromPoint(e.PointInWnd)) {
					auto iter = std::find_if(m_executableInfos.begin(), m_executableInfos.end(), [p = static_cast<UINT32>(pos.value())](const auto& info)->bool
					{
						return p >= info.StartPosition && p < info.StartPosition + info.Length;
					});
					if (iter != m_executableInfos.end()) {
						auto exe = iter->Link;
						exe = ((exe.front() == L'\"') ? L"" : L"\"") + boost::algorithm::trim_copy(exe) + ((exe.back() == L'\"') ? L"" : L"\"");
						SHELLEXECUTEINFO execInfo = {};
						execInfo.cbSize = sizeof(execInfo);
						execInfo.hwnd = GetWndPtr()->m_hWnd;
						execInfo.lpVerb = L"open";
						execInfo.lpFile = exe.c_str();
						execInfo.nShow = SW_SHOWDEFAULT;
						::ShellExecuteEx(&execInfo);			
						return;
					}
				}
			}

			MoveCaret(index.value(), point);
		}
	}
}

void CTextBox::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
	auto newPoint = GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient);
	std::vector<wchar_t> delimiters{ L' ', L'\t', L'\n' };

	if (auto index = GetActualCharPosFromPoint(newPoint)) {
		if (std::find(delimiters.begin(), delimiters.end(), m_text[index.value()]) == delimiters.end()) {
			size_t selBegin = index.value() - 1;
			for (; selBegin > 0; --selBegin) {
				if (std::find(delimiters.begin(), delimiters.end(), m_text[selBegin]) != delimiters.end()) {
					selBegin++;
					break;
				}
			}
			size_t selEnd = index.value() + 1;
			for (; selEnd < m_text.size(); ++selEnd) {
				if (std::find(delimiters.begin(), delimiters.end(), m_text[selEnd]) != delimiters.end()) {
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
			auto point = OriginCharRects()[index.value()].CenterPoint();
			MoveCaretWithShift(index.value(), point);
		}
	}
}

void CTextBox::Normal_MouseMove(const MouseMoveEvent& e){}

void CTextBox::Normal_Char(const CharEvent& e)
{
	bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

	if (heldControl) {
		return;
	}

	// normal character input. not TSF.
	if (e.Char >= L' ') {
		if (e.Char < 256) {
			WCHAR wc[] = { static_cast<WCHAR>(e.Char), '\0' };
			InsertAtSelection(wc);
		}
		else {
		}
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
			if (::GetAsyncKeyState(VK_CONTROL)) {
				if (auto pos = GetActualCharPosFromPoint(e.PointInWnd)) {
					auto iter = std::find_if(m_executableInfos.begin(), m_executableInfos.end(), [p = static_cast<UINT32>(pos.value())](const auto& info)->bool
					{
						return p >= info.StartPosition && p < info.StartPosition + info.Length;

					});
					if (iter != m_executableInfos.end()) {
						::SetCursor(::LoadCursor(NULL, IDC_HAND));
						*(e.HandledPtr) = TRUE;
						return;
					}
				}
			}
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
	//	execInfo.lpVerb = L"open";
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
	int selLen = std::get<caret::SelEnd>(m_carets.get()) - std::get<caret::SelBegin>(m_carets.get());
	if (selLen > 0) {
		std::wstring strCopy = m_text.substr(std::get<caret::SelBegin>(m_carets.get()), selLen);
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
			m_text.replace(std::get<caret::SelBegin>(m_carets.get()), std::get<caret::SelEnd>(m_carets.get()) - std::get<caret::SelBegin>(m_carets.get()), str);
			auto index = std::get<caret::SelBegin>(m_carets.get()) + str.size();
			auto point = OriginCharRects()[index].CenterPoint();

			MoveCaret(index, point);
			GlobalUnlock(hGlobal);
		}
	}
	return true;
}

CRectF CTextBox::GetRectInWnd() const
{
	if (m_pCell) {
		return m_pCell->GetEditRect();
	} else {
		return CD2DWControl::GetRectInWnd();
	}
}

CRectF CTextBox::GetPageRect() const
{
	CRectF rcPage(GetRectInWnd());
	rcPage.DeflateRect(m_pProp->Line->Width * 0.5f);
	rcPage.DeflateRect(*(m_pProp->Padding));
	return rcPage;
}


void CTextBox::ResetCaret()
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

void CTextBox::DrawCaret(const CRectF& rc)
{
	if (m_isFirstDrawCaret) {
		m_isFirstDrawCaret = false;
		ResetCaret();
	}
	if (m_bCaret) {
		GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_pProp->Format->Color, rc);
		GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}
}

void CTextBox::CancelEdit()
{
	m_text.assign(m_initText);
	m_carets.set(std::get<caret::CurCaret>(m_carets.get()), m_text.size(), m_text.size(), m_text.size(), m_text.size());
}

BOOL CTextBox::InsertAtSelection(LPCWSTR psz)
{
	m_text.replace(std::get<caret::SelBegin>(m_carets.get()), std::get<caret::SelEnd>(m_carets.get()) - std::get<caret::SelBegin>(m_carets.get()), psz);
	auto index = std::get<caret::SelBegin>(m_carets.get()) + lstrlen(psz);
	auto point = OriginCharRects()[index].CenterPoint();

	MoveCaret(index, point);
	return TRUE;
}

void CTextBox::ClearText()
{
	m_text.clear();
	auto index = 0;
	auto point = CPointF(0, GetLineHeight() * 0.5f);
	MoveCaret(index, point);
}

bool CTextBox::GetIsVisible()const
{
	if (m_pCell) {
		return m_pCell->GetIsVisible();
	} else {
		return true;
	}
}

void CTextBox::Render()
{
	CRectF pageRect(GetPageRect());

	if (!m_text.empty()) {
		std::vector<CRectF> charRects = ActualCharRects();
		std::vector<CRectF> selCharRects = ActualSelectionCharRects();

		//Draw Text
		auto rcWnd = GetRectInWnd();
		auto rect = ActualContentRect();
		auto origin = rect.LeftTop();

		GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->DrawTextLayout(origin, TextLayoutPtr(), GetWndPtr()->GetDirectPtr()->GetColorBrush(m_pProp->Format->Color), D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);
		//GetWndPtr()->GetDirectPtr()->DrawTextLayout((*(m_pProp->Format), m_text, GetActualContentRect());

		auto low_iter = std::partition_point(charRects.begin(), charRects.end(), [rcWnd](const auto& x)->bool { return x.bottom < rcWnd.top; });
		auto upp_iter = std::partition_point(charRects.begin(), charRects.end(), [rcWnd](const auto& x)->bool { return x.top < rcWnd.bottom; });

		for (size_t i = low_iter - charRects.begin(), end = upp_iter - charRects.begin(); i < end; i++) {
			switch (m_text[i]) {
				case L'\r':
					break;
				case L'\n':
					GetWndPtr()->GetDirectPtr()->DrawLineFeed(*(m_pProp->BlankLine), selCharRects[i]);
					break;
				case L'\t':
					GetWndPtr()->GetDirectPtr()->DrawTab(*(m_pProp->BlankLine), selCharRects[i]);
					break;
				case L' ':
					GetWndPtr()->GetDirectPtr()->DrawHalfSpace(*(m_pProp->BlankLine), selCharRects[i]);
				case L'　':
				default:
					break;
			}
		}

		//Draw Selection
		for (auto n = std::get<caret::SelBegin>(m_carets.get()); n < std::get<caret::SelEnd>(m_carets.get()); n++) {
			GetWndPtr()->GetDirectPtr()->FillSolidRectangle(
				SolidFill(CColorF(0, 140.f / 255, 255.f / 255, 100.f / 255)),
				selCharRects[n]);
		}
		//Draw Caret
		CRectF caretRect = charRects[std::get<caret::CurCaret>(m_carets.get())];
		caretRect.right = caretRect.left + 1;
		DrawCaret(caretRect);
		//Draw Composition line
		for (const auto& compositionInfo : m_compositionInfos) {
			if (compositionInfo.DisplayAttribute.lsStyle != TF_LS_NONE) {
				CPointF ptStart, ptEnd;
				for (auto n = compositionInfo.Start; n < compositionInfo.End; n++) {
					if (n == compositionInfo.Start || (charRects[n - 1].bottom + charRects[n - 1].Height() / 2.f) < charRects[n].bottom) {
						ptStart.SetPoint(charRects[n].left, charRects[n].bottom);
					}
					if (n == (compositionInfo.End - 1) || (charRects[n].bottom + charRects[n].Height() / 2.f) < charRects[n + 1].bottom) {
						ptEnd.SetPoint(charRects[n].right, charRects[n].bottom);
						GetWndPtr()->GetDirectPtr()->DrawSolidTriangleWave(*(m_pProp->CompositionLine), ptStart, ptEnd, 4.f, 8.f);
					}
				}
			}
		}

	} else {
		std::vector<CRectF> charRects = ActualCharRects();
		//Draw Caret
		CRectF caretRect = charRects[std::get<caret::CurCaret>(m_carets.get())];
		caretRect.right = caretRect.left + 1;
		DrawCaret(caretRect);
	}
}

FLOAT CTextBox::GetLineHeight()
{
	return GetWndPtr()->GetDirectPtr()->CalcTextSize(*(m_pProp->Format), L"").height;
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
	if (m_text.size() == 0) {
		return std::nullopt;
	}
	std::vector<CRectF> charRects(OriginCursorCharRects());

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
	if (m_text.size() == 0) {
		return std::nullopt;
	}
	std::vector<CRectF> charRects(ActualCursorCharRects());

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
	if (m_text.size() == 0) {
		return std::nullopt;
	}
	std::vector<CRectF> charRects(ActualCaptureCharRects());

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
	std::vector<CRectF> charRects(OriginCharRects());
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
	std::vector<CRectF> charRects(OriginCharRects());
	CRectF curRect(charRects[pos]);
	for (auto i = pos + 1; i < (int)charRects.size(); i++) {
		if (charRects[i].top != curRect.top) {
			return std::make_optional(i);
		}
	}
	return std::make_optional((int)charRects.size());

}

void CTextBox::ResetOriginRects()
{
	OriginCharRects.value.clear();
	OriginCursorCharRects.value.clear();
	OriginCaptureCharRects.value.clear();
}

void CTextBox::ResetActualRects()
{
	ActualCharRects.value.clear();
	ActualCursorCharRects.value.clear();
	ActualCaptureCharRects.value.clear();
	ActualSelectionCharRects.value.clear();
}

void CTextBox::UpdateScroll()
{
	//VScroll
	//Page
	m_pVScroll->SetScrollPage(GetPageRect().Height());
	//Range
	m_pVScroll->SetScrollRange(0, OriginContentRect().Height());

	//HScroll
	//Page
	m_pHScroll->SetScrollPage(GetPageRect().Width());
	//Range
	m_pHScroll->SetScrollRange(0, OriginContentRect().Width());

	//VScroll
	//Position
	CRectF rcClient(GetRectInWnd());
	CRectF rcVertical;
	FLOAT lineHalfWidth = m_pProp->Line->Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_pHScroll->GetIsVisible() ? (m_pHScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	//rcVertical.bottom = rcClient.bottom - lineHalfWidth;

	m_pVScroll->OnRect(RectEvent(GetWndPtr(), rcVertical));

	//HScroll
	//Position
	CRectF rcHorizontal;
	rcHorizontal.left = rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom - ::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_pVScroll->GetIsVisible() ? (m_pVScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;
	m_pHScroll->OnRect(RectEvent(GetWndPtr(), rcHorizontal));
}

void CTextBox::UpdateAll()
{
	TextLayoutPtr.value = nullptr;
	ResetOriginRects();
	UpdateScroll();
	ResetActualRects();
}


/***************/
/* CTextEditor */
/***************/

CTextEditor::CTextEditor(
	CD2DWControl* pParentControl,
	const std::shared_ptr<TextEditorProperty>& spProp,
	const std::shared_ptr<StatusBarProperty>& spStatusProp)
	:CD2DWControl(pParentControl),
	m_pProp(spProp),
	m_spTextBox(std::make_shared<CTextBox>(this, nullptr, spProp, L"", nullptr, nullptr)),
	m_spStatusBar(std::make_shared<CStatusBar>(this, spStatusProp))
{
	m_spTextBox->SetHasBorder(false);
	m_spTextBox->SetIsScrollable(true);

	m_spStatusBar->GetIsFocusable().set(false);
	
	m_open.Subscribe([this](HWND hWnd)
	{
		m_spTextBox->UpdateAll();
	}, 
	100);
	m_encoding.Subscribe([this](const encoding_type& e)
	{
		m_spStatusBar->SetText(str2wstr(std::string(nameof::nameof_enum(e))));
	});
}

std::tuple<CRectF, CRectF> CTextEditor::GetRects() const
{
	CRectF rcClient = GetRectInWnd();

	FLOAT statusHeight = m_spStatusBar->MeasureSize(GetWndPtr()->GetDirectPtr()).height;
	CRectF rcText(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom - statusHeight);
	CRectF rcStatus(rcClient.left, rcText.bottom, rcClient.right, rcClient.bottom);

	if (rcStatus.left > rcStatus.right) {
		auto i = 10;
	}

	return { rcText, rcStatus };
}


void CTextEditor::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcText, rcStatus] = GetRects();
	m_spTextBox->OnCreate(CreateEvt(GetWndPtr(), this, rcText));
	m_spStatusBar->OnCreate(CreateEvt(GetWndPtr(), this, rcStatus));
}

void CTextEditor::OnPaint(const PaintEvent& e)
{
	m_spTextBox->OnPaint(e);
	m_spStatusBar->OnPaint(e);
}

void CTextEditor::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	auto [rcText, rcStatus] = GetRects();
	m_spTextBox->OnRect(RectEvent(GetWndPtr(), rcText));
	m_spStatusBar->OnRect(RectEvent(GetWndPtr(), rcStatus));
	m_spTextBox->UpdateAll();
}


void CTextEditor::OnKeyDown(const KeyDownEvent& e)
{
	bool ctrl = ::GetAsyncKeyState(VK_CONTROL);
	bool shift = ::GetAsyncKeyState(VK_SHIFT);
	switch (e.Char) {
		case 'O':
			if (ctrl && shift) {
				OpenAs();
				*e.HandledPtr = TRUE;
			} else if (ctrl) {
				Open();
				*e.HandledPtr = TRUE;
			}
			break;
		case 'S':
			if (ctrl && shift) {
				SaveAs();
				*e.HandledPtr = TRUE;
			}else if (ctrl) {
				Save();
				*e.HandledPtr = TRUE;
			}
			break;
		default:
			CD2DWControl::OnKeyDown(e);
			break;
	}
}

void CTextEditor::Open()
{
	m_open.Execute(GetWndPtr()->m_hWnd);
}

void CTextEditor::OpenAs()
{
	m_open_as.Execute(GetWndPtr()->m_hWnd);
}

void CTextEditor::Save()
{
	m_save.Execute(GetWndPtr()->m_hWnd);
}

void CTextEditor::SaveAs()
{
	m_save_as.Execute(GetWndPtr()->m_hWnd);
}

void CTextEditor::Update()
{
	m_spTextBox->UpdateAll();
}

