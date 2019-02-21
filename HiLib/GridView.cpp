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
#include "MyGdiPlusHelper.h"
#include "Gdipluspixelformats.h"
#include "MyClipboard.h"
#include "SheetEventArgs.h"

#include <olectl.h>

#include "FullCommand.h"

#include "FindDlg.h"

#include "TextCell.h"
#include "MouseStateMachine.h"
#include "GridViewProperty.h"
#include "ResourceIDFactory.h"
#include "InplaceEdit.h"


extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;
UINT CGridView::WM_DELAY_UPDATE = ::RegisterWindowMessage(L"CGridView::WM_DELAY_UPDATE");

CMenu CGridView::ContextMenu;

CGridView::CGridView(
	std::shared_ptr<CGridViewProperty> spGridViewProp,
	CMenu* pContextMenu)
	:
	m_spGridViewProp(spGridViewProp),
	m_spBackgroundProperty(spGridViewProp->m_spBackgroundProperty),
	CSheet(spGridViewProp->m_spPropHeader, spGridViewProp->m_spPropCell, spGridViewProp->m_spPropCell, pContextMenu ? pContextMenu : &CGridView::ContextMenu),
	m_spDeltaScroll(spGridViewProp->m_spDeltaScroll),
	CWnd(),
	m_filterIosv(), m_filterWork(m_filterIosv), m_filterTimer(m_filterIosv),
	m_invalidateIosv(), m_invalidateWork(m_invalidateIosv), m_invalidateTimer(m_invalidateIosv),
	m_pMouseStateMachine(std::make_shared<CMouseStateMachine>(this))
{
	boost::thread th1(boost::bind(&boost::asio::io_service::run, &m_filterIosv));
	boost::thread th2(boost::bind(&boost::asio::io_service::run, &m_invalidateIosv));
	//RegisterArgs and CreateArgs
	RegisterClassExArgument()
		.lpszClassName(_T("CGridView"))
		.style(CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)(COLOR_3DFACE + 1));
	CreateWindowExArgument()
		.lpszClassName(_T("CGridView"))
		.lpszWindowName(_T("GridView"))
		.dwStyle(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"PropertyGridView"));
	//Scroll
	m_vertical.CreateWindowExArgument()
		.dwStyle(SBS_RIGHTALIGN|SBS_VERT|WS_CHILD|WS_VISIBLE)
		.hMenu((HMENU)1234);
	m_horizontal.CreateWindowExArgument()
		.dwStyle(SBS_BOTTOMALIGN|SBS_HORZ|WS_CHILD|WS_VISIBLE)
		.hMenu((HMENU)12345);
	//Add Message
	AddMsgHandler(WM_CREATE,&CGridView::OnCreate,this);
	AddMsgHandler(WM_CLOSE,&CGridView::OnClose,this);
	AddMsgHandler(WM_DESTROY,&CGridView::OnDestroy,this);
	AddMsgHandler(WM_ERASEBKGND,&CGridView::OnEraseBkGnd,this);
	AddMsgHandler(WM_SIZE,&CGridView::OnSize,this);
	AddMsgHandler(WM_PAINT,&CGridView::OnPaint,this);
	AddMsgHandler(WM_VSCROLL,&CGridView::OnVScroll,this);
	AddMsgHandler(WM_HSCROLL,&CGridView::OnHScroll,this);
	AddMsgHandler(WM_MOUSEWHEEL,&CGridView::OnMouseWheel,this);
	AddMsgHandler(WM_KILLFOCUS, &CGridView::OnKillFocus, this);

	AddMsgHandler(WM_RBUTTONDOWN,&CGridView::OnRButtonDown,this);
	AddMsgHandler(WM_LBUTTONDOWN,&CGridView::OnLButtonDown,this);
	AddMsgHandler(WM_LBUTTONUP,&CGridView::OnLButtonUp,this);
	AddMsgHandler(WM_LBUTTONDBLCLK,&CGridView::OnLButtonDblClk,this);
	AddMsgHandler(WM_CONTEXTMENU,&CGridView::OnContextMenu,this);

	AddMsgHandler(WM_MOUSEMOVE,&CGridView::OnMouseMove,this);
	AddMsgHandler(WM_MOUSELEAVE,&CGridView::OnMouseLeave,this);
	AddMsgHandler(WM_SETCURSOR,&CGridView::OnSetCursor,this);
	AddMsgHandler(WM_KEYDOWN,&CGridView::OnKeyDown,this);
	AddMsgHandler(WM_FILTER,&CGridView::OnFilter,this);
	AddMsgHandler(WM_LBUTTONDBLCLKTIMEXCEED,&CGridView::OnLButtonDblClkTimeExceed,this);
	AddMsgHandler(WM_DELAY_UPDATE, &CGridView::OnDelayUpdate, this);

	//AddCmdCdHandler(EN_CHANGE,&CGridView::OnCmdEnChange,this);
	AddCmdIDHandler(ID_HD_COMMAND_EDITHEADER,&CGridView::OnCommandEditHeader,this);
	AddCmdIDHandler(ID_HD_COMMAND_DELETECOLUMN,&CGridView::OnCommandDeleteColumn,this);

	AddCmdIDHandler(IDM_SELECTALL,&CGridView::OnCommandSelectAll,this);
	AddCmdIDHandler(IDM_DELETE,&CGridView::OnCommandDelete,this);
	AddCmdIDHandler(IDM_COPY,&CGridView::OnCommandCopy,this);
	AddCmdIDHandler(IDM_PASTE,&CGridView::OnCommandPaste,this);
	AddCmdIDHandler(IDM_FIND,&CGridView::OnCommandFind,this);

	CellLButtonClk.connect(std::bind(&CGridView::OnCellLButtonClk,this,std::placeholders::_1));
	CellContextMenu.connect(std::bind(&CGridView::OnCellContextMenu,this,std::placeholders::_1));
}

CGridView::~CGridView()
{
	m_filterTimer.cancel();
	m_invalidateTimer.cancel();
}

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
			boost::for_each(vstrFilter,[&](const std::wstring& str){
				if(str[0]==L'-' && str.size()>=2){
					std::wstring strMinus(str.substr(1));
					if(pCell->Filter(strMinus)){
						rowIter->DataPtr->SetVisible(false);				
					}
				}else if(!(pCell->Filter(str))){
					rowIter->DataPtr->SetVisible(false);
				}
			});
		}		
	}
}

//LRESULT CGridView::OnCmdEnChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
//{
//	::SendMessage(hWndCtl,WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),NULL);
//	return 0;
//}

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
	//Scroll
	m_vertical.Create(m_hWnd);
	m_horizontal.Create(m_hWnd);
	m_vertical.SetScrollPage(0);
	m_horizontal.SetScrollPage(0);
	m_vertical.SetScrollRange(0,0);
	m_horizontal.SetScrollRange(0,0);
	m_vertical.SetScrollPos(0);
	m_horizontal.SetScrollPos(0);

	//V4::FRectF rc2(0, 0, 400, 400);
	//m_pTextBox = new V4::D2DTextbox(nullptr);
	//m_pTextBox->CreateWindow(nullptr, nullptr, rc2, V4::STAT::VISIBLE, L"txtbox");

	return 0;
}

LRESULT CGridView::OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	m_vertical.DestroyWindow();
	m_horizontal.DestroyWindow();
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
	CRect rcClient(GetClientRect());
#ifndef USE_ID2D1DCRenderTarget
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1_SIZE_U{ (UINT)rcClient.Width(), (UINT)rcClient.Height() });
#else
	if (m_upBuffDC.get() == nullptr || rcClient.Width() > m_upBuffDC->GetSize().cx || rcClient.Height() > m_upBuffDC->GetSize().cy) {
		CClientDC dc(m_hWnd);
		m_upBuffDC.reset(new CBufferDC(dc, rcClient.Width(), rcClient.Height()));
	}
#endif
	SizeChanged();
	SubmitUpdate();
	return 0;
}


LRESULT CGridView::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
#ifdef USE_ID2D1DCRenderTarget
	CRect rcClient(GetClientRect());
	m_pDirect->BeginDraw(m_upBuffDC->GetHDC(), rcClient);
#else
	m_pDirect->BeginDraw();
#endif

	//Because of CLIPCHILD, No region clip necessary
	//CRect rcClip = GetClientRect();
	//if(m_vertical.IsWindowVisible()){	
	//	rcClip.right -= m_vertical.GetWindowRect().Width();
	//}
	//if(m_horizontal.IsWindowVisible()){
	//	rcClip.bottom -= m_horizontal.GetWindowRect().Height();
	//}
	//m_pDirect->GetHwndRenderTarget()->PushAxisAlignedClip(m_pDirect->Pixels2Dips(rcClip), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	m_pDirect->ClearSolid(*(m_spGridViewProp->m_spBackgroundProperty->m_brush));

	OnPaint(PaintEvent(this, *m_pDirect));

	//m_pDirect->GetHwndRenderTarget()->PopAxisAlignedClip();

	m_pDirect->EndDraw();
#ifdef USE_ID2D1DCRenderTarget
	CRgn rgn;
	rgn.CreateRectRgnIndirect(rcClient);
	dc.SelectClipRgn(rgn);
	dc.BitBlt(rcClient.left, rcClient.top,
		rcClient.Width(),
		rcClient.Height(),
		*m_upBuffDC.get(), 0, 0, SRCCOPY);
	dc.SelectClipRgn(NULL);
#endif
	return 0;
}

LRESULT CGridView::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	InvalidateRect(NULL, FALSE);
	return 0;
}


LRESULT CGridView::OnVScroll(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	SetFocus();

	SCROLLINFO si={0};
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_ALL;
	m_vertical.GetScrollInfo(&si);
	int nPos=si.nPos;
	switch(LOWORD(wParam)){
	case SB_TOP:
		si.nPos=si.nMin;
		break;
	case SB_BOTTOM:
		si.nPos=si.nMax;
		break;
	case SB_LINEUP:
		si.nPos-=1;
		break;
	case SB_LINEDOWN:
		si.nPos+=1;
		break;
	case SB_PAGEUP:
		si.nPos-=si.nPage;
		break;
	case SB_PAGEDOWN:
		si.nPos+=si.nPage;
		break;
	case SB_THUMBPOSITION:
		si.nPos=si.nPos;
		break;
	case SB_THUMBTRACK:
		si.nPos=si.nTrackPos;
		break;
	default:
		break;
	}
	m_vertical.SetScrollPos(si.nPos);
	Scroll();
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnHScroll(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	SetFocus();

	SCROLLINFO si={0};
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_ALL;
	m_horizontal.GetScrollInfo(&si);
	int nPos=si.nPos;
	switch(LOWORD(wParam)){
	case SB_LEFT:
		si.nPos=si.nMin;
		break;
	case SB_RIGHT:
		si.nPos=si.nMax;
		break;
	case SB_LINELEFT:
		si.nPos-=1;
		break;
	case SB_LINERIGHT:
		si.nPos+=1;
		break;
	case SB_PAGELEFT:
		si.nPos-=si.nPage;
		break;
	case SB_PAGERIGHT:
		si.nPos+=si.nPage;
		break;
	case SB_THUMBPOSITION:
		si.nPos=si.nPos;
		break;
	case SB_THUMBTRACK:
		si.nPos=si.nTrackPos;
		break;
	default:
		break;
	}
	m_horizontal.SetScrollPos(si.nPos);
	Scroll();
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnMouseWheel(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	int nScrolled = *m_spDeltaScroll*GET_WHEEL_DELTA_WPARAM(wParam)/120;
        if (nScrolled > 0){
            for(int i=0;i<nScrolled;i++){
                PostMessage(WM_VSCROLL, SB_LINEUP, 0);
			}
		}else{
            for(int i=0;i>nScrolled;i--){
                PostMessage(WM_VSCROLL, SB_LINEDOWN, 0);
			}
		}
	return 0;
}

void CGridView::Invalidate()
{
	InvalidateRect(NULL,FALSE);
}

void CGridView::DelayUpdate()
{
	m_invalidateTimer.expires_from_now(boost::posix_time::milliseconds(30));
	m_invalidateTimer.async_wait([this](const boost::system::error_code& error)->void {

		if (error == boost::asio::error::operation_aborted) {
		} else {
			PostMessage(WM_DELAY_UPDATE, NULL, NULL);
		}
	});
}

LRESULT CGridView::OnDelayUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOST_LOG_TRIVIAL(trace) << L"CGridView::OnDelayUpdate";

	CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"OnDelayUpdate Total")
	SignalPreDelayUpdate();
	SignalPreDelayUpdate.disconnect_all_slots();
	PostUpdate(Updates::Filter);
	//Need to remove EnsureVisibleFocusedCell. Otherwise scroll to 0 when scrolling
	m_setUpdate.erase(Updates::EnsureVisibleFocusedCell);
	SubmitUpdate();
	return 0;
}

void CGridView::UpdateScrolls()
{
	if(!Visible())return;

	//Client
	CRect rcClient(GetClientRect());

	//Origin
	//d2dw::CPointF ptOrigin(GetOriginPoint());

	//Scroll Range
	d2dw::CRectF rcCells(GetCellsRect());
	m_vertical.SetScrollRange(0,m_pDirect->Dips2PixelsY(rcCells.Height()));
	m_horizontal.SetScrollRange(0,rcCells.Width());

	//Scroll Page
	d2dw::CRectF rcPage(GetPageRect());

	m_vertical.SetScrollPage(m_pDirect->Dips2PixelsY(rcPage.Height()));
	m_horizontal.SetScrollPage(m_pDirect->Dips2PixelsX(rcPage.Width()));

	//EnableShow Vertical
	SCROLLINFO si={0};
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_RANGE|SIF_PAGE|SIF_POS;
	m_vertical.GetScrollInfo(&si);
		
	if(si.nMax<=(int)si.nPage){
		if(m_vertical.IsWindowEnabled()){
			m_vertical.EnableWindow(FALSE);
			m_vertical.ShowWindow(SW_HIDE);
		}
	}else{
		if(!m_vertical.IsWindowEnabled()){
			m_vertical.EnableWindow(TRUE);
			m_vertical.ShowWindow(SW_SHOW);			
		}
	}
	//EnableShow Horizontal
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_RANGE|SIF_PAGE|SIF_POS;
	m_horizontal.GetScrollInfo(&si);
		
	if(si.nMax<=(int)si.nPage){
		if(m_horizontal.IsWindowEnabled()){
			m_horizontal.EnableWindow(FALSE);
			m_horizontal.ShowWindow(SW_HIDE);
		}
	}else{
		if(!m_horizontal.IsWindowEnabled()){
			m_horizontal.EnableWindow(TRUE);
			m_horizontal.ShowWindow(SW_SHOW);			
		}
	}

	//Position scroll
	CRect rcVertical(rcClient);
	CRect rcHorizontal(rcClient);

	rcVertical.left=rcClient.right-::GetSystemMetrics(SM_CXVSCROLL);
	rcVertical.top=0;
	rcVertical.bottom-=(m_horizontal.IsWindowVisible())?::GetSystemMetrics(SM_CYHSCROLL):0;
	m_vertical.MoveWindow(rcVertical,TRUE);

	rcHorizontal.left=0;
	rcHorizontal.top=rcClient.bottom-::GetSystemMetrics(SM_CYHSCROLL);
	rcHorizontal.right-=(m_vertical.IsWindowVisible())?::GetSystemMetrics(SM_CXVSCROLL):0;
	m_horizontal.MoveWindow(rcHorizontal,TRUE);
}

CPoint CGridView::GetScrollPos()const
{
	return CPoint(m_horizontal.GetScrollPos(),m_vertical.GetScrollPos());
}

void CGridView::SetScrollPos(const CPoint& ptScroll)
{
	m_horizontal.SetScrollPos(ptScroll.x);
	m_vertical.SetScrollPos(ptScroll.y);
}

FLOAT CGridView::GetVerticalScrollPos()const
{
	return m_vertical.GetScrollPos();
}

FLOAT CGridView::GetHorizontalScrollPos()const
{
	return m_horizontal.GetScrollPos();
}

LRESULT CGridView::OnRButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	if (m_isFocusable) { SetFocus(); }
	bHandled=false;
	CPoint ptClient((short)LOWORD(lParam),(short)HIWORD(lParam));	
	RButtonDownEvent e(this, *m_pDirect, (UINT)wParam, ptClient);
	CSheet::OnRButtonDown(e);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	if (m_isFocusable) { SetFocus(); }
	SetCapture();

	CPoint ptClient((short)LOWORD(lParam),(short)HIWORD(lParam));	
	LButtonDownEvent e(this, *m_pDirect, (UINT)wParam,ptClient);
	m_pMouseStateMachine->LButtonDown(e);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnLButtonUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	ReleaseCapture();

	CPoint ptClient((short)LOWORD(lParam),(short)HIWORD(lParam));	
	LButtonUpEvent e(this, *m_pDirect, (UINT)wParam,ptClient);
	m_pMouseStateMachine->LButtonUp(e);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnLButtonDblClk(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	CPoint ptClient((short)LOWORD(lParam),(short)HIWORD(lParam));	
	LButtonDblClkEvent e(this, *m_pDirect, (UINT)wParam,ptClient);
	m_pMouseStateMachine->LButtonDblClk(e);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnLButtonDblClkTimeExceed(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint ptClient((short)LOWORD(lParam), (short)HIWORD(lParam));
	LButtonDblClkTimeExceedEvent e(this, *m_pDirect, (UINT)wParam, ptClient);
	m_pMouseStateMachine->LButtonDblClkTimeExceed(e);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint ptClient((short)LOWORD(lParam), (short)HIWORD(lParam));
	MouseLeaveEvent e(this, *m_pDirect, (UINT)wParam, ptClient);
	m_pMouseStateMachine->MouseLeave(e);
	SubmitUpdate();
	return 0;
}


LRESULT CGridView::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	//TrackMouseEvent
    TRACKMOUSEEVENT stTrackMouseEvent;
    stTrackMouseEvent.cbSize = sizeof(stTrackMouseEvent);
    stTrackMouseEvent.dwFlags = TME_LEAVE;
    stTrackMouseEvent.hwndTrack = m_hWnd;
    ::TrackMouseEvent( &stTrackMouseEvent );

	CPoint ptClient((short)LOWORD(lParam),(short)HIWORD(lParam));	
	MouseMoveEvent e(this, *m_pDirect, (UINT)wParam,ptClient);
	CSheet::OnMouseMove(e);
	PostUpdate(Updates::Invalidate);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnSetCursor(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	bHandled = FALSE; //Default Handled = FALSE meand Arrow
	if((UINT)LOWORD(lParam) == HTCLIENT){
		CSheet::OnSetCursor(SetCursorEvent(this, (UINT)LOWORD(lParam), bHandled));
	}else{
		bHandled = FALSE;
	}
	//PostUpdate(Updates::Invalidate);
	SubmitUpdate();
	return 0;
}

LRESULT CGridView::OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
//	m_pTextBox->WndProc(nullptr, uMsg, wParam, lParam);

	OnKeyDown(KeyDownEvent(wParam, lParam & 0xFF, lParam>>16 & 0xFF));
	//PostUpdate(Updates::Scrolls);
	SubmitUpdate();
	bHandled = FALSE;
	return 0;
}

void CGridView::OnKeyDown(const KeyDownEvent& e)
{
	switch (e.Char)
	{
	case VK_HOME:
		SendMessage(WM_VSCROLL, SB_TOP, 0);
		break;
	case VK_END:
		SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
		break;
	case VK_PRIOR: // Page Up
		SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
		break;
	case VK_NEXT: // Page Down
		SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
		break;
	//case VK_DELETE:
	//	{
	//		BOOL bHandled=FALSE;
	//		OnCommandDelete(0,0,NULL,bHandled);
	//		break;
	//	}
	default:
		break;
	}

	CSheet::OnKeyDown(e);
}

void CGridView::OnContextMenu(const ContextMenuEvent& e)
{
	if(!Visible())return;
	auto cell = Cell(m_pDirect->Pixels2Dips(e.Point));
	if(!cell){
		CMenu* pMenu = GetContextMenuPtr(); 
		if(pMenu){
			CPoint ptScreen(e.Point);
			HWND hWnd = e.WindowPtr->m_hWnd;
			SetContextMenuRowColumn(CRowColumn());
			::ClientToScreen(hWnd, &ptScreen);
			::SetForegroundWindow(hWnd);
			pMenu->TrackPopupMenu(0,ptScreen.x,ptScreen.y,hWnd);
		}
	}else{
		cell->OnContextMenu(e);
	}
}

LRESULT CGridView::OnContextMenu(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	CPoint ptClient((short)LOWORD(lParam),(short)HIWORD(lParam));	
	ScreenToClient(ptClient);//Necessary to convert Client
	ContextMenuEvent e(this, ptClient);
	OnContextMenu(e);
	SubmitUpdate();
	return 0;
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

//Status CGridView::SaveGIFWithNewColorTable(
//  Image *pImage,
//  IStream* pIStream,
//  const CLSID* clsidEncoder,
//  DWORD nColors,
//  BOOL fTransparent
//)
//{
//    Status stat = GenericError;
//
//    // GIF codec supports 256 colors maximum.
//    if (nColors > 256)
//        nColors = 256;
//    if (nColors < 2)
//        nColors = 2;
//
//    // Make a new 8-BPP pixel indexed bitmap that is the same size as the source image.
//    DWORD   dwWidth = pImage->GetWidth();
//    DWORD   dwHeight = pImage->GetHeight();
//
//    // Always use PixelFormat8BPPIndexed, because that is the color table
//    // based interface to the GIF codec.
//    Bitmap  bitmap(dwWidth, dwHeight, PixelFormat8bppIndexed);
//
//    stat = bitmap.GetLastStatus();
//
//    if (stat != Ok)
//        return stat;        // No point in continuing.
//
//    // Allocate the new color table.
//    DWORD   dwSizeColorPalette;
//    dwSizeColorPalette = sizeof(ColorPalette);      // Size of core structure.
//    dwSizeColorPalette += sizeof(ARGB)*(nColors-1);   // The other entries.
//
//    // Allocate some raw space to back the ColorPalette structure pointer.
//    ColorPalette *ppal = (ColorPalette *)new BYTE[dwSizeColorPalette];
//    if (ppal == NULL) return OutOfMemory;
//
//    ZeroMemory(ppal, dwSizeColorPalette);
//
//    // Initialize a new color table with entries that are determined by
//    // some optimal palette finding algorithm; for demonstration 
//    // purposes, just do a grayscale. 
//    if (fTransparent)
//        ppal->Flags = PaletteFlagsHasAlpha;
//    else
//        ppal->Flags = 0; 
//    ppal->Flags |= PaletteFlagsGrayScale;
//    ppal->Count = nColors;
//    for (UINT i = 0; i < nColors; i++)
//    {
//        int Alpha = 0xFF;       // Colors are opaque by default.
//        int Intensity = i*0xFF/(nColors-1); // even distribution 
//
//        // The GIF encoder makes the first entry in the palette with a
//        // zero alpha the "transparent" color in the GIF.
//        // For demonstration purposes, the first one is picked arbitrarily.
//
//        if ( i == 0 && fTransparent) // Make this color index...
//            Alpha = 0;          // Transparent
//        
//        // Create a gray scale for demonstration purposes.
//        // Otherwise, use your favorite color reduction algorithm
//        // and an optimum palette for that algorithm generated here.
//        // For example, a color histogram, or a median cut palette.
//        ppal->Entries[i] = Color::MakeARGB( Alpha, 
//                                            Intensity, 
//                                            Intensity, 
//                                            Intensity );
//    }
//
//    // Set the palette into the new Bitmap object.
//    bitmap.SetPalette(ppal);
//
//
//    // Use GetPixel below to pull out the color data of Image.
//    // Because GetPixel isn't defined on an Image, make a copy in a Bitmap 
//    // instead. Make a new Bitmap that is the same size of the image that
//    // you want to export. Otherwise, you might try to interpret the native 
//    // pixel format of the image by using a LockBits call.
//    // Use PixelFormat32BppARGB so that you can wrap a Graphics around it.
//    Bitmap BmpCopy(dwWidth, dwHeight, PixelFormat32bppARGB); 
//    stat = BmpCopy.GetLastStatus();
//    if (stat == Ok)
//    {
//        Graphics g(&BmpCopy);
//
//        // Transfer the Image to the Bitmap.
//        stat = g.DrawImage(pImage, 0, 0, dwWidth, dwHeight);
//
//        // g goes out of scope here and cleans up.
//    }
//
//    if (stat != Ok)
//    {
//        delete [] (LPBYTE) ppal;
//        ppal = NULL;
//        return stat;
//    }
//
//    // Lock the whole of the bitmap for writing.
//    BitmapData  bitmapData;
//    CRect        rect(0, 0, dwWidth, dwHeight);
//
//    stat = bitmap.LockBits(
//      &rect,
//      ImageLockModeWrite,
//      PixelFormat8bppIndexed,
//      &bitmapData);
//
//    if (stat == Ok)
//    {
//        // Write to the temporary buffer provided by LockBits.
//        // Copy the pixels from the source image in this loop.
//        // Because you want an index, convert RGB to the appropriate
//        // palette index here.
//        BYTE *pixels;
//        if (bitmapData.Stride > 0)
//            pixels = (BYTE*) bitmapData.Scan0;
//        else
//            // If the Stride is negative, Scan0 points to the last             // scanline in the buffer.
//            // To normalize the loop, obtain the start of the buffer,
//            // which is located (Height-1) scanlines previous.
//            pixels = (BYTE*) bitmapData.Scan0 + bitmapData.Stride*(dwHeight-1);
//        UINT stride = abs(bitmapData.Stride);
//
//        // Top-down and bottom-up is not relevant to this algorithm.
//
//        for(UINT row = 0; row < dwHeight; ++row)
//        {
//          for(UINT col = 0; col < dwWidth; ++col)
//          {
//              // Map palette indexes for a grayscale.
//              // If you use some other technique to color convert,
//              // put your favorite color reduction algorithm here.
//              Color     pixel;
//              UINT      i8BppPixel = row * stride + col;
//
//              BmpCopy.GetPixel(col, row, &pixel);
//
//              // Use luminance/chrominance conversion to get grayscale.
//              // Basically, turn the image into black and white TV: YCrCb.
//              // You do not have to to calculate Cr or Cb because you 
//              // throw away the color anyway.
//              // Y = Red * 0.299 + Green * 0.587 + Blue * 0.114
//
//              // This expression is best as integer math for performance,
//              // however, because GetPixel listed earlier is the slowest 
//              // part of this loop, the expression is left as 
//              // floating point for clarity.
//              double luminance = (pixel.GetRed() * 0.299) +
//                                (pixel.GetGreen() * 0.587) +
//                                (pixel.GetBlue() * 0.114);
//
//              // Gray scale is an intensity map from black to white.
//              // Compute the index to the gray scale entry that  
//              // approximates the luminance, and then round the index.      
//              // Also, constrain the index choices by the number of colors to do
//              pixels[i8BppPixel] = (BYTE)(luminance * (nColors-1)/255 +0.5);
//          }
//        }
//    // To commit the changes, unlock the portion of the bitmap.  
//        stat = bitmap.UnlockBits(&bitmapData);
//    }
//
//    // If destination work was successful, see whether the source was successful.
//    if (stat == Ok) stat = BmpCopy.GetLastStatus();
//
//    // See whether the code has been successful to this point.
//    if (stat == Ok)
//    {
//    // Write it out to the disk.
//		//EncoderParameters encs[3];
//		//ULONG depth = 8;
//		//ULONG compression = EncoderValueCompressionLZW;
//		//ULONG quality = 0;
//		//encs->Count = 3;
//		//encs->Parameter[0].Guid = EncoderCompression;
//		//encs->Parameter[0].NumberOfValues = 1;
//		//encs->Parameter[0].Type = EncoderParameterValueTypeLong;
//		//encs->Parameter[0].Value = &compression;
//
//		//encs->Parameter[1].Guid = EncoderColorDepth;
//		//encs->Parameter[1].NumberOfValues = 1;
//		//encs->Parameter[1].Type = EncoderParameterValueTypeLong;
//		//encs->Parameter[1].Value = &depth;
//
//		//encs->Parameter[2].Guid = EncoderQuality;
//		//encs->Parameter[2].NumberOfValues = 1;
//		//encs->Parameter[2].Type = EncoderParameterValueTypeLong;
//		//encs->Parameter[2].Value = &quality;
//        stat =  bitmap.Save(pIStream, clsidEncoder, NULL);
//    }
//
//    // Clean up after yourself.
//    delete [] (LPBYTE) ppal;
//    ppal = NULL;
//    // BmpCopy goes away on its own when it falls out of scope.
//
//    return stat;
//}
//LRESULT CGridView::OnCommandPrintScreen(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
//{
//	CPoint ptScroll=GetScrollPos();
//	SetScrollPos(CPoint(0,0));
//	CSize sz(MeasureSize());
//	CClientDC dc(m_hWnd);
//
//	CBufferDC dcBuff(dc, sz.cx, sz.cy);
//
//	d2dw::CRectF rcPaint = GetPaintRect();
//
//	dcBuff.SetBkMode(TRANSPARENT);
//	//Bitmap
//	{
//
//		PaintEvent paintEvent(&dcBuff);
//		OnPaintAll(paintEvent);
//	
//		//Copy Bitmap to Clipboard
//		CClipboard clipboard;
//		if(clipboard.Open(m_hWnd)!=0){
//			clipboard.Empty();
//			clipboard.SetData(CF_BITMAP,dcBuff.GetBitMap());
//			clipboard.Close();
//		}
//	}
//
//	//JPEG,PNG,GIF
//	{
//		//Initialize GDI+
//		GdiplusStartupInput gdiplusStartupInput;
//		ULONG_PTR gdiplusToken;
//		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//		{
//			auto hWnd = m_hWnd;
//			std::function<void(HBITMAP,LPCTSTR,LPCTSTR)> setNonRegisteredTypeToClipboard =  [hWnd,this](HBITMAP hBitmap, LPCTSTR mimetype, LPCTSTR format)->void 
//			{
//				IStream* pIStream = NULL;
//				if(::CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pIStream)!=S_OK){
//					throw std::exception("Error on OnCommandPrintScreen");
//				}
//
//				CLSID clsid;	
//
//				Bitmap bitmap(hBitmap, (HPALETTE)GetStockObject(DEFAULT_PALETTE));
//
//				if (GdiplusHelper::GetEncoderClsid(mimetype, &clsid) < 0){
//					throw std::exception("Error on OnCommandPrintScreen");
//				}
//
//				//Status status = SaveGIFWithNewColorTable(&bitmap,
//				//						  pIStream,
//				//						  &clsid,
//				//						  256,
//				//						  FALSE);
//				Status status;
//
//
//				if(_tcsicmp(mimetype, L"image/jpeg")==0){
//					EncoderParameters encs[1];
//					ULONG quality = 80;
//					encs->Count = 1;
//
//					encs->Parameter[0].Guid = EncoderQuality;
//					encs->Parameter[0].NumberOfValues = 1;
//					encs->Parameter[0].Type = EncoderParameterValueTypeLong;
//					encs->Parameter[0].Value = &quality;
//					status = bitmap.Save(pIStream, &clsid, encs);
//				}else if(_tcsicmp(mimetype, L"image/tiff")==0){
//					EncoderParameters encs[2];
//					ULONG depth = 24;
//					ULONG compression = EncoderValueCompressionLZW;
//					encs->Count = 2;
//
//					encs->Parameter[0].Guid = EncoderColorDepth;
//					encs->Parameter[0].NumberOfValues = 1;
//					encs->Parameter[0].Type = EncoderParameterValueTypeLong;
//					encs->Parameter[0].Value = &depth;
//
//					encs->Parameter[1].Guid = EncoderCompression;
//					encs->Parameter[1].NumberOfValues = 1;
//					encs->Parameter[1].Type = EncoderParameterValueTypeLong;
//					encs->Parameter[1].Value = &compression;
//
//
//					status = bitmap.Save(pIStream, &clsid, encs);
//				}else{
//					status = bitmap.Save(pIStream, &clsid, NULL);
//				}
//
//				if(status != Status::Ok){
//					pIStream->Release();
//					throw std::exception("Error on OnCommandPrintScreen");
//				}
//
//				HGLOBAL hGlobal = NULL;
//				if(::GetHGlobalFromStream(pIStream, &hGlobal)!=S_OK){
//					pIStream->Release();
//					throw std::exception("Error on OnCommandPrintScreen");
//				}
//
//				//Copy to Clipboard
//				CClipboard clipboard;
//				if(clipboard.Open(hWnd)!=0){
//					//::EmptyClipboard();
//					if(_tcsicmp(mimetype, L"image/tiff")==0){
//						clipboard.SetData(CF_TIFF, hGlobal);
//					}else{
//						clipboard.SetData(::RegisterClipboardFormat(format), hGlobal);
//					}
//					clipboard.Close();
//				}
//				pIStream->Release();
//			};
//
//			setNonRegisteredTypeToClipboard(dcBuff.GetBitMap(), L"image/jpeg", L"JFIF");
//			setNonRegisteredTypeToClipboard(dcBuff.GetBitMap(), L"image/png", L"PNG");
//			setNonRegisteredTypeToClipboard(dcBuff.GetBitMap(), L"image/gif", L"GIF");
//			setNonRegisteredTypeToClipboard(dcBuff.GetBitMap(), L"image/tiff", L"TIFF");
//		}
//		//Terminate GDI+
//		GdiplusShutdown(gdiplusToken);
//	}
//
//	//EnhMetaFile
//	{
//		//SIZEL lt;
//		//SIZEL rb;
//		//lt.cx = -rcPaint.left;
//		//lt.cy = -rcPaint.top;
//		//rb.cx = sz.cx + rcPaint.left;
//		//rb.cy = sz.cy + rcPaint.top;
//		//DPtoHIMETRIC(&lt);
//		//DPtoHIMETRIC(&rb);
//		//auto rcEnhMeta = CRect(lt.cx, lt.cy, rb.cx, rb.cy);
//		//SIZEL          sizel;
//		//sizel.cx = sz.cx;
//		//sizel.cy = sz.cy;
//		//DPtoHIMETRIC(&sizel);
//		//auto rcEnhMeta=CRect(0,0,sizel.cx,sizel.cy);
//
//		//CDC dcEnhMeta((HDC)CreateEnhMetaFile(dc,NULL,rcEnhMeta,L"DcmDesigner"));
//
//		//dcEnhMeta.SetBkMode(TRANSPARENT);
//
//		//PaintEventArgs paintEventArgs(&dcEnhMeta);
//		//OnPaintAll(paintEventArgs);
//		
//		HENHMETAFILE hEmf = GetAllEnhMetaFileData();
//
//		//Copy EnhMetaFile to Clipboard
//		CClipboard clipboard;
//		if(clipboard.Open(m_hWnd)!=0){
//			//::EmptyClipboard();
//			clipboard.SetData(CF_ENHMETAFILE,hEmf);
//			clipboard.Close();
//		}
//	}
//
//	SetScrollPos(ptScroll);
//	return 0;
//}

//HGLOBAL CGridView::GetPaintMetaFileData()
//{
//	HMETAFILE      hmf;
//	HGLOBAL        hglobal;
//	LPMETAFILEPICT lpmf;
//	SIZEL          sizel;
//	CSize sz(GetClientRect().Size());
//
//	CDC dc((HDC)CreateMetaFile(NULL));
//
//	SetMapMode(dc, MM_ANISOTROPIC);
//	SetWindowOrgEx(dc, 0, 0, NULL);
//	SetWindowExtEx(dc, sz.cx, sz.cy, NULL);
//
//	//Draw
//	dc.FillRect(GetClientRect(),((HBRUSH)GetStockObject(GRAY_BRUSH)));
//	dc.SetBkMode(TRANSPARENT);
//
//	PaintEvent paintEvent(&dc);
//	OnPaint(paintEvent);
//
//	hmf = CloseMetaFile(dc);
//
//	sizel.cx = sz.cx;
//	sizel.cy = sz.cy;
//	DPtoHIMETRIC(&sizel);
//
//	hglobal = GlobalAlloc(GPTR, sizeof(METAFILEPICT));
//	lpmf = (LPMETAFILEPICT)GlobalLock(hglobal);
//	lpmf->hMF = hmf;
//	lpmf->mm = MM_ANISOTROPIC;
//	lpmf->xExt = sizel.cx;
//	lpmf->yExt = sizel.cy;
//	GlobalUnlock(hglobal);
//
//	return hglobal;
//}
//
//HENHMETAFILE CGridView::GetPaintEnhMetaFileData()
//{
//	CSize sz(GetClientRect().Size());
//	CClientDC dc(m_hWnd);
//	SIZEL sizel;
//	sizel.cx = sz.cx;
//	sizel.cy = sz.cy;
//	DPtoHIMETRIC(&sizel);
//	auto rcEnhMeta=CRect(0,0,sizel.cx,sizel.cy);
//
//	CDC dcEnhMeta((HDC)CreateEnhMetaFile(dc,NULL,rcEnhMeta,L"DcmDesigner"));
//
//	dcEnhMeta.SetBkMode(TRANSPARENT);
//
//	PaintEvent paintEvent(&dcEnhMeta);
//	OnPaint(paintEvent);
//
//	return CloseEnhMetaFile(dcEnhMeta);
//}
//
//HENHMETAFILE CGridView::GetAllEnhMetaFileData()
//{
//	CSize sz(MeasureSize());
//	CClientDC dc(m_hWnd);
//	SIZEL sizel;
//	sizel.cx = sz.cx;
//	sizel.cy = sz.cy;
//	DPtoHIMETRIC(&sizel);
//	auto rcEnhMeta=CRect(0,0,sizel.cx,sizel.cy);
//
//	CDC dcEnhMeta((HDC)CreateEnhMetaFile(dc,NULL,rcEnhMeta,L"DcmDesigner"));
//
//	dcEnhMeta.SetBkMode(TRANSPARENT);
//
//	PaintEvent paintEvent(&dcEnhMeta);
//	OnPaintAll(paintEvent);
//
//	return CloseEnhMetaFile(dcEnhMeta);
//}

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


//void CGridView::OnPaintAll(const PaintEvent& e)
//{
//	DEBUG_OUTPUT_COLUMN_VISIBLE_DICTIONARY
//
//	if(!Visible())return;
//
//	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
//	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
//	//Check
//	CheckEqualRange(rowDictionary.begin(),rowDictionary.end(), colDictionary.find(0), colDictionary.end(),
//		[](CCell* pCell, Compares comp)->void{
//			switch(comp){
//			case Compares::Diff:
//			case Compares::DiffNE:
//				pCell->SetChecked(true);
//				break;
//			case Compares::Same:
//				pCell->SetChecked(false);
//				break;
//			default:
//				pCell->SetChecked(true);
//				break;
//			}
//		});
//
//	//Paint
//
//	for(auto colIter=colDictionary.rbegin(),colEnd=colDictionary.rend();colIter!=colEnd;++colIter){
//		for(auto rowIter=rowDictionary.rbegin(),rowEnd=rowDictionary.rend();rowIter!=rowEnd;++rowIter){
//			colIter->DataPtr->Cell(rowIter->DataPtr.get())->OnPaint(e);
//		}
//	}
//}

void CGridView::OnPaint(const PaintEvent& e)
{
	CRect rcClient(GetClientRect());
	////Paint Background
	//if(*m_spBackgroundProperty->m_usePicture && ::GetFileAttributes(m_spBackgroundProperty->m_picturePath->c_str())!=0xffffffff){

	//	Gdiplus::GdiplusStartupInput gdiSI;
	//	ULONG_PTR gdiToken;

	//	// GDI+Start
	//	GdiplusStartup( &gdiToken, &gdiSI, NULL );

	//	Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromFile(m_spBackgroundProperty->m_picturePath->c_str());
	//	HDC hPicDC = ::CreateCompatibleDC(NULL);  
	//	HBITMAP hBitmap = NULL;
	//	pBitmap->GetHBITMAP(Gdiplus::Color::Transparent, &hBitmap);
	//	::SelectObject(hPicDC,hBitmap);


	//	switch(*m_spBackgroundProperty->m_picturePosition)
	//	{
	//		case PicturePositon::Fill:
	//		{
	//			double widthRatio = double(rcClient.Width())/double(pBitmap->GetWidth());
	//			double heightRatio = double(rcClient.Height())/double(pBitmap->GetHeight());
	//			FLOAT left,top,width,height;
	//			if(widthRatio > heightRatio){
	//				width = static_cast<FLOAT>(pBitmap->GetWidth()*widthRatio);
	//				height = static_cast<FLOAT>(pBitmap->GetHeight()*widthRatio);
	//			}else{
	//				width = static_cast<FLOAT>(pBitmap->GetWidth()*heightRatio);
	//				height = static_cast<FLOAT>(pBitmap->GetHeight()*heightRatio);
	//			}
	//			left = (rcClient.Width()-width)/2;
	//			top = (rcClient.Height()-height)/2;
	//			e.DCPtr->StretchBlt(left,top,width,height,hPicDC,0,0,pBitmap->GetWidth(),pBitmap->GetHeight(),SRCCOPY);
	//			break;
	//		}
	//		case PicturePositon::Fit:
	//		{
	//			double widthRatio = double(rcClient.Width())/double(pBitmap->GetWidth());
	//			double heightRatio = double(rcClient.Height())/double(pBitmap->GetHeight());
	//			FLOAT left,top,width,height;
	//			if(widthRatio > heightRatio){
	//				width = static_cast<FLOAT>(pBitmap->GetWidth()*heightRatio);
	//				height = static_cast<FLOAT>(pBitmap->GetHeight()*heightRatio);
	//			}else{
	//				width = static_cast<FLOAT>(pBitmap->GetWidth()*widthRatio);
	//				height = static_cast<FLOAT>(pBitmap->GetHeight()*widthRatio);
	//			}
	//			left = (rcClient.Width()-width)/2;
	//			top = (rcClient.Height()-height)/2;
	//			e.DCPtr->FillRect(rcClient,*(m_spBackgroundProperty->m_brush));
	//			e.DCPtr->StretchBlt(left,top,width,height,hPicDC,0,0,pBitmap->GetWidth(),pBitmap->GetHeight(),SRCCOPY);
	//			break;
	//		}
	//		case PicturePositon::Stretch:
	//		{
	//			e.DCPtr->StretchBlt(0,0,rcClient.Width(),rcClient.Height(),hPicDC,0,0,pBitmap->GetWidth(),pBitmap->GetHeight(),SRCCOPY);
	//			break;
	//		}
	//		case PicturePositon::Tile:
	//		{
	//			int x = static_cast<int>(ceil(double(rcClient.Width())/double(pBitmap->GetWidth())));
	//			int y = static_cast<int>(ceil(double(rcClient.Height())/double(pBitmap->GetHeight())));
	//			for(int i=0;i<x;i++){
	//				for(int j=0;j<y;j++){
	//					e.DCPtr->BitBlt(pBitmap->GetWidth()*i,pBitmap->GetHeight()*j,pBitmap->GetWidth(),pBitmap->GetHeight(),hPicDC,0,0,SRCCOPY);
	//				}
	//			}
	//			break;
	//		}
	//		case PicturePositon::Center:
	//		{
	//			int left = static_cast<int>((int(rcClient.Width())-int(pBitmap->GetWidth()))*0.5);
	//			int top = static_cast<int>((int(rcClient.Height())-int(pBitmap->GetHeight()))*0.5);
	//			e.DCPtr->FillRect(rcClient,*(m_spBackgroundProperty->m_brush));
	//			e.DCPtr->BitBlt(left,top,pBitmap->GetWidth(),pBitmap->GetHeight(),hPicDC,0,0,SRCCOPY);
	//			break;
	//		}
	//		default:
	//		{
	//			e.DCPtr->FillRect(rcClient,*(m_spBackgroundProperty->m_brush));
	//			break;
	//		}
	//	}
	//	

	//	//iStream->Release();
	//	//iPicture->Release();
	//	//::DeleteObject(hPic);
 //   // GDI+ E
	//	::DeleteObject(hBitmap);
	//	::DeleteObject(hPicDC);
	//	delete pBitmap;
	//	Gdiplus::GdiplusShutdown( gdiToken );
	//}else{
	//	e.DCPtr->FillRect(rcClient,*(m_spBackgroundProperty->m_brush));
	//}

	//e.DCPtr->SetBkMode(TRANSPARENT);

	//DEBUG_OUTPUT_COLUMN_VISIBLE_DICTIONARY

	if(!Visible())return;

	//Update PaintDictionary
	UpdateRowPaintDictionary();
	UpdateColumnPaintDictionary();

	DEBUG_OUTPUT_COLUMN_PAINT_DICTIONARY
		
	//Check
	//{
	//	auto& colDictionary=m_columnAllDictionary.get<IndexTag>();//Should be all
	//	auto& rowDictionary=m_rowPaintDictionary.get<IndexTag>();
	//	CheckEqualRange(rowDictionary.begin(),rowDictionary.end(), colDictionary.find(0), colDictionary.end(),
	//		[](CCell* pCell, Compares comp)->void{
	//			switch(comp){
	//			case Compares::Diff:
	//			case Compares::DiffNE:
	//				pCell->SetChecked(true);
	//				break;
	//			case Compares::Same:
	//				pCell->SetChecked(false);
	//				break;
	//			default:
	//				pCell->SetChecked(true);
	//				break;
	//			}
	//		});
	//}

	//Paint
	{
		auto& colDictionary=m_columnPaintDictionary.get<IndexTag>();
		auto& rowDictionary=m_rowPaintDictionary.get<IndexTag>();
		for(auto colIter=colDictionary.rbegin(),colEnd=colDictionary.rend();colIter!=colEnd;++colIter){
			for(auto rowIter=rowDictionary.rbegin(),rowEnd=rowDictionary.rend();rowIter!=rowEnd;++rowIter){
				colIter->DataPtr->Cell(rowIter->DataPtr.get())->OnPaint(e);
			}
		}
	}

	//Paint Focused Line
	if (::GetFocus() == m_hWnd || (m_pEdit && ::GetFocus() == m_pEdit->m_hWnd)) {
		d2dw::CRectF rcFocus(m_pDirect->Pixels2Dips(rcClient));
		rcFocus.DeflateRect(1.0f, 1.0f);
		m_pDirect->DrawSolidRectangle(*(m_spHeaderProperty->FocusedLine), rcFocus);
		//e.DCPtr->MoveToEx(rcFocus.left, rcFocus.top);
		//e.DCPtr->LineTo(rcFocus.left, rcFocus.bottom);
		//e.DCPtr->LineTo(rcFocus.right, rcFocus.bottom);
		//e.DCPtr->LineTo(rcFocus.right, rcFocus.top);
		//e.DCPtr->LineTo(rcFocus.left, rcFocus.top);
		//e.DCPtr->SelectPen(hPenOld);
	}



	//Paint Column Drag Target Line
	m_spRowDragger->OnPaintDragLine(this, e);
	m_spColDragger->OnPaintDragLine(this, e);
	if (m_spItemDragger) { m_spItemDragger->OnPaintDragLine(this, e); }

	//{
	//	auto& colDictionary=m_columnAllDictionary.get<IndexTag>();
	//	auto& rowVDictionary=m_rowVisibleDictionary.get<IndexTag>();
	//	auto dragToAllIndex = m_spColDragger->GetDragToAllIndex();
	//	auto dragFromAllIndex = m_spColDragger->GetDragFromAllIndex();
	//	if(dragToAllIndex!=CColumn::kInvalidIndex && dragToAllIndex!=dragFromAllIndex){
	//		FLOAT x=0;
	//		if(dragToAllIndex<=colDictionary.begin()->Index){
	//			auto pColumn = colDictionary.begin()->DataPtr;
	//			x = pColumn->GetLeft();
	//		}else if(dragToAllIndex>boost::prior(colDictionary.end())->Index){
	//			auto pColumn = boost::prior(colDictionary.end())->DataPtr;
	//			x = pColumn->GetRight();
	//		}else{
	//			auto pColumn = Index2Pointer<ColTag, AllTag>(dragToAllIndex);
	//			x = pColumn->GetLeft();
	//		}
	//		FLOAT y0 = rowVDictionary.begin()->DataPtr->GetTop();
	//		FLOAT y1 = boost::prior(rowVDictionary.find(0))->DataPtr->GetBottom();

	//		auto width = m_spHeaderProperty->GetPenPtr()->GetWidth();
	//		CPen pen(m_spHeaderProperty->GetPenPtr()->GetPenStyle(),
	//			m_spHeaderProperty->GetPenPtr()->GetWidth(),
	//			CColor(RGB(255,0,0)));
	//		HPEN hPen = e.DCPtr->SelectPen(pen);

	//		e.DCPtr->MoveToEx(x,y0);
	//		e.DCPtr->LineTo(x,y1);

	//		e.DCPtr->SelectPen(hPen);
	//	}
	//}

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

	//if(m_setrocoSelected.empty()){
	//	return 0;
	//}else if(m_setrocoSelected.size()==1){
	//	auto iterRoCo=m_setrocoSelected.begin();
	//	auto pCell=iterRoCo->GetColumnPtr()->Cell(iterRoCo->GetRowPtr());
	//	if(auto p=std::dynamic_pointer_cast<CSheetCell>(pCell)){
	//		strCopy=p->GetSheetString();
	//	}else{
	//		strCopy=pCell->GetString();
	//	}
	//}else{
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
	//}
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
	if(!pCell)return;

	auto rcClip(GetPageRect());
	auto rcCell(pCell->GetRect());
	LONG hScrollAdd = 0, vScrollAdd = 0;

	//Right has priority (Right can Overwrite ScrollPos)
	
	if(rcCell.right>rcClip.right){
		hScrollAdd = rcCell.right - rcClip.right;	
	}else if(rcCell.left<rcClip.left){
		hScrollAdd = max(rcCell.left - rcClip.left, rcCell.right - rcClip.right);		
	}

	if(hScrollAdd){
		m_horizontal.SetScrollPos(m_horizontal.GetScrollPos() + hScrollAdd );
	}

	//Bottom has priority (Bottom can Overwrite ScrollPos)
	
	if(rcCell.bottom>rcClip.bottom){
		vScrollAdd = rcCell.bottom - rcClip.bottom;
	}else if(rcCell.top<rcClip.top){
		vScrollAdd = max(rcCell.top - rcClip.top, rcCell.bottom - rcClip.bottom);
	}

	if(vScrollAdd){
		m_vertical.SetScrollPos(m_vertical.GetScrollPos() + vScrollAdd);
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
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"Updates::Sort")
					SortAllInSubmitUpdate();
				break;
			}
			case Updates::Filter:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"Updates::Filter")
					FilterAll();
				break;
			}
			case Updates::Rect:
			{
				MoveWindow(m_pDirect->Dips2Pixels(GetUpdateRect()), FALSE);
			}
			case Updates::RowVisible:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"Updates::RowVisible")
					UpdateRowVisibleDictionary();
				//::OutputDebugStringA("m_rowAllDictionary\r\n");
				//boost::range::for_each(m_rowAllDictionary, [](const RowData& data) {
				//	::OutputDebugStringA((boost::format("Display:%1%, Pointer:%2%\r\n") % data.Index%data.DataPtr.get()).str().c_str());
				//});
				break;
			}
			case Updates::ColumnVisible:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"Updates::ColumnVisible")
					UpdateColumnVisibleDictionary();
				break;
			}
			case Updates::Column:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"Updates::Column")
					UpdateColumn();
				break;
			}
			case Updates::Row:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"Updates::Row")
					UpdateRow();
				break;
			}
			case Updates::Scrolls:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"Updates::Scrolls")
					UpdateScrolls();
				break;
			}
			case Updates::EnsureVisibleFocusedCell:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"Updates::EnsureVisibleFocusedCell")
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