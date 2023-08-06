#include "PdfView.h"
#include <format>
#include <boost/algorithm/string/join.hpp>
#include "PDFDoc.h"
#include "PDFPage.h"
#include "PDFCaret.h"
#include "PDFBitmapDrawer.h"
#include "D2DWWindow.h"
#include "Scroll.h"
#include "Debug.h"
#include "MyMenu.h"
#include "ResourceIDFactory.h"
#include <functional>
#include <ranges>
#include "HiLibResource.h"
#include "PDFViewport.h"
#include "PdfViewStateMachine.h"
#include "TextBoxDialog.h"
#include "DialogProperty.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "Button.h"
#include "ButtonProperty.h"
#include "strconv.h"
#include "Dispatcher.h"
#include "ThreadPool.h"

/**************************/
/* Constructor/Destructor */
/**************************/

CPdfView::CPdfView(CD2DWControl* pParentControl, const std::shared_ptr<PdfViewProperty>& pProp)
	:CD2DWControl(pParentControl),
    m_pProp(pProp),
	m_pdfDrawer(std::make_unique<CD2DPDFBitmapDrawer>()),
	m_caret(this),
	m_pMachine(std::make_unique<CPdfViewStateMachine>(this)),
	m_spVScroll(std::make_shared<CVScroll>(this, pProp->VScrollPropPtr)),
	m_spHScroll(std::make_shared<CHScroll>(this, pProp->HScrollPropPtr)),
	m_prevScale(0.f),
	m_initialScaleMode(InitialScaleMode::Width),
	PDF(make_reactive_property<CPDFDoc>()),
	Scale(make_reactive_property<FLOAT>(0.f)),
	CurrentPage(make_reactive_property<int>(0)),
	TotalPage(make_reactive_property<int>(0)),
	Find(make_reactive_wstring(L"")){}

CPdfView::~CPdfView() = default;

/****************/
/* EventHandler */
/****************/

void CPdfView::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	auto [rcVertical, rcHorizontal] = GetRects();
	m_spVScroll->OnCreate(CreateEvt(GetWndPtr(), this, rcVertical));
	m_spHScroll->OnCreate(CreateEvt(GetWndPtr(), this, rcHorizontal));
	Find->subscribe([this](const reactive_wstring::notify_type& notify)
	{
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	}
	, shared_from_this());
}

CRectF CPdfView::GetRenderRectInWnd()
{
    CRectF rc = GetRectInWnd();
	rc.DeflateRect(m_pProp->FocusedLine->Width * 0.5f);
	rc.DeflateRect(*(m_pProp->Padding));
	return rc;
}

CSizeF CPdfView::GetRenderSize()
{
	return GetRenderRectInWnd().Size();
}

CSizeF CPdfView::GetRenderContentSize()
{
	if (PDF) {
		return PDF->get_const().GetSize() * Scale->get_const();
	} else {
		return CSizeF(0.f, 0.f);
	}
};

std::tuple<int, int> CPdfView::GetPageAndIndexFromWndPoint(const CPointF& ptInWnd)
{
	auto [page, ptInPdfiumPage] = Wnd2PdfiumPage(ptInWnd);
	if (page >= 0) {
		return std::make_tuple(page, PDF->get_const().GetPage(page)->GetCursorCharIndexAtPos(ptInPdfiumPage));
	} else {
		return std::make_tuple(page, -1);
	}
}

void CPdfView::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);
}

void CPdfView::OnDestroy(const DestroyEvent& e)
{
	m_caret.StopBlink();

	CD2DWControl::OnDestroy(e);
}

void CPdfView::OnEnable(const EnableEvent& e)
{
	//if (e.Enable) {
	//	if (!PDF) {
	//		Open(m_path.get());
	//	}
	//} else {
	//	if (PDF) {
	//		Close();
	//	}
	//}
}

void CPdfView::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);
	UpdateScroll();
}

void CPdfView::OnMouseWheel(const MouseWheelEvent& e)
{
	if(::GetAsyncKeyState(VK_CONTROL)){
		FLOAT factor = static_cast<FLOAT>(std::pow(1.1f, (std::abs(e.Delta) / WHEEL_DELTA)));
		FLOAT multiply = (e.Delta > 0) ? factor : 1/factor;
		FLOAT prevScale = Scale->get_const();
		Scale->set(std::clamp(Scale->get_const() * multiply, 0.1f, 8.f));
		m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() * Scale->get_const() / prevScale);
		m_spHScroll->SetScrollPos(m_spHScroll->GetScrollPos() * Scale->get_const() / prevScale);

	} else {
		m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() - m_spVScroll->GetScrollDelta() * e.Delta / WHEEL_DELTA);
	}
}

void CPdfView::OnWndKillFocus(const KillFocusEvent& e)
{
	m_caret.StopBlink();
}

/**********/
/* Normal */
/**********/

void CPdfView::Normal_LButtonDown(const LButtonDownEvent& e)
{
}

void CPdfView::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
}

CRectF CPdfView::Wnd2Ctrl(const CRectF& rcInWnd)
{
	return rcInWnd - GetRenderRectInWnd().LeftTop();
}
CPointF CPdfView::Wnd2Ctrl(const CPointF& ptInWnd)
{
	return ptInWnd - GetRenderRectInWnd().LeftTop();
}
CRectF CPdfView::Ctrl2Wnd(const CRectF& rcInCtrl)
{
	return rcInCtrl + GetRenderRectInWnd().LeftTop();
}
CPointF CPdfView::Ctrl2Wnd(const CPointF& ptInCtrl)
{
	return ptInCtrl + GetRenderRectInWnd().LeftTop();
}

CRectF CPdfView::Ctrl2Doc(const CRectF& rcInCtrl)
{
	CPointF ptScroll = CPointF(GetHScrollPtr()->GetScrollPos(), GetVScrollPtr()->GetScrollPos());
	return (rcInCtrl + ptScroll) / Scale->get_const();
}
CPointF CPdfView::Ctrl2Doc(const CPointF& ptInCtrl)
{
	CPointF ptScroll = CPointF(GetHScrollPtr()->GetScrollPos(), GetVScrollPtr()->GetScrollPos());
	return (ptInCtrl + ptScroll) / Scale->get_const() ;
}
CRectF CPdfView::Doc2Ctrl(const CRectF& rcInDoc)
{
	CPointF ptScroll = CPointF(GetHScrollPtr()->GetScrollPos(), GetVScrollPtr()->GetScrollPos());
	return rcInDoc * Scale->get_const() - ptScroll;
}
CPointF CPdfView::Doc2Ctrl(const CPointF& ptInDoc)
{
	CPointF ptScroll = CPointF(GetHScrollPtr()->GetScrollPos(), GetVScrollPtr()->GetScrollPos());
	return ptInDoc * Scale->get_const() - ptScroll;
}

CRectF CPdfView::Doc2Page(const int& i, const CRectF& rcInDoc)
{
	return rcInDoc - PDF->get_const().GetPageRects()[i].LeftTop();
}
CPointF CPdfView::Doc2Page(const int& i, const CPointF& ptInDoc)
{
	return ptInDoc - PDF->get_const().GetPageRects()[i].LeftTop();
}
CRectF CPdfView::Page2Doc(const int& i, const CRectF& rcInPage)
{
	return rcInPage + PDF->get_const().GetPageRects()[i].LeftTop();
}
CPointF CPdfView::Page2Doc(const int& i, const CPointF& ptInPage)
{
	return ptInPage + PDF->get_const().GetPageRects()[i].LeftTop();
}

std::tuple<int, CPointF> CPdfView::Doc2Page(const CPointF& ptInDoc)
{
	const std::vector<CRectF>& rectsInDoc = PDF->get_const().GetPageRects();

	auto iter = std::find_if(rectsInDoc.cbegin(), rectsInDoc.cend(), [pt = ptInDoc](const CRectF& rc) { return rc.PtInRect(pt); });
	if (iter != rectsInDoc.cend()) {
		return {std::distance(rectsInDoc.cbegin(), iter), ptInDoc - iter->LeftTop()};
	} else {
		return {-1, CPointF()};
	}
}

CRectF CPdfView::Page2PdfiumPage(const int& page, const CRectF& rcInPage)
{
	CSizeF sz = PDF->get_const().GetPage(page)->GetSize();
	return CRectF(
		rcInPage.left,
		sz.height - rcInPage.top,
		rcInPage.right,
		sz.height - rcInPage.bottom);
}
CPointF CPdfView::Page2PdfiumPage(const int& page, const CPointF& ptInPage)
{
	CSizeF sz = PDF->get_const().GetPage(page)->GetSize();
	return CPointF(
		ptInPage.x,
		sz.height - ptInPage.y);
}

CRectF CPdfView::PdfiumPage2Page(const int& page, const CRectF& rcInPdfiumPage)
{
	CSizeF sz = PDF->get_const().GetPage(page)->GetSize();
	return CRectF(
		rcInPdfiumPage.left,
		sz.height - rcInPdfiumPage.top,
		rcInPdfiumPage.right,
		sz.height - rcInPdfiumPage.bottom);
}

CPointF CPdfView::PdfiumPage2Page(const int& page, const CPointF& ptInPdfiumPage)
{
	CSizeF sz = PDF->get_const().GetPage(page)->GetSize();
	return CPointF(
		ptInPdfiumPage.x,
		sz.height - ptInPdfiumPage.y);
}

std::tuple<int, CPointF> CPdfView::Wnd2PdfiumPage(const CPointF& ptInWnd)
{
	try {
		auto ptInCtrl = Wnd2Ctrl(ptInWnd);
		auto ptInDoc = Ctrl2Doc(ptInCtrl);
		auto [page, ptInPage] = Doc2Page(ptInDoc);
		if (page < 0) {
			return { page, CPointF() };
		} else {
			auto ptInPdfiumPage = Page2PdfiumPage(page, ptInPage);
			return { page, ptInPdfiumPage };
		}
	}
	catch(...){
		return { -1, CPointF() };
	}
}

CRectF CPdfView::PdfiumPage2Wnd(const int& i, const CRectF rcInPdfiumPage)
{
	auto rcInPage = PdfiumPage2Page(i, rcInPdfiumPage);
	auto rcInDoc = Page2Doc(i, rcInPage);
	auto rcInCtrl = Doc2Ctrl(rcInDoc);
	return Ctrl2Wnd(rcInCtrl);
}

CPointF CPdfView::PdfiumPage2Wnd(const int& i, const CPointF ptInPdfiumPage)
{
	auto ptInPage = PdfiumPage2Page(i, ptInPdfiumPage);
	auto ptInDoc = Page2Doc(i, ptInPage);
	auto ptInCtrl = Doc2Ctrl(ptInDoc);
	return Ctrl2Wnd(ptInCtrl);
}

void CPdfView::Normal_Paint(const PaintEvent& e)
{
	if (!PDF || !PDF->get_const().IsOpen()) { return; }
	
	bool debug = m_pMachine->IsStateNormalDebug();
	std::vector<std::wstring> logs;

	logs.push_back(std::format(
		L"Drawer Thread:{}/{}, Task:{}",
		m_pdfDrawer->GetThreadPoolPtr()->GetActiveThreadCount(),
		m_pdfDrawer->GetThreadPoolPtr()->GetTotalThreadCount(),
		m_pdfDrawer->GetThreadPoolPtr()->GetQueuedTaskCount()));

	logs.push_back(std::format(L"Scroll(x, y):\t({}, {})", m_spHScroll->GetScrollPos(), m_spVScroll->GetScrollPos()));

	//Clip
	GetWndPtr()->GetDirectPtr()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	//PaintBackground
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_pProp->NormalFill), GetRectInWnd());

	//PaintContent
	CRectF rcInWnd = GetRenderRectInWnd();
	CRectF rcInCtrl = Wnd2Ctrl(rcInWnd);
	CRectF rcInDoc = Ctrl2Doc(rcInCtrl);

	logs.push_back(std::format(L"Paint Rect in Doc:\t({}, {}, {}, {})", rcInDoc.left, rcInDoc.top, rcInDoc.right, rcInDoc.bottom));

	const std::vector<CRectF>& orgRectsInDoc = PDF->get_const().GetPageRects();
	
	//std::vector<CRectF> scaledRectsInDoc;
	//std::transform(orgRectsInDoc.cbegin(), orgRectsInDoc.cend(), std::back_inserter(scaledRectsInDoc), 
	//	[scale = m_scale](const CRectF& rc) { return rc * scale; });
	
	std::vector<CRectF> intersectRectsInDoc;
	std::transform(orgRectsInDoc.cbegin(), orgRectsInDoc.cend(), std::back_inserter(intersectRectsInDoc), [rcInDoc](const CRectF& rc) { return rc.IntersectRect(rcInDoc); });

	std::vector<CRectF> intersectRectsInWnd;
	std::transform(intersectRectsInDoc.cbegin(), intersectRectsInDoc.cend(), std::back_inserter(intersectRectsInWnd), [this](const CRectF& rc) { return Ctrl2Wnd(Doc2Ctrl(rc)); });

	auto cur = std::max_element(intersectRectsInDoc.cbegin(), intersectRectsInDoc.cend(), [](const CRectF& a, const CRectF& b) { return a.Height() < b.Height(); });
	CurrentPage->set(std::distance(intersectRectsInDoc.cbegin(), cur) + 1);
	logs.push_back(std::format(L"Current Page:\t{}", CurrentPage->get_const()));
	TotalPage->set(PDF->get_const().GetPageCount());
	auto first = std::find_if(intersectRectsInDoc.cbegin(), intersectRectsInDoc.cend(), [](const CRectF& rc) { return rc.Height() > 0; });
	auto last = std::find_if(intersectRectsInDoc.crbegin(), intersectRectsInDoc.crend(), [](const CRectF& rc) { return rc.Height() > 0; });

	auto begin = std::distance(intersectRectsInDoc.cbegin(), first);
	auto end = (std::min)(std::distance(intersectRectsInDoc.cbegin(), last.base()), PDF->get_const().GetPageCount());

	auto callback = [this]()->void { GetWndPtr()->GetDispatcherPtr()->PostInvoke([pWnd = GetWndPtr()]() { pWnd->InvalidateRect(NULL, FALSE); }); };

	for (auto i = begin; i < end; i++){
		CRectF rcClipInPage(Doc2Page(i, intersectRectsInDoc[i]));
		CRectU rcScaledClipInWnd(CRectF2CRectU(rcClipInPage * Scale->get_const()));
		CRectF rcFullInPage(PDF->get_const().GetPage(i)->GetSize());
		CRectF rcScaledFullInPage(rcFullInPage * Scale->get_const());
		CSizeU szBitmap(m_pdfDrawer->GetPrimaryBitmapSize());

		CSizeF szPixcelBmp(PDF->get_const().GetPage(i)->GetSize());
		CSizeF szPixcelPntBmp(PDF->get_const().GetPage(i)->GetSize() * Scale->get_const());
	
		CPointF ptDstClipInWnd(std::round(intersectRectsInWnd[i].left),
			std::round(intersectRectsInWnd[i].top));

		CPointF ptDstLeftTopInWnd(Ctrl2Wnd(Doc2Ctrl(Page2Doc(i, CPointF()))));

		CRectF rcDstInWnd(
			std::round(ptDstLeftTopInWnd.x),
			std::round(ptDstLeftTopInWnd.y),
			std::round(ptDstLeftTopInWnd.x) + std::round(rcScaledFullInPage.Width()),
			std::round(ptDstLeftTopInWnd.y) + std::round(rcScaledFullInPage.Height()));

		FLOAT blurScale = (std::min)({512.f / szPixcelBmp.width, 512.f / szPixcelBmp.height, 0.2f});
		bool drawFullPage = (rcScaledFullInPage.Width() * rcScaledFullInPage.Height()) < (szBitmap.width * szBitmap.height / 8);
		PdfBmpKey fullKey{ .PagePtr = PDF->get_const().GetPage(i).get(), .HashCode = PDF->get_const().GetPage(i)->GetHashCode(), .Scale = Scale->get_const(), .Rotate = PDF->get_const().GetPage(i)->Rotate.get(), .Rect = rcFullInPage };
		PdfBmpKey blurKey{ .PagePtr = PDF->get_const().GetPage(i).get(), .HashCode = PDF->get_const().GetPage(i)->GetHashCode(), .Scale = blurScale, .Rotate = PDF->get_const().GetPage(i)->Rotate.get(), .Rect = rcFullInPage };
		PdfBmpKey clipKey{ .PagePtr = PDF->get_const().GetPage(i).get(), .HashCode = PDF->get_const().GetPage(i)->GetHashCode(), .Scale = Scale->get_const(), .Rotate = PDF->get_const().GetPage(i)->Rotate.get(), .Rect = rcClipInPage };

		logs.push_back(std::format(L"Page:\t{}", i + 1));
		logs.push_back(std::format(L"OriginalSize:\t({},{})", szPixcelBmp.width, szPixcelBmp.height));
		logs.push_back(std::format(L"ScaledSize:\t({},{})", szPixcelPntBmp.width, szPixcelPntBmp.height));
		//logs.push_back(std::format(L"DrawMode:\t{}", drawFullPage ? L"Full":L"Partial").c_str());


		if (drawFullPage) {
			if (m_pdfDrawer->DrawPDFPageBitmap(GetWndPtr()->GetDirectPtr(), fullKey, ptDstLeftTopInWnd, callback)) {
				logs.push_back(std::format(L"DrawMode:\t{}", L"Full"));
			} else {
				if (m_pdfDrawer->DrawPDFPageBlurBitmap(GetWndPtr()->GetDirectPtr(), blurKey, rcDstInWnd, callback)) {
					logs.push_back(std::format(L"DrawMode:\t{}", L"FullBlur"));
				} else {
					logs.push_back(std::format(L"DrawMode:\t{}", L"FullNA"));
				}
			}
		} else {
			if (m_pdfDrawer->DrawPDFPageClipBitmap(GetWndPtr()->GetDirectPtr(), clipKey, ptDstClipInWnd, callback)) {
				logs.push_back(std::format(L"DrawMode:\t{}", L"Clip"));
			} else {
				if (m_pdfDrawer->DrawPDFPageBlurBitmap(GetWndPtr()->GetDirectPtr(), blurKey, rcDstInWnd, callback)) {
					logs.push_back(std::format(L"DrawMode:\t{}", L"ClipBlur"));
				} else {
					logs.push_back(std::format(L"DrawMode:\t{}", L"ClipNA"));
				}
				//m_pdfDrawer->DrawPDFPageClipBitmap(GetWndPtr()->GetDirectPtr(), clipKey, ptDstClipInWnd, callback);//Just order

				std::vector<PdfBmpKey> keys = m_pdfDrawer->FindFulKeys([clipKey, pPage = PDF->get_const().GetPage(i).get(), scale = Scale->get_const()](const PdfBmpKey& key)->bool{
					return 
						key != clipKey &&
						key.PagePtr == clipKey.PagePtr && 
						key.Scale == clipKey.Scale &&
						key.Rotate == clipKey.Rotate &&
						!key.Rect.IsRectNull();
				});
				for (const PdfBmpKey& key : keys) {
					CPointF ptClipInWnd = Ctrl2Wnd(Doc2Ctrl(Page2Doc(i, key.Rect.LeftTop())));
					if (m_pdfDrawer->DrawPDFPageClipBitmap(GetWndPtr()->GetDirectPtr(), key, ptClipInWnd, callback)) {
						logs.push_back(std::format(L"DrawMode:\t{}", L"ClipPartial"));
					} else {

					}
				}
			}
		}
	}

	auto bitmapKeyPages = [this](const std::unique_ptr<CD2DAtlasBitmap<PdfBmpKey>>& pBmp) {
		std::vector<PdfBmpKey> keys = pBmp->Keys();
		std::vector<int> pages;
		std::transform(keys.cbegin(), keys.cend(), std::back_inserter(pages), [this](const PdfBmpKey& key) {return PDF->get_const().GetPageIndex(key.PagePtr) + 1; });
		std::wstring pagesString;
		std::for_each(pages.cbegin(), pages.cend(), [&pagesString](const int page) { pagesString += std::to_wstring(page) + L" "; });
		return pagesString;
	};
	logs.push_back(std::format(L"FullOrClipBitmapCount:\t{}", m_pdfDrawer->GetAtlasFullOrClipBitmap()->Count()).c_str());
	logs.push_back(std::format(L"FullOrClipBitmapKeys:\t{}", bitmapKeyPages(m_pdfDrawer->GetAtlasFullOrClipBitmap())).c_str());

	logs.push_back(std::format(L"BlurBitmapCount:\t{}", m_pdfDrawer->GetAtlasBlurBitmap()->Count()).c_str());
	logs.push_back(std::format(L"BlurBitmapKeys:\t{}", bitmapKeyPages(m_pdfDrawer->GetAtlasBlurBitmap())).c_str());

	if(debug){
		std::wstring text = boost::algorithm::join(logs, L"\n");
		GetWndPtr()->GetDirectPtr()->DrawTextFromPoint(*(m_pProp->Format), text, GetRenderRectInWnd().LeftTop());
	}

	//Paint Caret
	if (m_caret.IsCaret()) {
		auto [page_index, char_index] = m_caret.Current;
		CRectF rcCaretInPdfiumpage = PDF->get_const().GetPage(page_index)->GetCaretRect(char_index);
		CRectF rcCaretInWnd = PdfiumPage2Wnd(page_index, rcCaretInPdfiumpage);
		if (rcCaretInWnd.Width() < 1) {
			rcCaretInWnd.right = rcCaretInWnd.left + 1.f;
		}

		GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		GetWndPtr()->GetDirectPtr()->FillSolidRectangle(m_pProp->Format->Color, PdfiumPage2Wnd(page_index, rcCaretInPdfiumpage));
		GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}

	//Paint Selected Text
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND::D2D1_PRIMITIVE_BLEND_MIN);
	auto [page_first_index, char_begin_index] = m_caret.SelectBegin;
	auto [page_last_index, char_end_index] = m_caret.SelectEnd;

	std::vector<CRectF> rcSelsInPdfium;
	std::vector<CRectF> rcSelsInWnd;
	if (page_first_index == page_last_index) {
		rcSelsInPdfium = PDF->get_const().GetPage(page_first_index)->GetSelectedTextRects(char_begin_index, char_end_index);
		std::transform(rcSelsInPdfium.cbegin(), rcSelsInPdfium.cend(), std::back_inserter(rcSelsInWnd), [i = page_first_index, this](const CRectF& rcInPdfium) { return PdfiumPage2Wnd(i, rcInPdfium); });
	} else {
		for (auto i = page_first_index; i <= page_last_index; i++) {
			if (i == page_first_index) {
				rcSelsInPdfium = PDF->get_const().GetPage(i)->GetSelectedTextRects(char_begin_index, PDF->get_const().GetPage(i)->GetTextSize());
			} else if (i == page_last_index) {
				rcSelsInPdfium = PDF->get_const().GetPage(i)->GetSelectedTextRects(0, char_end_index);
			} else {
				rcSelsInPdfium = PDF->get_const().GetPage(i)->GetSelectedTextRects(0, PDF->get_const().GetPage(i)->GetTextSize());
			}
			std::transform(rcSelsInPdfium.cbegin(), rcSelsInPdfium.cend(), std::back_inserter(rcSelsInWnd),
				[i = i, this](const CRectF& rcInPdfium) { return PdfiumPage2Wnd(i, rcInPdfium); });
		}
	}
	std::for_each(rcSelsInWnd.cbegin(), rcSelsInWnd.cend(), [this](const CRectF& rcSelInWnd) { GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_pProp->SelectedFill), rcSelInWnd); });
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND::D2D1_PRIMITIVE_BLEND_SOURCE_OVER);

	//Mouse Rects
	if (debug) {
		for (auto i = begin; i < end; i++) {
			const std::vector<CRectF>& rcMousInPdfium = PDF->get_const().GetPage(i)->GetTextMouseRects();
			std::vector<CRectF> rcMousInWnd;
			std::transform(rcMousInPdfium.cbegin(), rcMousInPdfium.cend(), std::back_inserter(rcMousInWnd),
				[i = i, this](const CRectF& rcInPdfium) { return PdfiumPage2Wnd(i, rcInPdfium); });
			std::for_each(rcMousInWnd.cbegin(), rcMousInWnd.cend(),
				[this](const CRectF& rcMouInWnd) { GetWndPtr()->GetDirectPtr()->DrawSolidRectangle(SolidLine(1.f, 0.f, 0.f, 1.f, 1.f), rcMouInWnd); });
		}
	}	 


	//Paint Find
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND::D2D1_PRIMITIVE_BLEND_MIN);
	for (auto i = begin; i < end; i++) {
		const std::vector<CRectF>& rcFndsInPdfium = PDF->get_const().GetPage(i)->GetFindRects(Find->get_const());
		std::vector<CRectF> rcFndsInWnd;
		std::transform(rcFndsInPdfium.cbegin(), rcFndsInPdfium.cend(), std::back_inserter(rcFndsInWnd),
			[i = i, this](const CRectF& rcInPdfium) { return PdfiumPage2Wnd(i, rcInPdfium); });
		std::for_each(rcFndsInWnd.cbegin(), rcFndsInWnd.cend(),
			[this](const CRectF& rcFndInWnd) { GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_pProp->FindHighliteFill), rcFndInWnd); });
	}
	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND::D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
	

	//Paint Scroll
	UpdateScroll();
	m_spVScroll->OnPaint(e);
	m_spHScroll->OnPaint(e);

	//PaintScrollHighlite
	FLOAT fullHeight = PDF->get_const().GetSize().height;
	CRectF rcThumbInWnd = m_spVScroll->GetThumbRangeRect();
	for (auto i = 0; i < PDF->get_const().GetPageCount(); i++) {
		SolidFill fill(*m_pProp->FindHighliteFill);
		fill.Color.a = 1.f;
		CRectF rcThumbPageInWnd(
			rcThumbInWnd.left + 2.f,
			rcThumbInWnd.top + rcThumbInWnd.Height() * PDF->get_const().GetPageRects()[i].top / fullHeight,
			rcThumbInWnd.right - 2.f,
			rcThumbInWnd.top + rcThumbInWnd.Height() * PDF->get_const().GetPageRects()[i].bottom / fullHeight);

		const std::vector<CRectF>& rcFndsInPdfium = PDF->get_const().GetPage(i)->GetFindRects(Find->get_const());
		std::vector<CRectF> rcHighsInWnd;
		std::transform(rcFndsInPdfium.cbegin(), rcFndsInPdfium.cend(), std::back_inserter(rcHighsInWnd),
			[i = i, this, rcThumbPageInWnd, srcSize = PDF->get_const().GetPage(i)->GetSize()](const CRectF& rcInPdfium) {
				CRectF rcInPage = PdfiumPage2Page(i, rcInPdfium);
				auto rcHighInWnd = CRectF(
						rcThumbPageInWnd.left,
						rcThumbPageInWnd.top + rcThumbPageInWnd.Height() * rcInPage.top / srcSize.height,
						rcThumbPageInWnd.right,
						rcThumbPageInWnd.top + rcThumbPageInWnd.Height() * rcInPage.bottom / srcSize.height);
				if (rcHighInWnd.Height() < 1.f) {
					rcHighInWnd.bottom = rcHighInWnd.top + 1.f;
				}
				return rcHighInWnd;
			});
		std::for_each(rcHighsInWnd.cbegin(), rcHighsInWnd.cend(),
			[this, fill](const CRectF& rcHighInWnd) { GetWndPtr()->GetDirectPtr()->FillSolidRectangle(fill, rcHighInWnd); });
	}


	//Paint Focused Line
	if (GetIsFocused() ){
		CRectF rcFocus(GetRectInWnd());
		rcFocus.DeflateRect(1.0f, 1.0f);
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(*(m_pProp->FocusedLine), rcFocus);
	}

	GetWndPtr()->GetDirectPtr()->PopAxisAlignedClip();
}

void CPdfView::Normal_KeyDown(const KeyDownEvent& e)
{
	bool shift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;

	switch (e.Char) {
		case 'O':
		{
			if (ctrl) {
				OpenWithFileDialog();
			}
		}
		break;
		case 'A':
		{
			if (ctrl) {
				//m_carets.set(std::get<caret::CurCaret>(m_carets.get()), m_text.size(), 0, 0, m_text.size());
				//m_caretPoint.set(GetOriginCharRects()[m_text.size()].CenterPoint());
				//StartCaretBlink();
				*e.HandledPtr = TRUE;
			}
		}
		break;
		case 'C':
		{
			if (ctrl) {
				if (PDF) {
					PDF->get_const().CopyTextToClipboard(GetWndPtr()->m_hWnd, m_caret.SelectBegin, m_caret.SelectEnd);
				}
				*e.HandledPtr = TRUE;
			}
		}
		break;
		case VK_UP:
		{
			m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() - m_spVScroll->GetScrollDelta() * 1);	
		}
		break;
		case VK_DOWN:
		{
			m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() + m_spVScroll->GetScrollDelta() * 1);	
		}
		break;
		case VK_LEFT:
		{
			m_spHScroll->SetScrollPos(m_spHScroll->GetScrollPos() - m_spHScroll->GetScrollDelta() * 1);	
		}
		break;
		case VK_RIGHT:
		{
			m_spHScroll->SetScrollPos(m_spHScroll->GetScrollPos() + m_spHScroll->GetScrollDelta() * 1);	
		}
		break;
		case VK_PRIOR:
		{
			m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() - m_spVScroll->GetScrollPage());	
		}
		break;
		case VK_NEXT:
		{
			m_spVScroll->SetScrollPos(m_spVScroll->GetScrollPos() + m_spVScroll->GetScrollPage());	
		}
		break;


		default:
		{
			CD2DWControl::OnKeyDown(e);
		}
		break;
	}
}

void CPdfView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	//CreateMenu
	CMenu menu(::CreatePopupMenu());
	//Add Row
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateClockwise");
	mii.dwTypeData = const_cast<LPWSTR>(L"Rotate Clockwise");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateCounterClockwise");
	mii.dwTypeData = const_cast<LPWSTR>(L"Rotate Counter Clockwise");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeletePage");
	mii.dwTypeData = const_cast<LPWSTR>(L"Delete Page");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	::SetForegroundWindow(GetWndPtr()->m_hWnd);
	int idCmd = menu.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		e.PointInScreen.x,
		e.PointInScreen.y,
		GetWndPtr()->m_hWnd);

	if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateClockwise")) {
		const std::unique_ptr<CPDFPage>& pPage = PDF->get_const().GetPage(CurrentPage->get_const() - 1);
		pPage->Rotate.set((pPage->Rotate.get() + 1) % 4);
	} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"RotateCounterClockwise")) {
		const std::unique_ptr<CPDFPage>& pPage = PDF->get_const().GetPage(CurrentPage->get_const() - 1);
		int rotate = pPage->Rotate.get() == 0 ? 4 : pPage->Rotate.get();
		pPage->Rotate.set((rotate - 1) % 4);
	} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeletePage")) {
		PDF->get_unconst().DeletePage(CurrentPage->get_const() - 1);
	}
	*e.HandledPtr = TRUE;

}

void CPdfView::Normal_KillFocus(const KillFocusEvent& e)
{
	m_caret.StopBlink();
}

/*************/
/* NormalPan */
/*************/

void CPdfView::NormalPan_SetCursor(const SetCursorEvent& e)
{
	SendPtInRectReverse(&CUIElement::OnSetCursor, e);
	if (!(*e.HandledPtr)) {
		HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
		::SetCursor(hCur);
		*(e.HandledPtr) = TRUE;
	}
}

/**************/
/* NormalText */
/**************/
void CPdfView::NormalText_LButtonDown(const LButtonDownEvent& e)
{
	auto [page, index] = GetPageAndIndexFromWndPoint(e.PointInWnd);
	if (page >= 0) {
		if (index >= 0) {
			auto rcInWnd = PdfiumPage2Wnd(page, PDF->get_const().GetPage(page)->GetTextCursorRects()[index]);
			auto point = rcInWnd.CenterPoint();
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				m_caret.MoveWithShift(page, index, point);
				return;
			} else {
				m_caret.Move(page, index, point);
			}
		} else {
			m_caret.Clear();
		}
	}
}

void CPdfView::NormalText_SetCursor(const SetCursorEvent& e)
{
	SendPtInRectReverse(&CUIElement::OnSetCursor, e);
	if (!(*e.HandledPtr)) {
		auto [page, index] = GetPageAndIndexFromWndPoint(e.PointInWnd);
		if (page >= 0) {
			if (index >= 0) {
				HCURSOR hCur = ::LoadCursor(NULL, IDC_IBEAM);
				::SetCursor(hCur);
				*(e.HandledPtr) = TRUE;
			} else {
				//HCURSOR hCur = ::LoadCursor(NULL, IDC_ARROW);
				//::SetCursor(hCur);
			}
		}
	}
}


/***************/
/* VScrollDrag */
/***************/
void CPdfView::VScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spVScroll->SetState(UIElementState::Dragged);
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}
void CPdfView::VScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spVScroll->SetState(UIElementState::Normal);
	m_spVScroll->SetStartDrag(0.f);
}
bool CPdfView::VScrl_Guard_SetCursor(const SetCursorEvent& e)
{
	return m_spVScroll->GetIsVisible() && m_spVScroll->GetThumbRect().PtInRect(GetWndPtr()->GetCursorPosInWnd());
}
bool CPdfView::VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spVScroll->GetIsVisible() && m_spVScroll->GetThumbRect().PtInRect(e.PointInWnd);
}
void CPdfView::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spVScroll->SetScrollPos(
		m_spVScroll->GetScrollPos() +
		(e.PointInWnd.y - m_spVScroll->GetStartDrag()) *
		m_spVScroll->GetScrollDistance() /
		m_spVScroll->GetRectInWnd().Height());
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}

void CPdfView::VScrlDrag_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
}

/***************/
/* HScrollDrag */
/***************/
void CPdfView::HScrlDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spHScroll->SetState(UIElementState::Dragged);
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CPdfView::HScrlDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spHScroll->SetState(UIElementState::Normal);
	m_spHScroll->SetStartDrag(0.f);
}
bool CPdfView::HScrl_Guard_SetCursor(const SetCursorEvent& e)
{
	return m_spHScroll->GetIsVisible() && m_spHScroll->GetThumbRect().PtInRect(GetWndPtr()->GetCursorPosInWnd());
}
bool CPdfView::HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spHScroll->GetIsVisible() && m_spHScroll->GetThumbRect().PtInRect(e.PointInWnd);
}

void CPdfView::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spHScroll->SetScrollPos(
		m_spHScroll->GetScrollPos() +
		(e.PointInWnd.x - m_spHScroll->GetStartDrag()) *
		m_spHScroll->GetScrollDistance() /
		m_spHScroll->GetRectInWnd().Width());
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

void CPdfView::HScrlDrag_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDOPEN));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
}

/***********/
/* Panning */
/***********/
void CPdfView::Panning_OnEntry(const LButtonBeginDragEvent& e)
{
	HScrlDrag_OnEntry(e);
	VScrlDrag_OnEntry(e);
}

void CPdfView::Panning_OnExit(const LButtonEndDragEvent& e)
{
	HScrlDrag_OnExit(e);
	VScrlDrag_OnExit(e);
}

void CPdfView::Panning_MouseMove(const MouseMoveEvent& e)
{
	m_spHScroll->SetScrollPos(
		m_spHScroll->GetScrollPos() + m_spHScroll->GetStartDrag() - e.PointInWnd.x);
	m_spHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));

	m_spVScroll->SetScrollPos(
		m_spVScroll->GetScrollPos() + m_spVScroll->GetStartDrag() - e.PointInWnd.y);
	m_spVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));

	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDGRAB));
	::SetCursor(hCur);
}

void CPdfView::Panning_SetCursor(const SetCursorEvent& e)
{
	HCURSOR hCur = ::LoadCursor(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSOR_HANDGRAB));
	::SetCursor(hCur);
	*(e.HandledPtr) = TRUE;
}

bool CPdfView::Panning_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_pMachine->IsStateNormalPan();
}

/************/
/* TextDrag */
/************/

void CPdfView::TextDrag_OnEntry(const LButtonBeginDragEvent& e) {}
void CPdfView::TextDrag_OnExit(const LButtonEndDragEvent& e) {}
void CPdfView::TextDrag_MouseMove(const MouseMoveEvent& e) 
{
	auto [page, index] = GetPageAndIndexFromWndPoint(e.PointInWnd);
	if (page >= 0) {
		if (index >= 0) {
			auto rcInWnd = PdfiumPage2Wnd(page, PDF->get_const().GetPage(page)->GetTextCursorRects()[index]);
			auto point = rcInWnd.CenterPoint();
			m_caret.MoveWithShift(page, index, point);
		}
	}
	
	if (page >= 0) {
		if (index >= 0) {
			HCURSOR hCur = ::LoadCursor(NULL, IDC_IBEAM);
			::SetCursor(hCur);
			//*(e.HandledPtr) = TRUE;
		} else {
			HCURSOR hCur = ::LoadCursor(NULL, IDC_ARROW);
			::SetCursor(hCur);
		}
	}	
}
void CPdfView::TextDrag_SetCursor(const SetCursorEvent& e) 
{
	NormalText_SetCursor(e);
}
bool CPdfView::TextDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return true;
}

void CPdfView::Error_StdException(const std::exception& e)
{
	::OutputDebugStringA(e.what());

	std::string msg = fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n",
		e.what(), GetLastErrorString());

	MessageBoxA(GetWndPtr()->m_hWnd, msg.c_str(), "Exception in StateMachine", MB_ICONWARNING);
}

void CPdfView::OpenWithFileDialog()
{
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	//ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//ofn.lpstrDefExt = L"txt";

	if (!GetOpenFileName(&ofn)) {
		DWORD errCode = CommDlgExtendedError();
		if (errCode) {
			throw std::exception(FILE_LINE_FUNC);
		}
	} else {
		::ReleaseBuffer(path);
		Open(path);
	}
}

void CPdfView::Reset(const reactive_property_ptr<CPDFDoc>& pdf)
{
	//Clear Current
	Clear();
	//Reset New
	PDF = pdf;
	//New FileIsInUse
	m_pFileIsInUse = CFileIsInUseImpl::CreateInstance(GetWndPtr()->m_hWnd, PDF->get_const().Path->get_const().c_str(), FUT_DEFAULT, OF_CAP_DEFAULT);
	GetWndPtr()->AddMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE, [pdf = PDF](UINT,LPARAM,WPARAM,BOOL&)->LRESULT
	{
		pdf->get_unconst().Close();
		return 0;
	});
	//Open
	if (!pdf->get_const().IsOpen()) {
		OpenWithPasswordHandling(PDF->get_const().Path->get_const());
	} else {
		//Do nothing
	}
}

void CPdfView::OpenWithPasswordHandling(const std::wstring& path)
{
	if (::PathFileExists(path.c_str())) {
		if (auto err = PDF->get_unconst().Open(path, L""); err == FPDF_ERR_SUCCESS) {
			if (Scale->get_const() < 0) {// < 0 means auto-scale
				FLOAT scaleX = GetRenderSize().width / PDF->get_const().GetPage(0)->GetSize().width;
				FLOAT scaleY = GetRenderSize().height / PDF->get_const().GetPage(0)->GetSize().height;
				switch (m_initialScaleMode) {
					case InitialScaleMode::MinWidthHeight:
						Scale->set((std::min)(scaleX, scaleY));
						break;
					case InitialScaleMode::Width:
						Scale->set(scaleX);
						break;
					case InitialScaleMode::Height:
						Scale->set(scaleY);
						break;
					default:
						Scale->set(1.f);
				}
			}
		} else if (err == FPDF_ERR_PASSWORD){
			std::shared_ptr<CTextBoxDialog> spDlg = std::make_shared<CTextBoxDialog>(GetWndPtr(), std::make_shared<DialogProperty>());
			spDlg->GetTitle().set(L"Password");
			spDlg->GetTextBlockPtr()->Text->set(L"Please input password");
			spDlg->GetOKButtonPtr()->GetContent().set(L"OK");
			spDlg->GetOKButtonPtr()->GetCommand().Subscribe([this, spDlg, path]() 
				{ 
					PDF->get_unconst().Open(path, spDlg->GetTextBoxPtr()->Text->get_const()); 
					//Need to call with dispatcher, otherwise remaing message in message que is not properly handled
					GetWndPtr()->GetDispatcherPtr()->PostInvoke([spDlg]() { spDlg->OnClose(CloseEvent(spDlg->GetWndPtr(), NULL, NULL)); });
				});
			spDlg->GetCancelButtonPtr()->GetContent().set(L"Cancel");
			spDlg->GetCancelButtonPtr()->GetCommand().Subscribe([this, spDlg]() 
				{ 
					//Need to call with dispatcher, otherwise remaing message in message que is not properly handled
					GetWndPtr()->GetDispatcherPtr()->PostInvoke([spDlg]() { spDlg->OnClose(CloseEvent(spDlg->GetWndPtr(), NULL, NULL)); });
				});
			spDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), CalcCenterRectF(CSizeF(300, 200))));
			GetWndPtr()->SetFocusedControlPtr(spDlg);
		} else {
			PDF.reset();
		}
	}
}

void CPdfView::Open(const std::wstring& path)
{
	//Clear Current
	Clear();
	//New FileIsInUse
	m_pFileIsInUse = CFileIsInUseImpl::CreateInstance(GetWndPtr()->m_hWnd, PDF->get_const().Path->get_const().c_str(), FUT_DEFAULT, OF_CAP_DEFAULT);
	GetWndPtr()->AddMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE, [pdf = PDF](UINT,LPARAM,WPARAM,BOOL&)->LRESULT
	{
		pdf->get_unconst().Close();
		return 0;
	});
	//Open
	OpenWithPasswordHandling(path);
}

void CPdfView::Close()
{
	Clear();
	PDF.reset();
}

void CPdfView::Clear()
{
	m_pdfDrawer->WaitAll();
	m_pdfDrawer->Clear();

	m_caret.Clear();
	GetWndPtr()->RemoveMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE);
	m_pFileIsInUse.Release();

	m_prevScale = 0.f;
	m_spVScroll->Clear();
	m_spHScroll->Clear();
}



bool CPdfView::Jump(const int& page)
{
	if (page < 1 || PDF->get_const().GetPageCount() < page) { return false; }

	std::vector<CRectF> rectsInDoc = PDF->get_const().GetPageRects();
	std::vector<CRectF> scaledRectsInDoc;
	std::transform(rectsInDoc.cbegin(), rectsInDoc.cend(), std::back_inserter(scaledRectsInDoc), 
		[scale = Scale->get_const()](const CRectF& rc) { return rc * scale; });

	m_spVScroll->SetScrollPos(scaledRectsInDoc[page - 1].top);

	return true;
}


std::tuple<CRectF, CRectF> CPdfView::GetRects() const
{
	CRectF rcClient(GetRectInWnd());
	CRectF rcVertical;
	FLOAT lineHalfWidth = m_pProp->FocusedLine->Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_spHScroll->GetIsVisible() ? (m_spHScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);

	CRectF rcHorizontal;
	rcHorizontal.left = rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom - ::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_spVScroll->GetIsVisible() ? (m_spVScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;

	return { rcVertical, rcHorizontal };
}

void CPdfView::UpdateScroll()
{
	//VScroll
	m_spVScroll->SetScrollRangePage(0, GetRenderContentSize().height, GetRenderSize().height);

	//HScroll
	m_spHScroll->SetScrollRangePage(0, GetRenderContentSize().width, GetRenderSize().width);

	//VScroll/HScroll Rect
	auto [rcVertical, rcHorizontal] = GetRects();
	m_spVScroll->OnRect(RectEvent(GetWndPtr(), rcVertical));
	m_spHScroll->OnRect(RectEvent(GetWndPtr(), rcHorizontal));
}


void CPdfView::Update() {}
