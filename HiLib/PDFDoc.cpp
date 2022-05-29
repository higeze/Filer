#include "PDFDoc.h"
#include "PDFPage.h"
#include <fpdf_edit.h>
#include <fpdfview.h>
#include <mutex>
#include <boost/algorithm/string.hpp>
#include "async_catch.h"
#include "ThreadPool.h"
#include "PdfView.h"
#include "PDFViewport.h"
#include "MyClipboard.h"


#include "strconv.h"

namespace sml = boost::sml;

void CPDFDoc::Init()
{
	const FPDF_LIBRARY_CONFIG config{ 2, nullptr, nullptr, 0 };
	FPDF_InitLibraryWithConfig(&config);
}

void CPDFDoc::Term()
{
	FPDF_DestroyLibrary();
}

CPDFDoc::CPDFDoc(const std::shared_ptr<PdfViewProperty>& spProp, std::function<void()> changed)
	:m_pPDFium(std::make_unique<CPDFiumSingleThread>()),
	m_pProp(spProp),
	m_changed(changed){}

CPDFDoc::~CPDFDoc() = default;

void CPDFDoc::Open(const std::wstring& path, const std::wstring& password)
{
	m_path = path;
	m_password = password;

	m_pDoc = std::move(m_pPDFium->UnqLoadDocument(wide_to_utf8(m_path).c_str(), wide_to_utf8(m_password).c_str()));

	m_pageCount = m_pPDFium->GetPageCount(m_pDoc.get());

	for (auto i = 0; i < m_pageCount; i++) {
		m_pages.emplace_back(std::make_unique<CPDFPage>(this, i));
	}

	for (const auto& pPage : m_pages) {
		m_sourceRectsInDoc.emplace_back(0.f, m_sourceSize.height, pPage->GetSourceSize().width, m_sourceSize.height + pPage->GetSourceSize().height);

		m_sourceSize.width = (std::max)(m_sourceSize.width, pPage->GetSourceSize().width);
		m_sourceSize.height += pPage->GetSourceSize().height;
	}
}

void CPDFDoc::RenderContent(const RenderDocContentEvent& e)
{
	if (!GetPageCount()) { return; }
	for (auto i = e.PageIndexBegin; i < e.PageIndexEnd; i++) {
		GetPage(i)->RenderContent(
			RenderPageContentEvent(e.DirectPtr, e.ViewportPtr, e.Scale, i));
	}
}

void CPDFDoc::RenderFind(const RenderDocFindEvent& e)
{
	if (!GetPageCount()) { return; }
	for (auto i = e.PageIndexBegin; i < e.PageIndexEnd; i++) {
		GetPage(i)->RenderFind(
			RenderPageFindEvent(e.DirectPtr, e.ViewportPtr, e.Find, i));
	}
}

void CPDFDoc::RenderFindLine(const RenderDocFindLineEvent& e)
{
	if (!GetPageCount()) { return; }
	FLOAT fullHeight = GetSourceSize().height;
	FLOAT top = 0.f;
	for (auto i = 0; i < GetPageCount(); i++) {
		auto rectHighlite = CRectF(
			e.Rect.left + 2.f,
			e.Rect.top + e.Rect.Height() * m_sourceRectsInDoc[i].top / fullHeight,
			e.Rect.right - 2.f,
			e.Rect.top + e.Rect.Height() * m_sourceRectsInDoc[i].bottom / fullHeight);

		GetPage(i)->RenderFindLine(RenderPageFindLineEvent(e.DirectPtr, e.ViewportPtr, e.Find, rectHighlite));
	}
}

void CPDFDoc::RenderSelectedText(const RenderDocSelectedTextEvent& e)
{
	if (!GetPageCount()) { return; }

	auto [page_begin_index, char_begin_index] = e.SelectedBegin;
	auto [page_end_index, char_end_index] = e.SelectedEnd;

	if (page_begin_index == page_end_index) {
		GetPage(page_begin_index)->RenderSelectedText(
			RenderPageSelectedTextEvent(e.DirectPtr, e.ViewportPtr, page_begin_index, char_begin_index, char_end_index));
	} else {
		for (auto i = page_begin_index; i <= page_end_index; i++) {
			if (i == page_begin_index) {
				GetPage(i)->RenderSelectedText(
							RenderPageSelectedTextEvent(e.DirectPtr, e.ViewportPtr, i, char_begin_index, GetPage(i)->GetTextSize()));
			} else if (i == page_end_index) {
				GetPage(i)->RenderSelectedText(
							RenderPageSelectedTextEvent(e.DirectPtr, e.ViewportPtr, i, 0, char_end_index));
			} else {
				GetPage(i)->RenderSelectedText(
							RenderPageSelectedTextEvent(e.DirectPtr, e.ViewportPtr, i, 0, GetPage(i)->GetTextSize()));
			}
		}
	}
}

void CPDFDoc::RenderCaret(const RenderDocCaretEvent& e)
{
	if (!GetPageCount()) { return; }

	GetPage(e.PageIndex)->RenderCaret(RenderPageCaretEvent(e.DirectPtr, e.ViewportPtr, e.PageIndex, e.CharIndex));
}


void CPDFDoc::CopyTextToClipboard(const CopyDocTextEvent& e)
{
	auto [page_begin_index, char_begin_index] = e.Begin;
	auto [page_end_index, char_end_index] = e.End;

	std::wstring copyText;
	if (page_begin_index == page_end_index) {
		copyText = GetPage(page_begin_index)->GetText().substr(char_begin_index, char_end_index - char_begin_index);
	} else {
		for (auto i = page_begin_index; i <= page_end_index; i++) {
			if (i == page_begin_index) {
				copyText.append(GetPage(page_begin_index)->GetText().substr(char_begin_index, GetPage(i)->GetTextSize()-char_begin_index));
			} else if (i == page_end_index) {
				copyText.append(GetPage(i)->GetText().substr(0, char_end_index));
			} else {
				copyText.append(GetPage(i)->GetText().substr(0, GetPage(i)->GetTextSize()));
			}
		}
	}

	if (!copyText.empty()) {
		HGLOBAL hGlobal = ::GlobalAlloc(GHND | GMEM_SHARE, (copyText.size() + 1) * sizeof(wchar_t));
		wchar_t* strMem = (wchar_t*)::GlobalLock(hGlobal);
		::GlobalUnlock(hGlobal);

		if (strMem != NULL) {
			::wcscpy_s(strMem, copyText.size() + 1, copyText.c_str());
			if (CClipboard clipboard; clipboard.Open(e.WndPtr->m_hWnd) != 0) {
				clipboard.Empty();
				clipboard.SetData(CF_UNICODETEXT, hGlobal);
			}
		}

	}

}



