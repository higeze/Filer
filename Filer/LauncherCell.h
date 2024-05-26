#pragma once
#include "FileIconCell.h"
#include "LauncherRow.h"
#include "BindColumn.h"
#include "Launcher.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "CellProperty.h"
#include "KnownFolder.h"
#include "ResourceIDFactory.h"
#include "FilerWnd.h"
#include "FilerApplication.h"

template<typename T>
class CLauncherCell:public CFileIconCell
{
	using base = CFileIconCell;
private:
	virtual std::wstring GetShortName()
	{
		if (auto pBindColumn = dynamic_cast<CBindColumn<T>*>(this->m_pColumn)) {
			return pBindColumn->GetItem<T>().GetShortName();
		} else {
			return nullptr;
		}
	}
public:
	using CFileIconCell::CFileIconCell;
	virtual ~CLauncherCell() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile() override
	{
		if (auto pBindColumn = dynamic_cast<CBindColumn<T>*>(this->m_pColumn)) {
			return pBindColumn->GetItem<T>().GetShellFile(
				[pSheet = this->m_pGrid]() {
					pSheet->DelayUpdate();
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
		pDirect->DrawTextInRect(GetFormat(), GetShortName(), this->Content2InnerBorder(rcPaint));
	}

	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override
	{
		if (auto pBindColumn = dynamic_cast<CBindColumn<T>*>(this->m_pColumn)) {
			auto item = pBindColumn->GetItem<T>();
			if (item.GetRunAs()) {
				this->GetShellFile()->RunAs();
			} else {
				this->GetShellFile()->Open();
			}
		}
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


		::SetForegroundWindow(this->m_pGrid->GetWndPtr()->m_hWnd);
		WORD retID = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, e.PointInScreen.x, e.PointInScreen.y, this->m_pGrid->GetWndPtr()->m_hWnd);
		if (retID == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"EditLauncher")) {
			//auto pFilerWnd = static_cast<CFilerWnd*>(this->m_pGrid->GetWndPtr());

			//auto pRow = static_cast<CBindRow<TItems...>*>(this->m_pRow);
			//auto pCol = static_cast<const CLaunchersColumn<TItems...>*>(this->m_pColumn);
			//auto& itemsSource = static_cast<CLaunchersGridView*>(this->m_pGrid)->GetItemsSource();
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
			auto pFilerWnd = static_cast<CFilerWnd*>(this->m_pGrid->GetWndPtr());

			//auto pRow = static_cast<const CLauncherRow<TItems...>*>(this->m_pRow);
			auto pCol = static_cast<const CBindColumn<T>*>(this->m_pColumn);
			auto pGrid = static_cast<CLauncherGridView*>(this->m_pGrid);
			auto& itemsSource = pGrid->GetItemsSource();
			auto index = pCol->GetIndex<AllTag>();

			itemsSource.erase(std::next(itemsSource->cbegin(), index));
			pGrid->Reload();
			CFilerApplication::GetInstance()->SerializeLauncher(pFilerWnd->GetLauncherPropPtr());
		}

	}
};