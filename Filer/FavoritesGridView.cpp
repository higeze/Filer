#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "BindColumn.h"
#include "BindRow.h"
#include "FavoritesColumn.h"
#include "FavoriteCell.h"

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
#include "EditorProperty.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CFavoritesGridView::CFavoritesGridView(CD2DWControl* pParentControl)
	:CBindGridView(pParentControl,
		arg<"bindtype"_s>() = BindType::Row,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{std::make_shared<CFavoritesColumn<std::shared_ptr<CFavorite>>>(this)})
{
	IsFocusable.set(false);
	m_pVScroll->SetVisibility(Visibility::Hidden);
	m_pHScroll->SetVisibility(Visibility::Disabled);

	m_spItemDragger = std::make_shared<CFavoritesItemDragger>();

	CellLButtonDblClk.connect(std::bind(&CFavoritesGridView::OnCellLButtonDblClk,this,std::placeholders::_1));
}

void CFavoritesGridView::OnCreate(const CreateEvt& e)
{
	//Base Create
	CBindGridView::OnCreate(e);

	//List
	OpenFavorites();

	SubmitUpdate();
}

void CFavoritesGridView::OnKeyDown(const KeyDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	switch (e.Char) {
		case 'R':
			if (::IsKeyDown(VK_CONTROL)) {
				Reload();
				(*e.HandledPtr) = true;
			}
			break;
		default:
			CBindGridView::OnKeyDown(e);
	}
}

void CFavoritesGridView::OpenFavorites()
{
	LOG_THIS_1("CFavoritesGridView::OpenFavorites");

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

	PostUpdate(Updates::Sort);
	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Invalidate);
}

void CFavoritesGridView::OnCellLButtonDblClk(const CellEventArgs& e)
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

	auto fromIter = ItemsSource->cbegin() + (from - GetFrozenCount<RowTag>());
	auto temp = *fromIter;
	ItemsSource.erase(fromIter);
	auto toIter = ItemsSource->cbegin() + (to - GetFrozenCount<RowTag>());
	ItemsSource.insert(toIter, temp);

	Reload();
}

void CFavoritesGridView::Reload()
{
	for (auto iter = m_spFavoritesProp->Favorites->cbegin(); iter != m_spFavoritesProp->Favorites->cend(); ++iter) {
		(*iter)->SetLockShellFile(nullptr);
	}
	OpenFavorites();
	SubmitUpdate();
}
