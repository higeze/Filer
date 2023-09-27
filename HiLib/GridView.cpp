#include <olectl.h>
#include <numeric>
#include "D2DWControl.h"
#include "Dispatcher.h"
#include "GridView.h"
#include "GridViewStateMachine.h"
#include "Cell.h"
#include "CellProperty.h"
#include "Row.h"
#include "ParentHeaderCell.h"
#include "MapColumn.h"
#include "GridViewResource.h"
#include "SheetCell.h"
#include "MyRgn.h"
#include "MyPen.h"
#include "MyColor.h"
#include "Debug.h"
#include "ApplicationProperty.h"
#include "Tracker.h"
#include "Cursorer.h"
#include "Dragger.h"
#include "SheetState.h"
#include "MyClipboard.h"
#include "SheetEventArgs.h"
#include "Celler.h"
#include "Scroll.h"
#include "CellTextbox.h"
#include "FindDlg.h"

#include "TextCell.h"
#include "MouseStateMachine.h"
#include "GridViewProperty.h"
#include "ResourceIDFactory.h"
#include "D2DWWindow.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CMenu CGridView::ContextMenu;

CGridView::CGridView(
	CD2DWControl* pParentControl,
	const std::shared_ptr<GridViewProperty>& spGridViewProp,
	CMenu* pContextMenu)
	:m_spGridViewProp(spGridViewProp),
	CSheet(pParentControl, spGridViewProp, pContextMenu ? pContextMenu : &CGridView::ContextMenu),
	m_pVScroll(std::make_unique<CVScroll>(this, spGridViewProp->VScrollPropPtr)),
	m_pHScroll(std::make_unique<CHScroll>(this, spGridViewProp->HScrollPropPtr))
{
	m_pVScroll->ScrollChanged.connect([this](){
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Invalidate);
	});

	m_pHScroll->ScrollChanged.connect([this](){
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Invalidate);
	});

	m_pMachine.reset(new CGridStateMachine(this));

	CellLButtonClk.connect(std::bind(&CGridView::OnCellLButtonClk, this, std::placeholders::_1));
	CellContextMenu.connect(std::bind(&CGridView::OnCellContextMenu, this, std::placeholders::_1));
}

CGridView::~CGridView() = default;

void CGridView::SortAllInSubmitUpdate()
{
	for (const auto& ptr : m_visCols) {
		if (ptr->GetSort() != Sorts::None) {
			this->Sort(ptr.get(), ptr->GetSort(), false);
		}
	}
}

void CGridView::ClearFilter()
{
	//Reset Filter
	for(auto iter=std::next(m_allRows.begin(), m_frozenRowCount), end=m_allRows.end(); iter!=end; ++iter){
		(*iter)->SetIsVisible(true);
	};
	//Clear Filter
	for(auto ptr : m_allCols ){
		Cell(GetFilterRowPtr(), ptr)->SetString(L"");	
	}
}

void CGridView::FilterAll()
{
	if (GetFilterRowPtr()) {
		//Reset Filter
		for (auto iter = std::next(m_allRows.begin(), m_frozenRowCount), end = m_allRows.end(); iter != end; ++iter) {
			(*iter)->SetIsVisible(true);
		};
		//Set Filter
		for (auto colIter = std::next(m_allCols.begin(), m_frozenColumnCount), colEnd = m_allCols.end(); colIter != colEnd; ++colIter) {
			auto strFilter = Cell((*colIter), GetFilterRowPtr())->GetString();
			if (strFilter.empty() || strFilter == std::wstring(L""))continue;
			std::vector<std::wstring> vstrFilter;
			boost::split(vstrFilter, strFilter, boost::is_space());
			for (auto rowIter = std::next(m_allRows.begin(), m_frozenRowCount), end = m_allRows.end(); rowIter != end; ++rowIter) {
				if (!(*rowIter)->GetIsVisible())continue;
				//Filter
				auto pCell = Cell(*rowIter, *colIter);
				for (const auto& str : vstrFilter) {
					if (str[0] == L'-' && str.size() >= 2) {
						std::wstring strMinus(str.substr(1));
						if (pCell->Filter(strMinus)) {
							(*rowIter)->SetIsVisible(false);
						}
					} else if (!(pCell->Filter(str))) {
						(*rowIter)->SetIsVisible(false);
					}
				}
			}
		}
	}
}

void CGridView::Invalidate()
{
	GetWndPtr()->InvalidateRect(NULL,FALSE);
}

void CGridView::DelayUpdate()
{
	m_invalidateTimer.run([this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]()->void
		{
			OnDelayUpdate();
		});
	}, std::chrono::milliseconds(50));
}

FLOAT CGridView::UpdateHeadersRow(FLOAT top)
{
	//Minus Cells
	for (auto iter = m_visRows.begin(), end = std::next(m_visRows.begin(), m_frozenRowCount); iter != end; ++iter) {
		(*iter)->SetTop(top, false);
		top += (*iter)->GetHeight();
	}
	return top;
}


FLOAT CGridView::UpdateCellsRow(FLOAT top, FLOAT pageTop, FLOAT pageBottom)
{
	//Helper functions
	std::function<bool(FLOAT, FLOAT, FLOAT, FLOAT)> isInPage = [](FLOAT min, FLOAT max, FLOAT top, FLOAT bottom)->bool {
		return (min < top && top < max) ||
			(min < bottom && bottom < max) ||
			(top < min && max < bottom);
	};
	//Plus Cells
	for (auto rowIter = m_visRows.begin() + m_frozenRowCount, rowEnd = m_visRows.end(); rowIter != rowEnd; ++rowIter) {
		auto rowNextIter = std::next(rowIter);
		if (IsVirtualPage()) {
			(*rowIter)->SetTop(top, false);
	
			FLOAT defaultHeight = (*rowIter)->GetVirtualHeight();
			FLOAT bottom = top + defaultHeight;
			if (isInPage(pageTop, pageBottom, top, top + defaultHeight) ||
				(rowNextIter!=rowEnd && isInPage(pageTop, pageBottom, bottom, bottom + (*rowNextIter)->GetVirtualHeight()))){
				if (HasSheetCell()) {
					for (auto& ptr : m_visCols) {
						std::shared_ptr<CCell> pCell = Cell(*rowIter, ptr);
						if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
							pSheetCell->UpdateAll();
						}
					}
				}
				top += (*rowIter)->GetHeight();
			} else {
				top += defaultHeight;
			}
			//std::cout << top << std::endl;
		} else {
			(*rowIter)->SetTop(top, false);
			if (HasSheetCell()) {
				for (auto& ptr : m_visCols) {
					std::shared_ptr<CCell> pCell = Cell(*rowIter, ptr);
					if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
						pSheetCell->UpdateAll();
					}
				}
			}
			top += (*rowIter)->GetHeight();
		}
//		prevRowIter = rowIter;
	}
	return top;
}

void CGridView::UpdateRow()
{
	if (!Visible()) { return; }

	FLOAT top = GetRectInWnd().top + GetCellProperty()->Line->Width * 0.5f;

	//Headers
	top = UpdateHeadersRow(top);

	//Page
	CRectF rcPage(GetPageRect());
	//rcPage.MoveToXY(0, 0);
	//rcPage.MoveToY(top);
	//FLOAT pageHeight = rcPage.Height();
	FLOAT scrollPos = GetVerticalScrollPos();

	//Cells
	top = UpdateCellsRow(top - scrollPos, rcPage.top, rcPage.bottom);

	//Scroll Virtical Range
	if (IsVirtualPage()) {
		m_pVScroll->SetScrollRange(0.f, GetCellsHeight());
		if (scrollPos != GetVerticalScrollPos()) {
			UpdateRow();
		}
	}

	//::OutputDebugStringA(fmt::format("Row of {}\r\n", typeid(*this).name()).c_str());
	//for (const auto& r : m_allRows) {
	//	::OutputDebugStringA(fmt::format("{}\r\n", r->GetStart()).c_str());
	//}

}

void CGridView::UpdateColumn()
{
	if (!Visible()) { return; }

	FLOAT left = GetRectInWnd().left + GetCellProperty()->Line->Width * 0.5f;

	for (auto& colPtr : m_visCols) {
		if (colPtr->GetIndex<VisTag>() == GetFrozenCount<ColTag>()) {
			left -= GetHorizontalScrollPos();
		}
		colPtr->SetLeft(left, false);
		for (auto rowPtr : m_visRows) {
			std::shared_ptr<CCell> pCell = Cell(rowPtr, colPtr);
			if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
				pSheetCell->UpdateAll();
			}
		}
		left += colPtr->GetWidth();
	}

	//::OutputDebugStringA(fmt::format("Column of {}\r\n", typeid(*this).name()).c_str());
	//for (const auto& r : m_allCols) {
	//	::OutputDebugStringA(fmt::format("{}\r\n", r->GetStart()).c_str());
	//}

}

void CGridView::UpdateScrolls()
{
	if(!Visible())return;

	//Client
	CRectF rcClient(GetRectInWnd());

	//Origin
	//CPointF ptOrigin(GetOriginPoint());

	//Scroll Range
	CRectF rcCells(GetCellsRect());
	m_pVScroll->SetScrollRange(0.f, rcCells.Height());
	m_pHScroll->SetScrollRange(0.f, rcCells.Width());

	//Scroll Page
	CRectF rcPage(GetPageRect());
	m_pVScroll->SetScrollPage(rcPage.Height());
	m_pHScroll->SetScrollPage(rcPage.Width());

	//Position scroll
	CRectF rcVertical;
	CRectF rcHorizontal;
	FLOAT lineHalfWidth = GetCellProperty()->Line->Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_pHScroll->GetIsVisible()?(m_pHScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	m_pVScroll->OnRect(RectEvent(GetWndPtr(), rcVertical));

	rcHorizontal.left= rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom-::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_pVScroll->GetIsVisible()?(m_pVScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;
	m_pHScroll->OnRect(RectEvent(GetWndPtr(), rcHorizontal));

}

CPointF CGridView::GetScrollPos()const
{
	return CPointF(m_pHScroll->GetScrollPos(), m_pVScroll->GetScrollPos());
}

void CGridView::SetScrollPos(const CPoint& ptScroll)
{
	m_pHScroll->SetScrollPos(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(ptScroll.x));
	m_pVScroll->SetScrollPos(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(ptScroll.y));
}

FLOAT CGridView::GetVerticalScrollPos()const
{
	return m_pVScroll->GetScrollPos();
}

FLOAT CGridView::GetHorizontalScrollPos()const
{
	return m_pHScroll->GetScrollPos();
}


void CGridView::OnCommandEditHeader(const CommandEvent& e)
{
	if(!m_rocoContextMenu.IsInvalid()){
		if(m_rocoContextMenu.GetRowPtr()==GetNameHeaderRowPtr()){
			if(auto pCell=std::dynamic_pointer_cast<CParentHeaderCell>(Cell(m_rocoContextMenu.GetRowPtr(), m_rocoContextMenu.GetColumnPtr()))){
				pCell->OnEdit(Event());
			}
		}
	}
}

void CGridView::OnCommandDeleteColumn(const CommandEvent& e)
{
	if(!m_rocoContextMenu.IsInvalid()){
		EraseColumn(m_rocoContextMenu.GetColumnPtr());
	}
}

void CGridView::OnCommandResizeSheetCell(const CommandEvent& e)
{
	if(!m_rocoContextMenu.IsInvalid()){
		if(auto p = std::dynamic_pointer_cast<CSheetCell>(Cell(m_rocoContextMenu.GetRowPtr(),m_rocoContextMenu.GetColumnPtr()))){
			p->Resize();
		}
	}
}


void CGridView::OnCommandSelectAll(const CommandEvent& e)
{
	SelectAll();
}

void CGridView::OnCommandCopy(const CommandEvent& e)
{
	//TODO High
	std::wstring strCopy;
	bool SelectModeRow = true;
	bool IncludesHeader = true;

	if(SelectModeRow){
		for (auto rowPtr : m_visRows) {
			if (rowPtr->GetIsSelected() || (IncludesHeader && rowPtr->GetIndex<VisTag>()<0)) {
				for (auto colPtr : m_visCols) {
					auto pCell = Cell(rowPtr, colPtr);
					strCopy.append(pCell->GetString());
					if (colPtr->GetIndex<VisTag>() == m_visCols.size() - 1) {
						strCopy.append(L"\r\n");
					} else {
						strCopy.append(L"\t");
					}
				}

			}
		}
	}else{
		for(auto rowPtr : m_visRows){
			bool bSelectedLine(false);
			bool bFirstLine(true);
			for(auto colPtr : m_visCols){
				auto pCell=Cell(rowPtr, colPtr);
				if(pCell->GetIsSelected()){
					bSelectedLine=true;
					if(bFirstLine){
						bFirstLine=false;
					}else{
						strCopy.append(L"\t");
					}
					strCopy.append(pCell->GetString());
				}
			}
			if(bSelectedLine){
				strCopy.append(L"\r\n");
			}
		}
	}
	//
	HGLOBAL hGlobal=::GlobalAlloc(GHND|GMEM_SHARE,(strCopy.size()+1)*sizeof(wchar_t));
	wchar_t* strMem=(wchar_t*)::GlobalLock(hGlobal);
	::GlobalUnlock(hGlobal);
	
	if(strMem!=NULL){
		::wcscpy_s(strMem,strCopy.size()+1,strCopy.c_str());
		CClipboard clipboard;
		if(clipboard.Open(GetWndPtr()->m_hWnd)!=0){	
			clipboard.Empty();
			clipboard.SetData(CF_UNICODETEXT,hGlobal);
			clipboard.Close();
		}
	}
}

void CGridView::UpdateAll()
{
	FilterAll();
	CSheet::UpdateAll();
	UpdateScrolls();
}

void CGridView::EnsureVisibleCell(const std::shared_ptr<CCell>& pCell)
{
	if(!pCell || !pCell->GetRowPtr()->GetIsVisible())return;

	//Page
	CRectF rcPage(GetPageRect());

	if (IsVirtualPage()) {
		//Helper functions
		std::function<bool(FLOAT, FLOAT, FLOAT, FLOAT)> isAllInPage = [](FLOAT min, FLOAT max, FLOAT top, FLOAT bottom)->bool {
			return (top > min&& top < max) &&
				(bottom > min&& bottom < max);
		};

		FLOAT pageHeight = rcPage.Height();
		FLOAT scrollPos = GetVerticalScrollPos();
		//Frozen
		if(pCell->GetRowPtr()->GetIndex<AllTag>() < pCell->GetSheetPtr()->GetFrozenCount<RowTag>() || pCell->GetColumnPtr()->GetIndex<AllTag>() < pCell->GetSheetPtr()->GetFrozenCount<ColTag>()){
			//Do nothing
		//Check if in page
		}  else if (isAllInPage(rcPage.top, rcPage.bottom, m_spCursorer->GetFocusedCell()->GetRowPtr()->GetTop(), m_spCursorer->GetFocusedCell()->GetRowPtr()->GetBottom())) {
			//Do nothing
		//Larger than bottom
		} else if (m_spCursorer->GetFocusedCell()->GetRowPtr()->GetBottom() > rcPage.bottom) {
			FLOAT height = 0.0f;
			FLOAT scroll = 0.0f;
			for (auto iter = m_visRows.begin() + m_spCursorer->GetFocusedCell()->GetRowPtr()->GetIndex<VisTag>(),
					frozen = m_visRows.begin() + m_frozenRowCount, 
					end = m_visRows.begin() + (std::max)(size_t(0), m_frozenRowCount -1);
					iter != end; --iter) {
				height += (*iter)->GetHeight();
				if (height >= pageHeight) {
					FLOAT heightToFocus = std::accumulate(m_visRows.begin() + m_frozenRowCount,
						m_visRows.begin() + m_spCursorer->GetFocusedCell()->GetRowPtr()->GetIndex<VisTag>() + 1, 0.0f,
						[](const FLOAT& acc, const std::shared_ptr<CRow>& ptr)->FLOAT { return acc + ptr->GetVirtualHeight(); });
					scroll = heightToFocus - pageHeight;
					break;
				} else if (iter == frozen) {
					scroll = 0.0f;
					break;
				}
			}
			scroll = std::ceilf(scroll);

			FLOAT top = UpdateCellsRow(rcPage.top - scroll, rcPage.top, rcPage.bottom);
			//Scroll Virtical Range
			if (IsVirtualPage()) {
				m_pVScroll->SetScrollRange(0, GetCellsHeight());
				m_pVScroll->SetScrollPage(rcPage.Height());
				m_pVScroll->SetScrollPos(scroll);
			} else {
				m_pVScroll->SetScrollPos(scroll);
			}
			//Smaller than top
		} else if (m_spCursorer->GetFocusedCell()->GetRowPtr()->GetTop() < rcPage.top) {
			FLOAT scroll = std::accumulate(std::next(m_visRows.begin(), m_frozenRowCount), std::next(m_visRows.begin(), m_spCursorer->GetFocusedCell()->GetRowPtr()->GetIndex<VisTag>()), 0.0f,
				[](const FLOAT& acc, const std::shared_ptr<CRow>& ptr)->FLOAT{ return acc + ptr->GetVirtualHeight(); });
			scroll = std::floorf(scroll);

			FLOAT top = UpdateCellsRow(rcPage.top - scroll, rcPage.top, rcPage.bottom);
			//Scroll Virtical Range
			if (IsVirtualPage()) {
				m_pVScroll->SetScrollRange(0, GetCellsHeight());
				m_pVScroll->SetScrollPage(rcPage.Height());
				m_pVScroll->SetScrollPos(scroll);
			} else {
				m_pVScroll->SetScrollPos(scroll);
			}
		}
	} else {
		auto rcCell(pCell->GetRectInWnd());
		FLOAT vScrollAdd = 0;
		//Bottom has priority (Bottom can Overwrite ScrollPos)

		if (rcCell.bottom > rcPage.bottom) {
			vScrollAdd = rcCell.bottom - rcPage.bottom;
		} else if (rcCell.top < rcPage.top) {
			vScrollAdd = (std::max)(rcCell.top - rcPage.top, rcCell.bottom - rcPage.bottom);
		}

		if (vScrollAdd) {
			m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() + vScrollAdd);
		}
	}
	auto rcCell(pCell->GetRectInWnd());
	FLOAT hScrollAdd = 0;

	//Right has priority (Right can Overwrite ScrollPos)

	if (rcCell.right > rcPage.right) {
		hScrollAdd = rcCell.right - rcPage.right;
	} else if (rcCell.left < rcPage.left) {
		hScrollAdd = (std::max)(rcCell.left - rcPage.left, rcCell.right - rcPage.right);
	}

	if (hScrollAdd) {
		m_pHScroll->SetScrollPos(m_pHScroll->GetScrollPos() + hScrollAdd);
	}
}

void CGridView::Jump(const std::shared_ptr<CCell>& spCell)
{
	if (auto sp = std::dynamic_pointer_cast<CTextCell>(spCell); sp && sp->GetEditMode() == EditMode::LButtonDownEdit) {
		sp->OnEdit(Event(GetWndPtr()));
	} else {
		m_spCursorer->OnCursor(spCell);
	}
	PostUpdate(Updates::EnsureVisibleFocusedCell);
}

CRectF CGridView::GetPaintRect()
{
	return GetRectInWnd();
}

std::pair<bool, bool> CGridView::GetHorizontalVerticalScrollNecessity()
{
	CRectF rcClient(GetRectInWnd());
	CRectF rcCells(GetCellsRect());
	CPointF ptOrigin(GetFrozenPoint());
	//First
	bool bEnableShowHorizontal = rcCells.right > rcClient.right || rcCells.left < ptOrigin.x ;
	bool bEnableShowVertical = rcCells.bottom > rcClient.bottom || rcCells.top < ptOrigin.y;
	//Second
	bEnableShowVertical = rcCells.bottom > (rcClient.bottom - (bEnableShowHorizontal?GetSystemMetrics(SM_CYHSCROLL):0));
	bEnableShowHorizontal = rcCells.right > (rcClient.right - (bEnableShowVertical?GetSystemMetrics(SM_CXVSCROLL):0));
	//Third
	bEnableShowVertical = rcCells.bottom > (rcClient.bottom - (bEnableShowHorizontal?GetSystemMetrics(SM_CYHSCROLL):0));
	bEnableShowHorizontal = rcCells.right > (rcClient.right - (bEnableShowVertical?GetSystemMetrics(SM_CXVSCROLL):0));

	return std::make_pair(bEnableShowHorizontal, bEnableShowVertical);
}

CRectF CGridView::GetPageRect()
{
	CRectF rcClient(GetRectInWnd());
	CRectF rcCells(GetCellsRect());
	CPointF ptOrigin(GetFrozenPoint());
	//First
	bool bEnableShowVertical = rcCells.bottom > rcClient.bottom || rcCells.top < ptOrigin.y;
	bool bEnableShowHorizontal = rcCells.right > rcClient.right || rcCells.left < ptOrigin.x ;
	//Second
	bEnableShowVertical = rcCells.bottom > (rcClient.bottom - (bEnableShowHorizontal?GetSystemMetrics(SM_CYHSCROLL):0)) || rcCells.top < ptOrigin.y;
	bEnableShowHorizontal = rcCells.right > (rcClient.right - (bEnableShowVertical?GetSystemMetrics(SM_CXVSCROLL):0)) || rcCells.left < ptOrigin.x;
	//Third
	bEnableShowVertical = rcCells.bottom > (rcClient.bottom - (bEnableShowHorizontal?GetSystemMetrics(SM_CYHSCROLL):0)) || rcCells.top < ptOrigin.y;
	bEnableShowHorizontal = rcCells.right > (rcClient.right - (bEnableShowVertical?GetSystemMetrics(SM_CXVSCROLL):0)) || rcCells.left < ptOrigin.x;

	return CRectF(ptOrigin.x, ptOrigin.y,
		rcClient.right - (bEnableShowVertical?GetSystemMetrics(SM_CXVSCROLL):0),
		rcClient.bottom - (bEnableShowHorizontal?GetSystemMetrics(SM_CYHSCROLL):0));
}

void CGridView::SubmitUpdate()
{
	//Prevent nest call
	if (m_isUpdating) {
		return;
	} else {
		m_isUpdating = true;
		for (const auto& type : m_setUpdate) {
			switch (type) {
			case Updates::Sort:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::Sort");
				SortAllInSubmitUpdate();
				break;
			}
			case Updates::Filter:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::Filter");
					FilterAll();
				break;
			}
			case Updates::RowVisible:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::RowVisible");
					UpdateRowVisibleDictionary();
				break;
			}
			case Updates::ColumnVisible:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::ColumnVisible");
					UpdateColumnVisibleDictionary();
				break;
			}
			case Updates::Column:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::Column");
					UpdateColumn();
				break;
			}
			case Updates::Row:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::Row");
					UpdateRow();
				break;
			}
			case Updates::Scrolls:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::Scrolls");
					UpdateScrolls();
				break;
			}
			case Updates::EnsureVisibleFocusedCell:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::EnsureVisibleFocusedCell");
					EnsureVisibleCell(m_spCursorer->GetFocusedCell());
					UpdateRow();
					UpdateColumn();
				break;
			}
			case Updates::Invalidate:
				Invalidate();
				break;
			default:
				break;
			}
		}
	m_setUpdate.clear();
	m_isUpdating = false;
	}
}

void CGridView::Clear()
{
	CSheet::Clear();
	//m_rowHeaderHeader = std::shared_ptr<CRow>();
	//m_rowNameHeader=std::shared_ptr<CRow>();
	//m_rowFilter=std::shared_ptr<CRow>();
}

CColumn* CGridView::GetParentColumnPtr(CCell* pCell)
{
	return pCell->GetColumnPtr();
}

void CGridView::OnCommandFind(const CommandEvent& e)
{
	//CFindDlg* pDlg = new CFindDlg(this);
	//pDlg->Create(GetWndPtr()->m_hWnd);
	//pDlg->ShowWindow(SW_SHOW);
}

void CGridView::FindNext(const std::wstring& findWord, bool matchCase, bool matchWholeWord)
{
	//Fiding start from focused cell
	//If focused cell is invalid(Not focused), MinMax Visible Cell is start point
	auto focused = m_spCursorer->GetFocusedCell();

	//Scan from Min to Max and Jump if Find
	auto jumpToFindNextCell = [&](auto rowIter, auto colIter, auto rowEnd, auto colEnd)->bool{
		while(1){

			auto spCell = Cell(*rowIter, *colIter);
			auto str = spCell->GetString();

			if(my::find(str, findWord, matchCase, matchWholeWord)){
				Jump(spCell);
				return true;
			}else{
				colIter++;
				if(colIter==colEnd){
					colIter = m_visCols.begin();
					rowIter++;
				}
				if(rowIter==rowEnd){
					return false;
				}
			}
		}
	};


	index_vector<std::shared_ptr<CRow>>::iterator rIter, rEnd;
	index_vector<std::shared_ptr<CColumn>>::iterator cIter, cEnd;
	//Find word from Min to Max

	if(!focused){
		rIter = m_visRows.begin();
		cIter = m_visCols.begin();
	}else{
		rIter = std::next(m_visRows.begin(), (focused->GetRowPtr()->GetIndex<VisTag>()));
		cIter = std::next(m_visCols.begin(), (focused->GetColumnPtr()->GetIndex<VisTag>()));
		cIter++;
		if(cIter==m_visCols.end()){
			cIter = m_visCols.begin();
			rIter++;
		}
	}
	rEnd = m_visRows.end();
	cEnd = m_visCols.end();
	if(rIter!=rEnd && cIter!=cEnd && jumpToFindNextCell(rIter, cIter, rEnd, cEnd)){
		return;
	}
	//Find word from begining to Min
	//If focused cell is invalid(Not focused), all range is already searched.
	if(!focused){
		rIter = m_visRows.begin();
		rEnd = m_visRows.end();
		cIter = m_visCols.begin();
		cEnd = m_visCols.end();

		if(jumpToFindNextCell(rIter, cIter, rEnd, cEnd)){
			return;
		}
	}

	GetWndPtr()->MessageBox((L"\"" + findWord + L"\" is not found!").c_str(), L"Find",MB_OK);

}

void CGridView::FindPrev(const std::wstring& findWord, bool matchCase, bool matchWholeWord)
{
	//Fiding start from focused cell
	//If focused cell is invalid(Not focused), MinMax Visible Cell is start point
	auto focused = m_spCursorer->GetFocusedCell();

	//Scan from Min to Max and Jump if Find
	auto jumpToFindPrevCell = [&](auto rowIter, auto colIter, auto rowEnd, auto colEnd)->bool{
		while(1){

			auto spCell = Cell(*rowIter, *colIter);
			auto str = spCell->GetString();

			if(my::find(str, findWord, matchCase, matchWholeWord)){
				Jump(spCell);
				return true;
			}else{
				colIter++;
				if(colIter==colEnd){
					colIter = m_visCols.rbegin();
					rowIter++;
				}
				if(rowIter==rowEnd){
					return false;
				}
			}
		}
	};


	index_vector<std::shared_ptr<CRow>>::reverse_iterator rIter, rEnd;
	index_vector<std::shared_ptr<CColumn>>::reverse_iterator cIter, cEnd;
	//Find word from Min to Max

	if(!focused){
		rIter = m_visRows.rbegin();
		cIter = m_visCols.rbegin();
	}else{
		//In case of reverse_iterator, one iterator plused. Therefore it is necessary to minus.
		rIter = std::next(m_visRows.rbegin(), focused->GetRowPtr()->GetIndex<VisTag>());
		cIter = std::next(m_visCols.rbegin(), focused->GetColumnPtr()->GetIndex<VisTag>());
		rIter--;
		cIter--;
		cIter++;
		if(cIter==m_visCols.rend()){
			cIter = m_visCols.rbegin();
			rIter++;
		}
	}
	rEnd = m_visRows.rend();
	cEnd = m_visCols.rend();
	if(rIter!=rEnd && cIter!=cEnd && jumpToFindPrevCell(rIter, cIter, rEnd, cEnd)){
		return;
	}
	//Find word from begining to Min
	//If focused cell is invalid(Not focused), all range is already searched.
	if(!focused){
		rIter = m_visRows.rbegin();
		rEnd = m_visRows.rend();
		cIter = m_visCols.rbegin();
		cEnd = m_visCols.rend();

		if(jumpToFindPrevCell(rIter, cIter, rEnd, cEnd)){
			return;
		}
	}

	GetWndPtr()->MessageBox((L"\"" + findWord + L"\" is not found!").c_str(), L"Find",MB_OK);

}

/**********/
/* Normal */
/**********/
void CGridView::Normal_Paint(const PaintEvent& e)
{
	if (!Visible())return;

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_spGridViewProp->BackgroundPropPtr->m_brush), GetRectInWnd());
	CSheet::Normal_Paint(e);

	//Paint Focused Line
	if (GetIsFocused() ){
		CRectF rcFocus(GetRectInWnd());
		rcFocus.DeflateRect(1.0f, 1.0f);
		GetWndPtr()->GetDirectPtr()->DrawSolidRectangleByLine(*(GetHeaderProperty()->FocusedLine), rcFocus);
	}

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PopAxisAlignedClip();
}

void CGridView::Normal_LButtonDown(const LButtonDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	CSheet::Normal_LButtonDown(e);
}
void CGridView::Normal_LButtonUp(const LButtonUpEvent& e)
{
	//setcapture	ReleaseCapture();

	CSheet::Normal_LButtonUp(e);
}
void CGridView::Normal_LButtonClk(const LButtonClkEvent& e)
{
	CSheet::Normal_LButtonClk(e);
}
void CGridView::Normal_LButtonSnglClk(const LButtonSnglClkEvent& e)
{
	CSheet::Normal_LButtonSnglClk(e);
}
void CGridView::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
	CSheet::Normal_LButtonDblClk(e);
}
void CGridView::Normal_RButtonDown(const RButtonDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	CSheet::Normal_RButtonDown(e);
}
void CGridView::Normal_MouseMove(const MouseMoveEvent& e)
{
	//TrackMouseEvent
	TRACKMOUSEEVENT stTrackMouseEvent;
	stTrackMouseEvent.cbSize = sizeof(stTrackMouseEvent);
	stTrackMouseEvent.dwFlags = TME_LEAVE;
	stTrackMouseEvent.hwndTrack = GetWndPtr()->m_hWnd;
	::TrackMouseEvent(&stTrackMouseEvent);

	CSheet::Normal_MouseMove(e);
}
void CGridView::Normal_MouseLeave(const MouseLeaveEvent& e)
{
	CSheet::Normal_MouseLeave(e);
}
bool CGridView::Normal_Guard_SetCursor(const SetCursorEvent& e)
{
	return CSheet::Normal_Guard_SetCursor(e);
}
void CGridView::Normal_SetCursor(const SetCursorEvent& e)
{
	CSheet::Normal_SetCursor(e);
}
void CGridView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	if (!Visible())return;
	auto cell = Cell(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (!cell) {
		CMenu* pMenu = GetContextMenuPtr();
		if (pMenu) {
			HWND hWnd = GetWndPtr()->m_hWnd;
			SetContextMenuRowColumn(CRowColumn());
			::SetForegroundWindow(hWnd);
			pMenu->TrackPopupMenu(0, e.PointInScreen.x, e.PointInScreen.y, hWnd);
		}
	} else {
		CSheet::Normal_ContextMenu(e);
	}
}

std::shared_ptr<CCell> CGridView::TabNext(const std::shared_ptr<CCell>& spCurCell)
{
	if (m_gridViewMode == GridViewMode::None) {
		std::pair<size_t, size_t> firstIndexes = std::make_pair(0, 0);
		std::shared_ptr<CCell> spJumpCell(nullptr);

		// Find first indexes
		if (!spCurCell) {
			// (0, 0)
		} else {
			// Current Indexes
			firstIndexes = std::make_pair(spCurCell->GetRowPtr()->GetIndex<VisTag>(), spCurCell->GetColumnPtr()->GetIndex<VisTag>());

		}
		// Search
		for (auto iter = m_frozenTabStops.cbegin(); iter != m_frozenTabStops.cend(); ++iter) {
			if ((iter->first * iter->second + iter->second) > (firstIndexes.first * firstIndexes.second + firstIndexes.second)) {
				auto spCell = Cell<VisTag>(iter->first, iter->second);
				if (spCell) {
					spJumpCell = spCell;
					break;
				}
			}
		}
		return spJumpCell;
	} else if (m_gridViewMode == GridViewMode::ExcelLike) {
		int row_count = m_visRows.size() - m_frozenRowCount;
		int col_count = m_visCols.size() - m_frozenColumnCount;
		int row_cur = spCurCell->GetRowPtr()->GetIndex<VisTag>() - m_frozenRowCount;
		int col_cur = spCurCell->GetColumnPtr()->GetIndex<VisTag>() - m_frozenColumnCount;
		if ((row_cur == row_count - 1 && col_cur == col_count - 1) ||
			(row_cur < 0 || col_cur < 0)){
			return Cell<VisTag>(0, 0);
		} else {
			int sum = (row_cur * col_count) + (col_cur + 1);
			int row_new = sum / col_count;
			int col_new = sum % col_count;
			return Cell<VisTag>(row_new + m_frozenRowCount, col_new + m_frozenColumnCount);
		}
	} else {
		return Cell<VisTag>(0, 0);
	}
}
std::shared_ptr<CCell> CGridView::TabPrev(const std::shared_ptr<CCell>& spCurCell)
{
	std::pair<size_t, size_t> firstIndexes = std::make_pair(0, 0);
	std::shared_ptr<CCell> spJumpCell(nullptr);

	// Find first indexes
	if (!spCurCell) {
		// (0, 0)
	} else {
		// Current Indexes
		firstIndexes = std::make_pair(spCurCell->GetRowPtr()->GetIndex<VisTag>(), spCurCell->GetColumnPtr()->GetIndex<VisTag>());

	}
	// Search
	for (auto iter = m_frozenTabStops.crbegin(); iter != m_frozenTabStops.crend(); ++iter) {
		if ((iter->first * iter->second + iter->second)  < (firstIndexes.first * firstIndexes.second + firstIndexes.second)) {
			auto spCell = Cell<VisTag>(iter->first, iter->second);
			if (spCell) {
				spJumpCell = spCell;
				break;
			}
		}
	}
	return spJumpCell;
}


void CGridView::Normal_KeyDown(const KeyDownEvent& e)
{
	switch (e.Char) {
	case VK_TAB:
		{
			std::shared_ptr<CCell> spCurCell = m_spCursorer->GetCurrentCell();
			auto shift = ::GetAsyncKeyState(VK_SHIFT);
			std::shared_ptr<CCell> spJumpCell = shift ? TabPrev(spCurCell) : TabNext(spCurCell);

			if (spJumpCell) {
				(*e.HandledPtr) = true;
				Jump(spJumpCell);
			}
		}
		break;
	case VK_HOME:
		m_pVScroll->SetScrollPos(0);
		(*e.HandledPtr) = true;
		break;
	case VK_END:
		m_pVScroll->SetScrollPos(m_pVScroll->GetScrollRange().second - m_pVScroll->GetScrollPage());
		(*e.HandledPtr) = true;
		break;
	case VK_PRIOR: // Page Up
		m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollPage());
		(*e.HandledPtr) = true;
		break;
	case VK_NEXT: // Page Down
		m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() + m_pVScroll->GetScrollPage());
		(*e.HandledPtr) = true;
		break;
	default:
		break;
	}	
	if (!(*e.HandledPtr)) {
		CSheet::Normal_KeyDown(e);
	}
}

/***************/
/* VScrollDrag */
/***************/
void CGridView::VScrlDrag_OnEntry()
{
	m_pVScroll->SetState(UIElementState::Dragged);
}
void CGridView::VScrlDrag_OnExit()
{
	m_pVScroll->SetStartDrag(0.f);
	m_pVScroll->SetState(UIElementState::Normal);
}
void CGridView::VScrlDrag_LButtonDown(const LButtonDownEvent& e)
{
	//setcaptureSetCapture();
	m_pVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}
bool CGridView::VScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_pVScroll->GetIsVisible() && m_pVScroll->GetThumbRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}
void CGridView::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pVScroll->SetScrollPos(
		m_pVScroll->GetScrollPos() +
		(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y) - m_pVScroll->GetStartDrag()) *
		m_pVScroll->GetScrollDistance() /
		m_pVScroll->GetRectInWnd().Height());
	m_pVScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsY(e.PointInClient.y));
}

/***************/
/* HScrollDrag */
/***************/
void CGridView::HScrlDrag_OnEntry()
{
	m_pHScroll->SetState(UIElementState::Dragged);
}

void CGridView::HScrlDrag_OnExit()
{
	m_pHScroll->SetStartDrag(0.f);
	m_pHScroll->SetState(UIElementState::Normal);
}

void CGridView::HScrlDrag_LButtonDown(const LButtonDownEvent& e)
{
	m_pHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

bool CGridView::HScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_pHScroll->GetIsVisible() && m_pHScroll->GetThumbRect().PtInRect(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
}

void CGridView::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pHScroll->SetScrollPos(
		m_pHScroll->GetScrollPos() +
		(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x) - m_pHScroll->GetStartDrag()) *
		m_pHScroll->GetScrollDistance() /
		m_pHScroll->GetRectInWnd().Width());
	m_pHScroll->SetStartDrag(GetWndPtr()->GetDirectPtr()->Pixels2DipsX(e.PointInClient.x));
}

/********/
/* Edit */
/********/
void CGridView::Edit_OnEntry(const BeginEditEvent& e)
{
	::OutputDebugStringA("Edit_OnEntry\r\n");

	if (auto pCell = dynamic_cast<CTextCell*>(e.CellPtr)) {

		SetEditPtr(std::make_shared<CCellTextBox>(
			this,
			pCell->GetCellPropertyPtr(),
			pCell->GetString(),
			pCell,
			[pCell](const std::basic_string<TCHAR>& str) -> void {
				if (pCell->CanSetStringOnEditing()) {
					pCell->SetString(str);
				} else {
					if (pCell->GetString() != str) {
						pCell->OnPropertyChanged(L"value");
					}
				}
			},
			[pCell](const std::basic_string<TCHAR>& str)->void {
				// Need to EditPtr to nullptr first. Otherwise exception occur
				//pCell->GetSheetPtr()->GetGridPtr()->SetEditPtr(nullptr);
				pCell->SetString(str);
				//pCell->SetState(UIElementState::Normal);//After Editing, Change Normal
			}));
		GetEditPtr()->OnCreate(CreateEvt(GetWndPtr(), this, pCell->GetRectInWnd()));
		SetFocusedControlPtr(GetEditPtr());
		PostUpdate(Updates::Invalidate);
		//SetCapture();
	}
	m_isEditExiting = false;
}

void CGridView::Edit_OnExit()
{
	::OutputDebugStringA("Edit_OnExit\r\n");
	//To avoid situation Edit_OnExit->OnKillFocus->Edit_OnExit
	if (!m_isEditExiting){
		m_isEditExiting = true;
		GetEditPtr()->OnClose(CloseEvent(GetWndPtr(), NULL, NULL, nullptr));
		SetEditPtr(nullptr);
		m_isEditExiting = false;
	}
}

void CGridView::Edit_MouseMove(const MouseMoveEvent& e)
{
	if (GetEditPtr()->GetRectInWnd().PtInRect(e.PointInWnd)) {
		GetEditPtr()->OnMouseMove(e);
	} else {
		Normal_MouseMove(e);
	}
}

bool CGridView::Edit_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return !GetEditPtr()->GetRectInWnd().PtInRect(e.PointInWnd);
}

void CGridView::Edit_LButtonDown(const LButtonDownEvent& e)
{
	if (GetEditPtr()->GetRectInWnd().PtInRect(e.PointInWnd)) {
		GetEditPtr()->OnLButtonDown(e);
	}
}

void CGridView::Edit_LButtonUp(const LButtonUpEvent& e)
{
	if (GetEditPtr()->GetRectInWnd().PtInRect(e.PointInWnd)) {
		GetEditPtr()->OnLButtonUp(e);
	} else {
		Normal_LButtonUp(e);
	}
}

void CGridView::Edit_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	if (GetEditPtr()->GetRectInWnd().PtInRect(e.PointInWnd)) {
		GetEditPtr()->OnLButtonBeginDrag(e);
	}
}
void CGridView::Edit_LButtonEndDrag(const LButtonEndDragEvent& e)
{
	GetEditPtr()->OnLButtonEndDrag(e);
}


// Edit to Normal to Edit
// Edit to Normal
//     Edit_Guard_KeyDown_Tab:true, Edit_KeyDonw_Tab:TRUE
//     Edit_Guard_KeyDown_ToNormal
// Edit to Other Control
//     Edit_Guard_KeyDown_Tab:true, Edit_KeyDonw_Tab:TRUE
// Normal to Edit
// Normal to Normal
// Normal to Other Control

void CGridView::Edit_KeyDown_Tab(const KeyDownEvent& e, boost::sml::back::process<BeginEditEvent> process)
{
	if (m_pJumpCell) {
		Jump(m_pJumpCell);
		m_pJumpCell.reset();
		PostUpdate(Updates::EnsureVisibleFocusedCell);
		SubmitUpdate();
		(*e.HandledPtr) = TRUE;
	}
}

bool CGridView::Edit_Guard_KeyDown_ToNormal_Tab(const KeyDownEvent& e, boost::sml::back::process<BeginEditEvent> process)
{
	if (e.Char == VK_TAB && !(::GetKeyState(VK_MENU) & 0x8000)) {
		//Commit Edit & Move to next Cell
		std::shared_ptr<CCell> spCurCell = std::dynamic_pointer_cast<CCell>(GetEditPtr()->GetCellPtr()->shared_from_this());
		auto shift = ::GetAsyncKeyState(VK_SHIFT);
		m_pJumpCell = shift ? TabPrev(spCurCell) : TabNext(spCurCell); // Set fro action
		return true;
	}
	return false;
}

bool CGridView::Edit_Guard_KeyDown_ToNormal(const KeyDownEvent& e)
{
	if (e.Char == VK_RETURN && !(::GetKeyState(VK_MENU) & 0x8000)) {
		//Commit Edit
		*(e.HandledPtr) = TRUE;
		return true;
	} else if (e.Char == VK_ESCAPE) {
		//CancelEdit
		GetEditPtr()->CancelEdit();
		*(e.HandledPtr) = TRUE;
		return true;
	} else {
		return false;
	}
}

void CGridView::Edit_KeyDown(const KeyDownEvent& e)
{
	if (!(*e.HandledPtr)) {
		GetEditPtr()->OnKeyDown(e);
	}
}

void CGridView::Edit_Char(const CharEvent& e)
{
	if (!(*e.HandledPtr)) {
		GetEditPtr()->OnChar(e);
	}
}


/******************/
/* Window Message */
/******************/
void CGridView::BeginEdit(CCell* pCell)
{
	OnBeginEdit(BeginEditEvent(GetWndPtr(), pCell));
	return;
}

void CGridView::EndEdit()
{
	OnEndEdit(EndEditEvent(GetWndPtr()));
	return;
}


/**************/
/* UI Message */
/**************/
void CGridView::OnFilter()
{
	FilterAll();
	SubmitUpdate();
}

void CGridView::OnDelayUpdate()
{
	LOG_SCOPED_TIMER_THIS_1("OnDelayUpdate Total");
	SignalPreDelayUpdate();
	SignalPreDelayUpdate.disconnect_all();
	PostUpdate(Updates::Filter);

	if (m_keepEnsureVisibleFocusedCell) {
		PostUpdate(Updates::EnsureVisibleFocusedCell);
	}
	else {
		m_setUpdate.erase(Updates::EnsureVisibleFocusedCell);
	}
	SubmitUpdate();
}


void CGridView::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	UpdateAll();
	SubmitUpdate();
}


void CGridView::OnRect(const RectEvent& e)
{
	OnRectWoSubmit(e);
	SubmitUpdate();
}

void CGridView::OnRectWoSubmit(const RectEvent& e)
{
	CRectF prevRect = m_rect;
	CSheet::OnRect(e);

	if (prevRect.left != m_rect.left) {
		PostUpdate(Updates::Column);
	}
	if (prevRect.top != m_rect.top) {
		PostUpdate(Updates::Row);
	}

	SizeChanged();
}

void CGridView::OnPaint(const PaintEvent& e)
{
	if (!Visible()) { return; }
	//GetWndPtr()->GetDirectPtr()->ClearSolid(*(m_spGridViewProp->BackgroundPropPtr->m_brush));
	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(*(m_spGridViewProp->BackgroundPropPtr->m_brush), GetRectInWnd());
	
	CSheet::OnPaint(e);

	if (m_pEdit && m_pEdit->GetIsVisible()) {
		m_pEdit->OnPaint(e);
	}
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);
	SubmitUpdate();
}

void CGridView::OnMouseWheel(const MouseWheelEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollDelta() * e.Delta / WHEEL_DELTA);
	SubmitUpdate();
}

void CGridView::OnSetFocus(const SetFocusEvent& e)
{
	CSheet::OnSetFocus(e);
	SubmitUpdate();
}

void CGridView::OnKillFocus(const KillFocusEvent& e)
{
	CSheet::OnKillFocus(e);
	SubmitUpdate();
}

void CGridView::OnSetCursor(const SetCursorEvent& e)
{
	*e.HandledPtr = FALSE; //Default Handled = FALSE means Arrow
	CSheet::OnSetCursor(e);
	SubmitUpdate();
}

void CGridView::OnContextMenu(const ContextMenuEvent& e)
{
	*e.HandledPtr = FALSE;
	CSheet::OnContextMenu(e);
	SubmitUpdate();
}
//
//void CGridView::OnCommand(const CommandEvent& e)
//{
//	switch (e.ID) {
//		case ID_HD_COMMAND_EDITHEADER:
//			OnCommandEditHeader(e);
//			break;
//		case ID_HD_COMMAND_DELETECOLUMN:
//			OnCommandDeleteColumn(e);
//			break;
//		case IDM_SELECTALL:
//			OnCommandSelectAll(e);
//			break;
//		case IDM_DELETE:
//			OnCommandDelete(e);
//			break;
//		case IDM_COPY:
//			OnCommandCopy(e);
//			break;
//		//case IDM_CUT:
//		//	OnCommandCut(e);
//		//	break;
//		case IDM_PASTE:
//			OnCommandPaste(e);
//			break;
//		case IDM_FIND:
//			OnCommandFind(e);
//			break;
//	}
//	SubmitUpdate();
//}






