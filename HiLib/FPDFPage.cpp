#include "FPdfPage.h"
#include "FPdfBitmap.h"
#include "FpdfTextPage.h"
#include "FPDFFormHandle.h"
#include "Debug.h"

CFPDFTextPage CFPDFPage::LoadTextPage() const
{
	FPDF_LOCK;
	return CFPDFTextPage(FPDFText_LoadPage(m_p.get()));
}

void CFPDFPage::OnAfterLoadPage(const CFPDFFormHandle& form)
{
	FPDF_LOCK;
	FORM_OnAfterLoadPage(m_p.get(), form.m_p);
}

void CFPDFPage::OnBeforeClosePage(const CFPDFFormHandle& form)
{
	FPDF_LOCK;
	FORM_OnBeforeClosePage(m_p.get(), form.m_p);
}

void CFPDFPage::DoPageAAction(const CFPDFFormHandle& form, int aaType)
{
	FPDF_LOCK;
	FORM_DoPageAAction(m_p.get(), form.m_p, aaType);
}