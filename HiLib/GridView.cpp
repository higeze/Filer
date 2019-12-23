#include <olectl.h>
#include <numeric>
#include "GridView.h"
#include "Cell.h"
#include "CellProperty.h"
#include "Row.h"
#include "ParentHeaderCell.h"
#include "ParentMapColumn.h"
#include "ParentRowHeaderColumn.h"
#include "GridViewResource.h"
#include "SheetCell.h"
#include "MyRgn.h"
#include "MyPen.h"
#include "MyColor.h"
#include "ConsoleTimer.h"
#include "ApplicationProperty.h"
#include "Tracker.h"
#include "Cursorer.h"
#include "Dragger.h"
#include "SheetState.h"
#include "MyClipboard.h"
#include "SheetEventArgs.h"
#include "SheetStateMachine.h"
#include "Celler.h"
#include "Scroll.h"
#include "Textbox.h"

#include "FullCommand.h"

#include "FindDlg.h"

#include "TextCell.h"
#include "MouseStateMachine.h"
#include "GridViewProperty.h"
#include "ResourceIDFactory.h"


extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;
UINT CGridView::WM_DELAY_UPDATE = ::RegisterWindowMessage(L"CGridView::WM_DELAY_UPDATE");

CMenu CGridView::ContextMenu;

void CGridView::ColumnInserted(CColumnEventArgs& e)
{
	//FilterAll();
	PostUpdate(Updates::Filter);
	CSheet::ColumnInserted(e);
	SignalColumnInserted(e.m_pColumn);
}
void CGridView::ColumnErased(CColumnEventArgs& e)
{
//	FilterAll();
	PostUpdate(Updates::Filter);
	CSheet::ColumnErased(e);
	SignalColumnErased(e.m_pColumn);
}
void CGridView::ColumnMoved(CMovedEventArgs<ColTag>& e)
{
	//FilterAll();
	CSheet::ColumnMoved(e);
	SignalColumnMoved((CColumn*)e.m_ptr, e.m_from, e.m_to);
}

void CGridView::OnCellContextMenu(CellContextMenuEventArgs& e)
{}

void CGridView::OnCellLButtonClk(CellEventArgs& e)
{
	if(e.CellPtr->GetRowPtr()==GetNameHeaderRowPtr().get())
	{	
		auto pCol=e.CellPtr->GetColumnPtr();
		Sorts sort=pCol->GetSort();
		//Reset Sort
		ResetColumnSort();
		//Sort
		switch(sort){
			case Sorts::None:
			case Sorts::Down:
				pCol->SetSort(Sorts::Up);
				break;
			case Sorts::Up:
				pCol->SetSort(Sorts::Down);
				break;
			default:
				pCol->SetSort(Sorts::None);
				break;
		}
		SubmitUpdate();
	}
}

void CGridView::SortAllInSubmitUpdate()
{
	auto& colDictionary = m_columnAllDictionary.get<IndexTag>();
	for (const auto& colData : colDictionary) {
		if (colData.DataPtr->GetSort() != Sorts::None) {
			this->Sort(colData.DataPtr.get(), colData.DataPtr->GetSort(), false);
		}
	}
}

void CGridView::ClearFilter()
{
	auto& rowDictionary=m_rowAllDictionary.get<IndexTag>();
	auto& colDictionary=m_columnAllDictionary.get<IndexTag>();

	//Reset Filter
	for(auto iter=rowDictionary.find(0),end=rowDictionary.end();iter!=end;++iter){
		iter->DataPtr->SetVisible(true);
	};
	//Clear Filter
	for(auto colIter=colDictionary.begin(),colEnd=colDictionary.end();colIter!=colEnd;++colIter){
		colIter->DataPtr->Cell(GetFilterRowPtr().get())->SetStringNotify(L"");	
	}
}

void CGridView::FilterAll()
{
	auto& rowDictionary=m_rowAllDictionary.get<IndexTag>();
	auto& colDictionary=m_columnAllDictionary.get<IndexTag>();

	//Reset Filter
	for(auto iter=rowDictionary.find(0),end=rowDictionary.end();iter!=end;++iter){
		iter->DataPtr->SetVisible(true);
	};
	//Set Filter
	for(auto colIter=colDictionary.find(0),colEnd=colDictionary.end();colIter!=colEnd;++colIter){
		auto strFilter=colIter->DataPtr->Cell(GetFilterRowPtr().get())->GetString();
		if(strFilter.empty() || strFilter==std::wstring(L""))continue;
		std::vector<std::wstring> vstrFilter;
		boost::split(vstrFilter,strFilter,boost::is_space());
		for(auto rowIter=rowDictionary.find(0),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
			if(!rowIter->DataPtr->GetVisible())continue;
			//Filter
			auto pCell=colIter->DataPtr->Cell(rowIter->DataPtr.get());
			for(const auto& str : vstrFilter){
				if(str[0]==L'-' && str.size()>=2){
					std::wstring strMinus(str.substr(1));
					if(pCell->Filter(strMinus)){
						rowIter->DataPtr->SetVisible(false);				
					}
				}else if(!(pCell->Filter(str))){
					rowIter->DataPtr->SetVisible(false);
				}
			}
		}		
	}
}

LRESULT CGridView::OnFilter(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	FilterAll();
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	//Direct2DWrite
	m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);
	return 0;
}

LRESULT CGridView::OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
		::SetForegroundWindow(hWnd);
	}

	DestroyWindow();
	return 0;
}

LRESULT CGridView::OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	return 0;
}

LRESULT CGridView::OnEraseBkGnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	//For Back buffering
	bHandled=TRUE;
	return 1;
}

LRESULT CGridView::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{	
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1_SIZE_U{ LOWORD(lParam), HIWORD(lParam) });
	SizeChanged();
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	m_pDirect->BeginDraw();

	m_pDirect->ClearSolid(*(m_spGridViewProp->BackgroundPropPtr->m_brush));
	PaintEvent e(this);
	OnPaint(e);
	if (m_pEdit) {
		m_pEdit->OnPaint(e);
	}
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);

	m_pDirect->EndDraw();
	return 0;
}

LRESULT CGridView::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	InvalidateRect(NULL, FALSE);
	return 0;
}

LRESULT CGridView::OnMouseWheel(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	m_keepEnsureVisibleFocusedCell = false;
	m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollDelta() * GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
	SubmitUpdate();
	return 0;
}

void CGridView::Invalidate()
{
	InvalidateRect(NULL,FALSE);
}

void CGridView::DelayUpdate()
{
	m_invalidateTimer.run([this] {
		PostMessage(WM_DELAY_UPDATE, NULL, NULL);
	}, std::chrono::milliseconds(50));
}

LRESULT CGridView::OnDelayUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	spdlog::info("CGridView::OnDelayUpdate");

	CONSOLETIMER("OnDelayUpdate Total");
	SignalPreDelayUpdate();
	SignalPreDelayUpdate.disconnect_all_slots();
	PostUpdate(Updates::Filter);

	if (m_keepEnsureVisibleFocusedCell) {
		PostUpdate(Updates::EnsureVisibleFocusedCell);
	} else {
		m_setUpdate.erase(Updates::EnsureVisibleFocusedCell);
	}
	SubmitUpdate();
	return 0;
}

FLOAT CGridView::UpdateHeadersRow(FLOAT top)
{
	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();

	//Minus Cells
	for (auto iter = rowDictionary.begin(), end = rowDictionary.find(0); iter != end; ++iter) {
		iter->DataPtr->SetTopWithoutSignal(top);
		top += iter->DataPtr->GetHeight();
	}
	return top;
}


FLOAT CGridView::UpdateCellsRow(FLOAT top, FLOAT pageTop, FLOAT pageBottom)
{
	//Helper functions
	std::function<bool(FLOAT, FLOAT, FLOAT, FLOAT)> isInPage = [](FLOAT min, FLOAT max, FLOAT top, FLOAT bottom)->bool {
		return (top > min && top < max) ||
			(bottom > min && bottom < max);
	};
	//Plus Cells
	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
	for (auto rowIter = rowDictionary.find(0), rowEnd = rowDictionary.end(),rowNextIter = std::next(rowIter) ; rowIter != rowEnd; ++rowIter, ++rowNextIter) {
		
		if (IsVirtualPage()) {
			rowIter->DataPtr->SetTopWithoutSignal(top);
	
			FLOAT defaultHeight = rowIter->DataPtr->GetDefaultHeight();
			FLOAT bottom = top + defaultHeight;
			if (isInPage(pageTop, pageBottom, top, top + defaultHeight) ||
				(rowNextIter!=rowEnd && isInPage(pageTop, pageBottom, bottom, bottom + rowNextIter->DataPtr->GetDefaultHeight()))){
				if (HasSheetCell()) {
					auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();
					for (auto& colData : colDictionary) {
						std::shared_ptr<CCell> pCell = CSheet::Cell(rowIter->DataPtr, colData.DataPtr);
						if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
							pSheetCell->UpdateAll();
						}
					}
				}
				top += rowIter->DataPtr->GetHeight();
			} else {
				top += defaultHeight;
			}
			//std::cout << top << std::endl;
		} else {
			rowIter->DataPtr->SetTopWithoutSignal(top);
			if (HasSheetCell()) {
				auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();
				for (auto& colData : colDictionary) {
					std::shared_ptr<CCell> pCell = CSheet::Cell(rowIter->DataPtr, colData.DataPtr);
					if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
						pSheetCell->UpdateAll();
					}
				}
			}
			top += rowIter->DataPtr->GetHeight();
		}
//		prevRowIter = rowIter;
	}
	return top;
}

void CGridView::UpdateRow()
{
	if (!Visible()) { return; }

	FLOAT top = m_spCellProperty->Line->Width * 0.5f;

	//Headers
	top = UpdateHeadersRow(top);

	//Page
	d2dw::CRectF rcPage(m_pDirect->Pixels2Dips(GetClientRect()));
	rcPage.top = top;
	FLOAT pageHeight = rcPage.Height();
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

}

void CGridView::UpdateScrolls()
{
	if(!Visible())return;

	//Client
	d2dw::CRectF rcClient(m_pDirect->Pixels2Dips(GetClientRect()));

	//Origin
	//d2dw::CPointF ptOrigin(GetOriginPoint());

	//Scroll Range
	d2dw::CRectF rcCells(GetCellsRect());
	m_pVScroll->SetScrollRange(0.f, rcCells.Height());
	m_pHScroll->SetScrollRange(0.f, rcCells.Width());

	//Scroll Page
	d2dw::CRectF rcPage(GetPageRect());

	m_pVScroll->SetScrollPage(rcPage.Height());
	m_pHScroll->SetScrollPage(rcPage.Width());

	//EnableShow Vertical		
	m_pVScroll->SetVisible(m_pVScroll->GetScrollDistance() > m_pVScroll->GetScrollPage());

	//EnableShow Horizontal
	m_pHScroll->SetVisible(m_pHScroll->GetScrollDistance() > m_pHScroll->GetScrollPage());

	//Position scroll
	d2dw::CRectF rcVertical;
	d2dw::CRectF rcHorizontal;
	FLOAT lineHalfWidth = m_spCellProperty->Line->Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_pHScroll->GetVisible()?(m_pHScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	m_pVScroll->SetRect(rcVertical);

	auto a = m_pHScroll->GetVisible() ? (::GetSystemMetrics(SM_CYHSCROLL) + lineHalfWidth) : lineHalfWidth;

	rcHorizontal.left= rcClient.left + lineHalfWidth;
	rcHorizontal.top = rcClient.bottom-::GetSystemMetrics(SM_CYHSCROLL) - lineHalfWidth;
	rcHorizontal.right = rcClient.right - (m_pVScroll->GetVisible()?(m_pVScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	rcHorizontal.bottom = rcClient.bottom - lineHalfWidth;
	m_pHScroll->SetRect(rcHorizontal);

}

d2dw::CPointF CGridView::GetScrollPos()const
{
	return d2dw::CPointF(m_pHScroll->GetScrollPos(), m_pVScroll->GetScrollPos());
}

void CGridView::SetScrollPos(const CPoint& ptScroll)
{
	m_pHScroll->SetScrollPos(m_pDirect->Pixels2DipsX(ptScroll.x));
	m_pVScroll->SetScrollPos(m_pDirect->Pixels2DipsY(ptScroll.y));
}

FLOAT CGridView::GetVerticalScrollPos()const
{
	return m_pVScroll->GetScrollPos();
}

FLOAT CGridView::GetHorizontalScrollPos()const
{
	return m_pHScroll->GetScrollPos();
}


LRESULT CGridView::OnCommandEditHeader(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	if(!m_rocoContextMenu.IsInvalid()){
		if(m_rocoContextMenu.GetRowPtr()==GetNameHeaderRowPtr().get()){
			if(auto pCell=std::dynamic_pointer_cast<CParentHeaderCell>(m_rocoContextMenu.GetColumnPtr()->Cell(m_rocoContextMenu.GetRowPtr()))){
				pCell->OnEdit(EventArgs());
			}
		}
	}
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnCommandDeleteColumn(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	if(!m_rocoContextMenu.IsInvalid()){
		auto& dic = m_columnAllDictionary.get<PointerTag>();
		auto iter = dic.find(m_rocoContextMenu.GetColumnPtr());
		EraseColumn(iter->DataPtr);
		SubmitUpdate();
	}
	return 0;
}

LRESULT CGridView::OnCommandResizeSheetCell(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	if(!m_rocoContextMenu.IsInvalid()){
		if(auto p = std::dynamic_pointer_cast<CSheetCell>(CSheet::Cell(m_rocoContextMenu.GetRowPtr(),m_rocoContextMenu.GetColumnPtr()))){
			p->Resize();
		}
		SubmitUpdate();
	}
	return 0;
}


LRESULT CGridView::OnCommandSelectAll(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	SelectAll();
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnCommandCopy(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	//TODO High
	std::wstring strCopy;
	bool SelectModeRow = true;
	bool IncludesHeader = true;

	if(SelectModeRow){
		auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
		auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();

		for (auto rowIter = rowDictionary.begin(), rowEnd = rowDictionary.end(); rowIter != rowEnd; ++rowIter) {
			if (rowIter->DataPtr->GetSelected() || (IncludesHeader && rowIter->DataPtr->GetIndex<VisTag>()<0)) {
				for (auto colIter = colDictionary.begin(), colBegin = colDictionary.begin(), colEnd = colDictionary.end(); colIter != colEnd; ++colIter) {
					auto pCell = colIter->DataPtr->Cell(rowIter->DataPtr.get());
					strCopy.append(pCell->GetString());
					if (std::next(colIter) == colDictionary.end()) {
						strCopy.append(L"\r\n");
					} else {
						strCopy.append(L"\t");
					}
				}

			}
		}
	}else{
		auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
		auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();

		for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
			bool bSelectedLine(false);
			bool bFirstLine(true);
			for(auto colIter=colDictionary.begin(),colBegin=colDictionary.begin(),colEnd=colDictionary.end();colIter!=colEnd;++colIter){
				auto pCell=colIter->DataPtr->Cell(rowIter->DataPtr.get());
				if(pCell->GetSelected()){
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
		if(clipboard.Open(m_hWnd)!=0){	
			clipboard.Empty();
			clipboard.SetData(CF_UNICODETEXT,hGlobal);
			clipboard.Close();
		}
	}

	return 0;
}

void CGridView::UpdateAll()
{
	FilterAll();
	CSheet::UpdateAll();
	UpdateScrolls();
}

void CGridView::EnsureVisibleCell(const std::shared_ptr<CCell>& pCell)
{
	if(!pCell || !pCell->GetRowPtr()->GetVisible())return;

	//Page
	d2dw::CRectF rcPage(GetPageRect());

	if (IsVirtualPage()) {
		//Helper functions
		std::function<bool(FLOAT, FLOAT, FLOAT, FLOAT)> isAllInPage = [](FLOAT min, FLOAT max, FLOAT top, FLOAT bottom)->bool {
			return (top > min && top < max) &&
				(bottom > min && bottom < max);
		};
		//Dictionary
		auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();

		FLOAT pageHeight = rcPage.Height();
		FLOAT scrollPos = GetVerticalScrollPos();

		//Check if in page
		if (isAllInPage(rcPage.top, rcPage.bottom, m_spCursorer->GetFocusedCell()->GetRowPtr()->GetTop(), m_spCursorer->GetFocusedCell()->GetRowPtr()->GetBottom())) {
			//Do nothing
		//Larget than bottom
		} else if (m_spCursorer->GetFocusedCell()->GetRowPtr()->GetBottom() > rcPage.bottom) {
			FLOAT height = 0.0f;
			FLOAT scroll = 0.0f;
			for (auto iter = rowDictionary.find(m_spCursorer->GetFocusedCell()->GetRowPtr()->GetIndex<VisTag>()), zero = rowDictionary.find(0), end = std::prev(rowDictionary.find(0)); iter != end; --iter) {
				height += iter->DataPtr->GetHeight();
				if (height >= pageHeight) {
					FLOAT heightToFocus = std::accumulate(rowDictionary.find(0), std::next(rowDictionary.find(m_spCursorer->GetFocusedCell()->GetRowPtr()->GetIndex<VisTag>())), 0.0f,
						[](const FLOAT& acc, const RowData& data)->FLOAT { return acc + data.DataPtr->GetDefaultHeight(); });
					scroll = heightToFocus - pageHeight;
					break;
				} else if (iter == zero) {
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
			FLOAT scroll = std::accumulate(rowDictionary.find(0), rowDictionary.find(m_spCursorer->GetFocusedCell()->GetRowPtr()->GetIndex<VisTag>()), 0.0f,
				[](const FLOAT& acc, const RowData& data)->FLOAT{ return acc + data.DataPtr->GetDefaultHeight(); });
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
		auto rcCell(pCell->GetRect());
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
	auto rcCell(pCell->GetRect());
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

void CGridView::Jump(std::shared_ptr<CCell>& spCell)
{
	m_spCursorer->OnCursor(spCell);
	PostUpdate(Updates::EnsureVisibleFocusedCell);
	SubmitUpdate();
}

d2dw::CRectF CGridView::GetPaintRect()
{
	return m_pDirect->Pixels2Dips(GetClientRect());
}

std::pair<bool, bool> CGridView::GetHorizontalVerticalScrollNecessity()
{
	d2dw::CRectF rcClient(m_pDirect->Pixels2Dips(GetClientRect()));
	d2dw::CRectF rcCells(GetCellsRect());
	d2dw::CPointF ptOrigin(GetOriginPoint());
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

d2dw::CRectF CGridView::GetPageRect()
{
	d2dw::CRectF rcClient(m_pDirect->Pixels2Dips(GetClientRect()));
	d2dw::CRectF rcCells(GetCellsRect());
	d2dw::CPointF ptOrigin(GetOriginPoint());
	//First
	bool bEnableShowVertical = rcCells.bottom > rcClient.bottom || rcCells.top < ptOrigin.y;
	bool bEnableShowHorizontal = rcCells.right > rcClient.right || rcCells.left < ptOrigin.x ;
	//Second
	bEnableShowVertical = rcCells.bottom > (rcClient.bottom - (bEnableShowHorizontal?GetSystemMetrics(SM_CYHSCROLL):0)) || rcCells.top < ptOrigin.y;
	bEnableShowHorizontal = rcCells.right > (rcClient.right - (bEnableShowVertical?GetSystemMetrics(SM_CXVSCROLL):0)) || rcCells.left < ptOrigin.x;
	//Third
	bEnableShowVertical = rcCells.bottom > (rcClient.bottom - (bEnableShowHorizontal?GetSystemMetrics(SM_CYHSCROLL):0)) || rcCells.top < ptOrigin.y;
	bEnableShowHorizontal = rcCells.right > (rcClient.right - (bEnableShowVertical?GetSystemMetrics(SM_CXVSCROLL):0)) || rcCells.left < ptOrigin.x;

	return d2dw::CRectF(ptOrigin.x, ptOrigin.y,
		rcClient.right - (bEnableShowVertical?GetSystemMetrics(SM_CXVSCROLL):0),
		rcClient.bottom - (bEnableShowHorizontal?GetSystemMetrics(SM_CYHSCROLL):0));
}

std::shared_ptr<CDC> CGridView::GetClientDCPtr()const
{
	return std::make_shared<CClientDC>(m_hWnd);
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
				CONSOLETIMER("Updates::Sort");
					SortAllInSubmitUpdate();
				break;
			}
			case Updates::Filter:
			{
				CONSOLETIMER("Updates::Filter");
					FilterAll();
				break;
			}
			case Updates::Rect:
			{
				MoveWindow(m_pDirect->Dips2Pixels(GetUpdateRect()), FALSE);
			}
			case Updates::RowVisible:
			{
				CONSOLETIMER("Updates::RowVisible");
					UpdateRowVisibleDictionary();
				break;
			}
			case Updates::ColumnVisible:
			{
				CONSOLETIMER("Updates::ColumnVisible");
					UpdateColumnVisibleDictionary();
				break;
			}
			case Updates::Column:
			{
				CONSOLETIMER("Updates::Column");
					UpdateColumn();
				break;
			}
			case Updates::Row:
			{
				CONSOLETIMER("Updates::Row");
					UpdateRow();
				break;
			}
			case Updates::Scrolls:
			{
				CONSOLETIMER("Updates::Scrolls");
					UpdateScrolls();
				break;
			}
			case Updates::EnsureVisibleFocusedCell:
			{
				CONSOLETIMER("Updates::EnsureVisibleFocusedCell");
					EnsureVisibleCell(m_spCursorer->GetFocusedCell());
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
	m_rowHeaderHeader = std::shared_ptr<CRow>();
	m_rowNameHeader=std::shared_ptr<CRow>();
	m_rowFilter=std::shared_ptr<CRow>();
}

CColumn* CGridView::GetParentColumnPtr(CCell* pCell)
{
	return pCell->GetColumnPtr();
}

LRESULT CGridView::OnCommandFind(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	CFindDlg* pDlg = new CFindDlg(this);
	g_hDlgModeless = pDlg->Create(m_hWnd);
	pDlg->ShowWindow(SW_SHOW);
	return 0;
}

void CGridView::FindNext(const std::wstring& findWord, bool matchCase, bool matchWholeWord)
{
	//Fiding start from focused cell
	//If focused cell is invalid(Not focused), MinMax Visible Cell is start point
	auto focused = m_spCursorer->GetFocusedCell();

	auto& rowDict = m_rowVisibleDictionary.get<IndexTag>();
	auto& colDict = m_columnVisibleDictionary.get<IndexTag>();

	//Scan from Min to Max and Jump if Find
	auto jumpToFindNextCell = [&](RowDictionary::iterator rowIter, ColumnDictionary::iterator colIter, RowDictionary::iterator rowEnd, ColumnDictionary::iterator colEnd)->bool{
		while(1){

			auto spCell = CSheet::Cell(rowIter->DataPtr, colIter->DataPtr);
			auto str = spCell->GetString();

			if(my::find(str, findWord, matchCase, matchWholeWord)){
				Jump(spCell);
				return true;
			}else{
				colIter++;
				if(colIter==colEnd){
					colIter = colDict.begin();
					rowIter++;
				}
				if(rowIter==rowEnd){
					return false;
				}
			}
		}
	};


	RowDictionary::iterator rIter, rEnd;
	ColumnDictionary::iterator cIter, cEnd;
	//Find word from Min to Max

	if(!focused){
		rIter = rowDict.begin();
		cIter = colDict.begin();
	}else{
		rIter = rowDict.find(focused->GetRowPtr()->GetIndex<VisTag>());
		cIter = colDict.find(focused->GetColumnPtr()->GetIndex<VisTag>());
		cIter++;
		if(cIter==colDict.end()){
			cIter = colDict.begin();
			rIter++;
		}
	}
	rEnd = rowDict.end();
	cEnd = colDict.end();
	if(rIter!=rEnd && cIter!=cEnd && jumpToFindNextCell(rIter, cIter, rEnd, cEnd)){
		return;
	}
	//Find word from begining to Min
	//If focused cell is invalid(Not focused), all range is already searched.
	if(!focused){
		rIter = rowDict.begin();
		cIter = colDict.begin();
		rEnd = rowDict.end();
		cEnd = colDict.end();

		if(jumpToFindNextCell(rIter, cIter, rEnd, cEnd)){
			return;
		}
	}

	MessageBox((L"\"" + findWord + L"\" is not found!").c_str(), L"Find",MB_OK); 

}

void CGridView::FindPrev(const std::wstring& findWord, bool matchCase, bool matchWholeWord)
{
	//Fiding start from focused cell
	//If focused cell is invalid(Not focused), MinMax Visible Cell is start point
	auto focused = m_spCursorer->GetFocusedCell();

	auto& rowDict = m_rowVisibleDictionary.get<IndexTag>();
	auto& colDict = m_columnVisibleDictionary.get<IndexTag>();

	//Scan from Min to Max and Jump if Find
	auto jumpToFindPrevCell = [&](RowDictionary::reverse_iterator rowIter, ColumnDictionary::reverse_iterator colIter, RowDictionary::reverse_iterator rowEnd, ColumnDictionary::reverse_iterator colEnd)->bool{
		while(1){

			auto spCell = CSheet::Cell(rowIter->DataPtr, colIter->DataPtr);
			auto str = spCell->GetString();

			if(my::find(str, findWord, matchCase, matchWholeWord)){
				Jump(spCell);
				return true;
			}else{
				colIter++;
				if(colIter==colEnd){
					colIter = colDict.rbegin();
					rowIter++;
				}
				if(rowIter==rowEnd){
					return false;
				}
			}
		}
	};


	RowDictionary::reverse_iterator rIter, rEnd;
	ColumnDictionary::reverse_iterator cIter, cEnd;
	//Find word from Min to Max

	if(!focused){
		rIter = rowDict.rbegin();
		cIter = colDict.rbegin();
	}else{
		//In case of reverse_iterator, one iterator plused. Therefore it is necessary to minus.
		rIter = RowDictionary::reverse_iterator(rowDict.find(focused->GetRowPtr()->GetIndex<VisTag>()));
		cIter = ColumnDictionary::reverse_iterator(colDict.find(focused->GetColumnPtr()->GetIndex<VisTag>()));
		rIter--;
		cIter--;
		cIter++;
		if(cIter==colDict.rend()){
			cIter = colDict.rbegin();
			rIter++;
		}
	}
	rEnd = rowDict.rend();
	cEnd = colDict.rend();
	if(rIter!=rEnd && cIter!=cEnd && jumpToFindPrevCell(rIter, cIter, rEnd, cEnd)){
		return;
	}
	//Find word from begining to Min
	//If focused cell is invalid(Not focused), all range is already searched.
	if(!focused){
		rIter = rowDict.rbegin();
		cIter = colDict.rbegin();
		rEnd = rowDict.rend();
		cEnd = colDict.rend();

		if(jumpToFindPrevCell(rIter, cIter, rEnd, cEnd)){
			return;
		}
	}

	MessageBox((L"\"" + findWord + L"\" is not found!").c_str(), L"Find",MB_OK); 

}

void CGridView::Normal_LButtonDown(const LButtonDownEvent& e)
{
	OnLButtonDown(e);
	m_spCeller->OnLButtonDown(this, e);
	m_spCursorer->OnLButtonDown(this, e);
}
void CGridView::Normal_LButtonUp(const LButtonUpEvent& e)
{
	OnLButtonUp(e);
	m_spCursorer->OnLButtonUp(this, e);
	m_spCeller->OnLButtonUp(this, e);
}
void CGridView::Normal_LButtonClk(const LButtonClkEvent& e)
{
	OnLButtonClk(e);
	m_spCeller->OnLButtonClk(this, e);
}
void CGridView::Normal_LButtonSnglClk(const LButtonSnglClkEvent& e)
{
	OnLButtonSnglClk(e);
	m_spCeller->OnLButtonSnglClk(this, e);
}
void CGridView::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
	OnLButtonDblClk(e);
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
void CGridView::Normal_RButtonDown(const RButtonDownEvent& e)
{
	OnRButtonDown(e);
	m_spCursorer->OnRButtonDown(this, e);
}
void CGridView::Normal_MouseMove(const MouseMoveEvent& e)
{
	OnMouseMove(e);
	m_spCeller->OnMouseMove(this, e);
}
void CGridView::Normal_MouseLeave(const MouseLeaveEvent& e)
{
	OnMouseLeave(e);
	m_spCeller->OnMouseLeave(this, e);
}
bool CGridView::Normal_Guard_SetCursor(const SetCursorEvent& e)
{
	return e.HitTest == HTCLIENT;
}

void CGridView::Normal_SetCursor(const SetCursorEvent& e)
{
	OnSetCursor(e);
	m_spRowTracker->OnSetCursor(this, e);
	m_spColTracker->OnSetCursor(this, e);
	m_spCeller->OnSetCursor(this, e);
}
void CGridView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	OnContextMenu(e);
	m_spCeller->OnContextMenu(this, e);
}
void CGridView::Normal_KeyDown(const KeyDownEvent& e)
{
	OnKeyDown(e);
	m_spCursorer->OnKeyDown(this, e);
	m_spCeller->OnKeyDown(this, e);
}

//RowTrack
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

//ColTrack
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

//VScrollDrag
void CGridView::VScrlDrag_OnEntry()
{
	m_pVScroll->SetState(UIElementState::Dragged);
}

void CGridView::VScrlDrag_OnExit()
{
	m_pVScroll->SetState(UIElementState::Normal);
}

void CGridView::VScrlDrag_LButtonDown(const LButtonDownEvent& e)
{
	m_pVScroll->SetStartDrag(GetDirectPtr()->Pixels2DipsY(e.Point.y));
}

bool CGridView::VScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_pVScroll->GetVisible() && m_pVScroll->GetThumbRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point));
}

void CGridView::VScrlDrag_LButtonUp(const LButtonUpEvent& e)
{
	m_pVScroll->SetStartDrag(0.f);
}

void CGridView::VScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pVScroll->SetScrollPos(
		m_pVScroll->GetScrollPos() +
		(GetDirectPtr()->Pixels2DipsY(e.Point.y) - m_pVScroll->GetStartDrag()) *
		m_pVScroll->GetScrollDistance() /
		m_pVScroll->GetRect().Height());
	m_pVScroll->SetStartDrag(GetDirectPtr()->Pixels2DipsY(e.Point.y));
}

//HScrollDrag
void CGridView::HScrlDrag_OnEntry()
{
	m_pHScroll->SetState(UIElementState::Dragged);
}

void CGridView::HScrlDrag_OnExit()
{
	m_pHScroll->SetState(UIElementState::Normal);
}

void CGridView::HScrlDrag_LButtonDown(const LButtonDownEvent& e)
{
	m_pHScroll->SetStartDrag(GetDirectPtr()->Pixels2DipsX(e.Point.x));
}

bool CGridView::HScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_pHScroll->GetVisible() && m_pVScroll->GetThumbRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point));
}

void CGridView::HScrlDrag_LButtonUp(const LButtonUpEvent& e)
{
	m_pHScroll->SetStartDrag(0.f);
}

void CGridView::HScrlDrag_MouseMove(const MouseMoveEvent& e)
{
	m_pHScroll->SetScrollPos(
		m_pHScroll->GetScrollPos() +
		(GetDirectPtr()->Pixels2DipsX(e.Point.x) - m_pHScroll->GetStartDrag()) *
		m_pHScroll->GetScrollDistance() /
		m_pHScroll->GetRect().Height());
	m_pHScroll->SetStartDrag(GetDirectPtr()->Pixels2DipsX(e.Point.x));
}

//RowDrag
void CGridView::RowDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	m_spRowDragger->OnBeginDrag(this, e);
}

bool CGridView::RowDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spRowDragger->IsTarget(this, e);
}

void CGridView::RowDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spRowDragger->OnDrag(this, e);
}

void CGridView::RowDrag_LButtonUp(const LButtonUpEvent& e)
{
	m_spRowDragger->OnEndDrag(this, e);
}

void CGridView::RowDrag_MouseLeave(const MouseLeaveEvent& e)
{
	m_spRowDragger->OnLeaveDrag(this, e);
}

//ColDrag
void CGridView::ColDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	m_spColDragger->OnBeginDrag(this, e);
}

bool CGridView::ColDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spColDragger->IsTarget(this, e);
}

void CGridView::ColDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spColDragger->OnDrag(this, e);
}

void CGridView::ColDrag_LButtonUp(const LButtonUpEvent& e)
{
	m_spColDragger->OnEndDrag(this, e);
}

void CGridView::ColDrag_MouseLeave(const MouseLeaveEvent& e)
{
	m_spColDragger->OnLeaveDrag(this, e);
}

//ItemDrag
void CGridView::ItemDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	m_spItemDragger->OnBeginDrag(this, e);
}

bool CGridView::ItemDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spItemDragger->IsTarget(this, e);
}

void CGridView::ItemDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spItemDragger->OnDrag(this, e);
}

void CGridView::ItemDrag_LButtonUp(const LButtonUpEvent& e)
{
	m_spCursorer->OnLButtonUp(this, e);
	m_spCeller->OnLButtonUp(this, e);

	m_spItemDragger->OnEndDrag(this, e);
}

void CGridView::ItemDrag_MouseLeave(const MouseLeaveEvent& e)
{
	m_spItemDragger->OnLeaveDrag(this, e);
}

//Edit
void CGridView::Edit_BeginEdit(const BeginEditEvent& e)
{
	CTextCell* pCell = static_cast<CTextCell*>(e.CellPtr);

	D2DTextbox* pEdit = new D2DTextbox(
		this,
		pCell,
		pCell->GetPropertyPtr(),
		[pCell]() -> std::basic_string<TCHAR> {
			return pCell->GetString();
		},
		[pCell](const std::basic_string<TCHAR>& str) -> void {
			pCell->SetString(str);
		},
			[pCell](const std::basic_string<TCHAR>& str) -> void {
			if (pCell->CanSetStringOnEditing()) {
				pCell->SetString(str);
			}
		},
			[pCell]()->void {
			pCell->SetEditPtr(nullptr);
			pCell->GetSheetPtr()->GetGridPtr()->SetEditPtr(nullptr);
			pCell->SetState(UIElementState::Normal);//After Editing, Change Normal
		}
		);
	pEdit->OnCreate(CreateEvent(this, NULL, NULL));
	pCell->SetEditPtr(pEdit);
	SetEditPtr(pEdit);
}

void CGridView::Edit_OnExit()
{
	GetEditPtr()->OnClose(CloseEvent(this, NULL, NULL));
}
void CGridView::Edit_MouseMove(const MouseMoveEvent& e)
{
	if (GetEditPtr()->GetClientRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point))) {
		GetEditPtr()->OnMouseMove(e);
	}
	else {
		Normal_MouseMove(e);
	}
}

bool CGridView::Edit_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return !GetEditPtr()->GetClientRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point));
}

void CGridView::Edit_LButtonDown(const LButtonDownEvent& e)
{
	GetEditPtr()->OnLButtonDown(e);
}

void CGridView::Edit_LButtonUp(const LButtonUpEvent& e)
{
	if (GetEditPtr()->GetClientRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point))) {
		GetEditPtr()->OnLButtonUp(e);
	}
	else {
		Normal_LButtonUp(e);
	}
}

bool CGridView::Edit_Guard_KeyDown(const KeyDownEvent& e)
{

	if ((e.Char == VK_RETURN) && !(::GetKeyState(VK_MENU) & 0x8000) ||
		(e.Char == VK_TAB) && !(::GetKeyState(VK_MENU) & 0x8000)) {
		//Commit Edit
		return true;
	}
	else if (e.Char == VK_ESCAPE) {
		//CancelEdit
		GetEditPtr()->CancelEdit();
		return true;
	}
	else {
		return false;
	}
}

void CGridView::Edit_KeyDown(const KeyDownEvent& e)
{
	GetEditPtr()->OnKeyDown(e);
}

void CGridView::Edit_Char(const CharEvent& e)
{
	GetEditPtr()->OnChar(e);
}

//Window Message
void CGridView::BeginEdit(CCell* pCell)
{
	m_pMachine->process_event(BeginEditEvent(this, pCell));
	PostUpdate(Updates::Invalidate);
	SubmitUpdate();
	return;
}


LRESULT CGridView::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMachine->process_event(RButtonDownEvent(this, wParam, lParam));

	bHandled = FALSE;
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseStateMachine->LButtonDown(LButtonDownEvent(this, wParam, lParam));

	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseStateMachine->LButtonUp(LButtonUpEvent(this, wParam, lParam));

	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseStateMachine->LButtonDblClk(LButtonDblClkEvent(this, wParam, lParam));

	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnLButtonDblClkTimeExceed(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseStateMachine->LButtonDblClkTimeExceed(LButtonDblClkTimeExceedEvent(this, wParam, lParam));

	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseStateMachine->MouseLeave(MouseLeaveEvent(this, wParam, lParam));//TODOTODO
	SubmitUpdate();
	return 0;
}


LRESULT CGridView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMachine->process_event(MouseMoveEvent(this, wParam, lParam));

	PostUpdate(Updates::Invalidate);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMachine->process_event(SetCursorEvent(this, wParam, lParam, bHandled));

	bHandled = FALSE; //Default Handled = FALSE meand Arrow
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMachine->process_event(CharEvent(this, wParam, lParam));

	SubmitUpdate();
	bHandled = TRUE;
	return 0;
}

LRESULT CGridView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMachine->process_event(KeyDownEvent(this, wParam, lParam));

	PostUpdate(Updates::Invalidate);
	SubmitUpdate();
	bHandled = FALSE;
	return 0;
}

LRESULT CGridView::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint ptClient((short)LOWORD(lParam), (short)HIWORD(lParam));
	ScreenToClient(ptClient);//Necessary to convert Client
	m_pMachine->process_event(ContextMenuEvent(this, wParam, MAKELPARAM(ptClient.x, ptClient.y)));

	SubmitUpdate();
	return 0;
}



//GridView Message
void CGridView::OnLButtonDown(const LButtonDownEvent& e) 
{
	m_keepEnsureVisibleFocusedCell = false;
	if (m_isFocusable) { SetFocus(); }
	SetCapture();
}
void CGridView::OnLButtonUp(const LButtonUpEvent& e) 
{
	ReleaseCapture();
}
void CGridView::OnLButtonClk(const LButtonClkEvent& e) {}
void CGridView::OnLButtonSnglClk(const LButtonSnglClkEvent& e) {}
void CGridView::OnLButtonDblClk(const LButtonDblClkEvent& e) {}
void CGridView::OnLButtonBeginDrag(const LButtonBeginDragEvent& e) {}
void CGridView::OnRButtonDown(const RButtonDownEvent& e) 
{
	m_keepEnsureVisibleFocusedCell = false;
	if (m_isFocusable) { SetFocus(); }
}
void CGridView::OnMouseMove(const MouseMoveEvent& e) 
{ 
	//TrackMouseEvent
	TRACKMOUSEEVENT stTrackMouseEvent;
	stTrackMouseEvent.cbSize = sizeof(stTrackMouseEvent);
	stTrackMouseEvent.dwFlags = TME_LEAVE;
	stTrackMouseEvent.hwndTrack = m_hWnd;
	::TrackMouseEvent(&stTrackMouseEvent);
}
void CGridView::OnMouseLeave(const MouseLeaveEvent& e) {}
void CGridView::OnSetCursor(const SetCursorEvent& e){}
void CGridView::OnKeyDown(const KeyDownEvent& e)
{
	switch (e.Char) {
	case VK_HOME:
		m_pVScroll->SetScrollPos(0);
		return;
	case VK_END:
		m_pVScroll->SetScrollPos(m_pVScroll->GetScrollRange().second - m_pVScroll->GetScrollPage());
		return;
	case VK_PRIOR: // Page Up
		m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollPage());
		return;
	case VK_NEXT: // Page Down
		m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() + m_pVScroll->GetScrollPage());
		return;
	default:
		break;
	}

}
void CGridView::OnChar(const CharEvent& e) {}
void CGridView::OnBeginEdit(const BeginEditEvent& e) {}
void CGridView::OnPaint(const PaintEvent& e)
{
	if (!Visible())return;

	CRect rcClient(GetClientRect());

	//Update PaintDictionary
	UpdateRowPaintDictionary();
	UpdateColumnPaintDictionary();

	DEBUG_OUTPUT_COLUMN_PAINT_DICTIONARY

		//Paint
	{
		auto & colDictionary = m_columnPaintDictionary.get<IndexTag>();
		auto& rowDictionary = m_rowPaintDictionary.get<IndexTag>();
		for (auto colIter = colDictionary.rbegin(),colEnd = colDictionary.rend(); colIter != colEnd; ++colIter) {
			for (auto rowIter = rowDictionary.rbegin(),rowEnd = rowDictionary.rend(); rowIter != rowEnd; ++rowIter) {
				colIter->DataPtr->Cell(rowIter->DataPtr.get())->OnPaint(e);
			}
		}
	}

		//Paint Focused Line
		if (::GetFocus() == m_hWnd) {
			d2dw::CRectF rcFocus(m_pDirect->Pixels2Dips(rcClient));
			rcFocus.DeflateRect(1.0f, 1.0f);
			m_pDirect->DrawSolidRectangle(*(m_spHeaderProperty->FocusedLine), rcFocus);
		}

	//Paint Column Drag Target Line
	m_spRowDragger->OnPaintDragLine(this, e);
	m_spColDragger->OnPaintDragLine(this, e);
	if (m_spItemDragger) { m_spItemDragger->OnPaintDragLine(this, e); }
}
void CGridView::OnContextMenu(const ContextMenuEvent& e)
{
	if (!Visible())return;
	auto cell = Cell(m_pDirect->Pixels2Dips(e.Point));
	if (!cell) {
		CMenu* pMenu = GetContextMenuPtr();
		if (pMenu) {
			CPoint ptScreen(e.Point);
			HWND hWnd = e.WndPtr->m_hWnd;
			SetContextMenuRowColumn(CRowColumn());
			::ClientToScreen(hWnd, &ptScreen);
			::SetForegroundWindow(hWnd);
			pMenu->TrackPopupMenu(0, ptScreen.x, ptScreen.y, hWnd);
		}
	}
}

CGridView::CGridView(
	std::shared_ptr<GridViewProperty>& spGridViewProp,
	CMenu* pContextMenu)
	:
	m_spGridViewProp(spGridViewProp),
	CSheet(spGridViewProp->HeaderPropPtr, spGridViewProp->CellPropPtr, spGridViewProp->CellPropPtr, pContextMenu ? pContextMenu : &CGridView::ContextMenu),
	CWnd(),
	m_pMouseStateMachine(std::make_shared<CMouseStateMachine>(this)),
	m_pVScroll(std::make_unique<d2dw::CVScroll>(this, spGridViewProp->VScrollPropPtr)),
	m_pHScroll(std::make_unique<d2dw::CHScroll>(this, spGridViewProp->HScrollPropPtr)),
	m_pMachine(new boost::sml::sm<Machine>{this})
{
	//RegisterArgs and CreateArgs
	RegisterClassExArgument()
		.lpszClassName(_T("CGridView"))
		.style(CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)(COLOR_3DFACE + 1));
	CreateWindowExArgument()
		.lpszClassName(_T("CGridView"))
		.lpszWindowName(_T("GridView"))
		.dwStyle(WS_CHILD | WS_CLIPCHILDREN)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"PropertyGridView"));
	//Add Message
	AddMsgHandler(WM_CREATE, &CGridView::OnCreate, this);
	AddMsgHandler(WM_CLOSE, &CGridView::OnClose, this);
	AddMsgHandler(WM_DESTROY, &CGridView::OnDestroy, this);
	AddMsgHandler(WM_ERASEBKGND, &CGridView::OnEraseBkGnd, this);
	AddMsgHandler(WM_SIZE, &CGridView::OnSize, this);
	AddMsgHandler(WM_PAINT, &CGridView::OnPaint, this);
	AddMsgHandler(WM_MOUSEWHEEL, &CGridView::OnMouseWheel, this);
	AddMsgHandler(WM_KILLFOCUS, &CGridView::OnKillFocus, this);

	AddMsgHandler(WM_RBUTTONDOWN, &CGridView::OnRButtonDown, this);
	AddMsgHandler(WM_LBUTTONDOWN, &CGridView::OnLButtonDown, this);
	AddMsgHandler(WM_LBUTTONUP, &CGridView::OnLButtonUp, this);
	AddMsgHandler(WM_LBUTTONDBLCLK, &CGridView::OnLButtonDblClk, this);
	AddMsgHandler(WM_CONTEXTMENU, &CGridView::OnContextMenu, this);

	AddMsgHandler(WM_MOUSEMOVE, &CGridView::OnMouseMove, this);
	AddMsgHandler(WM_MOUSELEAVE, &CGridView::OnMouseLeave, this);
	AddMsgHandler(WM_SETCURSOR, &CGridView::OnSetCursor, this);
	AddMsgHandler(WM_CHAR, &CGridView::OnChar, this);
	AddMsgHandler(WM_KEYDOWN, &CGridView::OnKeyDown, this);
	AddMsgHandler(WM_FILTER, &CGridView::OnFilter, this);
	AddMsgHandler(WM_LBUTTONDBLCLKTIMEXCEED, &CGridView::OnLButtonDblClkTimeExceed, this);
	AddMsgHandler(WM_DELAY_UPDATE, &CGridView::OnDelayUpdate, this);

	//AddCmdCdHandler(EN_CHANGE,&CGridView::OnCmdEnChange,this);
	AddCmdIDHandler(ID_HD_COMMAND_EDITHEADER, &CGridView::OnCommandEditHeader, this);
	AddCmdIDHandler(ID_HD_COMMAND_DELETECOLUMN, &CGridView::OnCommandDeleteColumn, this);

	AddCmdIDHandler(IDM_SELECTALL, &CGridView::OnCommandSelectAll, this);
	AddCmdIDHandler(IDM_DELETE, &CGridView::OnCommandDelete, this);
	AddCmdIDHandler(IDM_COPY, &CGridView::OnCommandCopy, this);
	AddCmdIDHandler(IDM_PASTE, &CGridView::OnCommandPaste, this);
	AddCmdIDHandler(IDM_FIND, &CGridView::OnCommandFind, this);

	CellLButtonClk.connect(std::bind(&CGridView::OnCellLButtonClk, this, std::placeholders::_1));
	CellContextMenu.connect(std::bind(&CGridView::OnCellContextMenu, this, std::placeholders::_1));
}

CGridView::~CGridView() = default;






