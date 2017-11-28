#include "Dragger.h"
#include "Sheet.h"
#include "SheetState.h"
#include "Row.h"
#include "Column.h"

CSheetState* CColumnDragger::OnColumnHeaderBeginDrag(CSheet* pSheet, MouseEventArgs& e)
{
	m_dragFromIndex = pSheet->ColumnPointer2AllIndex(pSheet->Point2RowColumn(e.Point).GetColumnPtr());
	m_dragToIndex=COLUMN_INDEX_INVALID;
	return CSheetState::ColumnDragging();
}

CSheetState* CColumnDragger::OnColumnHeaderDrag(CSheet* pSheet, MouseEventArgs& e)
{
	if(e.Flags==MK_LBUTTON){
		

		auto visibleIndex = pSheet->X2VisibleColumnIndex(e.Point.x);

		auto& visibleDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();
		auto& allDictionary = pSheet->m_columnAllDictionary.get<IndexTag>();
		auto maxVisibleIndex = boost::prior(visibleDictionary.end())->DataPtr->GetVisibleIndex();
		auto minVisibleIndex = visibleDictionary.begin()->DataPtr->GetVisibleIndex();
		auto maxAllIndex = boost::prior(allDictionary.end())->DataPtr->GetAllIndex();
		auto minAllIndex = allDictionary.begin()->DataPtr->GetAllIndex();


		if(visibleIndex == COLUMN_INDEX_INVALID){
			m_dragToIndex = pSheet->ColumnVisibleIndex2AllIndex(visibleIndex);
		}else if(visibleIndex < minVisibleIndex){
			m_dragToIndex = pSheet->ColumnVisibleIndex2AllIndex(minVisibleIndex);
		}else if(visibleIndex > maxVisibleIndex){
			m_dragToIndex = pSheet->ColumnVisibleIndex2AllIndex(maxVisibleIndex) + 1;
		}else if(visibleIndex == minVisibleIndex){
			m_dragToIndex = pSheet->ColumnVisibleIndex2AllIndex(minVisibleIndex);
		}else if(visibleIndex == maxVisibleIndex){
			m_dragToIndex = pSheet->ColumnVisibleIndex2AllIndex(maxVisibleIndex) + 1;
		}else{
			auto allIndex = pSheet->ColumnVisibleIndex2AllIndex(visibleIndex);
			if(allIndex < m_dragFromIndex){
				m_dragToIndex = allIndex;
			}else if(allIndex > m_dragFromIndex){
				m_dragToIndex = pSheet->ColumnVisibleIndex2AllIndex(visibleIndex + 1);
			}else{
				m_dragToIndex = allIndex;		
			}
		}
		return CSheetState::ColumnDragging();

	}else{
		return CSheetState::Normal();
	}
}

CSheetState* CColumnDragger::OnColumnHeaderEndDrag(CSheet* pSheet, MouseEventArgs& e)
{
	if(m_dragToIndex == COLUMN_INDEX_INVALID){

	}else{
		auto& ptrDictionary = pSheet
			->m_columnAllDictionary.get<PointerTag>();
		auto spDragFromColumn=ptrDictionary.find(pSheet->ColumnAllIndex2Pointer(m_dragFromIndex))->DataPtr;
		//auto spDragToColumn=ptrDictionary.find(ColumnAllIndex2Pointer(m_dragToIndex))->DataPtr;
		if(m_dragToIndex!=spDragFromColumn->GetAllIndex()){
			pSheet->MoveColumn(m_dragToIndex, spDragFromColumn);
		}
	}
	m_dragFromIndex = COLUMN_INDEX_INVALID;
	m_dragToIndex = COLUMN_INDEX_INVALID;

	return CSheetState::Normal();
}
CSheetState* CColumnDragger::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CColumnDragger::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CColumnDragger::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CColumnDragger::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CColumnDragger::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	if(!pSheet->Visible())return CSheetState::Normal();

	auto visibleIndexes = pSheet->Point2VisibleIndexes(e.Point);
	//If Header except Filter
	if(IsDragable(pSheet, visibleIndexes)){
		if(e.Flags==MK_LBUTTON){
			return OnColumnHeaderBeginDrag(pSheet, e);
		}
	}
	return CSheetState::Normal();
}
CSheetState* CColumnDragger::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CColumnDragger::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e){return CSheetState::Normal();}

bool CColumnDragger::IsDragable(CSheet* pSheet, std::pair<size_type, size_type> visibleIndexes)
{
	auto& rowDictionary = pSheet->m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();

	auto maxRow = boost::prior(rowDictionary.end())->DataPtr->GetVisibleIndex();
	auto minRow = rowDictionary.begin()->DataPtr->GetVisibleIndex();
	auto maxCol = boost::prior(colDictionary.end())->DataPtr->GetVisibleIndex();
	auto minCol = colDictionary.begin()->DataPtr->GetVisibleIndex();

	auto spRow = pSheet->VisibleRow(visibleIndexes.first);
	if( visibleIndexes.first >= 0 || 
		visibleIndexes.first < minRow || visibleIndexes.first > maxRow || 
		visibleIndexes.second < minCol || visibleIndexes.second > maxCol){
		return false;
	}else if(spRow && !spRow->IsDragTrackable()){
		return false;
	}
	return true;
}

CSheetState* CColumnDragger::OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CColumnDragger::OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return OnColumnHeaderEndDrag(pSheet, e);		
}
CSheetState* CColumnDragger::OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CColumnDragger::OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CColumnDragger::OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	return OnColumnHeaderDrag(pSheet, e);
}
CSheetState* CColumnDragger::OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e)
{
	return OnColumnHeaderDrag(pSheet, e);
}
CSheetState* CColumnDragger::OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
{
	return CSheetState::ColumnDragging();
}

CSheetState* CSheetCellDragger::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}

CSheetState* CRowDragger::OnColumnHeaderBeginDrag(CSheet* pSheet, MouseEventArgs& e)
{
	m_dragFromIndex = pSheet->Coordinate2Index<RowTag, AllTag>(e.Point.y);
	m_dragToIndex = INDEX_INVALID;
	return CSheetState::RowDragging();
}

CSheetState* CRowDragger::OnColumnHeaderDrag(CSheet* pSheet, MouseEventArgs& e)
{
	if (e.Flags == MK_LBUTTON) {

		auto visibleIndex = pSheet->Coordinate2Index<RowTag, VisTag>(e.Point.y);



		//auto& visibleDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();
		//auto& allDictionary = pSheet->m_columnAllDictionary.get<IndexTag>();

		auto visMinMax = pSheet->GetMinMaxIndexes<RowTag, VisTag>();
		auto allMinMax = pSheet->GetMinMaxIndexes<RowTag, AllTag>();

		//auto maxVisibleIndex = boost::prior(visibleDictionary.end())->DataPtr->GetVisibleIndex();
		//auto minVisibleIndex = visibleDictionary.begin()->DataPtr->GetVisibleIndex();
		//auto maxAllIndex = boost::prior(allDictionary.end())->DataPtr->GetAllIndex();
		//auto minAllIndex = allDictionary.begin()->DataPtr->GetAllIndex();


		if (visibleIndex == INDEX_INVALID) {
			m_dragToIndex = pSheet->Vis2AllIndex<RowTag>(visibleIndex);
		}
		else if (visibleIndex < visMinMax.first) {
			m_dragToIndex = pSheet->Vis2AllIndex<RowTag>(visMinMax.first);
		}
		else if (visibleIndex > visMinMax.second) {
			m_dragToIndex = pSheet->Vis2AllIndex<RowTag>(visMinMax.second) + 1;
		}
		else if (visibleIndex == visMinMax.first) {
			m_dragToIndex = pSheet->Vis2AllIndex<RowTag>(visMinMax.first);
		}
		else if (visibleIndex == visMinMax.second) {
			m_dragToIndex = pSheet->Vis2AllIndex<RowTag>(visMinMax.second) + 1;
		}
		else {
			auto allIndex = pSheet->Vis2AllIndex<RowTag>(visibleIndex);
			if (allIndex < m_dragFromIndex) {
				m_dragToIndex = allIndex;
			}
			else if (allIndex > m_dragFromIndex) {
				m_dragToIndex = pSheet->Vis2AllIndex<RowTag>(visibleIndex + 1);
			}
			else {
				m_dragToIndex = allIndex;
			}
		}
		return CSheetState::RowDragging();

	}
	else {
		return CSheetState::Normal();
	}
}

CSheetState* CRowDragger::OnColumnHeaderEndDrag(CSheet* pSheet, MouseEventArgs& e)
{
	if (m_dragToIndex == INDEX_INVALID) {

	}else {
		//auto& ptrDictionary = pSheet
		//	->m_columnAllDictionary.get<PointerTag>();
		//auto spDragFromColumn = ptrDictionary.find(pSheet->ColumnAllIndex2Pointer(m_dragFromIndex))->DataPtr;
		////auto spDragToColumn=ptrDictionary.find(ColumnAllIndex2Pointer(m_dragToIndex))->DataPtr;
		//auto& 
		if (m_dragToIndex > 0 && m_dragToIndex != m_dragFromIndex) {//TODO Only when Row
			auto sp = pSheet->Index2Pointer < RowTag, AllTag> (m_dragFromIndex);
			pSheet->MoveImpl<RowTag>(m_dragToIndex, sp);
		}
	}
	m_dragFromIndex = INDEX_INVALID;
	m_dragToIndex = INDEX_INVALID;

	return CSheetState::Normal();
}
CSheetState* CRowDragger::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
CSheetState* CRowDragger::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
CSheetState* CRowDragger::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
CSheetState* CRowDragger::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
CSheetState* CRowDragger::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	if (!pSheet->Visible())return CSheetState::Normal();

	auto visIndexes = pSheet->Coordinates2Indexes<RowTag, VisTag>(e.Point);
	//If Header except Filter
	if (IsDragable(pSheet, visIndexes)) {
		if (e.Flags == MK_LBUTTON) {
			return OnColumnHeaderBeginDrag(pSheet, e);
		}
	}
	return CSheetState::Normal();
}
CSheetState* CRowDragger::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
CSheetState* CRowDragger::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e) { return CSheetState::Normal(); }

bool CRowDragger::IsDragable(CSheet* pSheet, std::pair<size_type, size_type> visibleIndexes)
{
	//auto& rowDictionary = pSheet->m_rowVisibleDictionary.get<IndexTag>();
	//auto& colDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();

	//auto maxRow = boost::prior(rowDictionary.end())->DataPtr->GetVisibleIndex();
	//auto minRow = rowDictionary.begin()->DataPtr->GetVisibleIndex();
	//auto maxCol = boost::prior(colDictionary.end())->DataPtr->GetVisibleIndex();
	//auto minCol = colDictionary.begin()->DataPtr->GetVisibleIndex();

	//auto spRow = pSheet->VisibleRow(visibleIndexes.first);
	//if (visibleIndexes.first >= 0 ||
	//	visibleIndexes.first < minRow || visibleIndexes.first > maxRow ||
	//	visibleIndexes.second < minCol || visibleIndexes.second > maxCol) {
	//	return false;
	//}
	//else if (spRow && !spRow->IsDragTrackable()) {
	//	return false;
	//}
	return true;
}

CSheetState* CRowDragger::OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CRowDragger::OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return OnColumnHeaderEndDrag(pSheet, e);
}
CSheetState* CRowDragger::OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CRowDragger::OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CRowDragger::OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	return OnColumnHeaderDrag(pSheet, e);
}
CSheetState* CRowDragger::OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e)
{
	return OnColumnHeaderDrag(pSheet, e);
}
CSheetState* CRowDragger::OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
{
	return CSheetState::RowDragging();
}
