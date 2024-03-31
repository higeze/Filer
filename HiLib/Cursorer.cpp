#include "Cursorer.h"
#include "GridView.h"
#include "GridView.h"
#include "SheetState.h"
#include "RowColumn.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "D2DWWindow.h"


bool CCursorer::IsCursorTargetCell(const std::shared_ptr<CCell>& cell)
{
	return cell &&
		cell->GetRowPtr()->GetIndex<AllTag>() >= cell->GetGridPtr()->GetFrozenCount<RowTag>() &&
		cell->GetColumnPtr()->GetIndex<AllTag>() >= cell->GetGridPtr()->GetFrozenCount<ColTag>();

}

void CCursorer::UpdateCursor(const std::shared_ptr<CCell>& cell, bool old, bool current, bool anchor, bool focus)
{
	if (old) { m_oldCell = m_currentCell; }//Old
	if (current) { m_currentCell = cell; }//Current
	if (anchor) { m_anchorCell = cell; }//Anchor
	if (focus) {
		if (m_focusedCell != cell) {
			BOOL bDummy(TRUE);
			if (m_focusedCell) { m_focusedCell->OnKillFocus(KillFocusEvent(cell->GetGridPtr()->GetWndPtr(), NULL, NULL, &bDummy)); }//Blur
			m_focusedCell = cell;
			m_focusedCell->OnSetFocus(SetFocusEvent(cell->GetGridPtr()->GetWndPtr(), 0,0,&bDummy));//Focus
			m_doubleFocusedCell = nullptr;//DoubleFocus
		} else if (m_focusedCell == cell) {
			m_doubleFocusedCell = cell;//DoubleFocus
		}
	}
}

void CCursorer::OnCursor(const std::shared_ptr<CCell>& cell)
{
	if(!IsCursorTargetCell(cell)){
		return;
	}
	
	UpdateCursor(cell);
	cell->GetGridPtr()->DeselectAll();
	cell->GetRowPtr()->SetIsSelected(true);//Select
}

void CCursorer::OnCursorDown(const std::shared_ptr<CCell>& cell)
{
	if (!IsCursorTargetCell(cell)) {
		return;
	}

	UpdateCursor(cell);
	m_isDragPossible = true;
	cell->GetRowPtr()->SetIsSelected(true);//Select
}

void CCursorer::OnCursorUp(const std::shared_ptr<CCell>& cell)
{
	if (!IsCursorTargetCell(cell)) {
		return;
	}

	if (m_isDragPossible) {
		m_isDragPossible = false;
		cell->GetGridPtr()->DeselectAll();
		cell->GetRowPtr()->SetIsSelected(true);//Select
	}
}

void CCursorer::OnCursorLeave(const std::shared_ptr<CCell>& cell)
{
	if (!IsCursorTargetCell(cell)) {
		return;
	}
	if (m_isDragPossible) {
		m_isDragPossible = false;
		//cell->GetGridPtr()->DeselectAll();
		//cell->GetRowPtr()->SetSelected(true);//Select
	}
}

void CCursorer::OnCursorCtrl(const std::shared_ptr<CCell>& cell)
{
	if (!IsCursorTargetCell(cell)) {
		return;
	}
	UpdateCursor(cell);

	cell->GetRowPtr()->SetIsSelected(!cell->GetRowPtr()->GetIsSelected());//Select
}

void CCursorer::OnCursorShift(const std::shared_ptr<CCell>& cell)
{
	if (!IsCursorTargetCell(cell)) {
		return;
	}
	UpdateCursor(cell, true, true, false, true);
	//m_oldCell=m_currentCell;//Old
	//m_currentCell=cell;//Current

	cell->GetGridPtr()->SelectBandRange(m_anchorCell->GetRowPtr(), m_oldCell->GetRowPtr(), false);
	cell->GetGridPtr()->SelectBandRange(m_anchorCell->GetRowPtr(), m_currentCell->GetRowPtr(), true);
}

void CCursorer::OnCursorCtrlShift(const std::shared_ptr<CCell>& cell)
{
	if (!IsCursorTargetCell(cell)) {
		return;
	}

	UpdateCursor(cell, true, true, false, true);
	cell->GetGridPtr()->SelectBandRange(m_anchorCell->GetRowPtr(), m_currentCell->GetRowPtr(), true);
}

void CCursorer::OnCursorClear(CGridView* pSheet)
{
	if(m_focusedCell){
		m_focusedCell->OnKillFocus(KillFocusEvent(pSheet->GetWndPtr(), NULL, NULL));//Blur
	}
	Clear();
	pSheet->DeselectAll();//Select
	pSheet->UnhotAll();//Hot
}

void CCursorer::OnLButtonDown(CGridView* pSheet, const LButtonDownEvent& e)
{
	if(pSheet->Empty()){
		return;
	}
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));

	//If out of sheet, reset curosr
	if(!cell){
		return OnCursorClear(pSheet);
	}

	//Focus, Select
	//if(cell->GetRowPtr()==pSheet->GetNameHeaderRowPtr().get() && cell->GetColumnPtr()==pSheet->GetHeaderColumnPtr().get()){
	//	pSheet->SelectAll();//TODO Make RowColumnHeaderCell
	//}
	//else {
		if (e.Flags & MK_CONTROL && e.Flags & MK_SHIFT) {
			return OnCursorCtrlShift(cell);
		}else if (e.Flags & MK_CONTROL) {
			return OnCursorCtrl(cell);
		}else if(e.Flags & MK_SHIFT){
			return OnCursorShift(cell);
		}else{
			if (cell->GetIsSelected()) {//Only case of selected cell, behaviour is wrong. up to reset.
				return OnCursorDown(cell);
			}
			else {
				return OnCursor(cell);
			}
		}
	//}
}

void CCursorer::OnLButtonUp(CGridView* pSheet, const LButtonUpEvent& e)
{
	if (pSheet->Empty()) {
		return;
	}
	//auto cell = pSheet->Cell(e.Point);

	//If out of sheet, reset curosr
	if (!m_currentCell) {
		return;
	}

	return OnCursorUp(m_currentCell);
}

void CCursorer::OnMouseLeave(CGridView* pSheet, const MouseLeaveEvent& e)
{
	if (pSheet->Empty()) {
		return;
	}
	//auto cell = pSheet->Cell(e.Point);

	//If out of sheet, reset curosr
	if (!m_currentCell) {
		return;
	}

	return OnCursorLeave(m_currentCell);
}

void CCursorer::OnRButtonDown(CGridView* pSheet, const RButtonDownEvent& e)
{
	if(pSheet->Empty()){
		return;
	}
	//Get RowColumn from Point
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));

	if(!cell){
		return OnCursorClear(pSheet);
	}

	//Focus, Select
	if(cell->GetIsSelected()){
		//Do Nothing
	}else{
		return OnCursor(cell);
	}
	return;
}

void CCursorer::OnKeyDown(CGridView* pSheet, const KeyDownEvent& e)
{
	if(pSheet->Empty()){
		*e.HandledPtr= FALSE;
		return;
	}

	auto moveCursor = [this, pSheet](UINT arrow, bool ctrl, bool shift)->void {
		std::shared_ptr<CCell> cell;
		if (!m_focusedCell) {//Not Focused yet. Cell(frozen, frozen) is Focused;
			cell = pSheet->Cell<VisTag>(pSheet->GetFrozenCount<RowTag>(), pSheet->GetFrozenCount<ColTag>());
		} else {//Move selection
			cell = m_currentCell;//TODO Low Current or Focused
			int roVisib = cell->GetRowPtr()->GetIndex<VisTag>();
			int coVisib = cell->GetColumnPtr()->GetIndex<VisTag>();
			switch (arrow) {
			case VK_LEFT:
				coVisib = ((std::max)(coVisib - 1, pSheet->GetFrozenCount<ColTag>()));
				break;
			case VK_RIGHT:
				coVisib = ((std::min)(coVisib + 1, (int)pSheet->GetContainer<ColTag, VisTag>().size() - 1));
				break;
			case VK_UP:
				roVisib = ((std::max)(roVisib - 1, pSheet->GetFrozenCount<RowTag>()));
				break;
			case VK_DOWN:
				roVisib = ((std::min)(roVisib + 1, (int)pSheet->GetContainer<RowTag, VisTag>().size() - 1));
				break;
			}
			cell = pSheet->Cell<VisTag>(roVisib, coVisib);
		}

		if (ctrl) {
			OnCursorCtrl(cell);
		} else if (shift) {
			OnCursorShift(cell);
		} else {
			OnCursor(cell);
		}
		((CGridView*)pSheet)->PostUpdate(Updates::EnsureVisibleFocusedCell);
	};

	UINT arrow = e.Char;
	bool ctrl = ::GetKeyState(VK_CONTROL) & 0x8000;
	bool shift = ::GetKeyState(VK_SHIFT) & 0x8000;
	switch (e.Char)
	{
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		arrow = e.Char;
		moveCursor(arrow, ctrl, shift);
		*e.HandledPtr = TRUE;
		break;
	case VK_ESCAPE:
		OnCursorClear(pSheet);
		break;
		*e.HandledPtr = TRUE;
	default:
		*e.HandledPtr = FALSE;
	}
	return;
}


std::vector<Indexes> CCursorer::GetFocusedRCs(CGridView* pSheet)const
{
	std::vector<Indexes> focusedRCs;
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


std::vector<Indexes> CCursorer::GetSelectedRCs(CGridView* pSheet)const
{
	std::vector<Indexes> selectedRCs;
	//auto& rowDictionary=pSheet->m_rowVisibleDictionary.get<IndexTag>();
	//auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();

	//for(auto& rowData : rowDictionary){
	//	for(auto& colData : colDictionary){
	//		auto cell = Cell(rowData.DataPtr, colData.DataPtr);
	//		if(cell->GetSelected()){
	//			selectedRCs.emplace_back(cell->GetRowPtr()->GetIndex<AllTag>(), cell->GetColumnPtr()->GetIndex<AllTag>());
	//		}
	//	}
	//}
	return selectedRCs;
}
std::vector<std::shared_ptr<CRow>> CCursorer::GetSelectedRows(CGridView* pSheet)const
{
	std::vector<std::shared_ptr<CRow>> selectedRows;
	auto& rowContainer=pSheet->GetContainer<RowTag, VisTag>();

	for(auto& ptr : rowContainer){
		if(ptr->GetIsSelected()){
			selectedRows.push_back(ptr);
		}
	}
	return selectedRows;
}
std::vector<std::shared_ptr<CColumn>> CCursorer::GetSelectedColumns(CGridView* pSheet)const
{
	std::vector<std::shared_ptr<CColumn>> selectedCols;
	//auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();

	//for(auto& colData : colDictionary){
	//	if(colData.DataPtr->GetSelected()){
	//		selectedCols.push_back(colData.DataPtr);
	//	}
	//}
	return selectedCols;
}

void CCursorer::SetFocusedRCs(CGridView* pSheet, std::vector<Indexes> rcs)
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
void CCursorer::SetSelectedRCs(CGridView* pSheet, std::vector<Indexes> rcs)
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

void CCursorer::SetSelectedRows(CGridView* pSheet, std::vector<std::shared_ptr<CRow>> rows)
{
	//if(rows.empty() || rows.size()==0)return;

	//for(auto& row : rows){
	//	row->SetSelected(true);
	//}
}

void CCursorer::SetSelectedColumns(CGridView* pSheet, std::vector<std::shared_ptr<CColumn>> cols)
{
	//if(cols.empty() || cols.size()==0)return;

	//for(auto& col : cols){
	//	col->SetSelected(true);
	//}
}


void CSheetCellCursorer::OnLButtonDown(CGridView* pSheet, const LButtonDownEvent& e)
{
	if(pSheet->Empty()){
		return;
	}
	//Get RowColumn from Point
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));

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

void CSheetCellCursorer::OnRButtonDown(CGridView* pSheet, const RButtonDownEvent& e)
{
	if(pSheet->Empty()){
		return;
	}
	//Get RowColumn from Point
	auto cell = pSheet->Cell(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));

	if(!cell){
		return OnCursorClear(pSheet);
	}
	//Focus, Select
	return OnCursor(cell);
}

void CSheetCellCursorer::OnKeyDown(CGridView* pSheet, const KeyDownEvent& e)
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

void CExcelLikeCursorer::OnKeyDown(CGridView* pSheet, const KeyDownEvent& e)
{
	if (pSheet->Empty()) {
		return;
	}

	auto moveCursor = [this, pSheet](UINT arrow, bool ctrl, bool shift)->void {
		std::shared_ptr<CCell> cell;
		if (!m_focusedCell) {//Not Focused yet. Cell(frozen, frozen) is Focused;
			cell = pSheet->Cell<VisTag>(pSheet->GetFrozenCount<RowTag>(), pSheet->GetFrozenCount<ColTag>());
		} else {//Move selection
			cell = m_currentCell;//TODO Low Current or Focused
			int roVisib = cell->GetRowPtr()->GetIndex<VisTag>();
			int coVisib = cell->GetColumnPtr()->GetIndex<VisTag>();
			switch (arrow) {
			case VK_LEFT:
				coVisib = ((std::max)(coVisib - 1, pSheet->GetFrozenCount<ColTag>()));
				break;
			case VK_RIGHT:
				coVisib = ((std::min)(coVisib + 1, (int)pSheet->GetContainer<ColTag, VisTag>().size() - 1));
				break;
			case VK_UP:
				roVisib = ((std::max)(roVisib - 1, pSheet->GetFrozenCount<RowTag>()));
				break;
			case VK_DOWN:
				roVisib = ((std::min)(roVisib + 1, (int)pSheet->GetContainer<RowTag, VisTag>().size() - 1));
				break;
			}
			cell = pSheet->Cell<VisTag>(roVisib, coVisib);
		}

		if (ctrl) {
			OnCursorCtrl(cell);
		} else if (shift) {
			OnCursorShift(cell);
		} else {
			OnCursor(cell);
		}
		((CGridView*)pSheet)->PostUpdate(Updates::EnsureVisibleFocusedCell);
	};

	UINT arrow = e.Char;
	bool ctrl = ::GetKeyState(VK_CONTROL) & 0x8000;
	bool shift = ::GetKeyState(VK_SHIFT) & 0x8000;
	switch (e.Char) {
	case VK_TAB:
		if (shift) {
			arrow = VK_LEFT;
		} else {
			arrow = VK_RIGHT;
		}
		ctrl = false; shift = false;
		moveCursor(arrow, ctrl, shift);
		break;
	case VK_RETURN:
		if (shift) {
			arrow = VK_UP;
		} else {
			arrow = VK_DOWN;
		}
		ctrl = false; shift = false;
		moveCursor(arrow, ctrl, shift);
		break;
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		arrow = e.Char;
		moveCursor(arrow, ctrl, shift);
		break;
	case VK_ESCAPE:
		OnCursorClear(pSheet);
		break;
	default:
		break;
	}

	return;
}







