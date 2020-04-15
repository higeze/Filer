#include "FileIconCell.h"
#include "Column.h"
#include "FileRow.h"
#include "ShellFile.h"
#include "MySize.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"
#include "FileIconCache.h"

CFileIconCell::CFileIconCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CCell(pSheet, pRow, pColumn, spProperty)
{}

CFileIconCell::~CFileIconCell() {}

std::shared_ptr<CShellFile> CFileIconCell::GetShellFile()
{
	if(auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)){
		return pFileRow->GetFilePointer();
	} else {
		return nullptr;
	}
}

void CFileIconCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	auto spFile = GetShellFile();
	if (spFile) {
		d2dw::CRectF rc = rcPaint;
		rc.bottom = rc.top + pDirect->Pixels2DipsY(16);
		rc.right = rc.left + pDirect->Pixels2DipsX(16);

		std::weak_ptr<CFileIconCell> wp(shared_from_this());
		std::function<void()> updated = [wp]()->void {
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
		};
		pDirect->DrawBitmap(pDirect->GetIconCachePtr()->GetFileIconBitmap(spFile->GetAbsoluteIdl(), spFile->GetPath(), spFile->GetExt(), updated), rc);
	}
}

d2dw::CSizeF CFileIconCell::MeasureContentSize(d2dw::CDirect2DWrite* pDirect)
{
	return d2dw::CSizeF(pDirect->Pixels2DipsX(16) , pDirect->Pixels2DipsY(16));
}

d2dw::CSizeF CFileIconCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)
{
	return MeasureContentSize(pDirect);
}
