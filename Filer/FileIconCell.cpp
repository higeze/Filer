#include "FileIconCell.h"
#include "MyDC.h"
#include "Column.h"
#include "FileRow.h"
#include "ShellFile.h"
#include "MySize.h"

CFileIconCell::CFileIconCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty){}

void CFileIconCell::PaintContent(CDC* pDC, CRect rcPaint)
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	auto spFile = pFileRow->GetFilePointer();
	CIcon icon = spFile->GetIcon();

	if(!icon.IsNull()){
		CRect rc = rcPaint;
		rc.bottom = rc.top + 16;
		rc.right = rc.left + 16;
		pDC->DrawIconEx(icon,rc,0,NULL,DI_NORMAL);
	}
}

CSize CFileIconCell::MeasureContentSize(CDC* pDC)
{
	return CSize(16, 16);
}

CSize CFileIconCell::MeasureContentSizeWithFixedWidth(CDC* pDC)
{
	return MeasureContentSize(pDC);
}
