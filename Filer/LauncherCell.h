#pragma once
#include "FileIconCell.h"
#include "LauncherRow.h"
#include "BindColumn.h"
#include "Favorite.h"

#include "PropertyWnd.h"
#include "PropertySerializer.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "CellProperty.h"
#include "KnownFolder.h"
#include "ResourceIDFactory.h"
#include "FilerWnd.h"
#include "FilerApplication.h"

template<typename... TItems>
class CLauncherCell:public CFileIconCell<TItems...>
{
	using base = CFileIconCell<TItems...>;
private:
	virtual std::wstring GetShortName()
	{
		if (auto pBindColumn = dynamic_cast<CBindColumn<TItems...>*>(this->m_pColumn)) {
			return std::get<std::shared_ptr<CFavorite>>(pBindColumn->GetTupleItems())->GetShortName();
		} else {
			return nullptr;
		}
	}
public:
	using CFileIconCell<TItems...>::CFileIconCell;
	virtual ~CLauncherCell() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile() override
	{
		if (auto pBindColumn = dynamic_cast<CBindColumn<TItems...>*>(this->m_pColumn)) {
			return std::get<std::shared_ptr<CFavorite>>(pBindColumn->GetTupleItems())->GetShellFile(
				[pSheet = this->m_pSheet]() {
					pSheet->GetGridPtr()->DelayUpdate();
				});
		} else {
			return nullptr;
		}
	}

	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override
	{
		//Paint Icon
		base::PaintContent(pDirect, rcPaint);

		//Paint Text
		pDirect->DrawTextInRect(*(this->m_spCellProperty->Format), GetShortName(), this->Content2InnerBorder(rcPaint));
	}

	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override
	{
		this->GetShellFile()->Open();
		(*e.HandledPtr) = true;
	}

	virtual void OnContextMenu(const ContextMenuEvent& e) override
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

			//auto pRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
			//auto pCol = static_cast<const CFavoritesColumn<TItems...>*>(this->m_pColumn);
			//auto& itemsSource = static_cast<CFavoritesGridView*>(this->m_pSheet)->GetItemsSource();
			//auto order = pRow->GetIndex<AllTag>();

			//TODOLOW
			//pFilerWnd->OnCommandOption<CFavorite>(L"Favorite", std::get<std::shared_ptr<CFavorite>>(itemsSource[order]),
			//									  [pFilerWnd](const std::wstring& prop)->void {
			//										  pFilerWnd->GetLeftFavoritesView()->Reload();
			//										  pFilerWnd->GetRightFavoritesView()->Reload();
			//										  pFilerWnd->InvalidateRect(NULL, FALSE);
			//										  ::SerializeProperty(pFilerWnd);
			//									  });
		} else if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeleteLauncher")) {
			auto pFilerWnd = static_cast<CFilerWnd*>(this->m_pSheet->GetWndPtr());

			//auto pRow = static_cast<const CLauncherRow<TItems...>*>(this->m_pRow);
			auto pCol = static_cast<const CBindColumn<TItems...>*>(this->m_pColumn);
			auto pGrid = static_cast<CLauncherGridView*>(this->m_pSheet);
			auto& itemsSource = pGrid->GetItemsSource();
			auto index = pCol->GetIndex<AllTag>();

			itemsSource.erase(std::next(itemsSource.cbegin(), index));
			pGrid->Reload();
			CFilerApplication::GetInstance()->SerializeLauncher(pFilerWnd->GetLauncherPropPtr());
		}

	}
};