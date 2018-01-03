#include "Cursorer.h"
#include "Sheet.h"
#include "GridView.h"
#include "SheetState.h"
#include "RowColumn.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"

ISheetState* CCursorer::OnCursorCtrl(CSheet* pSheet, EventArgs& e, CRowColumn roco)
{

	if(pSheet->Empty()){
		return ISheetState::Normal();
	}
	m_rocoOld=m_rocoCurrent;//Old
	m_rocoCurrent=roco;//Current
	m_rocoAnchor=roco;//Anchor
	if(m_rocoFocused!=roco){
		if(!m_rocoFocused.IsInvalid()){m_rocoFocused.GetColumnPtr()->Cell(m_rocoFocused.GetRowPtr())->OnKillFocus(e);}//Blur
		m_rocoFocused=roco;m_rocoFocused.GetColumnPtr()->Cell(m_rocoFocused.GetRowPtr())->OnSetFocus(e);//Focus
		m_rocoDoubleFocused = CRowColumn();
	}else if(m_rocoFocused==roco){
		m_rocoDoubleFocused = roco;
	}
	roco.GetColumnPtr()->Cell(roco.GetRowPtr())->SetSelected(true);//Select
	OnRowCursorCtrl(pSheet, e, roco.GetRowPtr());
	return ISheetState::Normal();
}

ISheetState* CCursorer::OnCursorShift(CSheet* pSheet, EventArgs& e, CRowColumn roco)
{

	m_rocoOld=m_rocoCurrent;//Old
	m_rocoCurrent=roco;//Current
	//No Anchor change
	pSheet->DeselectRange(m_rocoAnchor,m_rocoOld);
	pSheet->SelectRange(m_rocoAnchor,m_rocoCurrent);
	OnRowCursorShift(pSheet, e, roco.GetRowPtr());
	return ISheetState::Normal();
}

ISheetState* CCursorer::OnCursor(CSheet* pSheet, EventArgs& e, CRowColumn roco)
{

	if(roco.GetRowPtr()->GetIndex<AllTag>()<0 || roco.GetColumnPtr()->GetIndex<AllTag>()<0){
		//return CSheetState::Normal();
		auto a=1;
	}
	m_rocoOld=m_rocoCurrent;//Old
	m_rocoCurrent=roco;//Current
	m_rocoAnchor=roco;//Anchor
	if(m_rocoFocused!=roco){
		if(!m_rocoFocused.IsInvalid()){m_rocoFocused.GetColumnPtr()->Cell(m_rocoFocused.GetRowPtr())->OnKillFocus(e);}//Blur
		m_rocoFocused=roco;m_rocoFocused.GetColumnPtr()->Cell(m_rocoFocused.GetRowPtr())->OnSetFocus(e);//Focus
		m_rocoDoubleFocused = CRowColumn();

	}else if(m_rocoFocused==roco){
		m_rocoDoubleFocused = roco;
	}
	//pSheet->DeselectAll();
	//roco.GetColumnPtr()->Cell(roco.GetRowPtr())->SetSelected(true);//Select
	OnRowCursor(pSheet, e, roco.GetRowPtr());

	return ISheetState::Normal();
}

ISheetState* CCursorer::OnRowCursorCtrl(CSheet* pSheet, EventArgs& e, CRow* pRow)
{
	OnBandCursorCtrl(pSheet, e,pSheet->m_rowVisibleDictionary,pRow,CRowColumn(pRow,pSheet->Index2Pointer<ColTag, VisTag>(0).get()));
	return ISheetState::Normal();
}

ISheetState* CCursorer::OnRowCursorShift(CSheet* pSheet, EventArgs& e, CRow* pRow)
{
	OnBandCursorShift(pSheet, e,pSheet->m_rowVisibleDictionary,
		m_rocoOld.GetRowPtr(),m_rocoAnchor.GetRowPtr(),pRow,CRowColumn(pRow,pSheet->Index2Pointer<ColTag, VisTag>(0).get()));
	return ISheetState::Normal();
}

ISheetState* CCursorer::OnRowCursor(CSheet* pSheet, EventArgs& e, CRow* pRow)
{
	OnBandCursor(pSheet, e,pSheet->m_rowVisibleDictionary,pRow,CRowColumn(pRow,pSheet->Index2Pointer<ColTag, VisTag>(0).get()));
	return ISheetState::Normal();
}

ISheetState* CCursorer::OnColumnCursorCtrl(CSheet* pSheet, EventArgs& e, CColumn* pColumn)
{
	OnBandCursorCtrl(pSheet, e,pSheet->m_columnVisibleDictionary,pColumn,CRowColumn(pSheet->Index2Pointer<RowTag, VisTag>(0).get(),pColumn));
	return ISheetState::Normal();
}

ISheetState* CCursorer::OnColumnCursorShift(CSheet* pSheet, EventArgs& e, CColumn* pColumn)
{
	OnBandCursorShift(pSheet, e,pSheet->m_columnVisibleDictionary,
		m_rocoOld.GetColumnPtr(),m_rocoAnchor.GetColumnPtr(),pColumn,CRowColumn(pSheet->Index2Pointer<RowTag, VisTag>(0).get(),pColumn));
	return ISheetState::Normal();
}

ISheetState* CCursorer::OnColumnCursor(CSheet* pSheet, EventArgs& e, CColumn* pColumn)
{
	OnBandCursor(pSheet, e,pSheet->m_columnVisibleDictionary,pColumn,CRowColumn(pSheet->Index2Pointer<RowTag, VisTag>(0).get(),pColumn));
	return ISheetState::Normal();
}


ISheetState* CCursorer::OnCursorClear(CSheet* pSheet, EventArgs& e)
{
	m_rocoOld=CRowColumn();//Old
	m_rocoCurrent=CRowColumn();//Current
	m_rocoAnchor=CRowColumn();//Anchor

	if(!m_rocoFocused.IsInvalid()){m_rocoFocused.GetColumnPtr()->Cell(m_rocoFocused.GetRowPtr())->OnKillFocus(e);}//Blur
	m_rocoFocused=CRowColumn();//Focus
	m_rocoDoubleFocused=CRowColumn();//DoubleFocus
	pSheet->DeselectAll();//Select
	pSheet->UnhotAll();//Hot
	return ISheetState::Normal();
}

ISheetState* CCursorer::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return ISheetState::Normal();
	}
	//Get RowColumn from Point
	auto roco = pSheet->Point2RowColumn(e.Point);

	if(roco.IsInvalid()){
		return OnCursorClear(pSheet, EventArgs());
	}

	//Focus, Select
	if(roco.GetRowPtr()==pSheet->GetHeaderRowPtr().get() && roco.GetColumnPtr()==pSheet->GetHeaderColumnPtr().get()){
		pSheet->SelectAll();//TODO Make RowColumnHeaderCell
	}else if(roco.GetColumnPtr()==pSheet->GetHeaderColumnPtr().get()){
		if(e.Flags & MK_CONTROL){
			return OnRowCursorCtrl(pSheet, e,roco.GetRowPtr());
		}else if(e.Flags & MK_SHIFT){
			return OnRowCursorShift(pSheet, e,roco.GetRowPtr());
		}else{
			return OnRowCursor(pSheet, e,roco.GetRowPtr());
		}	
	//}else if(roco.GetRowPtr()==pSheet->GetHeaderRowPtr().get()){
	//	if(e.Flags & MK_CONTROL){
	//		return OnColumnCursorCtrl(pSheet, e,roco.GetColumnPtr());
	//	}else if(e.Flags & MK_SHIFT){
	//		return OnColumnCursorShift(pSheet, e,roco.GetColumnPtr());
	//	}else{
	//		return OnColumnCursor(pSheet, e,roco.GetColumnPtr());
	//	}	

	}else{
		if(e.Flags & MK_CONTROL){
			return OnCursorCtrl(pSheet, e,roco);
		}else if(e.Flags & MK_SHIFT){
			return OnCursorShift(pSheet, e,roco);
		}else{
			return OnCursor(pSheet, e,roco);
		}
	}
	return ISheetState::Normal();
}
ISheetState* CCursorer::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e){return ISheetState::Normal();}
ISheetState* CCursorer::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return ISheetState::Normal();
	}
	auto roco = pSheet->Point2RowColumn(e.Point);
	if(roco.IsInvalid()){
		return OnCursorClear(pSheet, EventArgs());
	}
	return ISheetState::Normal();
}
ISheetState* CCursorer::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return ISheetState::Normal();
	}
	//Get RowColumn from Point
	auto roco = pSheet->Point2RowColumn(e.Point);

	if(roco.IsInvalid()){
		return OnCursorClear(pSheet, EventArgs());
	}

	//Focus, Select
	if(roco.GetColumnPtr()->Cell(roco.GetRowPtr())->GetSelected()){
		//Do Nothing
	}else{
		if(roco.GetColumnPtr()==pSheet->GetHeaderColumnPtr().get()){
			return OnRowCursor(pSheet, e,roco.GetRowPtr());
		}else if(roco.GetRowPtr()==pSheet->GetHeaderRowPtr().get()){
			return OnColumnCursor(pSheet, e,roco.GetColumnPtr());
		}else{
			return OnCursor(pSheet, e,roco);
		}
	}
	return ISheetState::Normal();
}
ISheetState* CCursorer::OnMouseMove(CSheet* pSheet, MouseEventArgs& e){return ISheetState::Normal();}
ISheetState* CCursorer::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e){return ISheetState::Normal();}
ISheetState* CCursorer::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e){return ISheetState::Normal();}

ISheetState* CCursorer::OnKeyDown(CGridView* pSheet, KeyEventArgs& e)
{
	if(pSheet->Empty()){
		return ISheetState::Normal();
	}
	switch (e.Char)
	{
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		{
			//CRowColumn roco(m_rocoCurrent);

			//Not Focused yet. Cell(0, 0) is Focused;
			CRowColumn roco;
			if(m_rocoFocused.IsInvalid()){
				roco=CRowColumn(pSheet->Index2Pointer<RowTag, VisTag>(0).get(),pSheet->Index2Pointer<ColTag, VisTag>(0).get());
			}else{//Move selection
				roco=m_rocoCurrent;//TODO Low Current or Focused
				size_type roVisib= roco.GetRowPtr()->GetIndex<VisTag>();
				size_type coVisib= roco.GetColumnPtr()->GetIndex<VisTag>();
				switch(e.Char){
					case VK_LEFT:
						coVisib=(max(coVisib-1,0));
						break;
					case VK_RIGHT:
						coVisib=(min(coVisib+1,pSheet->GetColumnVisibleSize()-1));
						break;
					case VK_UP:
						roVisib=(max(roVisib-1,0));
						break;
					case VK_DOWN:
						roVisib=(min(roVisib+1,pSheet->GetRowVisibleSize()-1));
						break;
				}
				roco=CRowColumn(pSheet->Index2Pointer<RowTag, VisTag>(roVisib).get(),pSheet->Index2Pointer<ColTag, VisTag>(coVisib).get());
			}
			EventArgs e;
			if(::GetKeyState(VK_CONTROL)<0){
				OnCursorCtrl(pSheet, e, roco);
			}else if(::GetKeyState(VK_SHIFT)<0){
				OnCursorShift(pSheet, e, roco);
			}else{
				OnCursor(pSheet, e, roco);
			}

			pSheet->EnsureVisibleCell(roco.GetColumnPtr()->Cell(roco.GetRowPtr()));
			break;
		}
	case VK_ESCAPE:
		OnCursorClear(pSheet, EventArgs());
		break;
	default:
		break;
	}
	return ISheetState::Normal();
}

std::vector<RC> CCursorer::GetFocusedRCs(CSheet* pSheet)const
{
	std::vector<RC> focusedRCs;
	CRowColumn focusedRoCo = GetFocusedRowColumn();
	if(!focusedRoCo.IsInvalid()){
		std::shared_ptr<CCell> focusedCell = CSheet::Cell(focusedRoCo.GetRowPtr(), focusedRoCo.GetColumnPtr());
		focusedRCs.emplace_back(focusedCell->GetRowPtr()->GetIndex<AllTag>(), focusedCell->GetColumnPtr()->GetIndex<AllTag>());
		if(auto p = std::dynamic_pointer_cast<CSheet>(focusedCell)){
			auto rcs = p->GetCursorerPtr()->GetFocusedRCs(p.get());
			std::copy(rcs.begin(), rcs.end(),std::back_inserter(focusedRCs));
		}
		}
	return focusedRCs;
}


std::vector<RC> CCursorer::GetSelectedRCs(CSheet* pSheet)const
{
	std::vector<RC> selectedRCs;
	auto& rowDictionary=pSheet->m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();

	for(auto& rowData : rowDictionary){
		for(auto& colData : colDictionary){
			auto cell = CSheet::Cell(rowData.DataPtr, colData.DataPtr);
			if(cell->GetSelected()){
				selectedRCs.emplace_back(cell->GetRowPtr()->GetIndex<AllTag>(), cell->GetColumnPtr()->GetIndex<AllTag>());
			}
		}
	}
	return selectedRCs;
}
std::vector<CSheet::row_type> CCursorer::GetSelectedRows(CSheet* pSheet)const
{
	std::vector<row_type> selectedRows;
	auto& rowDictionary=pSheet->m_rowVisibleDictionary.get<IndexTag>();

	for(auto& rowData : rowDictionary){
		if(rowData.DataPtr->GetSelected()){
			selectedRows.push_back(rowData.DataPtr);
		}
	}
	return selectedRows;
}
std::vector<CSheet::column_type> CCursorer::GetSelectedColumns(CSheet* pSheet)const
{
	std::vector<column_type> selectedCols;
	auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();

	for(auto& colData : colDictionary){
		if(colData.DataPtr->GetSelected()){
			selectedCols.push_back(colData.DataPtr);
		}
	}
	return selectedCols;
}

void CCursorer::SetFocusedRCs(CSheet* pSheet, std::vector<RC> rcs)
{
	if(rcs.empty() || rcs.size()==0)return;
	auto rowMinMax = pSheet->GetMinMaxAllRowIndex();
	auto colMinMax = pSheet->GetMinMaxAllColumnIndex();
	auto rc = rcs[0];
	if(rowMinMax.first<=rc.Row && rc.Row<=rowMinMax.second &&
		colMinMax.first<=rc.Column && rc.Column<=colMinMax.second){
		auto cell = pSheet->Cell<AllTag>(rc.Row, rc.Column);
		pSheet->GetCursorerPtr()->SetFocusedRowColumn(CRowColumn(pSheet->Index2Pointer<RowTag, AllTag>(rcs[0].Row).get(), pSheet->Index2Pointer<ColTag, AllTag>(rcs[0].Column).get()));
		if( rcs.size()>1 ){
			if(auto p = std::dynamic_pointer_cast<CSheet>(cell)){
				p->GetCursorerPtr()->SetFocusedRowColumn(CRowColumn(p->Index2Pointer<RowTag, AllTag>(rcs[1].Row).get(), p->Index2Pointer<ColTag, AllTag>(rcs[1].Column).get()));		
			}
		}
	}

}
void CCursorer::SetSelectedRCs(CSheet* pSheet, std::vector<RC> rcs)
{
	if(rcs.empty() || rcs.size()==0)return;
	auto rowMinMax = pSheet->GetMinMaxAllRowIndex();
	auto colMinMax = pSheet->GetMinMaxAllColumnIndex();

	for(auto& rc : rcs){
		if(rowMinMax.first<=rc.Row && rc.Row<=rowMinMax.second &&
			colMinMax.first<=rc.Column && rc.Column<=colMinMax.second){
			pSheet->Cell<AllTag>(rc.Row, rc.Column)->SetSelected(true);
		}
	}
}
void CCursorer::SetSelectedRows(CSheet* pSheet, std::vector<row_type> rows)
{
	if(rows.empty() || rows.size()==0)return;

	for(auto& row : rows){
		row->SetSelected(true);
	}
	
}
void CCursorer::SetSelectedColumns(CSheet* pSheet, std::vector<column_type> cols)
{
	if(cols.empty() || cols.size()==0)return;

	for(auto& col : cols){
		col->SetSelected(true);
	}
}


ISheetState* CSheetCellCursorer::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return ISheetState::Normal();
	}
	//Get RowColumn from Point
	auto roco = pSheet->Point2RowColumn(e.Point);

	if(roco.IsInvalid()){
		return OnCursorClear(pSheet, EventArgs());
	}

	//Focus, Select

	if(e.Flags & MK_CONTROL){
		return OnCursorCtrl(pSheet, e,roco);
	}else if(e.Flags & MK_SHIFT){
		return OnCursorShift(pSheet, e,roco);
	}else{
		return OnCursor(pSheet, e,roco);
	}

	return ISheetState::Normal();
}

ISheetState* CSheetCellCursorer::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	if(pSheet->Empty()){
		return ISheetState::Normal();
	}
	//Get RowColumn from Point
	auto roco = pSheet->Point2RowColumn(e.Point);

	if(roco.IsInvalid()){
		return OnCursorClear(pSheet, EventArgs());
	}

	//Focus, Select
	return OnCursor(pSheet, e,roco);
}



