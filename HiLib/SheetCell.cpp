#include "SheetCell.h"
#include "CellProperty.h"
#include "Row.h"
#include "MyBrush.h"
#include "MyPen.h"
#include "MyRect.h"
#include "MyRgn.h"
#include "MapColumn.h"
//
#include "Tracker.h"
#include "Dragger.h"
#include "Cursorer.h"
#include "ResizeDlg.h"

#include "FunctionMenuItem.h"
#include "GridViewResource.h"
#include "SheetEventArgs.h"
#include "GridView.h"
#include "D2DWWindow.h"

CSheetCell::CSheetCell(
	CSheet* pSheet,
	CRow* pRow,
	CColumn* pColumn,
	std::shared_ptr<SheetProperty> spSheetProperty,
	std::shared_ptr<CellProperty> spCellProperty,
	CMenu* pMenu)
	:CSheet(pSheet, spSheetProperty), 
	CCell(pSheet,pRow,pColumn,spCellProperty,pMenu)
{
	m_spRowDragger = std::make_shared<CRowDragger>();
	m_spColDragger = std::make_shared<CSheetCellColDragger>();
	m_isWrappable = true;
}

void CSheetCell::AddRow()
{
	if (CanResizeRow() || CanResizeColumn()) {
		Resize(GetContainer<RowTag, AllTag>().size(), GetContainer<ColTag, AllTag>().size() - 1);
	} else {

	}
}

void CSheetCell::Resize()
{
	if(CanResizeRow() || CanResizeColumn()){
		CResizeDlg* pDlg = new CResizeDlg(this);
		g_hDlgModeless = pDlg->Create(m_pSheet->GetWndPtr()->m_hWnd);
		pDlg->ShowWindow(SW_SHOW);
	}else{
		
	}
}

void CSheetCell::OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
{
	CSheet::OnCellPropertyChanged(pCell, name);
	OnPropertyChanged(name);
	//if (!_tcsicmp(L"value", name)) {
	//	OnPropertyChanged(L"value");
	//} else if (!_tcsicmp(L"size", name)) {
	//	OnPropertyChanged(L"size");
	//}
}

void CSheetCell::ColumnInserted(CColumnEventArgs& e)
{
	CSheet::ColumnInserted(e);
	OnPropertyChanged(L"value");
}
void CSheetCell::ColumnErased(CColumnEventArgs& e)
{
	CSheet::ColumnErased(e);
	OnPropertyChanged(L"value");
}

void CSheetCell::RowInserted(CRowEventArgs& e)
{
	CSheet::RowInserted(e);
	OnPropertyChanged(L"value");
}
void CSheetCell::RowErased(CRowEventArgs& e)
{
	CSheet::RowErased(e);
	OnPropertyChanged(L"value");
}

FLOAT CSheetCell::GetTop()const
{
	return CCell::GetTop();
}

FLOAT CSheetCell::GetLeft()const
{
	return CCell::GetLeft();
}

CRectF CSheetCell::GetRectInWnd()const
{
	return CCell::GetRectInWnd();
}

CRectF CSheetCell::GetPaintRect()
{
	return CCell::GetRectInWnd();//TODO
}

CPointF CSheetCell::GetScrollPos()const
{
	return CPointF(0,0);
}

CSizeF CSheetCell::MeasureContentSize(CDirect2DWrite* pDirect)
{
	//Calc Content Rect
	CRectF rcContent(CSheet::MeasureSize());

	return rcContent.Size();
}

CSizeF CSheetCell::MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect)
{
	return CSheetCell::MeasureContentSize(pDirect);
}

void CSheetCell::OnRButtonDown(const RButtonDownEvent& e)
{
	CCell::OnRButtonDown(e);
	CSheet::OnRButtonDown(e);
}

void CSheetCell::OnCreate(const CreateEvent& e)
{
	CCell::OnCreate(e);
	CSheet::OnCreate(e);
}

void CSheetCell::OnLButtonDown(const LButtonDownEvent& e)
{
	CCell::OnLButtonDown(e);
	CSheet::OnLButtonDown(e);
}

void CSheetCell::OnLButtonUp(const LButtonUpEvent& e)
{
	CCell::OnLButtonUp(e);
	CSheet::OnLButtonUp(e);
}

void CSheetCell::OnLButtonClk(const LButtonClkEvent& e)
{
	CCell::OnLButtonClk(e);
	CSheet::OnLButtonClk(e);
}

void CSheetCell::OnLButtonSnglClk(const LButtonSnglClkEvent& e)
{
	CCell::OnLButtonSnglClk(e);
	CSheet::OnLButtonSnglClk(e);
}


void CSheetCell::OnLButtonDblClk(const LButtonDblClkEvent& e)
{
	CCell::OnLButtonDblClk(e);
	CSheet::OnLButtonDblClk(e);
}

void CSheetCell::OnLButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	CCell::OnLButtonBeginDrag(e);
	CSheet::OnLButtonBeginDrag(e);
}


void CSheetCell::OnMouseMove(const MouseMoveEvent& e)
{
	CCell::OnMouseMove(e);
	CSheet::OnMouseMove(e);
}

void CSheetCell::OnMouseLeave(const MouseLeaveEvent& e)
{
	CCell::OnMouseLeave(e);
	CSheet::OnMouseLeave(e);
}

void CSheetCell::OnSetCursor(const SetCursorEvent& e)
{
	CCell::OnSetCursor(e);
	CSheet::OnSetCursor(e);
}

void CSheetCell::OnSetFocus(const SetFocusEvent& e)
{
	CCell::OnSetFocus(e);
	CSheet::OnSetFocus(e);
}

void CSheetCell::OnKillFocus(const KillFocusEvent& e)
{
	CCell::OnKillFocus(e);
	CSheet::OnKillFocus(e);
}

void CSheetCell::OnKeyDown(const KeyDownEvent& e)
{
	CCell::OnKeyDown(e);
	CSheet::OnKeyDown(e);
}

void CSheetCell::OnChar(const CharEvent& e)
{
	CCell::OnChar(e);
	CSheet::OnChar(e);
}


void CSheetCell::PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint)
{
	CSheet::OnPaint(PaintEvent(m_pSheet->GetWndPtr()));
}

//std::shared_ptr<CDC> CSheetCell::GetClientDCPtr()const
//{
//	return m_pSheet->GetClientDCPtr();
//}
CGridView* CSheetCell::GetGridPtr()
{
	return m_pSheet->GetGridPtr();
}
//
//void CSheetCell::SetFocused(const bool& bFocused)
//{
//	CCell::SetFocused(bFocused);
//	CSheet::SetFocused(bFocused);
//}

bool CSheetCell::GetIsSelected()const
{
	return CCell::GetIsSelected();// || CSheet::GetSelected();
}

void CSheetCell::SetIsSelected(const bool& bSelected)
{
	//if(selected){
	//	CSheet::SelectAll();
	//}else{
	//	CSheet::DeselectAll();
	//}

	CCell::SetIsSelected(bSelected);
	//CSheet::SetSelected(bSelected);
}

bool CSheetCell::GetIsFocused()const
{
	return CCell::GetIsFocused();
}


bool CSheetCell::Filter(const std::wstring& strFilter)const
{
	for (auto colPtr : m_allCols) {
		for (auto rowPtr : m_allRows) {
			if (Cell(rowPtr, colPtr)->Filter(strFilter)) {
				return true;
			}
		}
	}
	return false;
}

void CSheetCell::OnContextMenu(const ContextMenuEvent& e)
{
	CSheet::OnContextMenu(e);
	if (*e.HandledPtr) { return; }
	CCell::OnContextMenu(e);
}

CColumn* CSheetCell::GetParentColumnPtr(CCell* pCell)
{
	return m_pSheet->GetParentColumnPtr(this);
}

void CSheetCell::OnPropertyChanged(const wchar_t* name)
{
	CSheet::OnPropertyChanged(name);
	CCell::OnPropertyChanged(name);
}

void CSheetCell::OnColumnPropertyChanged(CColumn* pCol, const wchar_t* name)
{
	CSheet::OnColumnPropertyChanged(pCol, name);
	CCell::OnPropertyChanged(name);
}

void CSheetCell::OnRowPropertyChanged(CRow* pRow, const wchar_t* name)
{
	CSheet::OnRowPropertyChanged(pRow, name);
	CCell::OnPropertyChanged(name);
}


void CSheetCell::UpdateRow()
{
	if (!Visible()) { return; }

	FLOAT top =
		GetTop() +
		CCell::GetCellPropertyPtr()->Line->Width * 0.5f +
		CCell::GetCellPropertyPtr()->Padding->top +
		CSheet::GetCellProperty()->Line->Width * 0.5f;

	for (auto rowPtr : m_visRows) {
		rowPtr->SetTop(top, false);
		for (auto colPtr : m_visCols) {
			std::shared_ptr<CCell> pCell = CSheet::Cell(rowPtr, colPtr);
			if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
				pSheetCell->UpdateAll();
			}
		}
		top += rowPtr->GetHeight();
	}
}

void CSheetCell::UpdateColumn()
{
	if (!Visible()) { return; }

	FLOAT left =
		GetLeft() +
		CCell::GetCellPropertyPtr()->Line->Width * 0.5f +
		CCell::GetCellPropertyPtr()->Padding->left +
		CSheet::GetCellProperty()->Line->Width * 0.5f;

	for (auto& colPtr : m_allCols) {
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



