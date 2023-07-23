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
#include "FPDFDocument.h"
#include "FPDFPage.h"
#include "FPDFTextPage.h"
#include "FPDFFormHandle.h"
#include "FPDFFileWrite.h"

void CPDFDoc::Init()
{
	const FPDF_LIBRARY_CONFIG config{ 2, nullptr, nullptr, 0 };
	FPDF_InitLibraryWithConfig(&config);
}

void CPDFDoc::Term()
{
	FPDF_DestroyLibrary();
}

int FormAlert(IPDF_JSPLATFORM*, FPDF_WIDESTRING, FPDF_WIDESTRING, int, int)
{
	printf("FormAlert called.\n");
	return 0;
}

CPDFDoc::CPDFDoc()
	:m_pDoc(std::make_unique<CFPDFDocument>())
{
	memset(&m_jsPlatForm, 0, sizeof(m_jsPlatForm));
	m_jsPlatForm.version = 1;
	m_jsPlatForm.app_alert = FormAlert;

	memset(&m_formFillInfo, 0, sizeof(m_formFillInfo));
	m_formFillInfo.version = 1;
	m_formFillInfo.m_pJsPlatform = &m_jsPlatForm;
}

CPDFDoc::~CPDFDoc()
{
	//Release FormHandlePtr strong reference
	m_optPages.reset();

	//Exit before m_formFillInfo destructed
	if (m_optFormHandlePtr.has_value()) {
		//m_optFormHandlePtr.value()->DoDocumentAAction(FPDFDOC_AACTION_WC);
		m_optFormHandlePtr.value()->ExitFormFillEnvironment();
	}
}

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
		if (m_pDoc->GetFileVersion(&version)) {
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
		m_optPageCount.emplace(m_pDoc->GetPageCount());
	}
	return m_optPageCount.value();
}

const std::shared_ptr<CFPDFFormHandle>& CPDFDoc::GetFormHandlePtr() const
{
	if (!m_optFormHandlePtr.has_value()) {
		std::shared_ptr<CFPDFFormHandle> pForm(std::make_shared<CFPDFFormHandle>());
		pForm->InitFormFillEnvironment(*m_pDoc, &m_formFillInfo);
		pForm->SetFormFieldHighlightColor(FPDF_FORMFIELD_UNKNOWN, 0xFFE4DD);
		pForm->SetFormFieldHighlightAlpha(100);
		//pForm->DoDocumentJSAction();
		//pForm->DoDocumentOpenAction();
		m_optFormHandlePtr.emplace(pForm);
	}
	return m_optFormHandlePtr.value();
}

const std::vector<std::unique_ptr<CPDFPage>>& CPDFDoc::GetPages() const
{
	if (!m_optPages.has_value()) {

		std::vector<std::unique_ptr<CPDFPage>> pages;
		for (auto i = 0; i < GetPageCount(); i++) {
			std::unique_ptr<CFPDFPage> pPage(std::make_unique<CFPDFPage>(m_pDoc->LoadPage(i)));
			std::unique_ptr<CFPDFTextPage> pTextPage(std::make_unique<CFPDFTextPage>(pPage->LoadTextPage()));
			pages.emplace_back(std::make_unique<CPDFPage>(std::move(pPage), std::move(pTextPage), GetFormHandlePtr()))->Rotate.Subscribe([this](const auto& i) 
				{ 
					m_optSize.reset();
					m_optPageRects.reset();
					IsDirty->set(true);
				});
		}
		m_optPages.emplace(std::move(pages));
	}
	return m_optPages.value();
}

const std::unique_ptr<CPDFPage>& CPDFDoc::GetPage(const int& index) const
{
	return GetPages()[index];
}

int CPDFDoc::GetPageIndex(const std::unique_ptr<CPDFPage>& pPage)
{
	auto iter = std::find(GetPages().cbegin(), GetPages().cend(), pPage);
	if (iter != GetPages().cend()) {
		return std::distance(GetPages().cbegin(), iter);
	} else {
		return -1;
	}
}

int CPDFDoc::GetPageIndex(const CPDFPage* pPage)
{
	auto iter = std::find_if(GetPages().cbegin(), GetPages().cend(), [pPage](const std::unique_ptr<CPDFPage>& p)->bool { return p.get() == pPage; });
	if (iter != GetPages().cend()) {
		return std::distance(GetPages().cbegin(), iter);
	} else {
		return -1;
	}
}

unsigned long CPDFDoc::Open(const std::wstring& path, const std::wstring& password)
{
	m_path = path;
	m_password = password;
	IsDirty->set(false);

	m_optFormHandlePtr.reset();
	m_optPages.reset();

    m_pDoc->LoadDocument(wide_to_utf8(path).c_str(), wide_to_utf8(password).c_str());

    unsigned long err = FPDF_GetLastError();
    if (!*m_pDoc) {
        return err;
	} else {
		m_optPages.reset();

		return FPDF_ERR_SUCCESS;
	}
}

unsigned long  CPDFDoc::Create()
{
	m_path = L"";
	m_password = L"";
	IsDirty->set(true);

	m_optFormHandlePtr.reset();
	m_optPages.reset();
	
	m_pDoc->CreateNewDocument();
	unsigned long err = FPDF_GetLastError();
	if (!*m_pDoc) {
		return err;
	} else {
		return FPDF_ERR_SUCCESS;
	}
}

void CPDFDoc::CopyTextToClipboard(HWND hWnd, const std::tuple<int, int>& begin, const std::tuple<int, int>& end) const
{
	auto [page_begin_index, char_begin_index] = begin;
	auto [page_end_index, char_end_index] = end;

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
			if (CClipboard clipboard; clipboard.Open(hWnd) != 0) {
				clipboard.Empty();
				clipboard.SetData(CF_UNICODETEXT, hGlobal);
			}
		}

	}
}

void CPDFDoc::DeletePage(int page_index)
{
	m_pDoc->Page_Delete(page_index);

	m_optFileVersion.reset();
	m_optPageCount.reset();
	m_optSize.reset();
	m_optPageRects.reset();
	m_optFormHandlePtr.reset();
	m_optPages.reset();
	IsDirty->set(true);
}

void CPDFDoc::Save()
{
	//To release binding between FPDF_DOCUMENT and File, Copy to new FPDF_DOCUMENT and Swap
	std::unique_ptr<CFPDFDocument> pNewDoc(std::make_unique<CFPDFDocument>());
	pNewDoc->CreateNewDocument();
	pNewDoc->ImportPages(*m_pDoc, nullptr, 0);
	int version = GetFileVersion();

	m_pDoc = std::move(pNewDoc);

	SaveWithVersion(m_path, 0, version);
}

void CPDFDoc::SaveWithVersion(const std::wstring& path, FPDF_DWORD flags, int fileVersion)
{
	CFPDFFileWrite fileWrite(path);
	FPDF_BOOL b = m_pDoc->SaveWithVersion(&fileWrite, flags, fileVersion);

	m_optFileVersion.reset();
	m_optPageCount.reset();
	m_optSize.reset();
	m_optPageRects.reset();
	m_optFormHandlePtr.reset();
	m_optPages.reset();
	IsDirty->set(false);
}



void CPDFDoc::ImportPages(const CPDFDoc& src_doc,
				FPDF_BYTESTRING pagerange,
				int index)
{
	m_pDoc->ImportPages(*src_doc.m_pDoc, pagerange, index);
	m_optFileVersion.reset();
	m_optPageCount.reset();
	m_optSize.reset();
	m_optPageRects.reset();
	m_optFormHandlePtr.reset();
	m_optPages.reset();
	IsDirty->set(true);
}

void CPDFDoc::ImportPagesByIndex(const CPDFDoc& src_doc,
	const int* page_indices,
	unsigned long length,
	int index)
{
	m_pDoc->ImportPagesByIndex(*src_doc.m_pDoc, page_indices, length, index);
}

void CPDFDoc::SplitSave() const
{
	for (auto i = 0; i < GetPageCount(); i++) {
		wchar_t srcPathWoExt[MAX_PATH] = { 0 };
		wcscpy_s(srcPathWoExt, m_path.c_str());
		::PathRemoveExtensionW(srcPathWoExt);

		wchar_t dstPath[MAX_PATH] = { 0 };
		swprintf_s(dstPath, L"%s_%02d.pdf", srcPathWoExt, i + 1);

		CPDFDoc dstDoc;
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
		CPDFDoc srcDoc;
		srcDoc.Open(srcPath);
		auto count = GetPageCount();
		ImportPages(srcDoc, NULL, count);
	}
}

CPDFDoc CPDFDoc::Extract(const std::wstring& page_indices) const
{
	CPDFDoc dstDoc;
	dstDoc.Create();
	dstDoc.ImportPages(*this, wide_to_sjis(page_indices).c_str(), 0);
	return dstDoc;
}

CPDFDoc CPDFDoc::Clone() const
{
	CPDFDoc dstDoc;
	dstDoc.Create();
	dstDoc.ImportPages(*this, nullptr, 0);
	return dstDoc;
}


