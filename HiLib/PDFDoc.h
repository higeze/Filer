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
	std::unique_ptr<CPDFiumSingleThread> m_pPDFium;

public:
	CPDFDoc(
		const std::shared_ptr<PdfViewProperty>& spProp,
		const std::wstring& path,
		const std::wstring& password, 
		CDirect2DWrite* pDirect,
		std::function<void()> changed);
	virtual ~CPDFDoc();

	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr() { return m_pPDFium; }
	CDirect2DWrite* GetDirectPtr() { return m_pDirect; }
	std::shared_ptr<PdfViewProperty> GetPropPtr() const { return m_pProp; }
	std::vector<CRectF>& GetPageRects() { return m_sourceRectsInDoc; }
	UNQ_FPDF_DOCUMENT& GetDocPtr() { return m_pDoc; }
	int GetPageCount() const { return m_pageCount; }
	CSizeF GetSourceSize() const { return m_sourceSize; }

	std::vector<std::unique_ptr<CPDFPage>>& GetPages() { return m_pages; }

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
