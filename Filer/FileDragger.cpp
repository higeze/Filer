#include "FileDragger.h"
#include "Sheet.h"
#include "SheetState.h"
#include "Row.h"
#include "Column.h"
#include "FilerGridView.h"
#include "FileDraggingState.h"

CSheetState* CFileDragger::OnBeginDrag(CSheet* pSheet, MouseEventArgs& e)
{
	m_ptDragStart = e.Point;
	return CFileDraggingState::FileDragging();
}

CSheetState* CFileDragger::OnDrag(CSheet* pSheet, MouseEventArgs& e)
{
	if(e.Flags==MK_LBUTTON){
		auto distance = std::pow(m_ptDragStart.x-e.Point.x , 2) + std::pow(m_ptDragStart.y-e.Point.y, 2);
		if(distance >100){
			if(auto p = dynamic_cast<CFilerGridView*>(pSheet)){
				p->Drag();
			}
		}
		return CFileDraggingState::FileDragging();

	}else{
		return CSheetState::Normal();
	}
}

CSheetState* CFileDragger::OnEndDrag(CSheet* pSheet, MouseEventArgs& e)
{
	//TODO

	return CSheetState::Normal();
}

CSheetState* CFileDragger::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CFileDragger::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CFileDragger::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CFileDragger::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CFileDragger::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	if(!pSheet->Visible())return CSheetState::Normal();

	auto visibleIndexes = pSheet->Point2VisibleIndexes(e.Point);
	//If Header except Filter
	if(IsDragable(pSheet, visibleIndexes)){
		if(e.Flags==MK_LBUTTON){
			return OnBeginDrag(pSheet, e);
		}
	}
	return CSheetState::Normal();
}
CSheetState* CFileDragger::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
CSheetState* CFileDragger::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e){return CSheetState::Normal();}

bool CFileDragger::IsDragable(CSheet* pSheet, std::pair<size_type, size_type> visibleIndexes)
{
	auto& rowDictionary = pSheet->m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();

	auto maxRow = boost::prior(rowDictionary.end())->DataPtr->GetVisibleIndex();
	auto minRow = rowDictionary.begin()->DataPtr->GetVisibleIndex();
	auto maxCol = boost::prior(colDictionary.end())->DataPtr->GetVisibleIndex();
	auto minCol = colDictionary.begin()->DataPtr->GetVisibleIndex();

	auto spRow = pSheet->VisibleRow(visibleIndexes.first);
	if( visibleIndexes.first < 0 || 
		visibleIndexes.first < minRow || visibleIndexes.first > maxRow || 
		visibleIndexes.second < minCol || visibleIndexes.second > maxCol){
		return false;
	}else if(spRow && !spRow->IsDragTrackable()){
		return true;
	}else{
		return false;
	}
}

CSheetState* CFileDragger::OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CFileDragger::OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return OnEndDrag(pSheet, e);		
}
CSheetState* CFileDragger::OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CFileDragger::OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return CSheetState::Normal();
}
CSheetState* CFileDragger::OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	return OnDrag(pSheet, e);
}
CSheetState* CFileDragger::OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e)
{
	return OnDrag(pSheet, e);
}
CSheetState* CFileDragger::OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
{
	return CFileDraggingState::FileDragging();
}
