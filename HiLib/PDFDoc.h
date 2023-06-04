#pragma once
#include "Direct2DWrite.h"
#include "getter_macro.h"
#include "FPdfDocument.h"

class CD2DWWindow;
class CPDFViewport;
class CPDFPage;

class CPDFDoc
{
public:
	static void Init();
	static void Term();

private:
	std::unique_ptr<CFPDFDocument> m_pDoc;
	std::wstring m_path;
	std::wstring m_password;
	FLOAT totalsize = 0;
	FLOAT pagespan = 10.f;

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
	int GetPageIndex(const std::unique_ptr<CPDFPage>& pPage);
	int GetPageIndex(const CPDFPage* pPage);
public:
	CPDFDoc();
	virtual ~CPDFDoc();
	CPDFDoc(CPDFDoc&& doc) = default;
	CPDFDoc& operator=(CPDFDoc&& doc) = default;

	unsigned long  Open(const std::wstring& path, const std::wstring& password = std::wstring());
	unsigned long  Create();


	std::wstring GetPath() const { return m_path; }
	bool GetIsDirty()const { return m_isDirty; }

	void CopyTextToClipboard(HWND hWnd, const std::tuple<int, int>& begin, const std::tuple<int, int>& end) const;
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
