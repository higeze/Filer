//#include "stdafx.h"
#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "GridViewProperty.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "FavoritesColumn.h"
#include "BindRow.h"
#include "FavoriteCell.h"

#include "ShellFile.h"
#include "ShellFolder.h"

#include "ConsoleTimer.h"
#include "ApplicationProperty.h"
#include "SheetEventArgs.h"
#include "Cursorer.h"
#include "FavoritesItemDragger.h"
#include "FilerWnd.h"
#include "Celler.h"
#include "Debug.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CFavoritesGridView::CFavoritesGridView(CFilerWnd* pWnd, std::shared_ptr<GridViewProperty> spGridViewProperty, std::shared_ptr<CFavoritesProperty> spFavoritesProp)
			:CBindGridView(spGridViewProperty,
						   spFavoritesProp->GetFavoritesPtr()),
			m_spFavoritesProp(spFavoritesProp),
			m_pFilerWnd(pWnd)
{
	m_isFocusable = false;

	//RegisterArgs and CreateArgs
	RegisterClassExArgument()
		.lpszClassName(_T("CFavoriteGridView"))
		.style(CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)(COLOR_3DFACE+1));
	CreateWindowExArgument()
		.lpszClassName(_T("CFavoriteGridView"))
		.lpszWindowName(_T("FavoriteGridView"))
		.dwStyle(WS_CHILD);

	//AddMsgHandler(WM_MOUSEACTIVATE, &CFavoritesGridView::OnMouseActivate, this);

	m_spItemDragger = std::make_shared<CFavoritesItemDragger>();

	CellLButtonDblClk.connect(std::bind(&CFavoritesGridView::OnCellLButtonDblClk,this,std::placeholders::_1));
}

LRESULT CFavoritesGridView::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	//Base Create
	CGridView::OnCreate(uMsg,wParam,lParam,bHandled);

	//List
	OpenFavorites();
	return 0;
}

//LRESULT CFavoritesGridView::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	bHandled = true;
//	return MA_NOACTIVATE;
//}

void CFavoritesGridView::OpenFavorites()
{
	SPDLOG_INFO("CFavoritesGridView::OpenFavorites");

	CONSOLETIMER("OpenFavorites Total");

	//Direct2DWrite
	m_pDirect->ClearTextLayoutMap();
	//Celler
	m_spCeller->Clear();
	//Cursor
	m_spCursorer->Clear();

	if (Empty()) {
		//IconColumn
		{
			auto pColumn = std::make_shared<CFavoritesColumn<std::shared_ptr<CFavorite>>>(this);
			PushColumn(pColumn);
		}
	}


	for (auto colPtr : m_allCols) {
		std::dynamic_pointer_cast<CMapColumn>(colPtr)->Clear();
		colPtr->SetIsMeasureValid(false);
	}

	PostUpdate(Updates::Sort);
	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Invalidate);
}

void CFavoritesGridView::OnCellLButtonDblClk(CellEventArgs& e)
{
	if(auto p = dynamic_cast<CFavoriteCell<std::shared_ptr<CFavorite>>*>(e.CellPtr)){
		auto pFile = p->GetShellFile();
		if (pFile != nullptr && typeid(*pFile) != typeid(CShellInvalidFile)) {
			FileChosen(p->GetShellFile());
		}
	}
}

void CFavoritesGridView::MoveRow(int indexTo, typename RowTag::SharedPtr spFrom)
{
	int from = spFrom->GetIndex<VisTag>();
	int to = indexTo > from ? indexTo - 1 : indexTo;

	auto& itemsSource = GetItemsSource();
	auto fromIter = itemsSource.cbegin() + (from - GetFrozenCount<RowTag>());
	auto temp = *fromIter;
	itemsSource.notify_erase(fromIter);
	auto toIter = itemsSource.cbegin() + (to - GetFrozenCount<RowTag>());
	itemsSource.notify_insert(toIter, temp);

	Reload();
}

void CFavoritesGridView::Reload()
{
	for (auto iter = m_spFavoritesProp->GetFavorites().begin(); iter != m_spFavoritesProp->GetFavorites().end(); ++iter) {
		(std::get<0>((*iter)))->SetLockShellFile(nullptr);
	}
	OpenFavorites();
}
