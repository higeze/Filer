#include "stdafx.h"
#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "GridViewProperty.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "FavoritesColumn.h"
#include "FileNameColumn.h"
#include "FavoriteRow.h"
#include "FavoriteCell.h"

#include "ShellFile.h"
#include "ShellFolder.h"

#include "ConsoleTimer.h"
#include "ApplicationProperty.h"
#include "SheetEventArgs.h"
#include "Cursorer.h"
#include "FavoritesItemDragger.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CFavoritesGridView::CFavoritesGridView(std::shared_ptr<CGridViewProperty> spGridViewProrperty/*, std::shared_ptr<CFavoritesProperty> spFavoritesProp*/)
			:CGridView(			
			spGridViewProrperty->m_spBackgroundProperty,
			spGridViewProrperty->m_spPropHeader,
			spGridViewProrperty->m_spPropCell,
			spGridViewProrperty->m_spPropCell,
			spGridViewProrperty->m_spDeltaScroll),
			m_spFavoritesProp(std::make_shared<CFavoritesProperty>())
{
	//RegisterArgs and CreateArgs
	RegisterClassExArgument()
		.lpszClassName(_T("CFavoriteGridView"))
		.style(CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)(COLOR_3DFACE+1));
	CreateWindowExArgument()
		.lpszClassName(_T("CFavoriteGridView"))
		.lpszWindowName(_T("FavoriteGridView"))
		.dwStyle(WS_CHILD | WS_VISIBLE)
		.hMenu((HMENU)1233211); 

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

void CFavoritesGridView::OpenFavorites()
{
	CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, "OpenFavorites")

	m_spCursorer->Clear();

	if(Empty()){
		InsertDefaultRowColumn();
	}

	try{

		//Clear RowDictionary From 0 to last
		auto& rowDictionary=m_rowAllDictionary.get<IndexTag>();
		rowDictionary.erase(rowDictionary.begin(),rowDictionary.end());

		//Enumerate child IDL
		for(size_t i = 0; i < m_spFavoritesProp->GetFavorites()->size(); i++) {
			InsertRow(CRow::kMaxIndex, std::make_shared<CFavoriteRow>(this, i));
		}

		for(auto iter=m_columnAllDictionary.begin();iter!=m_columnAllDictionary.end();++iter){
			std::dynamic_pointer_cast<CParentMapColumn>(iter->DataPtr)->Clear();
			iter->DataPtr->SetMeasureValid(false);
		}

	}catch(...){
		MessageBox(L"Error on Open", L"Error",MB_ICONWARNING);
	}

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
	SortAll();

	FilterAll();		

	SubmitUpdate();
}

void CFavoritesGridView::InsertDefaultRowColumn()
{
	//IconColumn
	{
		auto pColumn = std::make_shared<CFavoritesColumn>(this, m_spFavoritesProp->GetFavorites());
		InsertColumn(CColumn::kMaxIndex,pColumn);
	}
}

void CFavoritesGridView::OnCellLButtonDblClk(CellEventArgs& e)
{
	if(auto p = dynamic_cast<CFavoriteCell*>(e.CellPtr)){
		FileChosen(p->GetShellFile());
	}
}

void CFavoritesGridView::RowMoved(CMovedEventArgs<RowTag>& e)
{
	auto favorites(m_spFavoritesProp->GetFavorites());
	auto fromIter = favorites->begin(); std::advance(fromIter, e.m_from);
	auto temp = *fromIter;
	favorites->erase(fromIter);
	auto toIter = favorites->begin(); std::advance(toIter, e.m_to);
	favorites->insert(toIter, temp);
}
