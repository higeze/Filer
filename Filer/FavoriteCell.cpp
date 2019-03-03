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


CFavoriteCell::CFavoriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CFileIconCell(pSheet, pRow, pColumn, spProperty){}

std::shared_ptr<CShellFile> CFavoriteCell::GetShellFile()
{
	auto pRow = static_cast<CFavoriteRow*>(m_pRow);
	auto pCol = static_cast<CFavoritesColumn*>(m_pColumn);
	auto order = pRow->GetOrderIndex();
	if (!pCol->GetFavorites()->at(order)->GetShellFile()) {
		//if (!::PathFileExists(pCol->GetFavorites()->at(order)->GetPath().c_str())) {
		//	pCol->GetFavorites()->at(order)->SetShellFile(std::make_shared<CShellInvalidFile>());
		//}
		//else {
			pCol->GetFavorites()->at(order)->SetShellFile(CShellFolder::CreateShExFileFolder(pCol->GetFavorites()->at(order)->GetPath()));
		//}
	}
	auto spFile = pCol->GetFavorites()->at(order)->GetShellFile();
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

	return pCol->GetFavorites()->at(order)->GetShellFile();
}

std::wstring CFavoriteCell::GetShortName()
{
	auto pRow = static_cast<CFavoriteRow*>(m_pRow);
	auto pCol = static_cast<CFavoritesColumn*>(m_pColumn);
	return pCol->GetFavorites()->at(pRow->GetOrderIndex())->GetShortName();
}


void CFavoriteCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	//Paint Icon
	CFileIconCell::PaintContent(direct, rcPaint);

	//Paint Text
	direct.DrawTextInRect(*(m_spProperty->Format) , GetShortName(), rcPaint);

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
	menu.InsertMenuItem(0, TRUE, &mii);
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


		//auto pPropWnd = new CPropertyWnd<CFavorite>(
		//	m_pSheet->GetGridPtr()->GetGridViewPropPtr(),
		//	L"Favorite",
		//	pCol->GetFavorites()->at(order));

		//pPropWnd->PropertyChanged.connect([](const std::wstring& prop)->void {
		//});

		//CRect rc(0, 0, 0, 0);
		//pPropWnd->Create(m_pSheet->GetGridPtr()->m_hWnd, rc);
		//rc = pPropWnd->GetGridView()->GetDirect()->Dips2Pixels(pPropWnd->GetGridView()->GetRect());
		//::AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, 0);
		//pPropWnd->MoveWindow(0, 0, rc.Width() + ::GetSystemMetrics(SM_CXVSCROLL), min(500, rc.Height() + ::GetSystemMetrics(SM_CYVSCROLL) + 10), FALSE);
		//pPropWnd->CenterWindow();
		//pPropWnd->ShowWindow(SW_SHOW);
		//pPropWnd->UpdateWindow();
	}
}
