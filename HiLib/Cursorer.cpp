#include "Cursorer.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetState.h"
#include "RowColumn.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"


void CCursorer::OnCursor(std::shared_ptr<CCell>& cell)
{
	if (cell->GetRowPtr()->GetIndex<AllTag>()<0 || cell->GetColumnPtr()->GetIndex<AllTag>()<0) {
		return;
	}
	m_oldCell = m_currentCell;//Old
	m_currentCell = cell;//Current
	m_anchorCell = cell;//Anchor
	if (m_focusedCell != cell) {
		if (m_focusedCell) { m_focusedCell->OnKillFocus(EventArgs()); }//Blur
		m_focusedCell = cell;
		m_focusedCell->OnSetFocus(EventArgs());//Focus
		m_doubleFocusedCell = nullptr;//DoubleFocus
	}
	else if (m_focusedCell == cell) {
		m_doubleFocusedCell = cell;//DoubleFocus
	}
	OnBandCursor(cell->GetRowPtr());
}

void CCursorer::OnCursorCtrl(std::shared_ptr<CCell>& cell)
{
	m_oldCell=m_currentCell;//Old
	m_currentCell=cell;//Current
	m_anchorCell=cell;//Anchor
	if(m_focusedCell!=cell){
		if(m_focusedCell != nullptr){m_focusedCell->OnKillFocus(EventArgs());}//Blur
		m_focusedCell=cell;
		m_focusedCell->OnSetFocus(EventArgs());//Focus
		m_doubleFocusedCell = nullptr;//DoubleFocus
	}else if(m_focusedCell == cell){
		m_doubleFocusedCell = cell;//DoubleFocus
	}
	cell->SetSelected(true);//Select
	OnBandCursorCtrl(cell->GetRowPtr());
}

void CCursorer::OnCursorShift(std::shared_ptr<CCell>& cell)
{
	m_oldCell=m_currentCell;//Old
	m_currentCell=cell;//Current
	//No Anchor change
	cell->GetSheetPtr()->SelectRange(m_anchorCell, m_oldCell, false);
	cell->GetSheetPtr()->SelectRange(m_anchorCell, m_currentCell, true);
	OnBandCursorShift(m_oldCell->GetRowPtr(), m_anchorCell->GetRowPtr(), cell->GetRowPtr());
}

void CCursorer::OnCursorClear(CSheet* pSheet)
{
	if(m_focusedCell){m_focusedCell->OnKillFocus(EventArgs());}//Blur
	Clear();
	pSheet->DeselectAll();//Select
	pSheet->UnhotAll();//Hot
}

void CCursorer::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return;
	}
	auto cell = pSheet->Cell(e.Point);

	//If out of sheet, reset curosr
	if(cell.get() == nullptr){
		return OnCursorClear(pSheet);
	}

	//Focus, Select
	if(cell->GetRowPtr()==pSheet->GetHeaderRowPtr().get() && cell->GetColumnPtr()==pSheet->GetHeaderColumnPtr().get()){
		pSheet->SelectAll();//TODO Make RowColumnHeaderCell
	}else{
		if(e.Flags & MK_CONTROL){
			return OnCursorCtrl(cell);
		}else if(e.Flags & MK_SHIFT){
			return OnCursorShift(cell);
		}else{
			return OnCursor(cell);
		}
	}
}

void CCursorer::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return;
	}
	auto cell = pSheet->Cell(e.Point);
	if(!cell){
		return OnCursorClear(pSheet);
	}
	return;
}

void CCursorer::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return;
	}
	//Get RowColumn from Point
	auto cell = pSheet->Cell(e.Point);

	if(!cell){
		return OnCursorClear(pSheet);
	}

	//Focus, Select
	if(cell->GetSelected()){
		//Do Nothing
	}else{
		if(cell->GetColumnPtr() == pSheet->GetHeaderColumnPtr().get()){
			return OnBandCursor(cell->GetRowPtr());
		}else if(cell->GetRowPtr() == pSheet->GetHeaderRowPtr().get()){
			return OnBandCursor(cell->GetColumnPtr());
		}else{
			return OnCursor(cell);
		}
	}
	return;
}

void CCursorer::OnKeyDown(CSheet* pSheet, KeyEventArgs& e)
{
	if(pSheet->Empty()){
		return;
	}
	switch (e.Char)
	{
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		{
			
			std::shared_ptr<CCell> cell;
			if(!m_focusedCell){//Not Focused yet. Cell(0, 0) is Focused;
				cell = pSheet->Cell<VisTag>(0, 0);
			}else{//Move selection
				cell=m_currentCell;//TODO Low Current or Focused
				size_type roVisib= cell->GetRowPtr()->GetIndex<VisTag>();
				size_type coVisib= cell->GetColumnPtr()->GetIndex<VisTag>();
				switch(e.Char){
					case VK_LEFT:
						coVisib=((std::max)(coVisib-1,0));
						break;
					case VK_RIGHT:
						coVisib=((std::min)(coVisib+1,pSheet->GetMaxIndex<ColTag, VisTag>()));
						break;
					case VK_UP:
						roVisib=((std::max)(roVisib-1,0));
						break;
					case VK_DOWN:
						roVisib=((std::min)(roVisib+1,pSheet->GetMaxIndex<RowTag, VisTag>()));
						break;
				}
				cell=pSheet->Cell<VisTag>(roVisib, coVisib);
			}
			if(::GetKeyState(VK_CONTROL) & 0x8000){
				OnCursorCtrl(cell);
			}else if(::GetKeyState(VK_SHIFT) & 0x8000){
				OnCursorShift(cell);
			}else{
				OnCursor(cell);
			}
			((CGridView*)pSheet)->EnsureVisibleCell(cell);
			break;
		}
	case VK_ESCAPE:
		OnCursorClear(pSheet);
		break;
	default:
		break;
	}
	return;
}


std::vector<RC> CCursorer::GetFocusedRCs(CSheet* pSheet)const
{
	std::vector<RC> focusedRCs;
	//CRowColumn focusedRoCo = GetFocusedRowColumn();
	//if(!focusedRoCo.IsInvalid()){
	//	std::shared_ptr<CCell> focusedCell = CSheet::Cell(focusedRoCo.GetRowPtr(), focusedRoCo.GetColumnPtr());
	//	focusedRCs.emplace_back(focusedCell->GetRowPtr()->GetIndex<AllTag>(), focusedCell->GetColumnPtr()->GetIndex<AllTag>());
	//	if(auto p = std::dynamic_pointer_cast<CSheet>(focusedCell)){
	//		auto rcs = p->GetCursorerPtr()->GetFocusedRCs(p.get());
	//		std::copy(rcs.begin(), rcs.end(),std::back_inserter(focusedRCs));
	//	}
	//	}
	return focusedRCs;
}


std::vector<RC> CCursorer::GetSelectedRCs(CSheet* pSheet)const
{
	std::vector<RC> selectedRCs;
	//auto& rowDictionary=pSheet->m_rowVisibleDictionary.get<IndexTag>();
	//auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();

	//for(auto& rowData : rowDictionary){
	//	for(auto& colData : colDictionary){
	//		auto cell = CSheet::Cell(rowData.DataPtr, colData.DataPtr);
	//		if(cell->GetSelected()){
	//			selectedRCs.emplace_back(cell->GetRowPtr()->GetIndex<AllTag>(), cell->GetColumnPtr()->GetIndex<AllTag>());
	//		}
	//	}
	//}
	return selectedRCs;
}
std::vector<CSheet::row_type> CCursorer::GetSelectedRows(CSheet* pSheet)const
{
	std::vector<row_type> selectedRows;
	//auto& rowDictionary=pSheet->m_rowVisibleDictionary.get<IndexTag>();

	//for(auto& rowData : rowDictionary){
	//	if(rowData.DataPtr->GetSelected()){
	//		selectedRows.push_back(rowData.DataPtr);
	//	}
	//}
	return selectedRows;
}
std::vector<CSheet::column_type> CCursorer::GetSelectedColumns(CSheet* pSheet)const
{
	std::vector<column_type> selectedCols;
	//auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();

	//for(auto& colData : colDictionary){
	//	if(colData.DataPtr->GetSelected()){
	//		selectedCols.push_back(colData.DataPtr);
	//	}
	//}
	return selectedCols;
}

void CCursorer::SetFocusedRCs(CSheet* pSheet, std::vector<RC> rcs)
{
	//TODO
	//if(rcs.empty() || rcs.size()==0)return;
	//auto rowMinMax = pSheet->GetMinMaxAllRowIndex();
	//auto colMinMax = pSheet->GetMinMaxAllColumnIndex();
	//auto rc = rcs[0];
	//if(rowMinMax.first<=rc.Row && rc.Row<=rowMinMax.second &&
	//	colMinMax.first<=rc.Col && rc.Col<=colMinMax.second){
	//	auto cell = pSheet->Cell<AllTag>(rc.Row, rc.Col);
	//	m_focusedCell = pSheet->GetCursorerPtr()->SetFocusedRowColumn(CRowColumn(pSheet->Index2Pointer<RowTag, AllTag>(rcs[0].Row).get(), pSheet->Index2Pointer<ColTag, AllTag>(rcs[0].Col).get()));
	//	if( rcs.size()>1 ){
	//		if(auto p = std::dynamic_pointer_cast<CSheet>(cell)){
	//			p->GetCursorerPtr()->SetFocusedRowColumn(CRowColumn(p->Index2Pointer<RowTag, AllTag>(rcs[1].Row).get(), p->Index2Pointer<ColTag, AllTag>(rcs[1].Col).get()));		
	//		}
	//	}
	//}

}
void CCursorer::SetSelectedRCs(CSheet* pSheet, std::vector<RC> rcs)
{
	//if(rcs.empty() || rcs.size()==0)return;
	//auto rowMinMax = pSheet->GetMinMaxAllRowIndex();
	//auto colMinMax = pSheet->GetMinMaxAllColumnIndex();

	//for(auto& rc : rcs){
	//	if(rowMinMax.first<=rc.Row && rc.Row<=rowMinMax.second &&
	//		colMinMax.first<=rc.Col && rc.Col<=colMinMax.second){
	//		pSheet->Cell<AllTag>(rc.Row, rc.Col)->SetSelected(true);
	//	}
	//}
}

void CCursorer::SetSelectedRows(CSheet* pSheet, std::vector<row_type> rows)
{
	//if(rows.empty() || rows.size()==0)return;

	//for(auto& row : rows){
	//	row->SetSelected(true);
	//}
}

void CCursorer::SetSelectedColumns(CSheet* pSheet, std::vector<column_type> cols)
{
	//if(cols.empty() || cols.size()==0)return;

	//for(auto& col : cols){
	//	col->SetSelected(true);
	//}
}


void CSheetCellCursorer::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return;
	}
	//Get RowColumn from Point
	auto cell = pSheet->Cell(e.Point);

	if(!cell){
		return OnCursorClear(pSheet);
	}

	//Focus, Select

	if(e.Flags & MK_CONTROL){
		return OnCursorCtrl(cell);
	}else if(e.Flags & MK_SHIFT){
		return OnCursorShift(cell);
	}else{
		return OnCursor(cell);
	}
	return;
}

void CSheetCellCursorer::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return;
	}
	//Get RowColumn from Point
	auto cell = pSheet->Cell(e.Point);

	if(!cell){
		return OnCursorClear(pSheet);
	}
	//Focus, Select
	return OnCursor(cell);
}

void CSheetCellCursorer::OnKeyDown(CSheet* pSheet, KeyEventArgs& e)
{
	if (pSheet->Empty()) {
		return;
	}
	switch (e.Char)
	{
	case VK_ESCAPE:
		OnCursorClear(pSheet);
		break;
	default:
		break;
	}
	return;
}





