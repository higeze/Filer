#include "LauncherCell.h"
#include "LauncherRow.h"
#include "BindColumn.h"
#include "LauncherGridView.h"
#include "FilerWnd.h"
#include "FilerApplication.h"
#include "Launcher.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "CellProperty.h"
#include "ResourceIDFactory.h"

std::wstring CLauncherCell::GetShortName()
{
	if (auto pBindColumn = dynamic_cast<CBindColumn*>(this->m_pColumn)) {
		return pBindColumn->GetItem<std::shared_ptr<CLauncher>>()->GetShortName();
	} else {
		return nullptr;
	}
}


std::shared_ptr<CShellFile> CLauncherCell::GetShellFile()
{
	if (auto pBindColumn = dynamic_cast<CBindColumn*>(this->m_pColumn)) {
		return pBindColumn->GetItem<std::shared_ptr<CLauncher>>()->GetShellFile(
			[pSheet = this->m_pSheet]() {
				pSheet->GetGridPtr()->DelayUpdate();
			});
	} else {
		return nullptr;
	}
}

void CLauncherCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	//Paint Icon
	base::PaintContent(pDirect, rcPaint);

	//Paint Text
	pDirect->DrawTextInRect(*(this->m_spCellProperty->Format), GetShortName(), this->Content2InnerBorder(rcPaint));
}

void CLauncherCell::OnLButtonDblClk(const LButtonDblClkEvent& e)
{
	if (auto pBindColumn = dynamic_cast<CBindColumn*>(this->m_pColumn)) {
		auto pItem = pBindColumn->GetItem<std::shared_ptr<CLauncher>>();
		if (pItem->RunAs) {
			this->GetShellFile()->RunAs();
		} else {
			this->GetShellFile()->Open();
		}
	}
	(*e.HandledPtr) = true;
}

void CLauncherCell::OnContextMenu(const ContextMenuEvent& e)
{
	CMenu menu(::CreatePopupMenu());
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_STRING;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"EditLauncher");
	mii.dwTypeData = const_cast<LPWSTR>(L"Edit");
	mii.cch = 4;
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);

	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeleteLauncher");
	mii.dwTypeData = const_cast<LPWSTR>(L"Delete");
	mii.cch = 6;
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);


	::SetForegroundWindow(this->m_pSheet->GetWndPtr()->m_hWnd);
	WORD retID = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, e.PointInScreen.x, e.PointInScreen.y, this->m_pSheet->GetWndPtr()->m_hWnd);
	if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"EditLauncher")) {
		//auto pFilerWnd = static_cast<CFilerWnd*>(this->m_pSheet->GetWndPtr());

		//auto pRow = static_cast<CBindRow*>(this->m_pRow);
		//auto pCol = static_cast<const CLaunchersColumn*>(this->m_pColumn);
		//auto& itemsSource = static_cast<CLaunchersGridView*>(this->m_pSheet)->GetItemsSource();
		//auto order = pRow->GetIndex<AllTag>();

		//TODOLOW
		//pFilerWnd->OnCommandOption<CLauncher>(L"Favorite", std::get<std::shared_ptr<CLauncher>>(itemsSource[order]),
		//									  [pFilerWnd](const std::wstring& prop)->void {
		//										  pFilerWnd->GetLeftFavoritesView()->Reload();
		//										  pFilerWnd->GetRightFavoritesView()->Reload();
		//										  pFilerWnd->InvalidateRect(NULL, FALSE);
		//										  ::SerializeProperty(pFilerWnd);
		//									  });
	} else if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeleteLauncher")) {
		auto pFilerWnd = static_cast<CFilerWnd*>(this->m_pSheet->GetWndPtr());

		//auto pRow = static_cast<const CLauncherRow*>(this->m_pRow);
		auto pCol = static_cast<const CBindColumn*>(this->m_pColumn);
		auto pGrid = static_cast<CLauncherGridView*>(this->m_pSheet);
		auto& itemsSource = pGrid->ItemsSource;
		auto index = pCol->GetIndex<AllTag>();

		itemsSource.erase(std::next(itemsSource->cbegin(), index));
		pGrid->Reload();
		CFilerApplication::GetInstance()->SerializeLauncher(pFilerWnd->GetLauncherPropPtr());
	}

}
