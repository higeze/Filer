#pragma once
#include "FavoritesGridView.h"
#include "FavoritesColumn.h"
#include "BindRow.h"
#include "Favorite.h"
#include "FileIconCell.h"

#include "PropertyWnd.h"
#include "PropertySerializer.h"
#include "Column.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "MySize.h"
#include "MyFont.h"
#include "CellProperty.h"
#include "KnownFolder.h"
#include "ResourceIDFactory.h"
#include "FilerWnd.h"
#include "Filer.h"

template<typename... TItems>
class CFavoriteCell:public CFileIconCell<TItems...>
{
	using base = CFileIconCell<TItems...>;
private:
	virtual std::wstring GetShortName()
	{
		if (auto pBindRow = dynamic_cast<CBindRow<TItems...>*>(this->m_pRow)) {
			return std::get<std::shared_ptr<CFavorite>>(pBindRow->GetTupleItems())->GetShortName();
		} else {
			return nullptr;
		}
	}
public:
	using CFileIconCell<TItems...>::CFileIconCell;
	virtual ~CFavoriteCell(){}

	virtual std::shared_ptr<CShellFile> GetShellFile() override
	{
		if (auto pBindRow = dynamic_cast<CBindRow<TItems...>*>(this->m_pRow)) {
			return std::get<std::shared_ptr<CFavorite>>(pBindRow->GetTupleItems())->GetShellFile(
				[wp = std::weak_ptr(std::dynamic_pointer_cast<CFavoriteCell<TItems...>>(this->shared_from_this()))]() {
					if (auto sp = wp.lock()) {
						sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
					}
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

	virtual void OnContextMenu(const ContextMenuEvent& e) override
	{
		CMenu menu(::CreatePopupMenu());
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_STRING;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"EditFavorite");
		mii.dwTypeData = const_cast<LPWSTR>(L"Edit");
		mii.cch = 4;
		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

		menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);

		mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeleteFavorite");
		mii.dwTypeData = const_cast<LPWSTR>(L"Delete");
		mii.cch = 6;
		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);


		::SetForegroundWindow(this->m_pSheet->GetWndPtr()->m_hWnd);
		WORD retID = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, e.PointInScreen.x, e.PointInScreen.y, this->m_pSheet->GetWndPtr()->m_hWnd);
		if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"EditFavorite")) {
			auto pFilerWnd = static_cast<CFilerWnd*>(this->m_pSheet->GetWndPtr());

			auto pRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
			auto pCol = static_cast<const CFavoritesColumn<TItems...>*>(this->m_pColumn);
			auto& itemsSource = static_cast<CFavoritesGridView*>(this->m_pSheet)->GetItemsSource();
			auto order = pRow->GetIndex<AllTag>();

			//TODOLOW
			//pFilerWnd->OnCommandOption<CFavorite>(L"Favorite", std::get<std::shared_ptr<CFavorite>>(itemsSource[order]),
			//									  [pFilerWnd](const std::wstring& prop)->void {
			//										  pFilerWnd->GetLeftFavoritesView()->Reload();
			//										  pFilerWnd->GetRightFavoritesView()->Reload();
			//										  pFilerWnd->InvalidateRect(NULL, FALSE);
			//										  ::SerializeProperty(pFilerWnd);
			//									  });
		} else if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"DeleteFavorite")) {
			auto pFilerWnd = static_cast<CFilerWnd*>(this->m_pSheet->GetWndPtr());

			auto pRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
			auto pCol = static_cast<const CFavoritesColumn<TItems...>*>(this->m_pColumn);
			auto& itemsSource = static_cast<CFavoritesGridView*>(this->m_pSheet)->GetItemsSource();
			auto order = pRow->GetIndex<AllTag>();

			itemsSource.erase(std::next(itemsSource.cbegin(), order));
			pFilerWnd->GetLeftFavoritesView()->Reload();
			pFilerWnd->GetRightFavoritesView()->Reload();
			pFilerWnd->InvalidateRect(NULL, FALSE);
			::SerializeProperty(pFilerWnd);
		}

	}
};