#pragma once
#include "FavoritesGridView.h"
#include "FavoritesColumn.h"
#include "BindRow.h"
#include "Favorite.h"
#include "FileIconCell.h"
#include "Column.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "MySize.h"
#include "MyFont.h"
#include "CellProperty.h"
#include "KnownFolder.h"
#include "ResourceIDFactory.h"
#include "FilerWnd.h"
#include "FilerApplication.h"
#include "ToolTip.h"

template<typename T>
class CFavoriteCell:public CFileIconCell
{
	using base = CFileIconCell;
private:
	virtual std::wstring GetShortName()
	{
		if (auto pBindRow = dynamic_cast<CBindRow<T>*>(this->m_pRow)) {
			return pBindRow->GetItem<std::shared_ptr<CFavorite>>()->GetShortName();
		} else {
			return nullptr;
		}
	}
public:
	using CFileIconCell::CFileIconCell;
	virtual ~CFavoriteCell(){}

	virtual std::wstring GetString() override
	{
		if (auto pBindRow = dynamic_cast<CBindRow<T>*>(this->m_pRow)) {
			if (GetShellFile()) {
				return GetShellFile()->GetDispName();
			}
			return nullptr;
		} else {
			return nullptr;
		}
	}

	virtual std::shared_ptr<CShellFile> GetShellFile() override
	{
		if (auto pBindRow = dynamic_cast<CBindRow<T>*>(this->m_pRow)) {
			return pBindRow->GetItem<std::shared_ptr<CFavorite>>()->GetShellFile(
				[wp = std::weak_ptr(std::dynamic_pointer_cast<CFavoriteCell<T>>(this->shared_from_this()))]() {
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

			auto pRow = static_cast<CBindRow<T>*>(this->m_pRow);
			auto pCol = static_cast<const CFavoritesColumn<T>*>(this->m_pColumn);
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

			auto pRow = static_cast<CBindRow<T>*>(this->m_pRow);
			auto pCol = static_cast<const CFavoritesColumn<T>*>(this->m_pColumn);
			auto& itemsSource = static_cast<CFavoritesGridView*>(this->m_pSheet)->GetItemsSource();
			auto order = pRow->GetIndex<AllTag>();

			itemsSource.erase(std::next(itemsSource->cbegin(), order));
			pFilerWnd->GetLeftFavoritesView()->Reload();
			pFilerWnd->GetRightFavoritesView()->Reload();
			pFilerWnd->InvalidateRect(NULL, FALSE);
			CFilerApplication::GetInstance()->SerializeFavorites(pFilerWnd->GetFavoritesPropPtr());
		}

	}

	virtual void OnMouseEnter(const MouseEnterEvent& e) override
	{
		::OutputDebugStringA("OnMouseEnter\r\n");
		auto InitialShowDelay = ::GetDoubleClickTime();
		auto BetweenShowDelay = InitialShowDelay / 5;
		auto ShowDuration = InitialShowDelay * 10;

		auto GetCursorSize = []()->CSize {
			return CSize(::GetSystemMetrics(SM_CXCURSOR), ::GetSystemMetrics(SM_CYCURSOR));
		};

	//auto GetCursorIconSize = []()->CSize {
	//	CIcon icon(::GetCursor());
	//	CSize iconSize;
	//	ICONINFO ii;
	//	if (::GetIconInfo(icon, &ii)) {
	//		if (BITMAP bm; ::GetObjectW(ii.hbmMask, sizeof(bm), &bm) == sizeof(bm)) {
	//			iconSize.cx = bm.bmWidth;
	//			iconSize.cy = ii.hbmColor ? bm.bmHeight : bm.bmHeight / 2;
	//		}
	//		if (ii.hbmMask)  ::DeleteObject(ii.hbmMask);
	//		if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
	//	}
	//	return iconSize;
	//};


		auto initialEnter = false;
		auto betweenEnter = true;


	//if (leave) {
	//	if (pSheet->GetWndPtr()->GetToolTipControlPtr()) {
	//		pSheet->GetWndPtr()->GetToolTipControlPtr()->OnClose(CloseEvent(e.WndPtr, NULL, NULL));
	//	}
	//} else if (initialEnter || betweenEnter) {
		auto delay = initialEnter ? InitialShowDelay : BetweenShowDelay;

		if (this->m_pSheet->GetWndPtr()->GetToolTipControlPtr()) {
			this->m_pSheet->GetWndPtr()->GetToolTipControlPtr()->OnClose(CloseEvent(e.WndPtr, NULL, NULL));
		}

		if (!GetString().empty()) {
			e.WndPtr->GetToolTipDeadlineTimer().run([pSheet = this->m_pSheet, content = GetString(), sz = GetCursorSize()]()->void
			{
				auto spTT = std::make_shared<CToolTip>(
					pSheet->GetWndPtr(),
					std::make_shared<ToolTipProperty>());
				CPointF pt = pSheet->GetWndPtr()->GetCursorPosInWnd();
				pt.x += sz.cx / 2;
				spTT->OnCreate(CreateEvt(pSheet->GetWndPtr(), pSheet->GetWndPtr(), CRectF()));
				spTT->Content.set(content);
				spTT->Measure(CSizeF(FLT_MAX, FLT_MAX));
				spTT->Arrange(CRectF(pt, spTT->DesiredSize()));

			}, std::chrono::milliseconds(delay));
		}
	//} 
	}
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override
	{
		e.WndPtr->GetToolTipDeadlineTimer().stop();
		::OutputDebugStringA("OnMouseLeave\r\n");
		if (this->m_pSheet->GetWndPtr()->GetToolTipControlPtr()) {
			this->m_pSheet->GetWndPtr()->GetToolTipControlPtr()->OnClose(CloseEvent(e.WndPtr, NULL, NULL));
		}
	}
};