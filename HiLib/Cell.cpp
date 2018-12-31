#include "Cell.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"
#include "Row.h"
#include "Column.h"
#include "RowColumn.h"
#include "MyBrush.h"
#include "MyPen.h"
#include "MyRect.h"
#include "MySize.h"
#include "MyMenu.h"
#include "UnDoReDoManager.h"
#include "FullCommand.h"


CMenu CCell::ContextMenu;

CCell::CCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CCellProperty> spProperty,CMenu* pContextMenu)
	:m_pSheet(pSheet),
	m_pRow(pRow),
	m_pColumn(pColumn),
	m_spProperty(spProperty),
	m_pContextMenu(pContextMenu?pContextMenu:&CCell::ContextMenu){}

bool CCell::operator<(CCell& rhs)
{
	return _tcsicmp(this->GetSortString().c_str(), rhs.GetSortString().c_str())>0;
}
bool CCell::operator>(CCell& rhs)
{
	return _tcsicmp(this->GetSortString().c_str(), rhs.GetSortString().c_str())<0;
}

CSize CCell::GetInitSize(CDC* pDC)
{
	return GetFitSize(pDC);
}

CSize CCell::GetFitSize(CDC* pDC)
{
	if(!m_bFitMeasureValid){
		m_fitSize = MeasureSize(pDC);
		m_bFitMeasureValid = true;
	}
	return m_fitSize;
}

CSize CCell::GetActSize(CDC* pDC)
{
	if(!m_bActMeasureValid){
		auto width = m_pColumn->GetWidth();
		auto fitSize = GetFitSize(pDC);
		if(fitSize.cx <= width){
			m_bActMeasureValid = true;
			m_actSize.cx = width;
			m_actSize.cy = m_fitSize.cy;
		}else{
			m_actSize = MeasureSizeWithFixedWidth(pDC);
		}
	}
	return m_actSize;
}

CSize CCell::MeasureContentSize(CDC* pDC)
{
	return CSize();
}

CSize CCell::MeasureContentSizeWithFixedWidth(CDC* pDC)
{
	return CSize();
}

CSize CCell::MeasureSize(CDC* pDC)
{
	CRect rcContent(MeasureContentSize(pDC));

	//Calc CenterBorder Rect
	CRect rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
	m_bFitMeasureValid = true;
	return rcCenter.Size();	
}

CSize CCell::MeasureSizeWithFixedWidth(CDC* pDC)
{
	//Calc Content Rect
	CRect rcContent(MeasureContentSizeWithFixedWidth(pDC));

	//Calc CenterBorder Rect
	CRect rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
	m_bActMeasureValid = true;
	return rcCenter.Size();	
}

void CCell::SetString(const string_type& str)
{
	if(GetString()!=str){
		SetStringNotify(str);	
	}
}

void CCell::SetStringNotify(const string_type& str)
{
	SetStringCore(str);
	OnPropertyChanged(L"value");
}

void CCell::OnPropertyChanged(const wchar_t* name)
{
	if (!_tcsicmp(L"value", name)) {
		//Update valid flag
		m_bFitMeasureValid = false;
		m_bActMeasureValid = false;
	} else if (!_tcsicmp(L"size", name)) {
		m_bActMeasureValid = false;
	}
	//Notify to Row, Column and Sheet
	m_pRow->OnCellPropertyChanged(this, name);
	m_pColumn->OnCellPropertyChanged(this, name);
	m_pSheet->OnCellPropertyChanged(this, name);
}


CCell::coordinates_type CCell::GetTop()const
{
	return m_pRow->GetTop();
}

CCell::coordinates_type CCell::GetLeft()const
{
	return m_pColumn->GetLeft();
}

CRect CCell::GetRect()const
{
	return CRect(
		m_pColumn->GetLeft(),
		m_pRow->GetTop(),
		m_pColumn->GetRight(),
		m_pRow->GetBottom());
}

CRect CCell::CenterBorder2InnerBorder(CRect rcCenter)
{
	auto innerPenWidth = m_spProperty->GetPenPtr()->GetRightBottomPenWidth();
	auto outerPenWidth = m_spProperty->GetPenPtr()->GetLeftTopPenWidth();
	rcCenter-=CRect(innerPenWidth,innerPenWidth, outerPenWidth, outerPenWidth);
	return rcCenter;
}

CRect CCell::InnerBorder2Content(CRect rcInner)
{
	rcInner.DeflateRect(m_spProperty->GetPadding());
	return rcInner;
}

CRect CCell::Content2InnerBorder(CRect rcContent)
{
	rcContent.InflateRect(m_spProperty->GetPadding());
	return rcContent;
}

CRect CCell::InnerBorder2CenterBorder(CRect rcInner)
{
	//Calc CenterBorder Rect 
	auto innerPenWidth = m_spProperty->GetPenPtr()->GetRightBottomPenWidth();
	auto outerPenWidth = m_spProperty->GetPenPtr()->GetLeftTopPenWidth();
	rcInner+=CRect(innerPenWidth,innerPenWidth, outerPenWidth, outerPenWidth);
	return rcInner;
}

void CCell::PaintBackground(CDC* pDC, CRect rcPaint)
{
	CBrush br;
	if(GetChecked()){
		br=*m_spProperty->GetCheckedBrushPtr();
	//}else if(GetFocused()){
	//	spBrush=m_spProperty->GetFocusedBrushPtr();
	//}else if(GetSelected()){
	//	spBrush=m_spProperty->GetSelectedBrushPtr();
	}else/* if(GetSheetFocused())*/{
		br=*m_spProperty->GetBackgroundBrushPtr();
	}

	if (GetSelected()) {
		double alpha = 0.5;
		CColor cr = br.GetColor();
		CColor crSelected;
		if (::GetFocus() != m_pSheet->GetGridPtr()->m_hWnd) {
			crSelected = m_spProperty->GetUnfocusSelectedBrushPtr()->GetColor();
		}else{
			crSelected = m_spProperty->GetSelectedBrushPtr()->GetColor();
		}
		CRGB rgb(cr.GetRGB());
		CRGB rgbSelected(crSelected.GetRGB());
		rgb.SetR(static_cast<int>(rgbSelected.GetR()*alpha + rgb.GetR()*(1-alpha))); 
		rgb.SetG(static_cast<int>(rgbSelected.GetG()*alpha + rgb.GetG()*(1-alpha))); 
		rgb.SetB(static_cast<int>(rgbSelected.GetB()*alpha + rgb.GetB()*(1-alpha))); 
		cr.SetRGB(rgb);
		br = CBrush(cr);
	}

	if(m_state==UIElementState::Hot || m_state==UIElementState::Pressed){
		CColor cr = br.GetColor();
		CHSV hsv(cr.GetHSV());
		hsv.SetV(hsv.GetV()+20);
		cr.SetHSV(hsv);
		br = CBrush(cr);
	}


	pDC->FillRect(rcPaint,br);

	//if(GetFocused()){
	//	//rcPaint.DeflateRect(2,2);
	//	pDC->DrawFocusRect(rcPaint);	
	//	//pDC->DrawFocusRect(rcPaint);
	//}
}

void CCell::PaintLine(CDC* pDC, CRect rcPaint)
{
	HPEN hPenOld = pDC->SelectPen(*m_spProperty->GetPenPtr());
	//HBRUSH hBrOld=pDC->SelectBrush((HBRUSH)GetStockObject(HOLLOW_BRUSH));
	//pDC->Rectangle(rcPaint);
	pDC->MoveToEx(rcPaint.left,rcPaint.top);
	pDC->LineTo(rcPaint.left, rcPaint.bottom);
	pDC->LineTo(rcPaint.right, rcPaint.bottom);
	pDC->LineTo(rcPaint.right, rcPaint.top);
	pDC->LineTo(rcPaint.left, rcPaint.top);

	//if(GetFocused()){
	//	pDC->SelectPen(*m_spProperty->GetFocusedPenPtr());
	//	rcPaint.DeflateRect(1,1);
	//	pDC->MoveToEx(rcPaint.left,rcPaint.top);
	//	pDC->LineTo(rcPaint.left, rcPaint.bottom);
	//	pDC->LineTo(rcPaint.right, rcPaint.bottom);
	//	pDC->LineTo(rcPaint.right, rcPaint.top);
	//	pDC->LineTo(rcPaint.left, rcPaint.top);

	//}

	//pDC->SelectBrush(hBrOld);
	pDC->SelectPen(hPenOld);
}

void CCell::PaintFocus(CDC* pDC, CRect rcPaint)
{
	if(GetFocused()){
		HPEN hPenOld = pDC->SelectPen(*m_spProperty->GetFocusedPenPtr());
		rcPaint.DeflateRect(CRect(0,0,1,1));
		pDC->MoveToEx(rcPaint.left,rcPaint.top);
		pDC->LineTo(rcPaint.left, rcPaint.bottom);
		pDC->LineTo(rcPaint.right, rcPaint.bottom);
		pDC->LineTo(rcPaint.right, rcPaint.top);
		pDC->LineTo(rcPaint.left, rcPaint.top);
		pDC->SelectPen(hPenOld);
	}
}

void CCell::OnPaint(const PaintEvent& e)
{
	CRect rcClient(GetRect());
	CRect rcInner(CenterBorder2InnerBorder(rcClient));
	CRect rcContent(InnerBorder2Content(rcInner));
	PaintLine(e.DCPtr,rcClient);
	PaintBackground(e.DCPtr,rcInner);
	PaintContent(e.DCPtr,rcContent);
	PaintFocus(e.DCPtr, rcInner);
}

void CCell::OnLButtonDown(const LButtonDownEvent& e)
{
	CUIElement::OnLButtonDown(e);
	m_pSheet->CellLButtonDown(CellEventArgs(this));
}

void CCell::OnLButtonUp(const LButtonUpEvent& e)
{
	CUIElement::OnLButtonUp(e);
	SetState(UIElementState::Hot);
}

void CCell::OnLButtonDblClk(const LButtonDblClkEvent& e)
{
	m_pSheet->CellLButtonDblClk(CellEventArgs(this));
}

void CCell::OnContextMenu(const ContextMenuEvent& e)
{
	CMenu* pMenu = GetContextMenuPtr(); 
	if(pMenu){//TODO should use GetWnd not GetHWND?
		CPoint ptScreen(e.Point);
		HWND hWnd = m_pSheet->GetGridPtr()->m_hWnd;
		m_pSheet->SetContextMenuRowColumn(CRowColumn(m_pRow,m_pColumn));
		::ClientToScreen(hWnd, &ptScreen);
		::SetForegroundWindow(hWnd);
		pMenu->TrackPopupMenu(0,ptScreen.x,ptScreen.y,hWnd);
	}
	//HMENU hMenu = ::CreatePopupMenu();
	//MENUITEMINFO mii = {0};
	//mii.cbSize = sizeof(MENUITEMINFO);
	//mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID  | MIIM_STRING;
	//mii.fType = MFT_STRING;
	//mii.fState = MFS_ENABLED;
	//mii.wID = 98765;
	//mii.dwTypeData = L"TEST";
	//mii.cch = 4;
	////AddCmdIDHandler(mii.wID, 
	////	[&](WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)->LRESULT
	////	{
	////		return OnCommandMenuItem(wNotifyCode, wID, hWndCtl, bHandled);
	////	});
	//::InsertMenuItem(hMenu, 0, TRUE, &mii);
	//	CPoint ptScreen(e.Point);
	//	//m_rocoContextMenu.SetRowColumnPtr(e.CellPtr->GetRowPtr(),e.CellPtr->GetColumnPtr());
	//	ClientToScreen(m_pSheet->GetGridPtr(), &ptScreen);
	//	::SetForegroundWindow(m_pSheet->GetGridPtr());
	//	::TrackPopupMenu(hMenu,0,ptScreen.x,ptScreen.y,0,m_pSheet->GetGridPtr(),NULL);


	//m_pSheet->CellContextMenu(CellContextMenuEventArgs(this,e.Point));	
}
void CCell::OnSetFocus(const SetFocusEvent& e)
{
	OnPropertyChanged(L"focus");
}

void CCell::OnKillFocus(const KillFocusEvent& e)
{
	OnPropertyChanged(L"focus");
}

bool CCell::Filter(const string_type& strFilter)
{
	return boost::algorithm::to_lower_copy(GetString()).find(boost::algorithm::to_lower_copy(strFilter))!=std::wstring::npos;
}

bool CCell::GetSelected()const
{
	return m_bSelected || m_pRow->GetSelected() || m_pColumn->GetSelected() || m_pSheet->GetSelected();
}

void CCell::SetSelected(const bool& selected)
{
	if(m_bSelected!=selected){
		m_bSelected=selected;
		OnPropertyChanged(L"selected");
	}
}

bool CCell::GetFocused()const
{
	return m_pSheet->IsFocusedCell(this) /* || m_pSheet->GetFocused()*/;
}

bool CCell::GetDoubleFocused()const
{
	return m_pSheet->IsDoubleFocusedCell(this) /* || m_pSheet->GetFocused()*/;
}


bool CCell::GetChecked()const
{
	return m_bChecked;
}

void CCell::SetChecked(const bool& bChecked)
{
	if(m_bChecked!=bChecked){
		m_bChecked=bChecked;
		OnPropertyChanged(L"checked");
	}
}

bool CCell::IsComparable()const
{
	return true;
}

Compares CCell::EqualCell(CCell* pCell, std::function<void(CCell*, Compares)> action)
{
	return pCell->EqualCell(this, action);
}

Compares CCell::EqualCell(CEmptyCell* pCell, std::function<void(CCell*, Compares)> action)
{
	action(this, Compares::DiffNE);
	return Compares::DiffNE;
} 

Compares CCell::EqualCell(CTextCell* pCell, std::function<void(CCell*, Compares)> action)
{
	action(this, Compares::Diff);
	return Compares::Diff;
}

Compares CCell::EqualCell(CSheetCell* pCell, std::function<void(CCell*, Compares)> action)
{
	action(this, Compares::Diff);
	return Compares::Diff;
}

CCell::string_type CCell::GetString()
{
	return string_type();
}


