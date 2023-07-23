#include "FPDFDocument.h"
#include "FPDFPage.h"
#include "FPDFFormHandle.h"

CFPDFPage CFPDFDocument::LoadPage(int page_index)
{
	FPDF_LOCK;
	return CFPDFPage(FPDF_LoadPage(m_p.get(), page_index));
}

CFPDFFormHandle CFPDFDocument::InitFormFillEnvironment(FPDF_FORMFILLINFO* formInfo)
{
	FPDF_LOCK;
	return CFPDFFormHandle(FPDFDOC_InitFormFillEnvironment(m_p.get(), formInfo));
}