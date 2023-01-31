#pragma once

#include "Direct2DWrite.h"
#include "PDFiumSingleThread.h"
#include "PDFEvent.h"
#include "getter_macro.h"

class CD2DWWindow;
class CPDFViewport;
class CPDFPage;
class CPDFPageCache;

class CPDFDoc
{
public:
	FLOAT totalsize = 0;
	static void Init();
	static void Term();

private:
	//it is necessary to delare thread first. Constructor call first to last, Destructor call last to first.
	std::unique_ptr<CPDFiumSingleThread> m_pPDFium;
	CDirect2DWrite* m_pDirect;
	std::wstring m_path;
	std::wstring m_password;
	bool m_isDirty;

	UNQ_FPDF_DOCUMENT m_pDoc;
	std::vector<std::unique_ptr<CPDFPage>> m_pages;

	DECLARE_LAZY_GETTER(int, FileVersion);
	DECLARE_LAZY_GETTER(int, PageCount);
	DECLARE_LAZY_GETTER(CSizeF, Size);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, PageRects);
	

public:
	CPDFDoc();
	virtual ~CPDFDoc();
	CPDFDoc(CPDFDoc&& doc) = default;
	CPDFDoc& operator=(CPDFDoc&& doc) = default;

	void Open(const std::wstring& path, const std::wstring& password = std::wstring());
	void Create();

	std::unique_ptr<CPDFiumSingleThread>& GetPDFiumPtr() { return m_pPDFium; }
	UNQ_FPDF_DOCUMENT& GetDocPtr() { return m_pDoc; }
	std::wstring GetPath() { return m_path; }
	bool GetIsDirty()const { return m_isDirty; }

	std::vector<std::unique_ptr<CPDFPage>>& GetPages() { return m_pages; }

	std::unique_ptr<CPDFPage>& GetPage(int index)
	{
		FALSE_THROW(GetPageCount() && 0 <= index && index < GetPageCount());
		return m_pages[index];
	}


	void CopyTextToClipboard(const CopyDocTextEvent& e);
	void ImportPages(const CPDFDoc& src_doc,
					FPDF_BYTESTRING pagerange,
					int index);
	void Save();
	void SaveWithVersion(const std::wstring& path, FPDF_DWORD flags, int fileVersion);

	void SplitSave();
	void Merge(const std::vector<CPDFDoc>& srcDocs);
	void Merge(const std::vector<std::wstring>& srcPathes);
	CPDFDoc Extract(const std::wstring& page_indices);
	CPDFDoc Clone();



};
