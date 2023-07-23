#pragma once
#include "FPDF.h"
#include "D2DWTypes.h"

class CFPDFDocument;
class CFPDFPage;
class CFPDFBitmap;

class CFPDFFormHandle
{
	friend class CFPDFPage;
private:
	//typedef void  (__stdcall *FPDFDOC_ExitFormFillEnvironment)  (FPDF_FORMHANDLE hHandle);
	//struct delete_fpdf_formhandle
	//{
	//	void operator()(FPDF_FORMHANDLE p)
	//	{ 
	//		if(p){
	//			FPDF_LOCK;
	//			FPDFDOC_ExitFormFillEnvironment(p);
	//		}
	//	}
	//};
	//std::unique_ptr<std::remove_pointer_t<FPDF_FORMHANDLE>, delete_fpdf_formhandle> m_p;
	FPDF_FORMHANDLE m_p;
public:
	explicit CFPDFFormHandle(FPDF_FORMHANDLE p = nullptr)
		:m_p(p) {}
	CFPDFFormHandle(const CFPDFFormHandle&) = delete;
	CFPDFFormHandle& operator = (const CFPDFFormHandle&) = delete ;
	CFPDFFormHandle(CFPDFFormHandle&& rhs) = default;
	CFPDFFormHandle& operator = (CFPDFFormHandle&& rhs) = default;
	~CFPDFFormHandle()
	{
		ExitFormFillEnvironment();
	}

	operator bool() const { return m_p; }

	void InitFormFillEnvironment(const CFPDFDocument& document, const FPDF_FORMFILLINFO* formInfo);

	void ExitFormFillEnvironment()
	{
		FPDF_LOCK;
		if (m_p) {
			FPDFDOC_ExitFormFillEnvironment(m_p);
			m_p = nullptr;
		}
	}

	void SetFormFieldHighlightColor(int fieldType, unsigned long color)
	{
		FPDF_LOCK;
		FPDF_SetFormFieldHighlightColor(m_p, fieldType, color);
	}

	void SetFormFieldHighlightAlpha(unsigned char alpha)
	{
		FPDF_LOCK;
		FPDF_SetFormFieldHighlightAlpha(m_p, alpha);
	}

	void DoDocumentJSAction()
	{
		FPDF_LOCK;
		FORM_DoDocumentJSAction(m_p);
	}

	void DoDocumentOpenAction()
	{
		FPDF_LOCK;
		FORM_DoDocumentOpenAction(m_p);
	}

	void DoDocumentAAction(int aaType)
	{
		FPDF_LOCK;
		FORM_DoDocumentAAction(m_p, aaType);
	}

	void FFLDraw(
		CFPDFBitmap& bitmap,
		CFPDFPage& page,
		int start_x,
		int start_y,
		int size_x,
		int size_y,
		int rotate,
		int flags);

};
