#include "Cell.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetEventArgs.h"
#include "Row.h"
#include "Column.h"
#include "RowColumn.h"
#include "MyMenu.h"
#include "UnDoReDoManager.h"
#include "FullCommand.h"


CMenu CCell::ContextMenu;

CCell::CCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,CMenu* pContextMenu)
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

d2dw::CSizeF CCell::GetInitSize(d2dw::CDirect2DWrite& direct)
{
	return GetFitSize(direct);
}

d2dw::CSizeF CCell::GetFitSize(d2dw::CDirect2DWrite& direct)
{
	if(!m_bFitMeasureValid){
		m_fitSize = MeasureSize(direct);
		m_bFitMeasureValid = true;
	}
	return m_fitSize;
}

d2dw::CSizeF CCell::GetActSize(d2dw::CDirect2DWrite& direct)
{
	if(!m_bActMeasureValid){
		auto width = m_pColumn->GetWidth();
		auto fitSize = GetFitSize(direct);
		if(fitSize.width <= width){
			m_bActMeasureValid = true;
			m_actSize.width = width;
			m_actSize.height = m_fitSize.height;
		}else{
			m_actSize = MeasureSizeWithFixedWidth(direct);
		}
	}
	return m_actSize;
}

d2dw::CSizeF CCell::MeasureContentSize(d2dw::CDirect2DWrite& direct)
{
	return d2dw::CSizeF();
}

d2dw::CSizeF CCell::MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	return d2dw::CSizeF();
}

d2dw::CSizeF CCell::MeasureSize(d2dw::CDirect2DWrite& direct)
{
	d2dw::CRectF rcContent(MeasureContentSize(direct));

	//Calc CenterBorder Rect
	d2dw::CRectF rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
	m_bFitMeasureValid = true;
	return rcCenter.Size();	
}

d2dw::CSizeF CCell::MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	//Calc Content Rect
	d2dw::CRectF rcContent(MeasureContentSizeWithFixedWidth(direct));

	//Calc CenterBorder Rect
	d2dw::CRectF rcCenter=(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));
	m_bActMeasureValid = true;
	return rcCenter.Size();	
}

void CCell::SetString(const std::wstring& str)
{
	if(GetString()!=str){
		SetStringNotify(str);	
	}
}

void CCell::SetStringNotify(const std::wstring& str)
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

FLOAT CCell::GetLeft()const
{
	return m_pColumn->GetLeft();
}

FLOAT CCell::GetTop()const
{
	return m_pRow->GetTop();
}

d2dw::CRectF CCell::GetRect()const
{
	return d2dw::CRectF(
		m_pColumn->GetLeft(),
		m_pRow->GetTop(),
		m_pColumn->GetRight(),
		m_pRow->GetBottom());
}

d2dw::CRectF CCell::CenterBorder2InnerBorder(d2dw::CRectF rcCenter)
{
	auto halfLineWidth = m_spProperty->Line->Width*0.5;
	rcCenter-= d2dw::CRectF(halfLineWidth,halfLineWidth, halfLineWidth, halfLineWidth);
	return rcCenter;
}

d2dw::CRectF CCell::InnerBorder2Content(d2dw::CRectF rcInner)
{
	rcInner.DeflateRect(*(m_spProperty->Padding));
	return rcInner;
}

d2dw::CRectF CCell::Content2InnerBorder(d2dw::CRectF rcContent)
{
	rcContent.InflateRect(*(m_spProperty->Padding));
	return rcContent;
}

d2dw::CRectF CCell::InnerBorder2CenterBorder(d2dw::CRectF rcInner)
{
	//Calc CenterBorder Rect 
	auto halfLineWidth = m_spProperty->Line->Width*0.5;
	rcInner += d2dw::CRectF(halfLineWidth,halfLineWidth, halfLineWidth, halfLineWidth);
	return rcInner;
}

void CCell::PaintBackground(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	//Paint Normal
	direct.FillSolidRectangle(*(m_spProperty->NormalFill), rcPaint);
	//Selected
	if (GetSelected() && ::GetFocus() == m_pSheet->GetGridPtr()->m_hWnd) {
		direct.FillSolidRectangle(*(m_spProperty->SelectedFill), rcPaint);
	} else if (GetSelected()) {
		direct.FillSolidRectangle(*(m_spProperty->UnfocusSelectedFill), rcPaint);
	}
	//Hot, Pressed
	if (m_state == UIElementState::Hot || m_state == UIElementState::Pressed) {
		direct.FillSolidRectangle(*(m_spProperty->HotFill), rcPaint);
	}
}

void CCell::PaintLine(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	direct.DrawSolidRectangle(*(m_spProperty->Line), rcPaint);
}

void CCell::PaintFocus(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
	if(GetFocused()){
		rcPaint.DeflateRect(1.0f, 1.0f);
		direct.DrawSolidRectangle(*(m_spProperty->FocusedLine), rcPaint);
	}
}

void CCell::OnPaint(const PaintEvent& e)
{
	d2dw::CRectF rcClient(GetRect());
	d2dw::CRectF rcInner(CenterBorder2InnerBorder(rcClient));
	d2dw::CRectF rcContent(InnerBorder2Content(rcInner));
	PaintLine(e.Direct,rcClient);
	PaintBackground(e.Direct,rcInner);
	PaintContent(e.Direct,rcContent);
	PaintFocus(e.Direct, rcInner);
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

bool CCell::Filter(const std::wstring& strFilter)
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

std::wstring CCell::GetString()
{
	return std::wstring();
}


