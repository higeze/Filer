#include "FavoriteCell.h"
#include "MyDC.h"
#include "Column.h"
#include "FileRow.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "MySize.h"
#include "MyFont.h"
#include "CellProperty.h"
#include "Favorite.h"
#include "FavoriteRow.h"
#include "FavoritesColumn.h"
#include "ShellFile.h"
#include "KnownFolder.h"
#include "GridView.h"

CFavoriteCell::CFavoriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CFileIconCell(pSheet, pRow, pColumn, spProperty){}

std::shared_ptr<CShellFile> CFavoriteCell::GetShellFile()
{
	auto pRow = static_cast<CFavoriteRow*>(m_pRow);
	auto pCol = static_cast<CFavoritesColumn*>(m_pColumn);
	auto order = pRow->GetOrderIndex();
	if (!pCol->GetFavorites()->at(order).GetShellFile()) {
		if (pCol->GetFavorites()->at(order).GetPath().empty()) {
			pCol->GetFavorites()->at(order).SetShellFile(CKnownFolderManager::GetInstance()->GetDesktopFolder());
		}
		else {
			pCol->GetFavorites()->at(order).SetShellFile(CShellFolder::CreateShExFileFolder(pCol->GetFavorites()->at(order).GetPath()));
		}
	}
	auto spFile = pCol->GetFavorites()->at(order).GetShellFile();
	if (!m_conIconChanged.connected()) {
		std::weak_ptr<CFavoriteCell> wp(std::static_pointer_cast<CFavoriteCell>(shared_from_this()));
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

	return pCol->GetFavorites()->at(order).GetShellFile();
}

std::wstring CFavoriteCell::GetShortName()
{
	auto pRow = static_cast<CFavoriteRow*>(m_pRow);
	auto pCol = static_cast<CFavoritesColumn*>(m_pColumn);
	return pCol->GetFavorites()->at(pRow->GetOrderIndex()).GetShortName();
}


void CFavoriteCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	//Paint Icon
	CFileIconCell::PaintContent(direct, rcPaint);

	//Paint Text
	//TODOTODO
	//pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

	////Find font size
	//CFont font;
	//HFONT hFont = NULL;
	//CRect rcContent;
	//std::wstring str = GetShortName();
	//int i = 0;
	//do {
	//	font = CFont(m_spProperty->GetFontPtr()->GetPointSize() - i, m_spProperty->GetFontPtr()->GetFaceName());
	//	hFont = (HFONT)pDC->SelectFont(font);
	//	pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()), str.size(), rcContent,
	//		DT_CALCRECT | GetFormat()&~DT_WORDBREAK, NULL);
	//	pDC->SelectFont(hFont);
	//	i++;
	//} while (rcContent.Width()>16);
	//hFont = (HFONT)pDC->SelectFont(font);
	////rcPaint.top = rcPaint.bottom - rcContent.Height();
	//pDC->DrawTextEx(const_cast<LPTSTR>(str.c_str()), -1, rcPaint, GetFormat(), NULL);
	//pDC->SelectFont(hFont);
}