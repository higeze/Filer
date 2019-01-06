#include "FileIconCell.h"
#include "MyDC.h"
#include "Column.h"
#include "FileRow.h"
#include "ShellFile.h"
#include "MySize.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"

CFileIconCell::CFileIconCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty)
{
	//Be careful : It is impossible to plymorphism in constructor, assign signal here.
	//if (auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)) {
	//	auto spFile = pFileRow->GetFilePointer();
	//	if (auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)) {
	//		auto spFile = pFileRow->GetFilePointer();
	//		std::weak_ptr<const CFileIconCell> wp(shared_from_this());
	//		m_conIconChanged = spFile->SignalFileIconChanged.connect(
	//			[wp](std::weak_ptr<CShellFile> wpFile)->void {
	//			if (auto sp = wp.lock()) {
	//				sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
	//			}
	//		});
	//	}
	//}
}

CFileIconCell::~CFileIconCell() 
{
	m_conIconChanged.disconnect();
}

std::shared_ptr<CShellFile> CFileIconCell::GetShellFile()
{
	if(auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)){
		auto spFile = pFileRow->GetFilePointer();
		if (!m_conIconChanged.connected()) {
			std::weak_ptr<CFileIconCell> wp(shared_from_this());
			m_conIconChanged = spFile->SignalFileIconChanged.connect(
				[wp](CShellFile* pFile)->void {
				if (auto sp = wp.lock()) {
					auto con = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
						[wp]()->void {
						if (auto sp = wp.lock()) {
							sp->OnPropertyChanged(L"value");
						}
					});
					sp->m_conDelayUpdateAction = con;
					sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
				}
			});

				//m_conIconChanged = spFile->SignalFileIconChanged.connect(

				//	[wp](CShellFile* pFile)->void {
				//	if (auto sp = wp.lock()) {
				//		sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
				//	}
				//});
		}
		return spFile;
	} else {
		return nullptr;
	}
}

void CFileIconCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	//TODOTODO
	//auto spFile = GetShellFile();
	//CRect rc = rcPaint;
	//rc.bottom = rc.top + 16;
	//rc.right = rc.left + 16;

	//pDC->DrawIconEx(*(spFile->GetIcon().first), rc, 0, NULL, DI_NORMAL);
}

d2dw::CSizeF CFileIconCell::MeasureContentSize(d2dw::CDirect2DWrite& direct)
{
	return d2dw::CSizeF(16.0f, 16.0f);
}

d2dw::CSizeF CFileIconCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	return MeasureContentSize(direct);
}
