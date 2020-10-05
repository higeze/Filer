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

CFavoritesGridView::CFavoritesGridView(
	CD2DWControl* pParentControl,
	std::shared_ptr<GridViewProperty> spGridViewProp,
	std::shared_ptr<CFavoritesProperty> spFavoritesProp)
	:CBindGridView(pParentControl, spGridViewProp, spFavoritesProp->GetFavoritesPtr()),
	m_spFavoritesProp(spFavoritesProp)
{
	m_isFocusable = false;
	m_pVScroll->SetVisibility(Visibility::Hidden);
	m_pHScroll->SetVisibility(Visibility::Disabled);

	m_spItemDragger = std::make_shared<CFavoritesItemDragger>();

	CellLButtonDblClk.connect(std::bind(&CFavoritesGridView::OnCellLButtonDblClk,this,std::placeholders::_1));
}

void CFavoritesGridView::OnCreate(const CreateEvt& e)
{
	//Base Create
	CGridView::OnCreate(e);

	//List
	OpenFavorites();

	SubmitUpdate();
}

void CFavoritesGridView::OpenFavorites()
{
	SPDLOG_INFO("CFavoritesGridView::OpenFavorites");

	CONSOLETIMER("OpenFavorites Total");

	//Direct2DWrite
	GetWndPtr()->GetDirectPtr()->ClearTextLayoutMap();
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
	SubmitUpdate();
}

void CFavoritesGridView::MoveRow(int indexTo, typename RowTag::SharedPtr spFrom)
{
	int from = spFrom->GetIndex<VisTag>();
	int to = indexTo > from ? indexTo - 1 : indexTo;

	auto& itemsSource = GetItemsSource();
	auto fromIter = itemsSource.cbegin() + (from - GetFrozenCount<RowTag>());
	auto temp = *fromIter;
	itemsSource.erase(fromIter);
	auto toIter = itemsSource.cbegin() + (to - GetFrozenCount<RowTag>());
	itemsSource.insert(toIter, temp);

	Reload();
}

void CFavoritesGridView::Reload()
{
	for (auto iter = m_spFavoritesProp->GetFavorites().cbegin(); iter != m_spFavoritesProp->GetFavorites().cend(); ++iter) {
		(std::get<0>((*iter)))->SetLockShellFile(nullptr);
	}
	OpenFavorites();
	SubmitUpdate();
}
