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
#include "ResourceIDFactory.h"
#include "FilerWnd.h"
#include "FavoritesGridView.h"
#include "PropertyWnd.h"
#include "Filer.h"
#include "ShellFileFactory.h"


CFavoriteCell::CFavoriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CFileIconCell(pSheet, pRow, pColumn, spProperty){}

std::shared_ptr<CShellFile> CFavoriteCell::GetShellFile()
{
	auto pRow = static_cast<CFavoriteRow*>(m_pRow);
	auto pCol = static_cast<CFavoritesColumn*>(m_pColumn);
	auto order = pRow->GetOrderIndex();
	if (!pCol->GetFavorites()->at(order)->GetShellFile()) {
		pCol->GetFavorites()->at(order)->SetShellFile(CShellFileFactory::GetInstance()->CreateShellFilePtr(pCol->GetFavorites()->at(order)->GetPath()));
	}
	return pCol->GetFavorites()->at(order)->GetShellFile();
}

std::wstring CFavoriteCell::GetShortName()
{
	auto pRow = static_cast<CFavoriteRow*>(m_pRow);
	auto pCol = static_cast<CFavoritesColumn*>(m_pColumn);
	return pCol->GetFavorites()->at(pRow->GetOrderIndex())->GetShortName();
}


void CFavoriteCell::PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)
{
	//Paint Icon
	CFileIconCell::PaintContent(pDirect, rcPaint);

	//Paint Text
	pDirect->DrawTextInRect(*(m_spCellProperty->Format) , GetShortName(), Content2InnerBorder(rcPaint));
}

void CFavoriteCell::OnContextMenu(const ContextMenuEvent& e)
{
	CMenu menu(::CreatePopupMenu());
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID  | MIIM_STRING;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"EditFavorite");
	mii.dwTypeData = L"Edit";
	mii.cch = 4;
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);
	
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeleteFavorite");
	mii.dwTypeData = L"Delete";
	mii.cch = 6;
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	
	CPoint ptScreen(e.Point);
	::ClientToScreen(m_pSheet->GetGridPtr()->m_hWnd, &ptScreen);
	::SetForegroundWindow(m_pSheet->GetGridPtr()->m_hWnd);
	WORD retID = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, m_pSheet->GetGridPtr()->m_hWnd);
	if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"EditFavorite")) {

		CFilerWnd* pFilerWnd = static_cast<CFavoritesGridView*>(m_pSheet)->GetFilerWndPtr();

		auto pRow = static_cast<CFavoriteRow*>(m_pRow);
		auto pCol = static_cast<CFavoritesColumn*>(m_pColumn);
		auto order = pRow->GetOrderIndex();

		pFilerWnd->OnCommandOption<CFavorite>(L"Favorite", pCol->GetFavorites()->at(order),
			[pFilerWnd](const std::wstring& prop)->void {
			pFilerWnd->GetLeftFavoritesView()->Reload();
			pFilerWnd->GetRightFavoritesView()->Reload();
			pFilerWnd->InvalidateRect(NULL, FALSE);
			::SerializeProperty(pFilerWnd);
		});
	} else if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeleteFavorite")) {
		CFilerWnd* pFilerWnd = static_cast<CFavoritesGridView*>(m_pSheet)->GetFilerWndPtr();

		auto pRow = static_cast<CFavoriteRow*>(m_pRow);
		auto pCol = static_cast<CFavoritesColumn*>(m_pColumn);
		auto order = pRow->GetOrderIndex();

		pCol->GetFavorites()->erase(std::next(pCol->GetFavorites()->cbegin(), order));
		pFilerWnd->GetLeftFavoritesView()->Reload();
		pFilerWnd->GetRightFavoritesView()->Reload();
		pFilerWnd->InvalidateRect(NULL, FALSE);
		::SerializeProperty(pFilerWnd);
	}

}
