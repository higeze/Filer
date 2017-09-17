#include "FileIconStringCell.h"
#include "MyDC.h"
#include "Column.h"
#include "FileRow.h"
#include "ShellFile.h"
#include "MySize.h"
#include "MyFont.h"
#include "CellProperty.h"
#include "FavoriteRow.h"

CFileIconStringCell::CFileIconStringCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty){}

void CFileIconStringCell::PaintContent(CDC* pDC, CRect rcPaint)
{
	auto pFileRow = static_cast<CFavoriteRow*>(m_pRow);
	auto spFile = pFileRow->GetFilePointer();
	CIcon icon = spFile->GetIcon();

	if(!icon.IsNull()){
		CRect rc = rcPaint;
		rc.bottom = rc.top + 16;
		rc.right = rc.left + 16;
		pDC->DrawIconEx(icon,rc,0,NULL,DI_NORMAL);
	}

	pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

	//Find font size
	CFont font;
	HFONT hFont = NULL;
	CRect rcContent;
	std::wstring str = pFileRow->GetName();
	int i=0;
	do{
		font = CFont(m_spProperty->GetFontPtr()->GetPointSize() - i, m_spProperty->GetFontPtr()->GetFaceName());
		hFont=(HFONT)pDC->SelectFont(font);
		pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()),str.size(),rcContent,
		DT_CALCRECT|GetFormat()&~DT_WORDBREAK,NULL);
		pDC->SelectFont(hFont);
		i++;
	}while(rcContent.Width()>16);
	hFont = (HFONT)pDC->SelectFont(font);
	//rcPaint.top = rcPaint.bottom - rcContent.Height();
	pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()),-1,rcPaint,GetFormat(),NULL);		
	pDC->SelectFont(hFont);
}

CSize CFileIconStringCell::MeasureContentSize(CDC* pDC)
{
	return CSize(16, 16);
}

CSize CFileIconStringCell::MeasureContentSizeWithFixedWidth(CDC* pDC)
{
	return MeasureContentSize(pDC);
}
