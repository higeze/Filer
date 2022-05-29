#pragma once

#include "Direct2DWrite.h"
#include "PDFiumSingleThread.h"
#include "PDFEvent.h"

#include "getter_macro.h"

struct PdfViewProperty;
class CD2DWWindow;
class CPDFViewport;
class CPDFPage;

class CPDFDoc
{
public:
	static void Init();
	static void Term();

private:
	//it is necessary to delare thread first. Constructor call first to last, Destructor call last to first.
	std::unique_ptr<CPDFiumSingleThread> m_pPDFium;
	std::shared_ptr<PdfViewProperty> m_pProp;
	CDirect2DWrite* m_pDirect;
	std::wstring m_path;
	std::wstring m_password;
	std::function<void()> m_changed;

	UNQ_FPDF_DOCUMENT m_pDoc;
	int m_pageCount;
	CSizeF m_sourceSize;
	std::vector<CRectF> m_sourceRectsInDoc;
	std::vector<std::unique_ptr<CPDFPage>> m_pages;

public:
	CPDFDoc(
		const std::shared_ptr<PdfViewProperty>& spProp,
		std::function<void()> changed);
	virtual ~CPDFDoc();

	void Open(const std::wstring& path, const std::wstring& password);

	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr() { return m_pPDFium; }
	std::shared_ptr<PdfViewProperty> GetPropPtr() const { return m_pProp; }
	std::vector<CRectF>& GetPageRects() { return m_sourceRectsInDoc; }
	UNQ_FPDF_DOCUMENT& GetDocPtr() { return m_pDoc; }
	std::wstring GetPath() { return m_path; }

	int GetPageCount() const { return m_pageCount; }
	CSizeF GetSourceSize() const { return m_sourceSize; }

	std::vector<std::unique_ptr<CPDFPage>>& GetPages() { return m_pages; }

	//bool IsPageAvalable(int index)
	//{
	//	return GetPageCount() && 0 <= index && index < GetPageCount();
	//}

	std::unique_ptr<CPDFPage>& GetPage(int index)
	{
		//try {
			FALSE_THROW(GetPageCount() && 0 <= index && index < GetPageCount());
		//}
		//catch (...) {
		//	auto a = 1;
		//}
		return m_pages[index];
	}

	void RenderContent(const RenderDocContentEvent& e);
	void RenderFind(const RenderDocFindEvent& e);
	void RenderFindLine(const RenderDocFindLineEvent& e);
	void RenderSelectedText(const RenderDocSelectedTextEvent& e);
	void RenderCaret(const RenderDocCaretEvent& e);

	void CopyTextToClipboard(const CopyDocTextEvent& e);
};