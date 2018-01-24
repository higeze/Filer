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

#include "ShellFile.h"
#include "ShellFolder.h"

#include "ConsoleTimer.h"
#include "ApplicationProperty.h"
#include "SheetEventArgs.h"
#include "Cursorer.h"
#include "FavoritesItemDragger.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CFavoritesGridView::CFavoritesGridView(std::shared_ptr<CGridViewProperty> spGridViewProrperty, std::shared_ptr<CFavoritesProperty> spFavoritesProp)
			:CGridView(			
			spGridViewProrperty->m_spBackgroundProperty,
			spGridViewProrperty->m_spPropHeader,
			spGridViewProrperty->m_spPropCell,
			spGridViewProrperty->m_spPropCell,
			spGridViewProrperty->m_spDeltaScroll),
			m_spFavoritesProp(spFavoritesProp)
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
		rowDictionary.erase(rowDictionary.find(0),rowDictionary.end());

		//Enumerate child IDL
		for each(auto& favorite in m_spFavoritesProp->GetFavorites()){
			ULONG         chEaten;
			ULONG         dwAttributes;
 
			CComPtr<IShellFolder> pDesktop;
			::SHGetDesktopFolder(&pDesktop);

			CIDLPtr pIDL;
			HRESULT hr = NULL;
			if(favorite.GetPath() == L""){
				InsertRow(CRow::kMaxIndex,std::make_shared<CFavoriteRow>(this, std::make_shared<CShellFolder>(), favorite.GetShortName()));
			}else{
				hr = pDesktop->ParseDisplayName(
					NULL,
					NULL,
					const_cast<LPWSTR>(favorite.GetPath().c_str()),
					&chEaten,
					&pIDL,
					&dwAttributes);
			

				if (SUCCEEDED(hr))
				{
					CComPtr<IShellFolder> pFolder, pParentFolder;
					::SHBindToObject(pDesktop,pIDL,0,IID_IShellFolder,(void**)&pFolder);
					::SHBindToObject(pDesktop,pIDL.GetPreviousIDLPtr(),0,IID_IShellFolder,(void**)&pParentFolder);

					if(!pFolder){
						pFolder = pDesktop;
					}
					if(!pParentFolder){
						pParentFolder = pDesktop;
					}
					InsertRow(100,std::make_shared<CFavoriteRow>(this, std::make_shared<CShellFolder>(pFolder,pParentFolder, pIDL),favorite.GetShortName()));
				}
			}
		}



		//Path change //TODO
		//m_rowHeader->SetMeasureValid(false);
		//m_rowNameHeader->SetMeasureValid(false);
		//m_rowFilter->SetMeasureValid(false);

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
	//Row
	//m_rowHeader = std::make_shared<CParentHeaderRow>(this);
	//m_rowNameHeader=std::make_shared<CParentHeaderRow>(this);
	//m_rowFilter=std::make_shared<CParentRow>(this);

	//InsertRow(-1,m_rowHeader);
	//InsertRow(-1,m_rowNameHeader);
	//InsertRow(-1,m_rowFilter);

	//HeaderColumn
	//auto pHeaderColumn=std::make_shared<CParentRowHeaderColumn>(this);
	//InsertColumn(-1,pHeaderColumn);

	//IconColumn
	{
		auto pColumn = std::make_shared<CFavoritesColumn>(this);
		InsertColumn(CColumn::kMaxIndex,pColumn);
	}
	//NameColumn
	//{
	//	auto pColumn = std::make_shared<CFileNameColumn>(this);
	//	InsertColumn(COLUMN_INDEX_INVALID, pColumn);
	//}
}

void CFavoritesGridView::OnCellLButtonDblClk(CellEventArgs& e)
{
	auto pCell = e.CellPtr;
	if(auto p = dynamic_cast<CFileRow*>(e.CellPtr->GetRowPtr())){
		if(auto spFolder = std::dynamic_pointer_cast<CShellFolder>(p->GetFilePointer())){
			FolderChoiced(spFolder);
		}
	}
}

void CFavoritesGridView::RowMoved(CMovedEventArgs<RowTag>& e)
{
	std::vector<CFavorite> favorites(m_spFavoritesProp->GetFavorites());
	auto fromIter = favorites.begin(); std::advance(fromIter, e.m_from);
	auto temp = *fromIter;
	favorites.erase(fromIter);
	auto toIter = favorites.begin(); std::advance(toIter, e.m_to);
	favorites.insert(toIter, temp);
	//std::swap(favorites[e.m_from], favorites[e.m_to]);
	m_spFavoritesProp->SetFavorites(favorites);

}
