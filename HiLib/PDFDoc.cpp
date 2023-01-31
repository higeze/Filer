#include "PDFDoc.h"
#include "PDFPage.h"
#include "D2DWWindow.h"
#include <fpdf_edit.h>
#include <fpdfview.h>
#include <mutex>
#include <boost/algorithm/string.hpp>
#include "async_catch.h"
#include "ThreadPool.h"
#include "MyClipboard.h"
#include "strconv.h"

void CPDFDoc::Init()
{
	const FPDF_LIBRARY_CONFIG config{ 2, nullptr, nullptr, 0 };
	FPDF_InitLibraryWithConfig(&config);
}

void CPDFDoc::Term()
{
	FPDF_DestroyLibrary();
}

CPDFDoc::CPDFDoc()
	:m_pPDFium(std::make_unique<CPDFiumSingleThread>()){}

CPDFDoc::~CPDFDoc() = default;

const CSizeF& CPDFDoc::GetSize()
{
	if (!m_optSize.has_value()) {
		m_optSize.emplace();
		for (const auto& pPage : m_pages) {
			m_optSize->width = (std::max)(m_optSize->width, pPage->GetSize().width);
			m_optSize->height += pPage->GetSize().height;
		}
		m_optSize->height += (m_pages.size() - 1) * 10.f;
	}
	return m_optSize.value();
}

const std::vector<CRectF>& CPDFDoc::GetPageRects()
{
	if (!m_optPageRects.has_value()) {
		std::vector<CRectF> rects;
		FLOAT height = 0.f;
		for (const auto& pPage : m_pages) {
			rects.emplace_back(0.f, height, pPage->GetSize().width, height + pPage->GetSize().height);
			height += pPage->GetSize().height + 10.f;
		}
		m_optPageRects.emplace(rects);
	}
	return m_optPageRects.value();
}

const int& CPDFDoc::GetFileVersion()
{
	if (!m_optFileVersion.has_value()) {
		int version;
		if (m_pPDFium->GetFileVersion(m_pDoc.get(), &version)) {
			m_optFileVersion.emplace(version);
		} else {
			m_optFileVersion.emplace(0);
		}
	}
	return m_optFileVersion.value();
}

const int& CPDFDoc::GetPageCount()
{
	if (!m_optPageCount.has_value()) {
		m_optPageCount.emplace(m_pPDFium->GetPageCount(m_pDoc.get()));
	}
	return m_optPageCount.value();
}

void CPDFDoc::Open(const std::wstring& path, const std::wstring& password)
{
	m_path = path;
	m_password = password;
	m_isDirty = false;

	m_pDoc = std::move(m_pPDFium->UnqLoadDocument(wide_to_utf8(m_path).c_str(), wide_to_utf8(m_password).c_str()));

	m_pages.clear();
	for (auto i = 0; i < GetPageCount(); i++) {
		m_pages.emplace_back(std::make_unique<CPDFPage>(this, i))->Rotate.Subscribe([this](const auto& i) 
			{ 
				m_optSize.reset();
				m_optPageRects.reset();
				m_isDirty = true;
			});
	}
}

void CPDFDoc::Create()
{
	m_path = L"";
	m_password = L"";
	m_isDirty = true;
	m_pDoc = std::move(m_pPDFium->UnqCreateDocument());
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

void CPDFDoc::Save()
{
	//To release binding between FPDF_DOCUMENT and File, Copy to new FPDF_DOCUMENT and Swap
	UNQ_FPDF_DOCUMENT pNew = std::move(m_pPDFium->UnqCreateDocument());
	m_pPDFium->ImportPages(pNew.get(), m_pDoc.get(), nullptr, 0);
	m_pDoc = std::move(pNew);
	m_pages.clear();

	SaveWithVersion(m_path, 0, GetFileVersion());

	m_optPageCount.reset();
	m_optSize.reset();
	m_optPageRects.reset();
	m_isDirty = false;

	for (auto i = 0; i < GetPageCount(); i++) {
		m_pages.emplace_back(std::make_unique<CPDFPage>(this, i))->Rotate.Subscribe([this](const auto& i) 
			{ 
				m_optSize.reset();
				m_optPageRects.reset();
				m_isDirty = true;
			});
	}
}
void CPDFDoc::SaveWithVersion(const std::wstring& path, FPDF_DWORD flags, int fileVersion)
{
	CPDFiumFileWrite fileWrite(path);
	GetPDFiumPtr()->SaveWithVersion(GetDocPtr().get(), &fileWrite, flags, fileVersion);
}

void CPDFDoc::ImportPages(const CPDFDoc& src_doc,
				FPDF_BYTESTRING pagerange,
				int index)
{
	m_pPDFium->ImportPages(m_pDoc.get(), src_doc.m_pDoc.get(), pagerange, index);
	m_optPageCount.reset();
	m_optSize.reset();
	m_optPageRects.reset();
}

void CPDFDoc::SplitSave()
{
	for (auto i = 0; i < GetPageCount(); i++) {
		wchar_t srcPathWoExt[MAX_PATH] = { 0 };
		wcscpy_s(srcPathWoExt, m_path.c_str());
		::PathRemoveExtensionW(srcPathWoExt);

		wchar_t dstPath[MAX_PATH] = { 0 };
		swprintf_s(dstPath, L"%s_%02d.pdf", srcPathWoExt, i + 1);

		auto dstDoc = CPDFDoc();
		dstDoc.Create();

		std::array<int, 1> indices{ i };
		m_pPDFium->ImportPagesByIndex(dstDoc.m_pDoc.get(), m_pDoc.get(), indices.data(), indices.size(),  0);

		CPDFiumFileWrite fileWrite(dstPath);

		m_pPDFium->SaveWithVersion(dstDoc.m_pDoc.get(), &fileWrite, 0, GetFileVersion());
	}
}

void CPDFDoc::Merge(const std::vector<CPDFDoc>& srcDocs)
{
	for (const auto& srcDoc : srcDocs) {
		auto count = GetPageCount();
		ImportPages(srcDoc, NULL, count);
	}
}



void CPDFDoc::Merge(const std::vector<std::wstring>& srcPathes)
{
	for (const auto& srcPath : srcPathes) {
		CPDFDoc srcDoc;
		srcDoc.Open(srcPath);
		auto count = GetPageCount();
		ImportPages(srcDoc, NULL, count);
	}
}

CPDFDoc CPDFDoc::Extract(const std::wstring& page_indices)
{
	CPDFDoc dstDoc;
	dstDoc.Create();
	m_pPDFium->ImportPages(dstDoc.m_pDoc.get(), m_pDoc.get(), wide_to_sjis(page_indices).c_str(), 0);
	return dstDoc;
}

CPDFDoc CPDFDoc::Clone()
{
	CPDFDoc dstDoc;
	dstDoc.Create();
	m_pPDFium->ImportPages(dstDoc.m_pDoc.get(), m_pDoc.get(), nullptr, 0);
	return dstDoc;
}


