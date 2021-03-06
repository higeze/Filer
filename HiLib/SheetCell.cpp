#include "SheetCell.h"
#include "CellProperty.h"
#include "Row.h"
#include "MyBrush.h"
#include "MyPen.h"
#include "MyRect.h"
#include "MyRgn.h"
//
#include "ChildIndexColumn.h"
#include "ChildRowHeaderColumn.h"
#include "ParentMapColumn.h"
//
#include "Tracker.h"
#include "Dragger.h"
#include "Cursorer.h"
#include "ResizeDlg.h"

#include "FunctionMenuItem.h"
#include "GridViewResource.h"
#include "SheetEventArgs.h"
#include "GridView.h"

CMenu CSheetCell::SheetCellContextMenu;

CSheetCell::CSheetCell(
	CSheet* pSheet,
	CRow* pRow,
	CColumn* pColumn,
	std::shared_ptr<CellProperty> spProperty,
	std::shared_ptr<HeaderProperty> spHeaderProperty,
	std::shared_ptr<CellProperty> spFilterProperty,
	std::shared_ptr<CellProperty> spCellProperty,
	CMenu* pMenu)
	:CSheet(spHeaderProperty,
		spFilterProperty,
		spCellProperty), 
	CCell(pSheet,pRow,pColumn,spProperty,pMenu)
{
	//m_lineType = LineType::Both;
	m_spRowDragger = std::make_shared<CSheetCellRowDragger>();
	m_spColDragger = std::make_shared<CSheetCellColDragger>();
}

void CSheetCell::AddRow()
{
	if (CanResizeRow() || CanResizeColumn()) {
		Resize(GetMaxIndex<RowTag, AllTag>() + 1, GetMaxIndex<ColTag, AllTag>());
	} else {

	}
}

void CSheetCell::Resize()
{
	if(CanResizeRow() || CanResizeColumn()){
		CResizeDlg* pDlg = new CResizeDlg(this);
		g_hDlgModeless = pDlg->Create(m_pSheet->GetGridPtr()->m_hWnd);
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

//void CSheetCell::ColumnHeaderEndTrack(CColumnEventArgs& e)
//{
////	CSheet::ColumnHeaderEndTrack(e);
//	OnPropertyChanged(L"size");
//}

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

d2dw::CRectF CSheetCell::GetRect()const
{
	return CCell::GetRect();
}

d2dw::CRectF CSheetCell::GetPaintRect()
{
	return CCell::GetRect();//TODO
}

CPoint CSheetCell::GetScrollPos()const
{
	return CPoint(0,0);
}

d2dw::CSizeF CSheetCell::MeasureSize(d2dw::CDirect2DWrite& direct)
{
	//Calc Content Rect
	d2dw::CRectF rcContent(CSheet::MeasureSize());
	//Calc InnerBorder, CenterBorder Rect
	d2dw::CRectF rcCenter(InnerBorder2CenterBorder(Content2InnerBorder(rcContent)));

	return rcCenter.Size();
}

d2dw::CSizeF CSheetCell::MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)
{
	return MeasureSize(direct);
}

void CSheetCell::OnLButtonDown(const LButtonDownEvent& e)
{
	CCell::OnLButtonDown(e);
	CSheet::OnLButtonDown(e);
	SubmitUpdate();
}

void CSheetCell::OnLButtonUp(const LButtonUpEvent& e)
{
	CCell::OnLButtonUp(e);
	CSheet::OnLButtonUp(e);
	SubmitUpdate();
}

void CSheetCell::OnLButtonClk(const LButtonClkEvent& e)
{
	CCell::OnLButtonClk(e);
	CSheet::OnLButtonClk(e);
	SubmitUpdate();
}

void CSheetCell::OnLButtonSnglClk(const LButtonSnglClkEvent& e)
{
	CCell::OnLButtonSnglClk(e);
	CSheet::OnLButtonSnglClk(e);
	SubmitUpdate();
}


void CSheetCell::OnLButtonDblClk(const LButtonDblClkEvent& e)
{
	CCell::OnLButtonDblClk(e);
	CSheet::OnLButtonDblClk(e);
	SubmitUpdate();
}


void CSheetCell::OnMouseMove(const MouseMoveEvent& e)
{
	CCell::OnMouseMove(e);
	CSheet::OnMouseMove(e);
	SubmitUpdate();
}

void CSheetCell::OnMouseLeave(const MouseLeaveEvent& e)
{
	CCell::OnMouseLeave(e);
	CSheet::OnMouseLeave(e);
	SubmitUpdate();
}

void CSheetCell::OnSetCursor(const SetCursorEvent& e)
{
	CCell::OnSetCursor(e);
	CSheet::OnSetCursor(e);
	SubmitUpdate();
}

void CSheetCell::OnSetFocus(const SetFocusEvent& e)
{
	CCell::OnSetFocus(e);
	CSheet::OnSetFocus(e);
	SubmitUpdate();
}

void CSheetCell::OnKillFocus(const KillFocusEvent& e)
{
	CCell::OnKillFocus(e);
	CSheet::OnKillFocus(e);
	SubmitUpdate();
}

void CSheetCell::OnKeyDown(const KeyDownEvent& e)
{
	CCell::OnKeyDown(e);
	CSheet::OnKeyDown(e);
	SubmitUpdate();
}


bool CSheetCell::IsComparable()const
{
	return true;
}

Compares CSheetCell::EqualCell(CCell* pCell, std::function<void(CCell*, Compares)> action)
{
	return pCell->EqualCell(this, action);
}
Compares CSheetCell::EqualCell(CEmptyCell* pCell, std::function<void(CCell*, Compares)> action)
{
	action(this, Compares::DiffNE);
	return Compares::DiffNE;
} 
Compares CSheetCell::EqualCell(CTextCell* pCell, std::function<void(CCell*, Compares)> action)
{
	action(this, Compares::Diff);
	return Compares::Diff;
} 
Compares CSheetCell::EqualCell(CSheetCell* pCell, std::function<void(CCell*, Compares)> action)
{//TODO check size, fixed and soon

	//if Minus size is difference, all cells are not equal
	if(GetMinIndex<ColTag, AllTag>()!=pCell->GetMinIndex<ColTag, AllTag>() ||
		GetMinIndex<RowTag, AllTag>() !=pCell->GetMinIndex<RowTag, AllTag>()){
		boost::range::for_each(m_columnVisibleDictionary,[&](const ColumnData& colData){
			boost::range::for_each(this->m_rowVisibleDictionary,[&](const RowData& rowData){
				action(CSheet::Cell(rowData.DataPtr, colData.DataPtr).get(), Compares::Diff);
			});
		});
		action(this, Compares::Diff);
		return Compares::Diff;
	}

	auto comp = Compares::Same;
	//if size is difference, cell is not equal
	if(m_columnVisibleDictionary.size()!=pCell->m_columnVisibleDictionary.size() ||
		m_rowVisibleDictionary.size()!=pCell->m_rowVisibleDictionary.size()){
		comp = Compares::Diff;
	}
	//Larger size cells are checked
	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();

	for(auto colIter=colDictionary.begin(),colEnd=colDictionary.end();colIter!=colEnd;++colIter){
		for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
			auto pCellMe=colIter->DataPtr->Cell(rowIter->DataPtr.get());
			auto pCellOther=pCell->Cell<VisTag>(rowIter->Index,colIter->Index);
			if(pCellOther){
				auto cellComp = pCellOther->EqualCell(pCellMe.get(), action);//run equal cell to check
				comp = (comp==Compares::Same && cellComp==Compares::Same)?Compares::Same:Compares::Diff;
			}else{
				comp = Compares::Diff;
				action(this, comp);
			}
		}
	}
	action(this, comp);	
	return comp;
}

void CSheetCell::PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)
{
/*	CRgn rgn;
	rgn.CreateRectRgnIndirect(rcPaint);
	pDC->SelectClipRgn(rgn);*/	
	CSheet::OnPaint(PaintEvent(direct));
	//pDC->SelectClipRgn(NULL);
}

std::shared_ptr<HeaderProperty> CSheetCell::GetHeaderPropertyPtr()
{
	return m_spHeaderProperty;
}
std::shared_ptr<CellProperty> CSheetCell::GetCellPropertyPtr()
{
	return m_spProperty;
}

std::shared_ptr<CDC> CSheetCell::GetClientDCPtr()const
{
	return m_pSheet->GetClientDCPtr();
}
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

bool CSheetCell::GetSelected()const
{
	return CCell::GetSelected();// || CSheet::GetSelected();
}

void CSheetCell::SetSelected(const bool& bSelected)
{
	//if(selected){
	//	CSheet::SelectAll();
	//}else{
	//	CSheet::DeselectAll();
	//}

	CCell::SetSelected(bSelected);
	//CSheet::SetSelected(bSelected);
}

bool CSheetCell::Filter(const std::wstring& strFilter)const
{
	bool bMatch = false;
	auto iterCol = boost::find_if(m_columnAllDictionary, [&](const ColumnData& colData)->bool{
		auto iterRow = boost::find_if(m_rowAllDictionary, [&](const RowData& rowData)->bool{
			return colData.DataPtr->Cell(rowData.DataPtr.get())->Filter(strFilter);
		});
		return iterRow!=m_rowAllDictionary.end();
	});

	return iterCol!=m_columnAllDictionary.end();
}
CMenu* CSheetCell::GetContextMenuPtr()
{
	//Copy base menu
	SheetCellContextMenu = CMenu(*CCell::GetContextMenuPtr());

	//Resize
	if(CanResizeRow() || CanResizeColumn()){
		//Create Resize menu
		auto spResizeMenuItem = std::make_shared<CFunctionMenuItem>();
		spResizeMenuItem->SetID(IDM_RESIZE_SHEETCELL);
		spResizeMenuItem->SetMask(MIIM_TYPE|MIIM_ID);
		spResizeMenuItem->SetType(MFT_STRING);
		spResizeMenuItem->SetTypeData(L"Resize");
		spResizeMenuItem->SetFunction([&]()->void{Resize();});
		//Assign command
		GetGridPtr()->ReplaceCmdIDHandler(spResizeMenuItem->GetID(),
			[spResizeMenuItem](WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)->HRESULT
				{return spResizeMenuItem->OnCommand(wNotifyCode,wID,hWndCtl,bHandled);});

		//Create AddRow menu
		auto spAddRowMenuItem = std::make_shared<CFunctionMenuItem>();
		spAddRowMenuItem->SetID(IDM_ADDROW_SHEETCELL);
		spAddRowMenuItem->SetMask(MIIM_TYPE | MIIM_ID);
		spAddRowMenuItem->SetType(MFT_STRING);
		spAddRowMenuItem->SetTypeData(L"Add Row");
		spAddRowMenuItem->SetFunction([&]()->void {AddRow(); });
		//Assign command
		GetGridPtr()->ReplaceCmdIDHandler(spAddRowMenuItem->GetID(),
			[spAddRowMenuItem](WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)->HRESULT {return spAddRowMenuItem->OnCommand(wNotifyCode, wID, hWndCtl, bHandled); });

		//Insert menu
		if(SheetCellContextMenu.GetMenuItemCount()>0){
			SheetCellContextMenu.InsertSeparator(SheetCellContextMenu.GetMenuItemCount(), TRUE);
		}
		SheetCellContextMenu.InsertMenuItem(SheetCellContextMenu.GetMenuItemCount(), TRUE, spAddRowMenuItem.get());
		SheetCellContextMenu.InsertMenuItem(SheetCellContextMenu.GetMenuItemCount(), TRUE, spResizeMenuItem.get());
	}
	return &SheetCellContextMenu;
}

void CSheetCell::OnContextMenu(const ContextMenuEvent& e)
{
	if(!Visible())return;
	auto cell = Cell(static_cast<CGridView*>(e.WindowPtr)->GetDirect()->Pixels2Dips(e.Point));
	if(cell){
		cell->OnContextMenu(e);
	}else{
		CCell::OnContextMenu(e);
	}
	SubmitUpdate();
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


void CSheetCell::UpdateRow()
{
	if (!Visible()) { return; }

	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
	FLOAT top = GetTop() +
		m_spCellProperty->Line->Width * 0.5f * 2.0f +
		GetPropertyPtr()->Padding->top;

//	d2dw::CRectF rcPaint(GetGridPtr()->GetPaintRect());
//	FLOAT scrollPos = GetGridPtr()->GetVerticalScrollPos();

	//std::function<bool(FLOAT, FLOAT, FLOAT, FLOAT)> isPaint = [](FLOAT min, FLOAT max, FLOAT top, FLOAT bottom)->bool {
	//	return (top > min && top < max) ||
	//		(bottom > min && bottom < max);
	//};

	//	return static_cast<CSheetCell*>(m_pSheet)->GetTop()
	//			+ (FLOAT)(static_cast<CSheetCell*>(m_pSheet)->GetPropertyPtr()->Line->Width*0.5)
	//			+ (FLOAT)(static_cast<CSheetCell*>(m_pSheet)->GetPropertyPtr()->Line->Width*0.5)
	//			+ static_cast<CSheetCell*>(m_pSheet)->GetPropertyPtr()->Padding->top;

	for (auto iter = rowDictionary.begin(), end = rowDictionary.end(); iter != end; ++iter) {
		iter->DataPtr->SetTopWithoutSignal(top);
		//FLOAT defaultHeight = iter->DataPtr->GetDefaultHeight();
		//FLOAT bottom = top + defaultHeight;
		//if (isPaint(rcPaint.top, rcPaint.bottom, top, bottom)) {
		auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();
		for (auto& colData : colDictionary) {
			std::shared_ptr<CCell> pCell = CSheet::Cell(iter->DataPtr, colData.DataPtr);
			if (auto pSheetCell = std::dynamic_pointer_cast<CSheetCell>(pCell)) {
				pSheetCell->UpdateAll();
			}
		}
		top += iter->DataPtr->GetHeight();

		//} else {
		//	top += defaultHeight;
		//}
	}
	//}
}



