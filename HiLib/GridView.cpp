#include <olectl.h>
#include <numeric>
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
#include "ConsoleTimer.h"
#include "ApplicationProperty.h"
#include "Tracker.h"
#include "Cursorer.h"
#include "Dragger.h"
#include "SheetState.h"
#include "MyClipboard.h"
#include "SheetEventArgs.h"
#include "Celler.h"
#include "Scroll.h"
#include "Textbox.h"
#include "FindDlg.h"

#include "TextCell.h"
#include "MouseStateMachine.h"
#include "GridViewProperty.h"
#include "ResourceIDFactory.h"


extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;
UINT CGridView::WM_DELAY_UPDATE = ::RegisterWindowMessage(L"CGridView::WM_DELAY_UPDATE");

CMenu CGridView::ContextMenu;

CGridView::CGridView(
	std::shared_ptr<GridViewProperty>& spGridViewProp,
	CMenu* pContextMenu)
	:
	m_spGridViewProp(spGridViewProp),
	CSheet(spGridViewProp, pContextMenu ? pContextMenu : &CGridView::ContextMenu),
	CWnd(),
	m_pMouseMachine(new CMouseStateMachine(this)),
	m_pVScroll(std::make_unique<d2dw::CVScroll>(this, spGridViewProp->VScrollPropPtr, std::bind(&CGridView::OnVScrollPropertyChanged, this, phs::_1))),
	m_pHScroll(std::make_unique<d2dw::CHScroll>(this, spGridViewProp->HScrollPropPtr, std::bind(&CGridView::OnHScrollPropertyChanged, this, phs::_1)))
{
	m_pMachine.reset(new CGridStateMachine(this));

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
	AddMsgHandler(WM_SETFOCUS, &CGridView::OnSetFocus, this);

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
	AddMsgHandler(WM_SYSKEYDOWN, &CGridView::OnKeyDown, this);
	AddMsgHandler(WM_FILTER, &CGridView::OnFilter, this);
	AddMsgHandler(WM_LBUTTONDBLCLKTIMEXCEED, &CGridView::OnLButtonDblClkTimeExceed, this);
	AddMsgHandler(WM_DELAY_UPDATE, &CGridView::OnDelayUpdate, this);

	AddMsgHandler(WM_CANCELMODE, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT
		{
			m_pMouseMachine->process_event(CancelModeEvent(this, wParam, lParam));
			return 0;
		});
	AddMsgHandler(WM_CAPTURECHANGED, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& hHandled)->LRESULT
		{
			m_pMouseMachine->process_event(CaptureChangedEvent(this, wParam, lParam));
			return 0;
		});

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

void CGridView::OnCellContextMenu(CellContextMenuEventArgs& e)
{}

void CGridView::OnCellLButtonClk(CellEventArgs& e)
{
	//if(e.CellPtr->GetRowPtr()==GetNameHeaderRowPtr().get())
	//{	
	//	auto pCol=e.CellPtr->GetColumnPtr();
	//	Sorts sort=pCol->GetSort();
	//	//Reset Sort
	//	ResetColumnSort();
	//	//Sort
	//	switch(sort){
	//		case Sorts::None:
	//		case Sorts::Down:
	//			pCol->SetSort(Sorts::Up);
	//			break;
	//		case Sorts::Up:
	//			pCol->SetSort(Sorts::Down);
	//			break;
	//		default:
	//			pCol->SetSort(Sorts::None);
	//			break;
	//	}
	//	SubmitUpdate();
	//}
}

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
		(*iter)->SetVisible(true);
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
			(*iter)->SetVisible(true);
		};
		//Set Filter
		for (auto colIter = std::next(m_allCols.begin(), m_frozenColumnCount), colEnd = m_allCols.end(); colIter != colEnd; ++colIter) {
			auto strFilter = (*colIter)->Cell(GetFilterRowPtr().get())->GetString();
			if (strFilter.empty() || strFilter == std::wstring(L""))continue;
			std::vector<std::wstring> vstrFilter;
			boost::split(vstrFilter, strFilter, boost::is_space());
			for (auto rowIter = std::next(m_allRows.begin(), m_frozenRowCount), end = m_allRows.end(); rowIter != end; ++rowIter) {
				if (!(*rowIter)->GetVisible())continue;
				//Filter
				auto pCell = Cell(*rowIter, *colIter);
				for (const auto& str : vstrFilter) {
					if (str[0] == L'-' && str.size() >= 2) {
						std::wstring strMinus(str.substr(1));
						if (pCell->Filter(strMinus)) {
							(*rowIter)->SetVisible(false);
						}
					} else if (!(pCell->Filter(str))) {
						(*rowIter)->SetVisible(false);
					}
				}
			}
		}
	}
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
						std::shared_ptr<CCell> pCell = CSheet::Cell(*rowIter, ptr);
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
					std::shared_ptr<CCell> pCell = CSheet::Cell(*rowIter, ptr);
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

	FLOAT top = GetCellProperty()->Line->Width * 0.5f;

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

void CGridView::UpdateColumn()
{
	if (!Visible()) { return; }

	FLOAT left = GetCellProperty()->Line->Width / 2.0f;

	for (auto& colPtr : m_visCols) {
		if (colPtr->GetIndex<VisTag>() == GetFrozenCount<ColTag>()) {
			left -= GetHorizontalScrollPos();
		}
		colPtr->SetLeft(left, false);
		for (auto rowPtr : m_visRows) {
			std::shared_ptr<CCell> pCell = CSheet::Cell(rowPtr, colPtr);
			if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
				pSheetCell->UpdateAll();
			}
		}
		left += colPtr->GetWidth();
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
	FLOAT lineHalfWidth = GetCellProperty()->Line->Width * 0.5f;

	rcVertical.left = rcClient.right - ::GetSystemMetrics(SM_CXVSCROLL) - lineHalfWidth;
	rcVertical.top = rcClient.top + lineHalfWidth;
	rcVertical.right = rcClient.right - lineHalfWidth;
	rcVertical.bottom = rcClient.bottom - (m_pHScroll->GetVisible()?(m_pHScroll->GetScrollBandWidth() + lineHalfWidth) : lineHalfWidth);
	m_pVScroll->SetRect(rcVertical);

//	auto a = m_pHScroll->GetVisible() ? (::GetSystemMetrics(SM_CYHSCROLL) + lineHalfWidth) : lineHalfWidth;

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
		if(m_rocoContextMenu.GetRowPtr()==GetNameHeaderRowPtr()){
			if(auto pCell=std::dynamic_pointer_cast<CParentHeaderCell>(Cell(m_rocoContextMenu.GetRowPtr(), m_rocoContextMenu.GetColumnPtr()))){
				pCell->OnEdit(EventArgs());
			}
		}
	}
	return 0;
}

LRESULT CGridView::OnCommandDeleteColumn(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	if(!m_rocoContextMenu.IsInvalid()){
		EraseColumn(m_rocoContextMenu.GetColumnPtr());
	}
	return 0;
}

LRESULT CGridView::OnCommandResizeSheetCell(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	if(!m_rocoContextMenu.IsInvalid()){
		if(auto p = std::dynamic_pointer_cast<CSheetCell>(CSheet::Cell(m_rocoContextMenu.GetRowPtr(),m_rocoContextMenu.GetColumnPtr()))){
			p->Resize();
		}
	}
	return 0;
}


LRESULT CGridView::OnCommandSelectAll(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	SelectAll();
	return 0;
}

LRESULT CGridView::OnCommandCopy(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
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
}

d2dw::CRectF CGridView::GetPaintRect()
{
	return m_pDirect->Pixels2Dips(GetClientRect());
}

std::pair<bool, bool> CGridView::GetHorizontalVerticalScrollNecessity()
{
	d2dw::CRectF rcClient(m_pDirect->Pixels2Dips(GetClientRect()));
	d2dw::CRectF rcCells(GetCellsRect());
	d2dw::CPointF ptOrigin(GetFrozenPoint());
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
	d2dw::CPointF ptOrigin(GetFrozenPoint());
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
	//m_rowHeaderHeader = std::shared_ptr<CRow>();
	//m_rowNameHeader=std::shared_ptr<CRow>();
	//m_rowFilter=std::shared_ptr<CRow>();
}

CColumn* CGridView::GetParentColumnPtr(CCell* pCell)
{
	return pCell->GetColumnPtr();
}

bool CGridView::GetIsFocused()const
{
	auto hWndAct = ::GetActiveWindow();
	auto hWndFcs = ::GetFocus();
	auto hWndFore = ::GetForegroundWindow();

	return hWndFcs == m_hWnd ||
		(HWND)::GetWindowWord(hWndAct, GWL_HWNDPARENT) == m_hWnd ||
		(HWND)::GetWindowWord(hWndFcs, GWL_HWNDPARENT) == m_hWnd ||
		(HWND)::GetWindowWord(hWndFore, GWL_HWNDPARENT) == m_hWnd ||
		(HWND)::GetWindow(hWndAct, GW_OWNER) == m_hWnd ||
		(HWND)::GetWindow(hWndFcs, GW_OWNER) == m_hWnd ||
		(HWND)::GetWindow(hWndFore, GW_OWNER) == m_hWnd ||
		(HWND)::GetParent(hWndAct) == m_hWnd ||
		(HWND)::GetParent(hWndFcs) == m_hWnd ||
		(HWND)::GetParent(hWndFore) == m_hWnd;
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

	//Scan from Min to Max and Jump if Find
	auto jumpToFindNextCell = [&](auto rowIter, auto colIter, auto rowEnd, auto colEnd)->bool{
		while(1){

			auto spCell = CSheet::Cell(*rowIter, *colIter);
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

	MessageBox((L"\"" + findWord + L"\" is not found!").c_str(), L"Find",MB_OK); 

}

void CGridView::FindPrev(const std::wstring& findWord, bool matchCase, bool matchWholeWord)
{
	//Fiding start from focused cell
	//If focused cell is invalid(Not focused), MinMax Visible Cell is start point
	auto focused = m_spCursorer->GetFocusedCell();

	//Scan from Min to Max and Jump if Find
	auto jumpToFindPrevCell = [&](auto rowIter, auto colIter, auto rowEnd, auto colEnd)->bool{
		while(1){

			auto spCell = CSheet::Cell(*rowIter, *colIter);
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

	MessageBox((L"\"" + findWord + L"\" is not found!").c_str(), L"Find",MB_OK); 

}

/**********/
/* Normal */
/**********/
void CGridView::Normal_Paint(const PaintEvent& e)
{
	if (!Visible())return;

	CSheet::Normal_Paint(e);
	CRect rcClient(GetClientRect());

	//Paint Focused Line
	if (GetIsFocused() ){
		d2dw::CRectF rcFocus(m_pDirect->Pixels2Dips(rcClient));
		rcFocus.DeflateRect(1.0f, 1.0f);
		m_pDirect->DrawSolidRectangle(*(GetHeaderProperty()->FocusedLine), rcFocus);
	}
}

void CGridView::Normal_LButtonDown(const LButtonDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	if (m_isFocusable) { SetFocus(); }
//setcap	SetCapture();
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
	if (m_isFocusable) { SetFocus(); }

	CSheet::Normal_RButtonDown(e);
}
void CGridView::Normal_MouseMove(const MouseMoveEvent& e)
{
	//TrackMouseEvent
	TRACKMOUSEEVENT stTrackMouseEvent;
	stTrackMouseEvent.cbSize = sizeof(stTrackMouseEvent);
	stTrackMouseEvent.dwFlags = TME_LEAVE;
	stTrackMouseEvent.hwndTrack = m_hWnd;
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
	} else {
		CSheet::Normal_ContextMenu(e);
	}
}
void CGridView::Normal_KeyDown(const KeyDownEvent& e)
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
	m_pVScroll->SetState(UIElementState::Normal);
}
void CGridView::VScrlDrag_LButtonDown(const LButtonDownEvent& e)
{
	//setcaptureSetCapture();
	m_pVScroll->SetStartDrag(GetDirectPtr()->Pixels2DipsY(e.Point.y));
}
bool CGridView::VScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_pVScroll->GetVisible() && m_pVScroll->GetThumbRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point));
}
void CGridView::VScrlDrag_LButtonUp(const LButtonUpEvent& e)
{
	//setcaptureReleaseCapture();
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

/***************/
/* HScrollDrag */
/***************/
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
	return m_pHScroll->GetVisible() && m_pHScroll->GetThumbRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point));
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
		m_pHScroll->GetRect().Width());
	m_pHScroll->SetStartDrag(GetDirectPtr()->Pixels2DipsX(e.Point.x));
}

//Edit
//In case of Message, Check nullptr. In case of Guard, not.
//void CGridView::Edit_BeginEdit(const BeginEditEvent& e)
//{
//}

//void CGridView::Edit_EndEdit(const EndEditEvent& e)
//{
//	ReleaseCapture();
//	if (GetEditPtr()) {
//		GetEditPtr()->OnClose(CloseEvent(this, NULL, NULL));
//	}
//}

void CGridView::Edit_OnEntry(const BeginEditEvent& e)
{
	if (auto pCell = dynamic_cast<CTextCell*>(e.CellPtr)) {

		D2DTextbox* pEdit = new D2DTextbox(
			this,
			pCell,
			pCell->GetCellPropertyPtr(),
			pCell->GetString(),
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
				pCell->GetSheetPtr()->GetGridPtr()->SetEditPtr(nullptr);
				pCell->SetString(str);
				pCell->SetState(UIElementState::Normal);//After Editing, Change Normal
			}
			);
		//pEdit->OnCreate(CreateEvent(this, NULL, NULL));
		SetEditPtr(pEdit);
		PostUpdate(Updates::Invalidate);
		//SetCapture();
	}
}


void CGridView::Edit_OnExit()
{
	ReleaseCapture();
	if (auto pEdit = GetEditPtr()) {
		GetEditPtr()->OnClose(CloseEvent(this, NULL, NULL));
		delete pEdit;
	}
}
void CGridView::Edit_MouseMove(const MouseMoveEvent& e)
{
	if (GetEditPtr()) {
		if (GetEditPtr()->GetClientRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point))) {
			GetEditPtr()->OnMouseMove(e);
		} else {
			Normal_MouseMove(e);
		}
	}
}

bool CGridView::Edit_Guard_LButtonDown(const LButtonDownEvent& e)
{
	auto ret = !GetEditPtr()->GetClientRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point));
//	if (ret) { SetEditPtr(nullptr); }
	return ret;
}

void CGridView::Edit_LButtonDown(const LButtonDownEvent& e)
{
	if (GetEditPtr()) {
		GetEditPtr()->OnLButtonDown(e);
	}
}

void CGridView::Edit_LButtonUp(const LButtonUpEvent& e)
{
	if (GetEditPtr()) {
		if (GetEditPtr()->GetClientRect().PtInRect(GetDirectPtr()->Pixels2Dips(e.Point))) {
			//GetEditPtr()->OnLButtonUp(e);
		} else {
			Normal_LButtonUp(e);
		}
	}
}

bool CGridView::Edit_Guard_KeyDown(const KeyDownEvent& e)
{

	if ((e.Char == VK_RETURN && !(::GetKeyState(VK_MENU) & 0x8000)) ||
		(e.Char == VK_TAB && !(::GetKeyState(VK_MENU) & 0x8000))) {
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

bool CGridView::Edit_Guard_KeyDownWithNormal(const KeyDownEvent& e)
{
	return Edit_Guard_KeyDown(e);
}

bool CGridView::Edit_Guard_KeyDownWithoutNormal(const KeyDownEvent& e)
{
	return false;
}

void CGridView::Edit_KeyDown(const KeyDownEvent& e)
{
	if (GetEditPtr()) {

		GetEditPtr()->OnKeyDown(e);
	}
}

void CGridView::Edit_Char(const CharEvent& e)
{
	if (GetEditPtr()) {
		GetEditPtr()->OnChar(e);
	}
}


/******************/
/* Window Message */
/******************/
void CGridView::BeginEdit(CCell* pCell)
{
	OnBeginEdit(BeginEditEvent(this, pCell));
	return;
}

void CGridView::EndEdit()
{
	OnEndEdit(EndEditEvent(this));
	return;
}

LRESULT CGridView::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CSheet::OnRButtonDown(RButtonDownEvent(this, wParam, lParam));
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseMachine->process_event(LButtonDownEvent(this, wParam, lParam));
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseMachine->process_event(LButtonUpEvent(this, wParam, lParam));
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseMachine->process_event(LButtonDblClkEvent(this, wParam, lParam));
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnLButtonDblClkTimeExceed(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pMouseMachine->process_event(LButtonDblClkTimeExceedEvent(this, wParam, lParam));
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CSheet::OnMouseLeave(MouseLeaveEvent(this, wParam, lParam));
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	auto e = MouseMoveEvent(this, wParam, lParam);
	m_pMouseMachine->process_event(e);
	CSheet::OnMouseMove(e);
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE; //Default Handled = FALSE means Arrow
	CSheet::OnSetCursor(SetCursorEvent(this, wParam, lParam, bHandled));
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	auto e = CharEvent(this, wParam, lParam);
	m_pMouseMachine->process_event(e);
	CSheet::OnChar(e);
	bHandled = TRUE;
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CSheet::OnKeyDown(KeyDownEvent(this, wParam, lParam, bHandled));
	SubmitUpdate();
	return 0;
}
LRESULT CGridView::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	CPoint ptClient((short)LOWORD(lParam), (short)HIWORD(lParam));
	ScreenToClient(ptClient);//Necessary to convert Client
	CSheet::OnContextMenu(ContextMenuEvent(this, wParam, MAKELPARAM(ptClient.x, ptClient.y), bHandled));
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnFilter(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FilterAll();
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Direct2DWrite
	m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);
	return 0;
}

LRESULT CGridView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
		::SetForegroundWindow(hWnd);
	}

	DestroyWindow();
	return 0;
}

LRESULT CGridView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CGridView::OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//For Back buffering
	bHandled = TRUE;
	return 1;
}

LRESULT CGridView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1_SIZE_U{ LOWORD(lParam), HIWORD(lParam) });
	SizeChanged();
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	m_pDirect->BeginDraw();

	m_pDirect->ClearSolid(*(m_spGridViewProp->BackgroundPropPtr->m_brush));
	PaintEvent e(this);
	CSheet::OnPaint(e);
	if (m_pEdit && m_pEdit->GetIsVisible()) {
		m_pEdit->OnPaint(e);
	}
	m_pVScroll->OnPaint(e);
	m_pHScroll->OnPaint(e);

	m_pDirect->EndDraw();
	return 0;
}

LRESULT CGridView::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CSheet::OnSetFocus(SetFocusEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	SubmitUpdate();
	return 0;
}


LRESULT CGridView::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CSheet::OnKillFocus(KillFocusEvent(this, wParam, lParam));
	InvalidateRect(NULL, FALSE);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_keepEnsureVisibleFocusedCell = false;
	m_pVScroll->SetScrollPos(m_pVScroll->GetScrollPos() - m_pVScroll->GetScrollDelta() * GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnDelayUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SPDLOG_INFO("CGridView::OnDelayUpdate");

	CONSOLETIMER("OnDelayUpdate Total");
	SignalPreDelayUpdate();
	SignalPreDelayUpdate.disconnect_all_slots();
	PostUpdate(Updates::Filter);

	if (m_keepEnsureVisibleFocusedCell) {
		PostUpdate(Updates::EnsureVisibleFocusedCell);
	}
	else {
		m_setUpdate.erase(Updates::EnsureVisibleFocusedCell);
	}
	SubmitUpdate();
	return 0;
}





