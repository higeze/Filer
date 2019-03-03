//#include "stdafx.h"
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
#include "FilerWnd.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CFavoritesGridView::CFavoritesGridView(CFilerWnd* pWnd, std::shared_ptr<GridViewProperty> spGridViewProperty, std::shared_ptr<CFavoritesProperty> spFavoritesProp)
			:CGridView(spGridViewProperty),
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

LRESULT CFavoritesGridView::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = true;
	return MA_NOACTIVATE;
}

void CFavoritesGridView::OpenFavorites()
{
	BOOST_LOG_TRIVIAL(trace) << L"CFavoritesGridView::OpenFavorites";

	CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"OpenFavorites Total")

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

	PostUpdate(Updates::Sort);
	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Invalidate);

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
		auto pFile = p->GetShellFile();
		if (pFile != nullptr && typeid(*pFile) != typeid(CShellInvalidFile)) {
			FileChosen(p->GetShellFile());
		}
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

void CFavoritesGridView::Reload()
{
	for (auto iter = m_spFavoritesProp->GetFavorites()->begin(); iter != m_spFavoritesProp->GetFavorites()->end();++iter) {
		((*iter))->SetShellFile(nullptr);
	}
	PostUpdate(Updates::All);
	SubmitUpdate();
}
