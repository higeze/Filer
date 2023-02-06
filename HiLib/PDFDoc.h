#pragma once
#include "Direct2DWrite.h"
#include "PDFiumMultiThread.h"
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
	FLOAT pagespan = 10.f;
	static void Init();
	static void Term();

private:
	//it is necessary to delare thread first. Constructor call first to last, Destructor call last to first.
	std::unique_ptr<CPDFiumMultiThread> m_pPDFium;
	CDirect2DWrite* m_pDirect;
	std::wstring m_path;
	std::wstring m_password;
	mutable bool m_isDirty;

	//std::vector<std::unique_ptr<CPDFPage>> m_pages;

	DECLARE_LAZY_GETTER(int, FileVersion);
	DECLARE_LAZY_GETTER(int, PageCount);
	DECLARE_LAZY_GETTER(CSizeF, Size);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, PageRects);

protected:
	mutable std::optional<std::vector<std::unique_ptr<CPDFPage>>> m_optPages;
public:
	const std::unique_ptr<CPDFPage>& GetPage(const int& index) const;
	std::unique_ptr<CPDFPage>& GetPage(const int& index);
	
public:
	CPDFDoc(size_t threads = 3);
	virtual ~CPDFDoc();
	CPDFDoc(CPDFDoc&& doc) = default;
	CPDFDoc& operator=(CPDFDoc&& doc) = default;

	unsigned long  Open(const std::wstring& path, const std::wstring& password = std::wstring());
	unsigned long  Create();

	const std::unique_ptr<CPDFiumMultiThread>& GetPDFiumPtr() const { return m_pPDFium; }

	std::wstring GetPath() const { return m_path; }
	bool GetIsDirty()const { return m_isDirty; }

	//std::unique_ptr<CPDFPage>& GetPage(int index)
	//{
	//	FALSE_THROW(GetPageCount() && 0 <= index && index < GetPageCount());
	//	return m_pages[index];
	//}
	//const std::unique_ptr<CPDFPage>& GetPage(int index) const
	//{
	//	FALSE_THROW(GetPageCount() && 0 <= index && index < GetPageCount());
	//	return m_pages[index];
	//}

	void CopyTextToClipboard(const CopyDocTextEvent& e) const;
	void ImportPages(const CPDFDoc& src_doc,
					FPDF_BYTESTRING pagerange,
					int index);
	void ImportPagesByIndex(const CPDFDoc& src_doc,
		const int* page_indices,
		unsigned long length,
		int index);
	void Save();
	void SaveWithVersion(const std::wstring& path, FPDF_DWORD flags, int fileVersion);

	void SplitSave() const;
	void Merge(const std::vector<CPDFDoc>& srcDocs);
	void Merge(const std::vector<std::wstring>& srcPathes);
	CPDFDoc Extract(const std::wstring& page_indices) const;
	CPDFDoc Clone() const;



};
