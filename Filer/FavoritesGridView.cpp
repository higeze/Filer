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
#include "Celler.h"
#include "Debug.h"

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
	spdlog::info("CFavoritesGridView::OpenFavorites");

	CONSOLETIMER("OpenFavorites Total");
	{
		CONSOLETIMER("OpenFavorites Pre-Process");

		//Direct2DWrite
		m_pDirect->ClearTextLayoutMap();
		//Celler
		m_spCeller->Clear();

		//Cursor
		m_spCursorer->Clear();

		if (Empty()) {
			//IconColumn
			{
				auto pColumn = std::make_shared<CFavoritesColumn>(this, m_spFavoritesProp->GetFavorites());
				PushColumn(pColumn);
			}
		}
		//Clear RowDictionary From 0 to last
		m_allRows.idx_erase(m_allRows.begin()+m_frozenRowCount, m_allRows.end());
	}

	{
		CONSOLETIMER("OpenFavorites Enumeration");

		try {
			//Enumerate favorites
			for (size_t i = 0; i < m_spFavoritesProp->GetFavorites()->size(); i++) {
				PushRow(std::make_shared<CFavoriteRow>(this, i));
			}

			for (auto colPtr : m_allCols) {
				std::dynamic_pointer_cast<CParentMapColumn>(colPtr)->Clear();
				colPtr->SetMeasureValid(false);
			}

		} catch (...) {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	PostUpdate(Updates::Sort);
	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Invalidate);

	SubmitUpdate();
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
	for (auto iter = m_spFavoritesProp->GetFavorites()->begin(); iter != m_spFavoritesProp->GetFavorites()->end(); ++iter) {
		((*iter))->SetShellFile(nullptr);
	}
	OpenFavorites();
	//PostUpdate(Updates::All);
	//SubmitUpdate();
}
