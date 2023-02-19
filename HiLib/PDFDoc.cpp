#include "PDFDoc.h"
#include "PDFPage.h"
#include "D2DWWindow.h"
#include <fpdf_edit.h>
#include <fpdfview.h>
#include <mutex>
#include <boost/algorithm/string.hpp>
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

CPDFDoc::CPDFDoc(size_t threads)
	:m_pPDFium(std::make_unique<CPDFiumMultiThread>(threads)){}

CPDFDoc::~CPDFDoc() = default;

const CSizeF& CPDFDoc::GetSize() const
{
	if (!m_optSize.has_value()) {
		m_optSize.emplace();
		for (auto i = 0; i < GetPageCount(); i++) {
			m_optSize->width = (std::max)(m_optSize->width, GetPage(i)->GetSize().width);
			m_optSize->height += GetPage(i)->GetSize().height;
		}
		m_optSize->height += (GetPageCount() - 1) * pagespan;
	}
	return m_optSize.value();
}

const std::vector<CRectF>& CPDFDoc::GetPageRects() const
{
	if (!m_optPageRects.has_value()) {
		std::vector<CRectF> rects;
		FLOAT height = 0.f;
		for (auto i = 0; i < GetPageCount(); i++) {
			rects.emplace_back(0.f, height, GetPage(i)->GetSize().width, height + GetPage(i)->GetSize().height);
			height += GetPage(i)->GetSize().height + 10.f;
		}
		m_optPageRects.emplace(rects);
	}
	return m_optPageRects.value();
}

const int& CPDFDoc::GetFileVersion() const
{
	if (!m_optFileVersion.has_value()) {
		int version;
		if (m_pPDFium->GetFileVersion(&version)) {
			m_optFileVersion.emplace(version);
		} else {
			m_optFileVersion.emplace(0);
		}
	}
	return m_optFileVersion.value();
}

const int& CPDFDoc::GetPageCount() const
{
	if (!m_optPageCount.has_value()) {
		m_optPageCount.emplace(m_pPDFium->GetPageCount());
	}
	return m_optPageCount.value();
}

const std::unique_ptr<CPDFPage>& CPDFDoc::GetPage(const int& index) const
{
	if (!m_optPages.has_value()) {
		std::vector<std::unique_ptr<CPDFPage>> pages;
		for (auto i = 0; i < GetPageCount(); i++) {
			pages.emplace_back(std::make_unique<CPDFPage>(this, i))->Rotate.Subscribe([this](const auto& i) 
				{ 
					m_optSize.reset();
					m_optPageRects.reset();
					m_isDirty = true;
				});
		}
		m_optPages.emplace(std::move(pages));
	}
	return m_optPages.value()[index];
}

std::unique_ptr<CPDFPage>& CPDFDoc::GetPage(const int& index)
{
	if (!m_optPages.has_value()) {
		std::vector<std::unique_ptr<CPDFPage>> pages;
		for (auto i = 0; i < GetPageCount(); i++) {
			pages.emplace_back(std::make_unique<CPDFPage>(this, i))->Rotate.Subscribe([this](const auto& i) 
				{ 
					m_optSize.reset();
					m_optPageRects.reset();
					m_isDirty = true;
				});
		}
		m_optPages.emplace(std::move(pages));
	}
	return m_optPages.value()[index];
}


unsigned long CPDFDoc::Open(const std::wstring& path, const std::wstring& password)
{
	m_path = path;
	m_password = password;
	m_isDirty = false;

	auto err = m_pPDFium->LoadDocument(wide_to_utf8(m_path).c_str(), wide_to_utf8(m_password).c_str());
	m_optPages.reset();

	return err;
}

unsigned long  CPDFDoc::Create()
{
	m_path = L"";
	m_password = L"";
	m_isDirty = true;
	return m_pPDFium->CreateDocument();
}

void CPDFDoc::CopyTextToClipboard(const CopyDocTextEvent& e) const
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
	std::unique_ptr<CPDFiumMultiThread> pNewPDFium(std::make_unique<CPDFiumMultiThread>());
	pNewPDFium->CreateDocument();
	pNewPDFium->ImportPages(*m_pPDFium, nullptr, 0);
	m_pPDFium = std::move(pNewPDFium);
	m_optPages.reset();

	SaveWithVersion(m_path, 0, GetFileVersion());
}

void CPDFDoc::SaveWithVersion(const std::wstring& path, FPDF_DWORD flags, int fileVersion)
{
	CPDFiumFileWrite fileWrite(path);
	GetPDFiumPtr()->SaveWithVersion(&fileWrite, flags, fileVersion);

	m_optFileVersion.reset();
	m_optPageCount.reset();
	m_optSize.reset();
	m_optPageRects.reset();
	m_optPages.reset();
	m_isDirty = false;
}

void CPDFDoc::ImportPages(const CPDFDoc& src_doc,
				FPDF_BYTESTRING pagerange,
				int index)
{
	GetPDFiumPtr()->ImportPages(*src_doc.GetPDFiumPtr(), pagerange, index);
	m_optFileVersion.reset();
	m_optPageCount.reset();
	m_optSize.reset();
	m_optPageRects.reset();
	m_optPages.reset();
	m_isDirty = true;
}

void CPDFDoc::ImportPagesByIndex(const CPDFDoc& src_doc,
	const int* page_indices,
	unsigned long length,
	int index)
{
	GetPDFiumPtr()->ImportPagesByIndex(*src_doc.GetPDFiumPtr(), page_indices, length, index);
}

void CPDFDoc::SplitSave() const
{
	for (auto i = 0; i < GetPageCount(); i++) {
		wchar_t srcPathWoExt[MAX_PATH] = { 0 };
		wcscpy_s(srcPathWoExt, m_path.c_str());
		::PathRemoveExtensionW(srcPathWoExt);

		wchar_t dstPath[MAX_PATH] = { 0 };
		swprintf_s(dstPath, L"%s_%02d.pdf", srcPathWoExt, i + 1);

		CPDFDoc dstDoc(1);
		dstDoc.Create();

		std::array<int, 1> indices{ i };
		dstDoc.ImportPagesByIndex(*this, indices.data(), indices.size(),  0);

		dstDoc.SaveWithVersion(dstPath, 0, GetFileVersion());
	}
}

void CPDFDoc::Merge(const std::vector<CPDFDoc>& srcDocs)
{
	for (auto& srcDoc : srcDocs) {
		auto count = GetPageCount();
		ImportPages(srcDoc, NULL, count);
	}
}

void CPDFDoc::Merge(const std::vector<std::wstring>& srcPathes)
{
	for (const auto& srcPath : srcPathes) {
		CPDFDoc srcDoc(1);
		srcDoc.Open(srcPath);
		auto count = GetPageCount();
		ImportPages(srcDoc, NULL, count);
	}
}

CPDFDoc CPDFDoc::Extract(const std::wstring& page_indices) const
{
	CPDFDoc dstDoc(1);
	dstDoc.Create();
	dstDoc.ImportPages(*this, wide_to_sjis(page_indices).c_str(), 0);
	return dstDoc;
}

CPDFDoc CPDFDoc::Clone() const
{
	CPDFDoc dstDoc(1);
	dstDoc.Create();
	dstDoc.ImportPages(*this, nullptr, 0);
	return dstDoc;
}


