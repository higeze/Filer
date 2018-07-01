#include "FileIconCell.h"
#include "MyDC.h"
#include "Column.h"
#include "FileRow.h"
#include "ShellFile.h"
#include "MySize.h"
#include "Sheet.h"
#include "GridView.h"

CFileIconCell::CFileIconCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty){}

std::shared_ptr<CShellFile> CFileIconCell::GetShellFile()const
{
	if(auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)){
		//It is impossible to plymorphism in constructor, assign signal here.
		auto spFile =  pFileRow->GetFilePointer();
		if (spFile->SignalFileIconChanged.empty()) {
			auto spCell(shared_from_this());
			std::weak_ptr<const CFileIconCell> wpCell(spCell);
			spFile->SignalFileIconChanged.connect(
				[wpCell](std::weak_ptr<CShellFile> wpFile)->void {
				if (auto sp = wpCell.lock()) {
					sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
				}
			});
		}
		return spFile;
	} else {
		return nullptr;
	}
}

void CFileIconCell::PaintContent(CDC* pDC, CRect rcPaint)
{
	auto spFile = GetShellFile();
	CRect rc = rcPaint;
	rc.bottom = rc.top + 16;
	rc.right = rc.left + 16;

	pDC->DrawIconEx(*(spFile->GetIcon().first), rc, 0, NULL, DI_NORMAL);
}

CSize CFileIconCell::MeasureContentSize(CDC* pDC)
{
	return CSize(16, 16);
}

CSize CFileIconCell::MeasureContentSizeWithFixedWidth(CDC* pDC)
{
	return MeasureContentSize(pDC);
}
