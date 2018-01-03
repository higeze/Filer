#include "FileDragger.h"
#include "Sheet.h"
#include "SheetState.h"
#include "Row.h"
#include "Column.h"
#include "FilerGridView.h"
#include "FileDraggingState.h"

ISheetState* CFileDragger::OnBeginDrag(CSheet* pSheet, MouseEventArgs& e)
{
	m_ptDragStart = e.Point;
	return CFileDraggingState::FileDragging();
}

ISheetState* CFileDragger::OnDrag(CSheet* pSheet, MouseEventArgs& e)
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
		return ISheetState::Normal();
	}
}

ISheetState* CFileDragger::OnEndDrag(CSheet* pSheet, MouseEventArgs& e)
{
	//TODO

	return ISheetState::Normal();
}

ISheetState* CFileDragger::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e){return ISheetState::Normal();}
ISheetState* CFileDragger::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e){return ISheetState::Normal();}
ISheetState* CFileDragger::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e){return ISheetState::Normal();}
ISheetState* CFileDragger::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e){return ISheetState::Normal();}
ISheetState* CFileDragger::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	if(!pSheet->Visible())return ISheetState::Normal();

	auto visibleIndexes = pSheet->Point2VisibleIndexes(e.Point);
	//If Header except Filter
	if(IsDragable(pSheet, visibleIndexes)){
		if(e.Flags==MK_LBUTTON){
			return OnBeginDrag(pSheet, e);
		}
	}
	return ISheetState::Normal();
}
ISheetState* CFileDragger::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e){return ISheetState::Normal();}
ISheetState* CFileDragger::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e){return ISheetState::Normal();}

bool CFileDragger::IsDragable(CSheet* pSheet, std::pair<size_type, size_type> visibleIndexes)
{
	auto& rowDictionary = pSheet->m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();

	auto maxRow = boost::prior(rowDictionary.end())->DataPtr->GetIndex<VisTag>();
	auto minRow = rowDictionary.begin()->DataPtr->GetIndex<VisTag>();
	auto maxCol = boost::prior(colDictionary.end())->DataPtr->GetIndex<VisTag>();
	auto minCol = colDictionary.begin()->DataPtr->GetIndex<VisTag>();

	auto spRow = pSheet->Index2Pointer<RowTag, VisTag>(visibleIndexes.first);
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

ISheetState* CFileDragger::OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return ISheetState::Normal();
}
ISheetState* CFileDragger::OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return OnEndDrag(pSheet, e);		
}
ISheetState* CFileDragger::OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return ISheetState::Normal();
}
ISheetState* CFileDragger::OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return ISheetState::Normal();
}
ISheetState* CFileDragger::OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	return OnDrag(pSheet, e);
}
ISheetState* CFileDragger::OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e)
{
	return OnDrag(pSheet, e);
}
ISheetState* CFileDragger::OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
{
	return CFileDraggingState::FileDragging();
}
