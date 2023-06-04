#include "FPDFDocument.h"
#include "FPDFPage.h"

CFPDFPage CFPDFDocument::LoadPage(int page_index)
{
	FPDF_LOCK;
	return CFPDFPage(FPDF_LoadPage(m_p.get(), page_index));
}