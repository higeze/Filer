//#include "stdafx.h"
#include "LauncherGridView.h"
#include "LauncherProperty.h"
#include "LauncherItemDragger.h"
#include "LauncherRow.h"
#include "BindColumn.h"
#include "LauncherCell.h"

#include "GridViewProperty.h"

#include "ShellFile.h"
#include "ShellFolder.h"

#include "Debug.h"
#include "ApplicationProperty.h"
#include "SheetEventArgs.h"
#include "Cursorer.h"
#include "FavoritesItemDragger.h"
#include "FilerWnd.h"
#include "Celler.h"
#include "Debug.h"
#include "D2DWWindow.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CLauncherGridView::CLauncherGridView(
	CD2DWControl* pParentControl,
	const std::shared_ptr<GridViewProperty>& spGridViewProp,
	const std::shared_ptr<CLauncherProperty>& spLauncherProp)
	:CBindGridView(pParentControl, spGridViewProp, spLauncherProp->GetFavoritesPtr(),
		arg<"bindtype"_s>() = BindType::Column,
		arg<"rows"_s>() = std::vector<std::shared_ptr<CRow>>{std::make_shared<CLauncherRow<std::shared_ptr<CFavorite>>>(this)}),
	m_spLauncherProp(spLauncherProp)
{
	GetIsFocusable().set(false);
	m_pVScroll->SetVisibility(Visibility::Disabled);
	m_pHScroll->SetVisibility(Visibility::Hidden);

	m_spItemDragger = std::make_shared<CLauncherItemDragger>();
}

void CLauncherGridView::OnCreate(const CreateEvt& e)
{
	//Base Create
	CBindGridView::OnCreate(e);

	//List
	OpenFavorites();

	SubmitUpdate();
}

void CLauncherGridView::OpenFavorites()
{
	LOG_THIS_1("CLauncherGridView::OpenFavorites");

	LOG_SCOPED_TIMER_THIS_1("OpenFavorites Total");

	//Celler
	m_spCeller->Clear();
	//Cursor
	m_spCursorer->Clear();

	m_allCells.clear();
	//for (auto colPtr : m_allCols) {
	//	std::dynamic_pointer_cast<CMapColumn>(colPtr)->Clear();
	//	colPtr->SetIsMeasureValid(false);
	//}

	//PostUpdate(Updates::Sort);
	//PostUpdate(Updates::ColumnVisible);
	//PostUpdate(Updates::RowVisible);
	//PostUpdate(Updates::Column);
	//PostUpdate(Updates::Row);
	//PostUpdate(Updates::Invalidate);
}

//void CLauncherGridView::OnCellLButtonDblClk(const CellEventArgs& e)
//{
//	if(auto p = dynamic_cast<CLauncherCell<std::shared_ptr<CFavorite>>*>(e.CellPtr)){
//		auto spFile = p->GetShellFile();
//		if (spFile != nullptr) {
//			spFile->Open();
//		}
//	}
//}

void CLauncherGridView::MoveColumn(int indexTo, typename ColTag::SharedPtr spFrom)
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

void CLauncherGridView::Reload()
{
	//for (auto iter = m_spFavoritesProp->GetFavorites().cbegin(); iter != m_spFavoritesProp->GetFavorites().cend(); ++iter) {
	//	(std::get<0>((*iter)))->SetLockShellFile(nullptr);
	//}
	OpenFavorites();
	SubmitUpdate();
}