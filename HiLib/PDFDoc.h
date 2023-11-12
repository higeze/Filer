#pragma once
#include "Direct2DWrite.h"
#include "getter_macro.h"
#include "reactive_property.h"
#include "JsonSerializer.h"
#include "FPdfDocument.h"

class CD2DWWindow;
class CPDFViewport;
class CPDFPage;

class CPDFDoc
{
public:
	static void Init();
	static void Term();
public:
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<std::wstring> Path;
	reactive_property_ptr<std::wstring> Password;
	mutable reactive_property_ptr<bool> IsDirty;

private:
	std::unique_ptr<CFPDFDocument> m_pDoc;

	IPDF_JSPLATFORM m_jsPlatForm;
	FPDF_FORMFILLINFO m_formFillInfo;

	FLOAT totalsize = 0;
	FLOAT pagespan = 10.f;

	DECLARE_LAZY_GETTER(std::shared_ptr<CFPDFFormHandle>, FormHandlePtr);
	DECLARE_LAZY_GETTER(std::vector<std::unique_ptr<CPDFPage>>, Pages);
public: const std::unique_ptr<CPDFPage>& GetPage(const int& index) const;
	DECLARE_LAZY_GETTER(int, FileVersion);
	DECLARE_LAZY_GETTER(int, PageCount);
	DECLARE_LAZY_GETTER(CSizeF, Size);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, PageRects);

public:
	bool IsOpen() const
	{
		return m_pDoc && m_pDoc->operator bool();
	}
	int GetPageIndex(const std::unique_ptr<CPDFPage>& pPage) const;
	int GetPageIndex(const CPDFPage* pPage) const;
public:
	CPDFDoc();
	virtual ~CPDFDoc();
	CPDFDoc(const CPDFDoc& doc);
	CPDFDoc& operator=(const CPDFDoc& doc);
	CPDFDoc(CPDFDoc&& doc) = default;
	CPDFDoc& operator=(CPDFDoc&& doc) = default;
	bool operator==(const CPDFDoc& doc) const;
	explicit operator bool() const noexcept
	{
		return m_pDoc.operator bool() && m_pDoc->operator bool();
	}

	void InitializeFormFillInfo();

	unsigned long  Open(const std::wstring& path, const std::wstring& password = std::wstring());
	void Close();
	unsigned long  Create();

	void CopyTextToClipboard(HWND hWnd, const std::tuple<int, int>& begin, const std::tuple<int, int>& end) const;
	void DeletePage(int page_index);
	void ImportPages(const CPDFDoc& src_doc,
					FPDF_BYTESTRING pagerange,
					int index);
	void ImportPagesByIndex(const CPDFDoc& src_doc,
		const int* page_indices,
		unsigned long length,
		int index);
	void Save();
	void SaveAs(const std::wstring& path, int fileVersion, bool removeSecurity);


	void SplitSave() const;
	void Merge(const std::vector<CPDFDoc>& srcDocs);
	void Merge(const std::vector<std::wstring>& srcPathes);
	CPDFDoc Extract(const std::wstring& page_indices) const;
	CPDFDoc Clone() const;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
		CPDFDoc,
		Path,
		Password)
};
