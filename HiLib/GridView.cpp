#include <olectl.h>
#include <numeric>
#include "D2DWControl.h"
#include "Dispatcher.h"
#include "GridView.h"
#include "GridViewStateMachine.h"
#include "Cell.h"
#include "CellProperty.h"
#include "Row.h"
#include "HeaderCell.h"
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

/***********/
/* Default */
/***********/
CGridView::CGridView(
	CD2DWControl* pParentControl)
	:CD2DWControl(pParentControl),
	m_pVScroll(std::make_unique<CVScroll>(this)),
	m_pHScroll(std::make_unique<CHScroll>(this)),
	m_bSelected(false),
	//m_bFocused(false),
	m_pMachine(new CGridStateMachine(this)),
	m_allRows([](std::shared_ptr<CRow>& sp, size_t idx) { sp->SetIndex<AllTag>(idx); }),
	m_visRows([](std::shared_ptr<CRow>& sp, size_t idx) { sp->SetIndex<VisTag>(idx); }),
	m_pntRows([](std::shared_ptr<CRow>& sp, size_t idx) { /* Do Nothing */ }),
	m_allCols([](std::shared_ptr<CColumn>& sp, size_t idx) { sp->SetIndex<AllTag>(idx); }),
	m_visCols([](std::shared_ptr<CColumn>& sp, size_t idx) { sp->SetIndex<VisTag>(idx); }),
	m_pntCols([](std::shared_ptr<CColumn>& sp, size_t idx) { /* Do Nothing */ })

{
	if(!m_spRowTracker){
		m_spRowTracker = std::make_shared<CTracker<RowTag>>();
	}
	if (!m_spColTracker) {
		m_spColTracker = std::make_shared<CTracker<ColTag>>();
	}
	if(!m_spRowDragger){
		m_spRowDragger = std::make_shared<CDragger<RowTag, ColTag>>();
	}
	if (!m_spColDragger) {
		m_spColDragger = std::make_shared<CDragger<ColTag, RowTag>>();
	}
	if (!m_spItemDragger) {
		m_spItemDragger = std::make_shared<CNullDragger>();
	}
	if(!m_spCursorer){
		m_spCursorer = std::make_shared<CCursorer>();
	}
	if (!m_spCeller) {
		m_spCeller = std::make_shared<CCeller>();
	}

	m_pVScroll->ScrollChanged.connect([this](){
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Invalidate);
	});

	m_pHScroll->ScrollChanged.connect([this](){
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Invalidate);
	});
}

CGridView::~CGridView() = default;

/**********/
/* Update */
/**********/
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
				//if (HasSheetCell()) {
				//	for (auto& ptr : m_visCols) {
				//		std::shared_ptr<CCell> pCell = Cell(*rowIter, ptr);
				//		if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
				//			pSheetCell->UpdateAll();
				//		}
				//	}
				//}
				top += (*rowIter)->GetHeight();
			} else {
				top += defaultHeight;
			}
			//std::cout << top << std::endl;
		} else {
			(*rowIter)->SetTop(top, false);
			//if (HasSheetCell()) {
			//	for (auto& ptr : m_visCols) {
			//		std::shared_ptr<CCell> pCell = Cell(*rowIter, ptr);
			//		if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
			//			pSheetCell->UpdateAll();
			//		}
			//	}
			//}
			top += (*rowIter)->GetHeight();
		}
//		prevRowIter = rowIter;
	}
	return top;
}

void CGridView::UpdateRow()
{
	if (!Visible()) { return; }

	FLOAT top = GetRectInWnd().top + GetNormalBorder().Width * 0.5f;

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

	FLOAT left = GetRectInWnd().left + GetNormalBorder().Width * 0.5f;

	for (auto& colPtr : m_visCols) {
		if (colPtr->GetIndex<VisTag>() == GetFrozenCount<ColTag>()) {
			left -= GetHorizontalScrollPos();
		}
		colPtr->SetLeft(left, false);
		for (auto rowPtr : m_visRows) {
			std::shared_ptr<CCell> pCell = Cell(rowPtr, colPtr);
			//if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
			//	pSheetCell->UpdateAll();
			//}
		}
		left += colPtr->GetWidth();
	}
}

void CGridView::UpdateScrolls()
{
	if (!Visible()) { return; }

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
	FLOAT lineHalfWidth = GetNormalBorder().Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_pHScroll->GetIsVisible()?(m_pHScroll->GetBandWidth() + lineHalfWidth) : lineHalfWidth);
	m_pVScroll->Arrange(rcVertical);

	rcHorizontal.left= rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom-::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_pVScroll->GetIsVisible()?(m_pVScroll->GetBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;
	m_pHScroll->Arrange(rcHorizontal);

}

void CGridView::UpdateFilter()
{
	if (GetFilterRowPtr()) {
		//Reset Filter
		for (auto iter = std::next(m_allRows.begin(), m_frozenRowCount), end = m_allRows.end(); iter != end; ++iter) {
			(*iter)->SetIsVisible(true);
		};
		//Set Filter
		for (auto colIter = m_allCols.begin(), colEnd = m_allCols.end(); colIter != colEnd; ++colIter) {
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

void CGridView::UpdateSort()
{
	for (const auto& ptr : m_visCols) {
		if (ptr->GetSort() != Sorts::None) {
			this->Sort(ptr.get(), ptr->GetSort(), false);
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


void CGridView::PostUpdate(Updates type)
{
	switch (type) {
	case Updates::All:
		m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::Row);
		m_setUpdate.insert(Updates::ColumnVisible);
		m_setUpdate.insert(Updates::Column);
		m_setUpdate.insert(Updates::Scrolls);
		m_setUpdate.insert(Updates::Sort);
		m_setUpdate.insert(Updates::Filter);
		m_setUpdate.insert(Updates::Invalidate);
		break;
	case Updates::EnsureVisibleFocusedCell:
		////0506m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::Row);
		////0506m_setUpdate.insert(Updates::ColumnVisible);
		m_setUpdate.insert(Updates::Column);
		m_setUpdate.insert(Updates::Scrolls);
		////0506m_setUpdate.insert(Updates::Sort);
		m_setUpdate.insert(type);
		m_setUpdate.insert(Updates::Invalidate);
		break;
	case Updates::Sort:
		////0506m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::Row);
		m_setUpdate.insert(type);
		m_setUpdate.insert(Updates::Invalidate);
		PostUpdate(Updates::EnsureVisibleFocusedCell);
	case Updates::Filter:
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
		m_setUpdate.insert(type);
	case Updates::RowVisible:
		m_setUpdate.insert(Updates::Sort);
	default:
		m_setUpdate.insert(type);
		break;
	}

}



/********/
/* Cell */
/********/

std::shared_ptr<CCell> CGridView::Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn) 
{
	return Cell(spRow.get(), spColumn.get());
}
std::shared_ptr<CCell> CGridView::Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow) 
{
	return Cell(spRow, spColumn);
}

std::shared_ptr<CCell> CGridView::Cell(const CPointF& pt)
{
	auto rowPtr = Coordinate2Pointer<RowTag>(pt.y);
	auto colPtr = Coordinate2Pointer<ColTag>(pt.x);
	if (rowPtr.get() != nullptr && colPtr.get() != nullptr){
		return Cell(rowPtr, colPtr);
	}
	else {
		return nullptr;
	}
}

/*****************/
/* Getter Setter */
/*****************/

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

/**********/
/* Action */
/**********/

void CGridView::Filter()
{
	UpdateFilter();
	SubmitUpdate();
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

void CGridView::Sort(CColumn* pCol, Sorts sort, bool postUpdate)
{
	//Sort
	switch(sort){
	case Sorts::Down:
		m_visRows.idx_stable_sort(m_visRows.begin() + m_frozenRowCount, m_visRows.end(),
			[pCol, this](const auto& lhs, const auto& rhs)->bool{
				auto cmp = _tcsicmp(Cell(lhs.get(), pCol)->GetSortString().c_str(), Cell(rhs.get(), pCol)->GetSortString().c_str());
				if (cmp > 0) {
					return true;
				} else if (cmp < 0) {
					return false;
				} else {
					if (lhs->GetIndex<AllTag>() == rhs->GetIndex<AllTag>()) {
						::OutputDebugString(std::format(L"{}=={}", lhs->GetIndex<AllTag>(), rhs->GetIndex<AllTag>()).c_str());
					}
					return lhs->GetIndex<AllTag>() > rhs->GetIndex<AllTag>();
				}
			});

		break;
	case Sorts::Up:
		m_visRows.idx_stable_sort(m_visRows.begin() + m_frozenRowCount, m_visRows.end(),
			[pCol, this](const auto& lhs, const auto& rhs)->bool {
				auto cmp = _tcsicmp(Cell(lhs.get(), pCol)->GetSortString().c_str(), Cell(rhs.get(), pCol)->GetSortString().c_str());
				if (cmp < 0) {
					return true;
				} else if (cmp == 0) {
					return lhs->GetIndex<AllTag>() < rhs->GetIndex<AllTag>();
				} else {
					return false;
				}
			});
		break;
	default:
		break;
	}
}

void CGridView::ResetSort()
{
	//Reset Sort Mark
	for(auto& ptr : m_allCols){
		ptr->SetSort(Sorts::None);
	}
}


void CGridView::Filter(int colDisp,std::function<bool(const std::shared_ptr<CCell>&)> predicate)
{
	for(auto rowIter= m_allRows.begin() + m_frozenRowCount;rowIter!=m_allRows.end(); ++rowIter){
		if(predicate(Cell(*rowIter, m_allCols[colDisp]))){
			(*rowIter)->SetIsVisible(true);
		}else{
			(*rowIter)->SetIsVisible(false);		
		}
	}
}

void CGridView::ResetFilter()
{
	auto setVisible = [](const auto& ptr) {ptr->SetIsVisible(true); };
	std::for_each(m_allRows.begin(), m_allRows.end(), setVisible);
	std::for_each(m_allCols.begin(), m_allCols.end(), setVisible);
}





void CGridView::OnCommandEditHeader(const CommandEvent& e)
{
	if(!m_rocoContextMenu.IsInvalid()){
		if(m_rocoContextMenu.GetRowPtr()==GetNameHeaderRowPtr()){
			if(auto pCell=std::dynamic_pointer_cast<CHeaderCell>(Cell(m_rocoContextMenu.GetRowPtr(), m_rocoContextMenu.GetColumnPtr()))){
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
//
//void CGridView::OnCommandResizeSheetCell(const CommandEvent& e)
//{
//	if(!m_rocoContextMenu.IsInvalid()){
//		if(auto p = std::dynamic_pointer_cast<CSheetCell>(Cell(m_rocoContextMenu.GetRowPtr(),m_rocoContextMenu.GetColumnPtr()))){
//			p->Resize();
//		}
//	}
//}


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
	UpdateFilter();
	UpdateRowVisibleDictionary();//TODO today
	UpdateColumnVisibleDictionary();
	UpdateColumn();
	UpdateRow();	
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
		if(pCell->GetRowPtr()->GetIndex<AllTag>() < pCell->GetGridPtr()->GetFrozenCount<RowTag>() || pCell->GetColumnPtr()->GetIndex<AllTag>() < pCell->GetGridPtr()->GetFrozenCount<ColTag>()){
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
			//Scroll Vertical Range
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
				UpdateSort();
				break;
			}
			case Updates::Filter:
			{
				LOG_SCOPED_TIMER_THIS_1("Updates::Filter");
					UpdateFilter();
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
/****************/
/* StateMachine */
/****************/
/**********/
/* Normal */
/**********/
void CGridView::Normal_Paint(const PaintEvent& e)
{
	if (!Visible())return;

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PushAxisAlignedClip(GetRectInWnd(), D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_ALIASED);

	GetWndPtr()->GetDirectPtr()->FillSolidRectangle(GetNormalBackground(), GetRectInWnd());

	UpdateRowPaintDictionary();
	UpdateColumnPaintDictionary();

	//Paint Cells
	for (auto rowIter = m_pntRows.rbegin(), rowEnd = m_pntRows.rend(); rowIter != rowEnd; ++rowIter) {
		for (auto colIter = m_pntCols.rbegin(), colEnd = m_pntCols.rend(); colIter != colEnd; ++colIter) {
			Cell(*rowIter, *colIter)->OnPaint(e);
		}
	}
	//Paint Drag Target Line
	m_spRowDragger->OnPaintDragLine(this, e);
	m_spColDragger->OnPaintDragLine(this, e);
	if (m_spItemDragger) { m_spItemDragger->OnPaintDragLine(this, e); }

	ProcessMessageToAll(&CD2DWControl::OnPaint, e);

	if (m_pEdit && m_pEdit->GetIsVisible()) {
		m_pEdit->OnPaint(e);
	}
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);

	GetWndPtr()->GetDirectPtr()->GetD2DDeviceContext()->PopAxisAlignedClip();
}
void CGridView::Normal_LButtonDown(const LButtonDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	m_spCeller->OnLButtonDown(this, e);
	m_spCursorer->OnLButtonDown(this, e);

}
void CGridView::Normal_LButtonUp(const LButtonUpEvent& e)
{
	m_spCursorer->OnLButtonUp(this, e);
	m_spCeller->OnLButtonUp(this, e);
}

void CGridView::Normal_LButtonClk(const LButtonClkEvent& e)
{
	m_spCeller->OnLButtonClk(this, e);
}

void CGridView::Normal_LButtonSnglClk(const LButtonSnglClkEvent& e)
{
	m_spCeller->OnLButtonSnglClk(this, e);
}

void CGridView::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
	if (m_spColTracker->IsTarget(this, e)) {
		m_spColTracker->OnDividerDblClk(this, e);
	}
	else if (m_spRowTracker->IsTarget(this, e)) {
		m_spRowTracker->OnDividerDblClk(this, e);
	}
	else {
		m_spCursorer->OnLButtonDblClk(this, e);
		m_spCeller->OnLButtonDblClk(this, e);
	}
}

void CGridView::Normal_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	m_spCeller->OnLButtonBeginDrag(this, e);
}

void CGridView::Normal_RButtonDown(const RButtonDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	m_spCursorer->OnRButtonDown(this, e);
}

void CGridView::Normal_MouseMove(const MouseMoveEvent& e)
{
	//TrackMouseEvent
	TRACKMOUSEEVENT stTrackMouseEvent;
	stTrackMouseEvent.cbSize = sizeof(stTrackMouseEvent);
	stTrackMouseEvent.dwFlags = TME_LEAVE;
	stTrackMouseEvent.hwndTrack = GetWndPtr()->m_hWnd;
	::TrackMouseEvent(&stTrackMouseEvent);

	m_spCeller->OnMouseMove(this, e);
}

void CGridView::Normal_MouseLeave(const MouseLeaveEvent& e)
{
	m_spCeller->OnMouseLeave(this, e);
}

bool CGridView::Normal_Guard_SetCursor(const SetCursorEvent& e)
{
	return e.HitTest == HTCLIENT;
}

void CGridView::Normal_SetCursor(const SetCursorEvent& e)
{
	if (!*e.HandledPtr) { 
		m_spRowTracker->OnSetCursor(this, e);
	}
	if (!*e.HandledPtr) {
		m_spColTracker->OnSetCursor(this, e);
	}
	if (!*e.HandledPtr) {
		m_spCeller->OnSetCursor(this, e);
	}
}

void CGridView::Normal_ContextMenu(const ContextMenuEvent& e){}

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
			return Cell<VisTag>(m_frozenRowCount, m_frozenColumnCount);
		} else {
			int sum = (row_cur * col_count) + (col_cur + 1);
			int row_new = sum / col_count;
			int col_new = sum % col_count;
			return Cell<VisTag>(row_new + m_frozenRowCount, col_new + m_frozenColumnCount);
		}
	} else {
		return Cell<VisTag>(m_frozenRowCount, m_frozenColumnCount);
	}
}

std::shared_ptr<CCell> CGridView::TabPrev(const std::shared_ptr<CCell>& spCurCell)
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
		for (auto iter = m_frozenTabStops.crbegin(); iter != m_frozenTabStops.crend(); ++iter) {
			if ((iter->first * iter->second + iter->second) < (firstIndexes.first * firstIndexes.second + firstIndexes.second)) {
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
		if ((row_cur == 0 && col_cur == 0) ||
			(row_cur < 0 || col_cur < 0)){
			return Cell<VisTag>(row_count - 1 + m_frozenRowCount, col_count - 1+ m_frozenColumnCount);
		} else {
			int sum = (row_cur * col_count) + (col_cur - 1);
			int row_new = sum / col_count;
			int col_new = sum % col_count;
			return Cell<VisTag>(row_new + m_frozenRowCount, col_new + m_frozenColumnCount);
		}
	} else {
		return Cell<VisTag>(m_frozenRowCount, m_frozenColumnCount);
	}
}

void CGridView::Normal_KeyDown(const KeyDownEvent& e)
{
	switch (e.Char) {
	case VK_TAB:
		{
		if (std::shared_ptr<CCell> spCurCell = m_spCursorer->GetCurrentCell()) {
			auto shift = ::GetAsyncKeyState(VK_SHIFT);
			std::shared_ptr<CCell> spJumpCell = shift ? TabPrev(spCurCell) : TabNext(spCurCell);

			if (spJumpCell) {
				Jump(spJumpCell);
				(*e.HandledPtr) = true;
			}
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
		m_spCursorer->OnKeyDown(this, e);
		m_spCeller->OnKeyDown(this, e);
	}
}

void CGridView::Normal_KeyTraceDown(const KeyTraceDownEvent& e)
{
	m_spCeller->OnKeyTraceDown(this, e);
}

void CGridView::Normal_KeyTraceUp(const KeyTraceUpEvent& e){}

void CGridView::Normal_Char(const CharEvent& e)
{
	m_spCeller->OnChar(this, e);
}

void CGridView::Normal_ImeStartComposition(const ImeStartCompositionEvent& e)
{
	m_spCeller->OnImeStartComposition(this, e);
}

void CGridView::Normal_SetFocus(const SetFocusEvent& e)
{
	if (e.HandledPtr) { *e.HandledPtr = FALSE; }
}

void CGridView::Normal_KillFocus(const KillFocusEvent& e)
{
	if (e.HandledPtr) { *e.HandledPtr = FALSE; }
}
/************/
/* RowTrack */
/************/
void CGridView::RowTrack_LButtonDown(const LButtonDownEvent& e)
{
	m_spRowTracker->OnBeginTrack(this, e);
}

bool CGridView::RowTrack_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_spRowTracker->IsTarget(this, e);
}

void CGridView::RowTrack_MouseMove(const MouseMoveEvent& e)
{
	m_spRowTracker->OnTrack(this, e);
}

void CGridView::RowTrack_LButtonUp(const LButtonUpEvent& e)
{
	m_spRowTracker->OnEndTrack(this, e);
}

void CGridView::RowTrack_MouseLeave(const MouseLeaveEvent& e)
{
	m_spRowTracker->OnLeaveTrack(this, e);
}

/************/
/* ColTrack */
/************/
void CGridView::ColTrack_LButtonDown(const LButtonDownEvent& e)
{
	m_spColTracker->OnBeginTrack(this, e);
}

bool CGridView::ColTrack_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_spColTracker->IsTarget(this, e);
}

void CGridView::ColTrack_MouseMove(const MouseMoveEvent& e)
{
	m_spColTracker->OnTrack(this, e);
}

void CGridView::ColTrack_LButtonUp(const LButtonUpEvent& e)
{
	m_spColTracker->OnEndTrack(this, e);
}

void CGridView::ColTrack_MouseLeave(const MouseLeaveEvent& e)
{
	m_spColTracker->OnLeaveTrack(this, e);
}

/***********/
/* RowDrag */
/***********/
void CGridView::RowDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spRowDragger->OnBeginDrag(this, e);
}

void CGridView::RowDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spRowDragger->OnEndDrag(this, e);
}

bool CGridView::RowDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spRowDragger->IsTarget(this, e);
}

void CGridView::RowDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spRowDragger->OnDrag(this, e);
}

void CGridView::RowDrag_MouseLeave(const MouseLeaveEvent& e)
{
	m_spRowDragger->OnLeaveDrag(this, e);
}
/***********/
/* ColDrag */
/***********/
void CGridView::ColDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spColDragger->OnBeginDrag(this, e);
}

void CGridView::ColDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spColDragger->OnEndDrag(this, e);
}

bool CGridView::ColDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spColDragger->IsTarget(this, e);
}

void CGridView::ColDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spColDragger->OnDrag(this, e);
}

void CGridView::ColDrag_MouseLeave(const MouseLeaveEvent& e)
{
	m_spColDragger->OnLeaveDrag(this, e);
}

/************/
/* ItemDrag */
/************/
void CGridView::ItemDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	::OutputDebugString(L"ItemDrag_BeginDrag\r\n");
	m_spItemDragger->OnBeginDrag(this, e);
}

void CGridView::ItemDrag_OnExit(const LButtonEndDragEvent& e)
{
	::OutputDebugString(L"ItemDrag_LButtonUp\r\n");
	//m_spCursorer->OnLButtonEndDrag(this, e);
	//m_spCeller->OnLButtonEndDrag(this, e);

	m_spItemDragger->OnEndDrag(this, e);
}

bool CGridView::ItemDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spItemDragger->IsTarget(this, e);
}

void CGridView::ItemDrag_MouseMove(const MouseMoveEvent& e)
{
	::OutputDebugString(L"ItemDrag_MouseMove\r\n");
	m_spItemDragger->OnDrag(this, e);
}

void CGridView::ItemDrag_MouseLeave(const MouseLeaveEvent& e)
{
	::OutputDebugString(L"ItemDrag_Leave\r\n");
	m_spItemDragger->OnLeaveDrag(this, e);
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
				//pCell->GetGridPtr()->GetGridPtr()->SetEditPtr(nullptr);
				pCell->SetString(str);
				//pCell->SetState(UIElementState::Normal);//After Editing, Change Normal
			}));
		GetEditPtr()->OnCreate(CreateEvt(GetWndPtr(), this, pCell->GetRectInWnd()));
		GetWndPtr()->SetFocusToControl(GetEditPtr());
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

/*********/
/* Error */
/*********/
void CGridView::Error_StdException(const std::exception& e)
{
	::OutputDebugStringA(e.what());

	std::string msg = fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n",
		e.what(), GetLastErrorString());

	MessageBoxA(GetWndPtr()->m_hWnd, msg.c_str(), "Exception in StateMachine", MB_ICONWARNING);
	Clear();
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

/******************/
/* Window Message */
/******************/

void CGridView::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	UpdateAll();
	SubmitUpdate();
}

void CGridView::Measure(const CSizeF& sz)
{
	SubmitUpdate();
	m_size = MeasureSize();
}

void CGridView::Arrange(const CRectF& rc)
{
	CRectF prevRect = m_rect;
	CD2DWControl::Arrange(rc);

	//if (prevRect.left != rc.left) {
		PostUpdate(Updates::Column);
	//}
	//if (prevRect.top != rc.top) {
		PostUpdate(Updates::Row);
	//}

	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
	SubmitUpdate();
}

void CGridView::OnPaint(const PaintEvent& e)
{
	m_pMachine->process_event(e);
}

void CGridView::OnMouseWheel(const MouseWheelEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetDeltaScroll() * e.Delta / WHEEL_DELTA);
	SubmitUpdate();
}

void CGridView::OnSetFocus(const SetFocusEvent& e)
{
	SubmitUpdate();
}

void CGridView::OnKillFocus(const KillFocusEvent& e)
{
	m_pMachine->process_event(e);
	SubmitUpdate();
}

void CGridView::OnSetCursor(const SetCursorEvent& e)
{
	*e.HandledPtr = FALSE; //Default Handled = FALSE means Arrow
	m_pMachine->process_event(e);
	SubmitUpdate();
}

void CGridView::OnContextMenu(const ContextMenuEvent& e)
{
	*e.HandledPtr = FALSE;
	m_pMachine->process_event(e);
	SubmitUpdate();
}
void CGridView::OnRButtonDown(const RButtonDownEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CGridView::OnLButtonDown(const LButtonDownEvent& e) 
{ 
	CD2DWControl::OnLButtonDown(e);
	if (!*e.HandledPtr) { m_pMachine->process_event(e); SubmitUpdate(); }
}
void CGridView::OnLButtonUp(const LButtonUpEvent& e) 
{
	CD2DWControl::OnLButtonUp(e);
	if (!*e.HandledPtr) { m_pMachine->process_event(e); SubmitUpdate(); }
}
void CGridView::OnLButtonClk(const LButtonClkEvent& e) 
{ 
	CD2DWControl::OnLButtonClk(e);
	if (!*e.HandledPtr) { m_pMachine->process_event(e); SubmitUpdate(); }
}
void CGridView::OnLButtonSnglClk(const LButtonSnglClkEvent& e) { m_pMachine->process_event(e); PostUpdate(Updates::Invalidate); SubmitUpdate(); }
void CGridView::OnLButtonDblClk(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CGridView::OnLButtonBeginDrag(const LButtonBeginDragEvent& e) 
{
	if (!*e.HandledPtr) {
		e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
		m_pMachine->process_event(e); SubmitUpdate();
	}
}
void CGridView::OnLButtonEndDrag(const LButtonEndDragEvent& e)
{
	if (!*e.HandledPtr) {
		e.WndPtr->ReleaseCapturedControlPtr();
		m_pMachine->process_event(e); SubmitUpdate();
	}
}
void CGridView::OnMouseMove(const MouseMoveEvent& e)
{ 
	CD2DWControl::OnMouseMove(e);
	if (!*e.HandledPtr) {
		m_pMachine->process_event(e);
		PostUpdate(Updates::Invalidate);
		SubmitUpdate();
	}
	
}
void CGridView::OnMouseLeave(const MouseLeaveEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CGridView::OnKeyDown(const KeyDownEvent& e) 
{ 
	CD2DWControl::OnKeyDown(e);
	if (!*e.HandledPtr) {
		m_pMachine->process_event(e); PostUpdate(Updates::Invalidate); SubmitUpdate();
	}
}
void CGridView::OnKeyTraceDown(const KeyTraceDownEvent& e)
{ 
	CD2DWControl::OnKeyTraceDown(e);
	if (!*e.HandledPtr) {
		m_pMachine->process_event(e); PostUpdate(Updates::Invalidate); SubmitUpdate();
	}
}

void CGridView::OnChar(const CharEvent& e) 
{ 
	CD2DWControl::OnChar(e);
	if (!*e.HandledPtr) {
		m_pMachine->process_event(e);  SubmitUpdate();
	}
}
void CGridView::OnImeStartComposition(const ImeStartCompositionEvent& e) { m_pMachine->process_event(e);  SubmitUpdate(); }
void CGridView::OnBeginEdit(const BeginEditEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CGridView::OnEndEdit(const EndEditEvent& e){ m_pMachine->process_event(e);  SubmitUpdate();}

void CGridView::SelectRange(std::shared_ptr<CCell>& cell1, std::shared_ptr<CCell>& cell2, bool doSelect)
{
	if (cell1 || cell2) { return; }

	auto	rowBeg= (std::min)(cell1->GetRowPtr()->GetIndex<VisTag>(), cell2->GetRowPtr()->GetIndex<VisTag>());
	auto	rowLast= (std::max)(cell1->GetRowPtr()->GetIndex<VisTag>(), cell2->GetRowPtr()->GetIndex<VisTag>());
	auto	colBeg= (std::min)(cell1->GetColumnPtr()->GetIndex<VisTag>(), cell2->GetColumnPtr()->GetIndex<VisTag>());
	auto	colLast= (std::max)(cell1->GetColumnPtr()->GetIndex<VisTag>(), cell2->GetColumnPtr()->GetIndex<VisTag>());

	for(auto col = colBeg; col<=colLast; col++){
		for(auto row = rowBeg; row<=rowLast; row++){
			Cell(m_visRows[row], m_visCols[col])->SetIsSelected(doSelect);
		}
	}	
}

FLOAT CGridView::GetCellsHeight()
{
	if (GetContainer<RowTag, VisTag>().size() == 0 || GetContainer<RowTag, VisTag>().size() <= m_frozenRowCount) {
		return 0.0f;
	}

	FLOAT top = m_visRows[m_frozenRowCount]->GetTop();
	FLOAT bottom = m_visRows.back()->GetBottom();

	return bottom - top;
}

FLOAT CGridView::GetCellsWidth()
{
	if (GetContainer<ColTag, VisTag>().size() == 0 || GetContainer<ColTag, VisTag>().size() <= m_frozenColumnCount) {
		return 0.0f;
	}

	FLOAT left = m_visCols[m_frozenColumnCount]->GetLeft();
	FLOAT right = m_visCols.back()->GetRight();

	return right - left;
}

CRectF CGridView::GetCellsRect()
{
	if(GetContainer<RowTag, VisTag>().size() == 0 || GetContainer<RowTag, VisTag>().size() <= m_frozenRowCount || GetContainer<ColTag, VisTag>().size() <= m_frozenColumnCount){
		return CRectF(0,0,0,0);
	}

	FLOAT left=m_visCols[m_frozenColumnCount]->GetLeft();
	FLOAT top= m_visRows[m_frozenRowCount]->GetTop();
	FLOAT right=m_visCols.back()->GetRight();
	FLOAT bottom=m_visRows.back()->GetBottom();

	return CRectF(left,top,right,bottom);
}

void CGridView::SelectAll()
{
	for (auto iter = std::next(m_visRows.begin(), m_frozenRowCount); iter!=m_visRows.end(); ++iter) {
		(*iter)->SetIsSelected(true);
	}
}

void CGridView::DeselectAll()
{
	auto setSelected = [](auto ptr) {ptr->SetIsSelected(false); };

	for(auto& colPtr : m_allCols){
		for(auto& rowPtr : m_allRows){
			setSelected(Cell(rowPtr, colPtr));
		}
	}
	std::for_each(m_allCols.begin(), m_allCols.end(), setSelected);
	std::for_each(m_allRows.begin(), m_allRows.end(), setSelected);
}

void CGridView::UnhotAll()
{
	for (const auto& colPtr : m_allCols) {
		for (const auto& rowPtr : m_allRows) {
			Cell(rowPtr, colPtr)->SetState(UIElementState::Normal);
		}
	}
}

bool CGridView::IsFocusedCell(const CCell* pCell)const
{
	return m_spCursorer->GetFocusedCell().get() == pCell;
}

bool CGridView::IsDoubleFocusedCell(const CCell* pCell)const
{
	return m_spCursorer->GetDoubleFocusedCell().get() == pCell;
}




void CGridView::Clear()
{
	m_setUpdate.clear();

	m_allRows.clear();
	m_visRows.clear();
	m_pntRows.clear();

	m_allCols.clear();
	m_visCols.clear();
	m_pntCols.clear();

	m_spCursorer->Clear();
	m_spCeller->Clear();
	//m_spColDragger->Clear();
	//m_spRowDragger->Clear();
	//m_spColTracker->Clear();
	//m_spRowTracker->Clear();

	m_rocoContextMenu=CRowColumn();

	//m_pHeaderColumn=std::shared_ptr<CColumn>();
	//m_rowHeader=std::shared_ptr<CRow>();

	m_bSelected = false;
	//m_bFocused = false;
	m_keepEnsureVisibleFocusedCell = false;
}

void CGridView::SetAllRowMeasureValid(bool valid)
{
	for (const auto& ptr : m_allRows) {
		ptr->SetIsMeasureValid(valid);
	}
}

void CGridView::SetAllRowFitMeasureValid(bool valid)
{
	for (const auto& ptr : m_allRows) {
		ptr->SetIsFitMeasureValid(valid);
	}
}

void CGridView::SetAllColumnMeasureValid(bool valid)
{
	for (const auto& ptr : m_allCols) {
		ptr->SetIsMeasureValid(valid);
	}
}

void CGridView::SetAllColumnFitMeasureValid(bool valid)
{
	for (const auto& ptr : m_allCols) {
		ptr->SetIsFitMeasureValid(valid);
	}
}


void CGridView::SetColumnAllCellMeasureValid(CColumn* pColumn, bool valid)
{
	for (const auto& ptr : m_allRows) {
		Cell(ptr.get(), pColumn)->SetActMeasureValid(false);
	}
}

void CGridView::OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
{
	if (!_tcsicmp(L"value", name)) {
		PostUpdate(Updates::Sort);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		//PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(L"size", name)) {
		//PostUpdate(Updates::Sort);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	}else if (!_tcsicmp(L"state", name)) {
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(L"focus", name)) {
		PostUpdate(Updates::Invalidate);		
	} else if (!_tcsicmp(L"selected", name)) {
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(L"checked", name)) {
		PostUpdate(Updates::Invalidate);
	}
}

void CGridView::OnRowPropertyChanged(CRow* pRow, const wchar_t* name)
{
	if (!_tcsicmp(name, L"selected")) {
		////0506PostUpdate(Updates::RowVisible);
		////0506PostUpdate(Updates::Column);
		////0506PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	} else if (_tcsicmp(name, L"visible") == 0) {
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);//
	}
}

void CGridView::OnColumnPropertyChanged(CColumn* pCol, const wchar_t* name)
{
	if (!_tcsicmp(name, L"visible") || !_tcsicmp(name, L"size")) {
		for (const auto& ptr : m_allRows) {
			ptr->SetIsMeasureValid(false);
		}
		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(name, L"selected")) {
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(name, L"sort")) {
		PostUpdate(Updates::Sort);
	}
}

void CGridView::ColumnInserted(const CColumnEventArgs& e)
{
	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CGridView::ColumnErased(const CColumnEventArgs& e)
{
	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CGridView::ColumnMoved(const CMovedEventArgs<ColTag>& e)
{
	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}


void CGridView::ColumnHeaderFitWidth(const CColumnEventArgs& e)
{
	e.m_pColumn->SetIsMeasureValid(false);
	for(const auto& ptr : m_allRows){
		Cell(ptr, e.m_pColumn)->SetActMeasureValid(false);
	}
	this->SetAllColumnMeasureValid(false);
	
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CGridView::RowInserted(const CRowEventArgs& e)
{
	for(auto& spCol : m_allCols){
		spCol->SetIsFitMeasureValid(false);
	};
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CGridView::RowErased(const CRowEventArgs& e)
{
	this->SetAllColumnFitMeasureValid(false);

	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CGridView::RowsErased(const CRowsEventArgs& e)
{
	//Remeasure column width irritate user, therefore Column measure doesn't run.
	//boost::for_each(m_columnAllDictionary,[&](const ColumnData& colData){
	//	colData.DataPtr->SetMeasureValid(false);
	//});
	this->SetAllColumnFitMeasureValid(false);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CGridView::RowMoved(const CMovedEventArgs<RowTag>& e)
{
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}


void CGridView::SizeChanged()
{
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}
void CGridView::Scroll()
{
	PostUpdate(Updates::Invalidate);
}

std::wstring CGridView::GetSheetString()
{
	std::wstring str;

	for(const auto& rowPtr : m_visRows){
		bool bFirstLine=true;
		for(const auto& colPtr : m_visCols){
			const auto& pCell = Cell(rowPtr, colPtr);
			if(bFirstLine){
				bFirstLine=false;
			}else{
				str.append(L"\t");
			}
			str.append(pCell->GetString());
		}
		str.append(L"\r\n");
	}
	return str;
}

void CGridView::UpdateRowVisibleDictionary()
{
	UpdateVisibleContainer<RowTag>();
}

void CGridView::UpdateColumnVisibleDictionary()
{
	UpdateVisibleContainer<ColTag>();
}

void CGridView::UpdateRowPaintDictionary()
{
	CRectF rcClip(GetPaintRect());
	CPointF ptOrigin(GetFrozenPoint());
	rcClip.left = ptOrigin.x;
	rcClip.top = ptOrigin.y;
	UpdatePaintContainer<RowTag>(
		rcClip.top,
		rcClip.bottom);
}

void CGridView::UpdateColumnPaintDictionary()
{
	CRectF rcClip(GetPaintRect());
	CPointF ptOrigin(GetFrozenPoint());
	rcClip.left = ptOrigin.x;
	rcClip.top = ptOrigin.y;
	UpdatePaintContainer<ColTag>(
		rcClip.left,
		rcClip.right);
}



void CGridView::EraseColumn(const std::shared_ptr<CColumn>& spCol, bool notify)
{
	m_allCols.idx_erase(std::find(m_allCols.begin(), m_allCols.end(), spCol));
	m_spCursorer->OnCursorClear(this);
	if (notify) {
		ColumnErased(CColumnEventArgs(spCol));
	}
}

void CGridView::EraseRow(const std::shared_ptr<CRow>& spRow, bool notify)
{
	m_allRows.idx_erase(std::find(m_allRows.begin(), m_allRows.end(), spRow));
	m_spCursorer->OnCursorClear(this);
	if (notify) {
		RowErased(CRowEventArgs(spRow));
	}
}

void CGridView::EraseRows(const std::vector<std::shared_ptr<CRow>>& vspRow, bool notify)
{
	for (auto rowPtr : vspRow) {
		EraseRow(rowPtr);
	}
}

void CGridView::InsertColumn(int pos, const std::shared_ptr<CColumn>& spColumn, bool notify)
{
	m_allCols.idx_insert(m_allCols.cbegin() + pos, spColumn);
	if (notify) {
		ColumnInserted(CColumnEventArgs(spColumn));
	}
}


void CGridView::PushColumn(const std::shared_ptr<CColumn>& spCol, bool notify)
{
	//spCol->InsertNecessaryRows();
	m_allCols.idx_push_back(spCol);
	if (notify) {
		ColumnInserted(CColumnEventArgs(spCol));
	}
}

void CGridView::InsertRow(int row, const std::shared_ptr<CRow>& spRow, bool notify)
{
	m_allRows.idx_insert(m_allRows.cbegin() + row, spRow);
	if (notify) {
		RowInserted(CRowEventArgs(spRow));
	}
}


void CGridView::PushRow(const std::shared_ptr<CRow>& spRow, bool notify)
{
	m_allRows.idx_push_back(spRow);
	if(notify){
		RowInserted(CRowEventArgs(spRow));
	}
}

FLOAT CGridView::GetColumnInitWidth(CColumn* pColumn)
{
	return std::accumulate(m_visRows.begin(), m_visRows.end(), 0.f,
		[this, pColumn](const FLOAT val, const std::shared_ptr<CRow>& rowPtr)->FLOAT {
			return (std::max)(Cell(rowPtr.get(), pColumn)->GetInitSize(GetWndPtr()->GetDirectPtr()).width, val);
		});
}

FLOAT CGridView::GetColumnFitWidth(CColumn* pColumn)
{
	return std::accumulate(m_visRows.begin(), m_visRows.end(), 0.f,
		[this, pColumn](const FLOAT val, const std::shared_ptr<CRow>& rowPtr)->FLOAT {
			return (std::max)(Cell(rowPtr.get(), pColumn)->GetFitSize(GetWndPtr()->GetDirectPtr()).width, val);
		});
}

FLOAT CGridView::GetRowHeight(CRow* pRow)
{
	return std::accumulate(m_visCols.begin(), m_visCols.end(), 0.f,
		[this, pRow](const FLOAT val, const std::shared_ptr<CColumn>& colPtr)->FLOAT {
			return (std::max)(Cell(pRow, colPtr.get())->GetActSize(GetWndPtr()->GetDirectPtr()).height, val);
		});
}

FLOAT CGridView::GetRowFitHeight(CRow* pRow)
{
	return std::accumulate(m_visCols.begin(), m_visCols.end(), 0.f,
						   [this, pRow](const FLOAT val, const std::shared_ptr<CColumn>& colPtr)->FLOAT {
							   return (std::max)(Cell(pRow, colPtr.get())->GetFitSize(GetWndPtr()->GetDirectPtr()).height, val);
						   });
}

bool CGridView::Empty()const
{
	return m_allRows.empty() || m_allCols.empty();
}
bool CGridView::Visible()const
{
	return (!m_visRows.empty()) && (!m_visCols.empty()) /*&& GetRectInWnd().Width() > 0 && GetRectInWnd().Height() > 0*/;
}

CPointF CGridView::GetFrozenPoint()
{
	if(!Visible()){
		return GetRectInWnd().LeftTop();
	}

	FLOAT x = 0;
	FLOAT y = 0;
	if (m_frozenRowCount == 0) {
		y = GetRectInWnd().top + GetNormalBorder().Width * 0.5f;
	} else {
		y = m_visRows[m_frozenRowCount - 1]->GetBottom();
	}

	if (m_frozenColumnCount == 0) {
		x = GetRectInWnd().left + GetNormalBorder().Width * 0.5f;
	} else {
		x = m_visCols[m_frozenColumnCount - 1]->GetRight();
	}

	return CPointF(x,y);
}


CSizeF CGridView::MeasureSize()const
{
	//CRectF rc(CSheet::GetRectInWnd());
	//return CSizeF(rc.Width(), rc.Height());
	if(!Visible()){return CSizeF();}
	FLOAT left=m_visCols.front()->GetLeft();
	FLOAT top=m_visRows.front()->GetTop();
	FLOAT right=m_visCols.back()->GetRight();
	FLOAT bottom=m_visRows.back()->GetBottom();

	CRectF rc(left, top, right, bottom);

	auto outerPenWidth = GetNormalBorder().Width * 0.5f;
	rc.InflateRect(outerPenWidth, outerPenWidth);
	return rc.Size();
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






