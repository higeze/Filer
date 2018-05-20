#include "FileIconCell.h"
#include "MyDC.h"
#include "Column.h"
#include "FileRow.h"
#include "ShellFile.h"
#include "MySize.h"
#include "Sheet.h"
#include "GridView.h"
#include "ThreadPool.h"

extern std::unique_ptr<ThreadPool> g_pThreadPool;

CFileIconCell::CFileIconCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty){}

std::shared_ptr<CShellFile> CFileIconCell::GetShellFile()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer();
}

void CFileIconCell::PaintContent(CDC* pDC, CRect rcPaint)
{
//	auto spFile = GetShellFile();
//	CIcon icon = spFile->GetIcon(false);
//
//	if (icon) {
//		CRect rc = rcPaint;
//		rc.bottom = rc.top + 16;
//		rc.right = rc.left + 16;
//		pDC->DrawIconEx(icon, rc, 0, NULL, DI_NORMAL);
//	}
//	else {
//		g_pThreadPool->add([this,spFile]
//		{
//			if (spFile->GetIcon(true)) {
//				m_pSheet->GetGridPtr()->DeadLineTimerInvalidate();
////				m_pSheet->GetGridPtr()->Invalidate();
//			}
//		});
//	}

	auto spFile = GetShellFile();
	CRect rc = rcPaint;
	rc.bottom = rc.top + 16;
	rc.right = rc.left + 16;
	if (spFile->HasIcon() || spFile->HasIconInCache()) {
		pDC->DrawIconEx(*spFile->GetIcon(true), rc, 0, NULL, DI_NORMAL);
	} else {
		pDC->DrawIconEx(*spFile->GetDefaultIcon(), rc, 0, NULL, DI_NORMAL);
		g_pThreadPool->add([this,spFile]
		{
			if (spFile->GetIcon(true)) {
				m_pSheet->GetGridPtr()->DeadLineTimerInvalidate();
			}
		});
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
