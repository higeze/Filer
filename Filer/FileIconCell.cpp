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
{}

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
		}
		return spFile;
	} else {
		return nullptr;
	}
}

void CFileIconCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	auto spFile = GetShellFile();
	d2dw::CRectF rc = rcPaint;
	rc.bottom = rc.top + direct.Pixels2DipsY(16);
	rc.right = rc.left + direct.Pixels2DipsX(16);
	
	try {
		direct.DrawIcon(spFile->GetIcon().first->operator HICON(), rc);
	}catch(...){}
	//pDC->DrawIconEx(*(spFile->GetIcon().first), rc, 0, NULL, DI_NORMAL);
}

d2dw::CSizeF CFileIconCell::MeasureContentSize(d2dw::CDirect2DWrite& direct)
{
	return d2dw::CSizeF(direct.Pixels2DipsX(16) , direct.Pixels2DipsY(16));
}

d2dw::CSizeF CFileIconCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	return MeasureContentSize(direct);
}
