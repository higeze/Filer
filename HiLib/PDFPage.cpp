#include "PDFPage.h"
#include "PDFDoc.h"
#include <mutex>
#include <boost/algorithm/string.hpp>
#include "ThreadPool.h"
#include "MyUniqueHandle.h"
#include "MyClipboard.h"

#include "strconv.h"
#include <math.h>
#include "ThreadPool.h"
#include "FPDFPage.h"
#include "FPDFTextPage.h"
#include "FPDFFormHandle.h"
#include "FPDFBitmap.h"
#include "MyClipboard.h"
#include "MyString.h"

std::vector<const_paragraph_iterator> text_to_paragraph_iterators(const std::wstring& text)
{
	using const_iterator = std::wstring::const_iterator;
	std::vector<std::tuple<const_iterator, const_iterator, const_iterator>> paras;
	const_iterator beg = text.cbegin();
	const_iterator end = text.cend();
	std::wregex re{L"\r\n?|\n"};
	const_iterator para_prev_end = text.begin();
	for (std::wsregex_iterator re_iter{beg, end, re}, re_end; re_iter != re_end; ++re_iter) {
		const_iterator para_beg = para_prev_end;
		const_iterator para_crlfbeg = std::next(beg, re_iter->position());
		const_iterator para_end = std::next(beg, re_iter->position() + re_iter->length());
		paras.emplace_back(para_beg, para_crlfbeg, para_end);
		para_prev_end = para_end;
	}
	if (para_prev_end != end) {
		paras.emplace_back(para_prev_end, end, end);
	}
	return paras;
}

/************/
/* CPdfPage */
/************/
CPDFPage::CPDFPage(const CPDFDoc* pDoc, int index)
	:m_pDoc(pDoc), m_index(index), IsDirty(false), Dummy(std::make_shared<int>(0))
{
	//m_pPage->OnAfterLoadPage(*m_pForm);
	//m_pPage->DoPageAAction(*m_pForm, FPDFPAGE_AACTION_OPEN);

	Rotate.set(GetFPDFPagePtr()->GetRotation());
	Rotate.subscribe([this](const int& value) 
	{
		GetFPDFPagePtr()->SetRotation(value);
		//m_optBitmap.reset();
		m_optSize.reset();
		m_optTextOrgRects.reset();
		m_optTextOrgCursorRects.reset();
		m_optTextCursorRects.reset();
		m_optTextOrgMouseRects.reset();
		m_optTextMouseRects.reset();
		m_optFind.reset();
		m_optSelectedText.reset();
		IsDirty.set(true);

	},Dummy);
}

CPDFPage::~CPDFPage() = default;
//{
	//m_pPage->DoPageAAction(*m_pForm, FPDFPAGE_AACTION_CLOSE);
	//m_pPage->OnBeforeClosePage(*m_pForm);
//}

const std::unique_ptr<CFPDFPage>& CPDFPage::GetFPDFPagePtr() const
{
	if (!m_optFPDFPagePtr.has_value()) {
		auto pFPDFPage = std::make_unique<CFPDFPage>(m_pDoc->GetFPDFDocPtr()->LoadPage(m_index));
		m_optFPDFPagePtr.emplace(std::move(pFPDFPage));
	}
	return m_optFPDFPagePtr.value();
}

const std::unique_ptr<CFPDFTextPage>& CPDFPage::GetFPDFTextPagePtr() const
{
	if (!m_optFPDFTextPagePtr.has_value()) {
		auto pFPDFTextPage = std::make_unique<CFPDFTextPage>(GetFPDFPagePtr()->LoadTextPage());
		m_optFPDFTextPagePtr.emplace(std::move(pFPDFTextPage));
	}
	return m_optFPDFTextPagePtr.value();
}

const CSizeF& CPDFPage::GetSize() const
{
	if (!m_optSize.has_value()) {
		m_optSize.emplace(
			GetFPDFPagePtr()->GetPageWidthF(),
			GetFPDFPagePtr()->GetPageHeightF());
	}
	return m_optSize.value();
}

const std::wstring& CPDFPage::GetText() const
{
	if (!m_optText.has_value()) {
		int charCount = GetFPDFTextPagePtr()->CountChars();
		std::wstring text(charCount, 0);
		int textCount = GetFPDFTextPagePtr()->GetText(0, charCount, reinterpret_cast<unsigned short*>(text.data()));
		m_optText.emplace(text);
	}
	return m_optText.value();
}

const std::vector<const_paragraph_iterator>& CPDFPage::GetParagraphIterators() const
{
	if (!m_optParagraphIterators.has_value()) {
		m_optParagraphIterators.emplace(text_to_paragraph_iterators(GetText()));
	}
	return m_optParagraphIterators.value();
}

int CPDFPage::GetTextSize() const
{
	return GetText().size();
}

// Pdfium Coordinates
// Å™Y Å®X
const std::vector<CRectF>& CPDFPage::GetTextOrgRects() const
{
	if (!m_optTextOrgRects.has_value()) {
		m_optTextOrgRects.emplace(GetFPDFTextPagePtr()->GetRects());
	}
	return m_optTextOrgRects.value();
}
auto isCRorLF = [](const wchar_t ch)->bool { return ch == L'\r' || ch == L'\n'; };
auto is_space = [](const wchar_t ch)->bool { return ch == L' ' || ch == L'Å@'; };
auto is_cr = [](const wchar_t ch)->bool { return ch == L'\r'; };
auto is_lf = [](const wchar_t ch)->bool { return ch == L'\n'; };

const std::vector<CRectF>& CPDFPage::GetTextOrgCursorRects() const
{
	if (!m_optTextOrgCursorRects.has_value()) {
		std::vector<CRectF> rects = GetTextOrgRects();
		const std::wstring& text = GetText();
		for (size_t i = 0; i < rects.size(); i++) {
			if (rects[i].top == 0 || rects[i].bottom == 0) {
				::OutputDebugString(std::format(L"'{}:{}'", i, int(text[i])).c_str());
			}
		}
		if (!rects.empty() && !text.empty()) {
			const std::vector<const_paragraph_iterator>& paras = GetParagraphIterators();
			for (auto iter = paras.cbegin(); iter != paras.cend(); ++iter) {
				::OutputDebugStringW(std::format(L"{}, {}, {}",
					std::distance(text.cbegin(), std::get<0>(*iter)),
					std::distance(text.cbegin(), std::get<1>(*iter)),
					std::distance(text.cbegin(), std::get<2>(*iter))).c_str());
				int beg_pos = std::distance(text.cbegin(), std::get<0>(*iter));
				int crlf_pos = std::distance(text.cbegin(), std::get<1>(*iter));
				int end_pos = std::distance(text.cbegin(), std::get<2>(*iter));
				std::vector<CRectF>::iterator beg_rect = rects.begin() + std::distance(text.cbegin(), std::get<0>(*iter));
				std::vector<CRectF>::iterator crlf_rect = rects.begin() + std::distance(text.cbegin(), std::get<1>(*iter));
				std::vector<CRectF>::iterator end_rect = rects.begin() + std::distance(text.cbegin(), std::get<2>(*iter));

				//Left Right
				for (auto iter_rect = beg_rect; iter_rect != end_rect; ++iter_rect) {
					if (iter_rect != beg_rect && iter_rect->IsRectNull()) {
						iter_rect->left = std::prev(iter_rect)->left;
						iter_rect->right = std::prev(iter_rect)->right;
					} else if (iter_rect->IsRectNull()) {
						auto a = 1;
					}
				}
				//Top Bottom
				std::vector<CRectF> range_rects = GetFPDFTextPagePtr()->GetRangeRects(beg_pos, end_pos);
				FLOAT max_top = std::max_element(range_rects.cbegin(), range_rects.cend(), [](const CRectF& left, const CRectF& right) { return left.top < right.top; })->top;
				FLOAT min_btm = std::min_element(range_rects.cbegin(), range_rects.cend(), [](const CRectF& left, const CRectF& right) { return left.bottom < right.bottom; })->bottom;
				std::for_each(beg_rect, end_rect, [&](CRectF& rc) {rc.top = max_top; rc.bottom = min_btm; });
				if (beg_rect != crlf_rect) {
					std::for_each(crlf_rect, end_rect, [&](CRectF& rc) {
						rc.left = rc.right = std::prev(crlf_rect)->right;
					});
				} else {
					auto a = 1;
				}
			}
		}
		m_optTextOrgCursorRects.emplace(rects);
	}
	return m_optTextOrgCursorRects.value();
}

const std::vector<CRectF>& CPDFPage::GetTextOrgMouseRects() const
{
	const float lr_factor = 0.8f;
	const float half_factor = 0.5f;
	const float tb_factor = 0.3f;
	if (!m_optTextOrgMouseRects.has_value()) {
		auto& text = GetText();
		auto mouseRects = GetTextOrgCursorRects();
		if (!mouseRects.empty()) {
			std::ranges::for_each(mouseRects, [&](CRectF& rc) {
				auto tb_offset = - rc.Height() * tb_factor;
				rc.top = (std::min)(rc.top + tb_offset, GetSize().height);
				rc.bottom = (std::max)(rc.bottom - tb_offset, 0.f);
			});

			const std::vector<const_paragraph_iterator>& paras = GetParagraphIterators();
			for (auto para_iter = paras.cbegin(); para_iter != paras.cend(); ++para_iter) {
				for (auto iter = std::get<0>(*para_iter); iter != std::get<2>(*para_iter); ++iter) {
					size_t index = std::distance(text.cbegin(), iter);
					//left
					if (iter == std::get<0>(*para_iter)) {
						mouseRects[index].left = (std::max)(mouseRects[index].left - mouseRects[index].Width() * lr_factor, 0.f);
					} else {
						mouseRects[index].left = (std::max)((mouseRects[index - 1].right + mouseRects[index].left) * 0.5f, mouseRects[index - 1].right);
					}
					//right
					if (iter == std::prev(std::get<2>(*para_iter))) {
						size_t last_str_index = std::distance(text.cbegin(), std::prev(std::get<1>(*para_iter)));
						mouseRects[index].right = (std::max)(mouseRects[index].right + mouseRects[last_str_index].Width() * lr_factor, 0.f);
					} else {
						mouseRects[index].right = (std::max)((mouseRects[index].right + mouseRects[index + 1].left) * 0.5f, mouseRects[index + 1].left);
					}
				}
			}
		}
		m_optTextOrgMouseRects.emplace(mouseRects);
	}
	return m_optTextOrgMouseRects.value();
}

const std::vector<CRectF>& CPDFPage::GetTextRects() const
{
	if (!m_optTextRects.has_value()) {
		m_optTextRects = GetTextOrgRects();
		RotateRects(m_optTextRects.value(), *Rotate);
	}
	return m_optTextRects.value();
}

const std::vector<CRectF>& CPDFPage::GetTextCursorRects() const
{
	if (!m_optTextCursorRects.has_value()) {
		m_optTextCursorRects = GetTextOrgCursorRects();
		RotateRects(m_optTextCursorRects.value(), *Rotate);
	}
	return m_optTextCursorRects.value();
}

const std::vector<CRectF>& CPDFPage::GetTextMouseRects() const
{
	if (!m_optTextMouseRects.has_value()) {
		m_optTextMouseRects = GetTextOrgMouseRects();
		RotateRects(m_optTextMouseRects.value(), *Rotate);
	}
	return m_optTextMouseRects.value();
}

const std::vector<CRectF>& CPDFPage::GetFindRects(const std::wstring& find_string)
{
	auto find = boost::trim_copy(find_string);
	if (!m_optFind.has_value() || m_optFind->Find != find) {
		std::vector<CRectF> rects;
		if (find.empty()) {
		} else {
//			for (auto i = 0; (i = GetText().find(find, i)) != std::wstring::npos; i++) {
			for (size_t i = 0; (i = GetText()|find_insensitive(find, i)) != std::wstring::npos; i++) {
			auto left = GetTextRects().at(i).left;
				auto right = GetTextRects().at(i + find.size() - 1).right;
				auto top = std::max_element(std::next(GetTextRects().cbegin() + i), std::next(GetTextRects().cbegin() + i + find.size() - 1),
					[](const auto& a, const auto& b) { return a.top < b.top; })->top;
				auto bottom = std::min_element(std::next(GetTextRects().cbegin() + i), std::next(GetTextRects().cbegin() + i + find.size() - 1),
					[](const auto& a, const auto& b) { return a.bottom < b.bottom; })->bottom;

				rects.emplace_back(left, top, right, bottom);
			}
			RotateRects(rects, *Rotate);
		}
		m_optFind.emplace(find, rects);
	}
	return m_optFind->FindRects;

	//auto find = boost::trim_copy(find_string);
	//if (!m_optFind.has_value() || m_optFind->Find != find) {
	//	std::vector<CRectF> rects;
	//	if (find.empty()) {
	//	} else {
	//		auto results  = GetFPDFTextPagePtr()->SearchResults(reinterpret_cast<FPDF_WIDESTRING>(find.c_str()));
	//		for (const auto res : results) {
	//			std::copy(std::get<2>(res).cbegin(), std::get<2>(res).cend(), std::back_inserter(rects));
	//		}
	//		RotateRects(rects, *Rotate);
	//	}
	//	m_optFind.emplace(find, rects);
	//}
	//return m_optFind->FindRects;
}

CFPDFBitmap CPDFPage::GetFPDFBitmap(const FLOAT& scale, const int& rotate, std::function<bool()> cancel)
{
	do {
		CSizeU sz(static_cast<UINT32>(std::round(GetFPDFPagePtr()->GetPageWidthF() * scale)),
			static_cast<UINT32>(std::round(GetFPDFPagePtr()->GetPageHeightF() * scale)));

		CFPDFBitmap fpdfBmp;
		FALSE_BREAK(fpdfBmp.CreateEx(sz.width, sz.height, FPDFBitmap_BGRA, NULL, 0, cancel));
		FALSE_BREAK(fpdfBmp);

		FALSE_BREAK(fpdfBmp.FillRect(0, 0, sz.width, sz.height, 0xFFFFFFFF, cancel)); // Fill white
		int flags = FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE;
		FALSE_BREAK(fpdfBmp.RenderPageBitmap(*GetFPDFPagePtr(), 0, 0, sz.width, sz.height, 0, flags, cancel));
		m_pDoc->GetFormHandle()->FFLDraw(fpdfBmp, *GetFPDFPagePtr(), 0, 0, sz.width, sz.height, 0, flags);

		return fpdfBmp;
	} while (1);

	return nullptr;
}

CFPDFBitmap CPDFPage::GetClipFPDFBitmap(const FLOAT& scale, const int& rotate, const CRectF& rect, std::function<bool()> cancel)
{
	do {
		CRectU scaledRect = CRectF2CRectU(rect * scale);

		TRUE_BREAK(scaledRect.Width() <= 0 || scaledRect.Height() <= 0);

		CFPDFBitmap fpdfBmp;
		fpdfBmp.CreateEx(scaledRect.Width(), scaledRect.Height(), FPDFBitmap_BGRA, NULL, 0);
		FALSE_BREAK(fpdfBmp);

		FALSE_BREAK(fpdfBmp.FillRect(0, 0, scaledRect.Width(), scaledRect.Height(), 0xFFFFFFFF)); // Fill white
		FS_MATRIX mat{scale, 0.f, 0.f, scale, -static_cast<float>(scaledRect.left), -static_cast<float>(scaledRect.top)};
		FS_RECTF rcf{0, 0, static_cast<float>(scaledRect.Width()), static_cast<float>(scaledRect.Height())};
		int flags = FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE;
		FALSE_BREAK(fpdfBmp.RenderPageBitmapWithMatrix(*GetFPDFPagePtr(), &mat, &rcf, flags));
		m_pDoc->GetFormHandle()->FFLDraw(fpdfBmp, *GetFPDFPagePtr(), scaledRect.left, scaledRect.right, scaledRect.Width(), scaledRect.Height(), 0, flags);

		return fpdfBmp;
	} while (1);

	return nullptr;

}

CFPDFBitmap CPDFPage::GetThumbnailFPDFBitmap()
{
	return CFPDFBitmap(GetFPDFPagePtr()->GetThumbnailAsBitmap());
}


UHBITMAP CPDFPage::GetBitmap(HDC hDC, const FLOAT& scale, const int&, std::function<bool()> cancel)
{
	do {
		CSizeF sz(GetFPDFPagePtr()->GetPageWidthF() * scale, GetFPDFPagePtr()->GetPageHeightF() * scale);

		BITMAPINFOHEADER bmih
		{
			.biSize = sizeof(BITMAPINFOHEADER),
			.biWidth = static_cast<LONG>(std::round(sz.width)),
			.biHeight = -static_cast<LONG>(std::round(sz.height)),
			.biPlanes = 1,
			.biBitCount = 32,
			.biCompression = BI_RGB,
			.biSizeImage = 0,
		};

		void* bitmapBits = nullptr;

		UHBITMAP phBmp(::CreateDIBSection(hDC, reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0));
		FALSE_BREAK(phBmp);

		CFPDFBitmap fpdfBmp;
		FALSE_BREAK(fpdfBmp.CreateEx(bmih.biWidth, -bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3), cancel));
		FALSE_BREAK(fpdfBmp);

		FALSE_BREAK(fpdfBmp.FillRect(0, 0, bmih.biWidth, -bmih.biHeight, 0xFFFFFFFF, cancel)); // Fill white
		int flags = FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE;
		FALSE_BREAK(fpdfBmp.RenderPageBitmap(*GetFPDFPagePtr(), 0, 0, bmih.biWidth, -bmih.biHeight, 0, flags, cancel));
		m_pDoc->GetFormHandle()->FFLDraw(fpdfBmp, *GetFPDFPagePtr(), 0, 0, bmih.biWidth, -bmih.biHeight, 0, flags);

		return phBmp;
	} while (1);

	return nullptr;
}

CComPtr<ID2D1Bitmap1> CPDFPage::GetD2D1Bitmap(const CDirect2DWrite* pDirect, const FLOAT& scale, const int&, std::function<bool()> cancel)
{
	do {
		CSizeU sz(static_cast<UINT32>(std::round(GetFPDFPagePtr()->GetPageWidthF() * scale)),
			static_cast<UINT32>(std::round(GetFPDFPagePtr()->GetPageHeightF() * scale)));

		CFPDFBitmap fpdfBmp;
		FALSE_BREAK(fpdfBmp.CreateEx(sz.width, sz.height, FPDFBitmap_BGRx, NULL, 0, cancel));
		FALSE_BREAK(fpdfBmp);

		FALSE_BREAK(fpdfBmp.FillRect(0, 0, sz.width, sz.height, 0xFFFFFFFF, cancel)); // Fill white
		int flags = FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE;
		FALSE_BREAK(fpdfBmp.RenderPageBitmap(*GetFPDFPagePtr(), 0, 0, sz.width, sz.height, 0, flags, cancel));
		m_pDoc->GetFormHandle()->FFLDraw(fpdfBmp, *GetFPDFPagePtr(), 0, 0, sz.width, sz.height, 0, flags);

		UINT32 pitch = fpdfBmp.GetStride();
		void* srcData = fpdfBmp.GetBuffer();

		CComPtr<ID2D1Bitmap1> pD2D1Bitmap;
		FAILED_BREAK(pDirect->GetD2DDeviceContext()->CreateBitmap(
					sz,
					nullptr, 0,
					D2D1::BitmapProperties1(
						D2D1_BITMAP_OPTIONS_NONE,
						D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
						&pD2D1Bitmap
					));
		CRectU rc(sz);
		FAILED_BREAK(pD2D1Bitmap->CopyFromMemory(&rc, srcData, pitch));

		return pD2D1Bitmap;
	} while (1);

	return nullptr;
}


UHBITMAP CPDFPage::GetDDBitmap(HDC hDC, const FLOAT& scale, const int&, std::function<bool()> cancel)
{
	do {
		CSizeF sz(GetFPDFPagePtr()->GetPageWidthF() * scale, GetFPDFPagePtr()->GetPageHeightF() * scale);
		BITMAPINFO bmi
		{
			.bmiHeader = BITMAPINFOHEADER
			{
			.biSize = sizeof(BITMAPINFOHEADER),
			.biWidth = static_cast<LONG>(std::round(sz.width)),
			.biHeight = -static_cast<LONG>(std::round(sz.height)),
			.biPlanes = 1,
			.biBitCount = 32,
			.biCompression = BI_RGB,
			.biSizeImage = 0,},
		};

		void* bitmapBits = nullptr;

		UHBITMAP phDIB(::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &bitmapBits, nullptr, 0));
		FALSE_BREAK(phDIB);

		CFPDFBitmap fpdfBmp;
		FALSE_BREAK(fpdfBmp.CreateEx(bmi.bmiHeader.biWidth, -bmi.bmiHeader.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmi.bmiHeader.biWidth * bmi.bmiHeader.biBitCount) + 31) & ~31) >> 3), cancel));
		FALSE_BREAK(fpdfBmp);

		FALSE_BREAK(fpdfBmp.FillRect(0, 0, bmi.bmiHeader.biWidth, -bmi.bmiHeader.biHeight, 0xFFFFFFFF, cancel)); // Fill white
		int flags = FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE;
		FALSE_BREAK(fpdfBmp.RenderPageBitmap(*GetFPDFPagePtr(), 0, 0, bmi.bmiHeader.biWidth, -bmi.bmiHeader.biHeight, 0, flags, cancel));
		m_pDoc->GetFormHandle()->FFLDraw(fpdfBmp, *GetFPDFPagePtr(), 0, 0, bmi.bmiHeader.biWidth, -bmi.bmiHeader.biHeight, 0, flags);

		//Convert to DDB
		BITMAP bm;
		::GetObject(phDIB.get(), sizeof(BITMAP), &bm);
		UHBITMAP phDDB(::CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight));
		FALSE_BREAK(::SetDIBits(hDC, phDDB.get(), 0, bm.bmHeight, bm.bmBits, &bmi, 0));

		return phDDB;
	} while (1);

	return nullptr;
}

UHBITMAP CPDFPage::GetClipBitmap(HDC hDC, const FLOAT& scale, const int&, const CRectF& rectInPage, std::function<bool()> cancel)
{
	do {
		CRectU scaledRectInPage = CRectF2CRectU(rectInPage * scale);
		BITMAPINFOHEADER bmih
		{
			.biSize = sizeof(BITMAPINFOHEADER),
			.biWidth = static_cast<LONG>(scaledRectInPage.Width()),
			.biHeight = -static_cast<LONG>(scaledRectInPage.Height()),
			.biPlanes = 1,
			.biBitCount = 32,
			.biCompression = BI_RGB,
			.biSizeImage = 0,
		};

		TRUE_BREAK(bmih.biWidth == 0 || bmih.biHeight == 0);

		void* bitmapBits = nullptr;

		UHBITMAP phBmp(::CreateDIBSection(hDC, reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0));
		FALSE_BREAK(phBmp);

		CFPDFBitmap fpdfBmp;
		fpdfBmp.CreateEx(bmih.biWidth, -bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3));
		FALSE_BREAK(fpdfBmp);

		FALSE_BREAK(fpdfBmp.FillRect(0, 0, bmih.biWidth, -bmih.biHeight, 0xFFFFFFFF)); // Fill white
		FS_MATRIX mat{scale, 0.f, 0.f, scale, -static_cast<float>(scaledRectInPage.left), -static_cast<float>(scaledRectInPage.top)};
		FS_RECTF rcf{0, 0, static_cast<float>(bmih.biWidth), static_cast<float>(-bmih.biHeight)};
		FALSE_BREAK(fpdfBmp.RenderPageBitmapWithMatrix(*GetFPDFPagePtr(), &mat, &rcf, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE));

		return phBmp;
	} while (1);

	return nullptr;
}

CRectF CPDFPage::GetCaretRect(const int index)
{
	if (!GetTextCursorRects().empty()) {
		auto rcInPdfiumPage = GetTextCursorRects()[index];
		switch (*Rotate) {
			case 0:
				rcInPdfiumPage.right = rcInPdfiumPage.left + 1.f;
				break;
			case 1:
				rcInPdfiumPage.bottom = rcInPdfiumPage.top + 1.f;
				break;
			case 2:
				rcInPdfiumPage.left = rcInPdfiumPage.right - 1.f;
				break;
			case 3:
				rcInPdfiumPage.top = rcInPdfiumPage.bottom - 1.f;
				break;
		}
		return rcInPdfiumPage;
	} else {
		return CRectF();
	}
}

CRectF CPDFPage::RotateRect(const CRectF& rc, const int& rotate) const
{
	CSizeF size = GetSize();
	const FLOAT pw = size.width;
	const FLOAT ph = size.height;


	auto RectTransform = [](const CRectF& rc, FLOAT a, FLOAT b, FLOAT c, FLOAT d, FLOAT tx, FLOAT ty)->CRectF
	{
		auto PointTransform = [](const CPointF& pt, FLOAT a, FLOAT b, FLOAT c, FLOAT d, FLOAT tx, FLOAT ty)->CPointF
		{
			return CPointF(((a * pt.x) + (c * pt.y) + tx), ((b * pt.x) + (d * pt.y) + ty));
		};
		CPointF pt1(rc.left, rc.top); pt1 = PointTransform(pt1, a, b, c, d, tx, ty);
		CPointF pt2(rc.right, rc.bottom); pt2 = PointTransform(pt2, a, b, c, d, tx, ty);

		if (pt1.x > pt2.x) { std::swap(pt1.x, pt2.x); }
		if (pt1.y < pt2.y) { std::swap(pt1.y, pt2.y); }

		return CRectF(pt1.x, pt1.y, pt2.x, pt2.y);
	};

	switch (rotate) {
		default:
		case 0: // 0 degrees
		{
			//const UXMatrix t = { 1.0, 0.0, 0.0, -1.0, 0.0, ph };
			return RectTransform(rc, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
			break;
		}

		case 1: // 90 degrees
		{
			//const UXMatrix t = { 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			return  RectTransform(rc, 0.f, -1.f, 1.f, 0.f, 0.f, ph);
			break;
		}

		case 2: // 180 degrees
		{
			//const UXMatrix t = { -1.0, 0.0, 0.0, 1.0, pw, 0.0 };
			return RectTransform(rc, -1.f, 0.f, 0.f, -1.f, pw, ph);
			break;
		}

		case 3: // 270 degrees
		{
			//const UXMatrix t = { 0.0, -1.0, -1.0, 0.0, pw, ph };
			return RectTransform(rc, 0.f, 1.f, -1.f, 0.f, pw, 0.f);
			break;
		}
	}
}

void CPDFPage::RotateRects(std::vector<CRectF>& rects, const int& rotate) const
{
	for (CRectF& rc : rects) {
		rc = RotateRect(rc, rotate);
	}
}

const std::vector<CRectF>& CPDFPage::GetSelectedTextRects(const int& begin, const int& end)
{
	if (!m_optSelectedText.has_value() || m_optSelectedText->Begin != begin || m_optSelectedText->End != end) {
		std::vector<CRectF> rectsInPdfiumPage(GetFPDFTextPagePtr()->GetRangeRects(begin, end));
		RotateRects(rectsInPdfiumPage, *Rotate);
		m_optSelectedText.emplace(begin, end, rectsInPdfiumPage);
	}
	return m_optSelectedText->SelectedRects;
}

int CPDFPage::GetCursorCharIndexAtPos(const CPointF& ptInPdfiumPage)
{
	const auto& textMouseRects = GetTextMouseRects();
	auto iter = std::find_if(textMouseRects.cbegin(), textMouseRects.cend(),
		[ptInPdfiumPage](const CRectF& rc)->bool {
			return rc.left <= ptInPdfiumPage.x && ptInPdfiumPage.x < rc.right &&
				rc.top >= ptInPdfiumPage.y && ptInPdfiumPage.y > rc.bottom;
		});
	if (iter == textMouseRects.cend()) {
		return -1;
	} else {
		return std::distance(textMouseRects.cbegin(), iter);
	}
}



