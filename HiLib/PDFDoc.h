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
	//int m_pageCount;
	CSizeF m_sourceSize;
	std::vector<CRectF> m_sourceRectsInDoc;
	std::vector<std::unique_ptr<CPDFPage>> m_pages;

	LAZY_GETTER(int, PageCount)

public:
	CPDFDoc(
		const std::shared_ptr<PdfViewProperty>& spProp,
		std::function<void()> changed);
	virtual ~CPDFDoc();

	void Open(const std::wstring& path, const std::wstring& password);
	void Create();

	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr() { return m_pPDFium; }
	std::shared_ptr<PdfViewProperty> GetPropPtr() const { return m_pProp; }
	std::vector<CRectF>& GetPageRects() { return m_sourceRectsInDoc; }
	UNQ_FPDF_DOCUMENT& GetDocPtr() { return m_pDoc; }
	std::wstring GetPath() { return m_path; }

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
	void ImportPages(const CPDFDoc& src_doc,
					FPDF_BYTESTRING pagerange,
					int index);
	void SaveAsCopy(const std::wstring& path);

	void SplitSaveAsCopy();
	void Merge(const std::vector<std::wstring>& srcPathes);



};
