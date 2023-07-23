#include "FPDFFormHandle.h"
#include "FPDFDocument.h"
#include "FPDFBitmap.h"
#include "FPDFPage.h"

void CFPDFFormHandle::InitFormFillEnvironment(const CFPDFDocument& document, const FPDF_FORMFILLINFO* formInfo)
{
	FPDF_LOCK;
	ExitFormFillEnvironment();
	m_p = FPDFDOC_InitFormFillEnvironment(document.m_p.get(), const_cast<FPDF_FORMFILLINFO*>(formInfo));
}


void CFPDFFormHandle::FFLDraw(
	CFPDFBitmap& bitmap,
	CFPDFPage& page,
	int start_x,
	int start_y,
	int size_x,
	int size_y,
	int rotate,
	int flags)
{
	FPDF_LOCK;
	FPDF_FFLDraw(m_p,
		bitmap.m_p.get(),
		page.m_p.get(),
		start_x,
		start_y,
		size_x,
		size_y,
		rotate,
		flags);
}