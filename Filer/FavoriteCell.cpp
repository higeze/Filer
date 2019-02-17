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
	direct.DrawTextInRect(*(m_spProperty->FontAndColor) , GetShortName(), rcPaint);

	//Find font size
	//std::wstring str = GetShortName();
	//d2dw::CFontF font = m_spProperty->FontAndColor->Font;
	//d2dw::CSizeF size = direct.CalcTextSize(font, str);

	//while (size.width > direct.Pixels2DipsX(16)){
	//	font.Size -= 1.0f;
	//	size = direct.CalcTextSize(font, str);
	//}
	//font.TextAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER;
	//font.ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_FAR;
	//rcPaint.right = rcPaint.left + direct.Pixels2DipsX(16);
	//rcPaint.bottom = rcPaint.top + direct.Pixels2DipsY(16);

	//direct.DrawTextLayout(d2dw::FontAndColor(font, m_spProperty->FontAndColor->Color), str, rcPaint);
}