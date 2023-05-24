#include "UniqueFPdfDocument.h"
#include "UniqueFPdfPage.h"

CUniqueFPdfPage CUniqueFPdfDocument::LoadPage(int page_index)
{
	FPDF_LOCK;
	return CUniqueFPdfPage(FPDF_LoadPage(get(), page_index));
}