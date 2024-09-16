#pragma once
#include "Direct2DWrite.h"
#include "getter_macro.h"
#include "reactive_property.h"
#include "JsonSerializer.h"
#include "ShellFile.h"
#include "FPDFDocument.h"
#include "FPDFFormHandle.h"
#include "PDFPage.h"

class CD2DWWindow;
class CPDFViewport;

class CPDFDoc: public CShellFile
{
/**********/
/* static */
/**********/
public:
	static void Init();
	static void Term();

/*********************/
/* reactive_property */
/*********************/
public:
	reactive_property_ptr<std::wstring> Path;
	reactive_property_ptr<std::wstring> Password;
	mutable reactive_property_ptr<bool> IsDirty;

/*******************/
/* member variable */
/*******************/
private:
	std::shared_ptr<int> Dummy;

	std::unique_ptr<CFPDFDocument> m_pDoc;
	IPDF_JSPLATFORM m_jsPlatForm;
	FPDF_FORMFILLINFO m_formFillInfo;

	FLOAT totalsize = 0;
	FLOAT pagespan = 10.f;

/***************/
/* lazy getter */
/***************/
public: const std::unique_ptr<CPDFPage>& GetPage(const int& index) const;
	DECLARE_LAZY_GETTER_PRIVATE(std::vector<std::unique_ptr<CPDFPage>>, Pages);
	DECLARE_LAZY_GETTER(std::unique_ptr<CFPDFFormHandle>, FormHandle);
	DECLARE_LAZY_GETTER(int, FileVersion);
	DECLARE_LAZY_GETTER(int, PageCount);
	DECLARE_LAZY_GETTER(CSizeF, Size);
	DECLARE_LAZY_GETTER(std::vector<CRectF>, PageRects);

	DECLARE_RESET_OPTS(Pages, FormHandle, FileVersion, PageCount, Size, PageRects)


public:
	const std::unique_ptr<CFPDFDocument>& GetFPDFDocPtr() const { return  m_pDoc; }
	bool IsOpen() const;
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
	explicit operator bool() const noexcept;

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

/********/
/* json */
/********/
public:
	friend void to_json(json& j, const CPDFDoc& o)
	{
		json_safe_to(j, "Path", o.Path);
		json_safe_to(j, "Password", o.Password);
	}

	friend void from_json(const json& j, CPDFDoc& o)
	{
		json_safe_from(j, "Path", o.Path);
		json_safe_from(j, "Password", o.Password);
		o.Load(*o.Path);
	}
};
