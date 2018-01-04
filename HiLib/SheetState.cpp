//#include "SheetState.h"
//#include "Sheet.h"
//#include "RowColumn.h"
//#include "Column.h"
//#include "Cell.h"
//#include "IMouseObserver.h"
//#include "Tracker.h"
//#include "Dragger.h"
//#include "GridView.h"
//
//ISheetState* CNormalState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	if (!pSheet->Visible()) {
//		return ISheetState::Normal();
//	}
//	auto roco = pSheet->Point2RowColumn(e.Point);
//
//	for(auto& observer : pSheet->m_mouseObservers){
//		auto state = observer->OnLButtonDown(pSheet, e);
//		if(state != ISheetState::Normal())
//		{
//			return state;
//		}
//	}
//	
//	if(!roco.IsInvalid()){
//		roco.GetColumnPtr()->Cell(roco.GetRowPtr())->OnLButtonDown(e);
//	}
//	return ISheetState::Normal();
//}
//
//ISheetState* CNormalState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
//{
//	if(!pSheet->Visible())return ISheetState::Normal();
//	auto roco = pSheet->Point2RowColumn(e.Point);
//
//	for(auto& observer : pSheet->m_mouseObservers){
//		auto state = observer->OnLButtonUp(pSheet, e);
//		if(state != ISheetState::Normal())
//		{
//			return state;
//		}
//	}
//
//	if(!roco.IsInvalid()){
//		roco.GetColumnPtr()->Cell(roco.GetRowPtr())->OnLButtonUp(e);
//	}
//	return ISheetState::Normal();
//}
//
//ISheetState* CNormalState::OnLButtonSnglClk(CSheet* pSheet, MouseEventArgs& e)
//{
//	if (!pSheet->Visible())return ISheetState::Normal();
//
//	auto roco = pSheet->Point2RowColumn(e.Point);
//	if (!roco.IsInvalid()) {
//		roco.GetColumnPtr()->Cell(roco.GetRowPtr())->OnLButtonSnglClk(e);
//	}
//	return ISheetState::Normal();
//};
//
//ISheetState* CNormalState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
//{
//	if(!pSheet->Visible())return ISheetState::Normal();
//	auto roco = pSheet->Point2RowColumn(e.Point);
//
//	for(auto& observer : pSheet->m_mouseObservers){
//		auto state = observer->OnLButtonDblClk(pSheet, e);
//		if(state != ISheetState::Normal())
//		{
//			return state;
//		}
//	}
//
//	if(!roco.IsInvalid()){
//		roco.GetColumnPtr()->Cell(roco.GetRowPtr())->OnLButtonDblClk(e);
//	}
//	return ISheetState::Normal();
//};
//
//ISheetState* CNormalState::OnLButtonBeginDrag(CSheet* pSheet, MouseEventArgs& e)
//{
//	if (!pSheet->Visible()) {
//		return ISheetState::Normal();
//	}
//
//	auto visIndexes = pSheet->Coordinates2Indexes<VisTag>(e.Point);
//
//	if (visIndexes.first <= pSheet->GetMaxIndex<RowTag, VisTag>() &&
//		visIndexes.first >= pSheet->GetMinIndex<RowTag, VisTag>() &&
//		visIndexes.second <= pSheet->GetMaxIndex<ColTag, VisTag>() &&
//		visIndexes.second >= pSheet->GetMinIndex<ColTag, VisTag>()) {
//
//		if (pSheet->Index2Pointer<RowTag, VisTag>(visIndexes.first)->IsDragTrackable()) {
//			return ISheetState::RowDragging();
//		}
//		else if (pSheet->Index2Pointer<ColTag, VisTag>(visIndexes.second)->IsDragTrackable()) {
//			return ISheetState::ColumnDragging();
//		}
//		else {
//			return ISheetState::ItemDragging();
//		}
//	}
//
//	if (visIndexes.first <= pSheet->GetMaxIndex<RowTag, VisTag>() &&
//		visIndexes.first >= pSheet->GetMinIndex<RowTag, VisTag>() &&
//		pSheet->Index2Pointer<RowTag, VisTag>(visIndexes.first)->IsDragTrackable()) {
//		return ISheetState::RowDragging();
//	}
//	else if (visIndexes.second <= pSheet->GetMaxIndex<ColTag, VisTag>() &&
//		visIndexes.second >= pSheet->GetMinIndex<ColTag, VisTag>() &&
//		pSheet->Index2Pointer<ColTag, VisTag>(visIndexes.second)->IsDragTrackable()) {
//		return ISheetState::ColumnDragging();
//	}
//	else if (visIndexes.first <= pSheet->GetMaxIndex<RowTag, VisTag>() &&
//		visIndexes.first >= pSheet->GetMinIndex<RowTag, VisTag>() &&
//		visIndexes.second <= pSheet->GetMaxIndex<ColTag, VisTag>() &&
//		visIndexes.second >= pSheet->GetMinIndex<ColTag, VisTag>() &&
//
//
//
//	//If Header except Filter
//	if (IsDragable(pSheet, visIndex)) {
//		if (e.Flags == MK_LBUTTON) {
//			return OnHeaderBeginDrag(pSheet, e);
//		}
//	}
//	return CSheetState::Normal();
//}
//
//
//ISheetState* CNormalState::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	if(!pSheet->Visible())return ISheetState::Normal();
//	auto roco = pSheet->Point2RowColumn(e.Point);
//
//	for(auto& observer : pSheet->m_mouseObservers){
//		auto state = observer->OnRButtonDown(pSheet, e);
//		if(state != ISheetState::Normal())
//		{
//			return state;
//		}
//	}
//	//TODOTODO
//	//if(!roco.IsInvalid()){
//	//	roco.GetColumnPtr()->Cell(roco.GetRowPtr())->OnRButtonDown(e);
//	//}
//	return ISheetState::Normal();
//}
//
//ISheetState* CNormalState::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
//{
//	if(!pSheet->Visible())return ISheetState::Normal();
//	auto roco = pSheet->Point2RowColumn(e.Point);
//
//	for(auto& observer : pSheet->m_mouseObservers){
//		auto state = observer->OnMouseMove(pSheet, e);
//		if(state != ISheetState::Normal())
//		{
//			return state;
//		}
//	}
//
//	CRowColumn rocoMouse = pSheet->GetMouseRowColumn();
//	if(!rocoMouse.IsInvalid() && rocoMouse!=roco){
//		rocoMouse.GetColumnPtr()->Cell(rocoMouse.GetRowPtr())->OnMouseLeave(e);
//	}
//	pSheet->SetMouseRowColumn(roco);
//
//	if(!roco.IsInvalid()){
//		roco.GetColumnPtr()->Cell(roco.GetRowPtr())->OnMouseMove(e);
//	}
//	return ISheetState::Normal();
//};
//
//ISheetState* CNormalState::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)
//{
//	if(!pSheet->Visible())return ISheetState::Normal();
//	auto roco = pSheet->Point2RowColumn(e.Point);
//
//	for(auto& observer : pSheet->m_mouseObservers){
//		auto state = observer->OnMouseLeave(pSheet, e);
//		if(state != ISheetState::Normal())
//		{
//			return state;
//		}
//	}
//
//	CRowColumn rocoMouse = pSheet->GetMouseRowColumn();
//	if(!rocoMouse.IsInvalid()){
//		pSheet->UnhotAll();
//		//CSheet::Cell(m_rocoMouse.GetRowPtr(), m_rocoMouse.GetColumnPtr())->SetState(UIElementState::Normal);
//		CSheet::Cell(rocoMouse.GetRowPtr(), rocoMouse.GetColumnPtr())->OnMouseLeave(e);
//	}
//	pSheet->SetMouseRowColumn(roco);
//	return ISheetState::Normal();
//}
//
//ISheetState* CNormalState::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
//{
//	if(!pSheet->Visible())return ISheetState::Normal();
//
//	for(auto& observer : pSheet->m_mouseObservers){
//		auto state = observer->OnSetCursor(pSheet, e);
//		if(state != ISheetState::Normal())
//		{
//			return state;
//		}
//	}
//	CPoint pt;
//	::GetCursorPos(&pt);
//	::ScreenToClient(pSheet->GetGridPtr()->m_hWnd, &pt);
//	auto roco = pSheet->Point2RowColumn(pt);
//	if(!roco.IsInvalid()){
//		roco.GetColumnPtr()->Cell(roco.GetRowPtr())->OnSetCursor(e);
//	}
//	return ISheetState::Normal();
//}
//
//
//
//ISheetState* CTrackingState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spTracker->OnTrackLButtonDown(pSheet, e);
//}
//ISheetState* CTrackingState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spTracker->OnTrackLButtonUp(pSheet, e);		
//}
//ISheetState* CTrackingState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spTracker->OnTrackLButtonDblClk(pSheet, e);		
//}
//ISheetState* CTrackingState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
//{
//	return ISheetState::Tracking();	
//}
//ISheetState* CTrackingState::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spTracker->OnTrackRButtonDown(pSheet, e);		
//}
//ISheetState* CTrackingState::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spTracker->OnTrackMouseMove(pSheet, e);		
//}
//ISheetState* CTrackingState::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spTracker->OnTrackMouseLeave(pSheet, e);		
//}
//ISheetState* CTrackingState::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
//{
//	return pSheet->m_spTracker->OnTrackSetCursor(pSheet, e);		
//}
//
//
//ISheetState* CColumnDraggingState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spColDragger->OnDragLButtonDown(pSheet, e);
//}
//ISheetState* CColumnDraggingState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spColDragger->OnDragLButtonUp(pSheet, e);		
//}
//ISheetState* CColumnDraggingState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spColDragger->OnDragLButtonDblClk(pSheet, e);		
//}
//ISheetState* CColumnDraggingState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
//{
//	return ISheetState::ColumnDragging();		
//}
//ISheetState* CColumnDraggingState::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spColDragger->OnDragRButtonDown(pSheet, e);		
//}
//ISheetState* CColumnDraggingState::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spColDragger->OnDragMouseMove(pSheet, e);		
//}
//ISheetState* CColumnDraggingState::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spColDragger->OnDragMouseLeave(pSheet, e);		
//}
//ISheetState* CColumnDraggingState::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
//{
//	return pSheet->m_spColDragger->OnDragSetCursor(pSheet, e);		
//}
//
//ISheetState* CColumnDraggingState::State()
//{
//	static CColumnDraggingState state;
//	return &state;
//}
//
//
//
//ISheetState* CRowDraggingState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spRowDragger->OnDragLButtonDown(pSheet, e);
//}
//ISheetState* CRowDraggingState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spRowDragger->OnDragLButtonUp(pSheet, e);
//}
//ISheetState* CRowDraggingState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spRowDragger->OnDragLButtonDblClk(pSheet, e);
//}
//ISheetState* CRowDraggingState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
//{
//	return ISheetState::RowDragging();
//}
//ISheetState* CRowDraggingState::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spRowDragger->OnDragRButtonDown(pSheet, e);
//}
//ISheetState* CRowDraggingState::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spRowDragger->OnDragMouseMove(pSheet, e);
//}
//ISheetState* CRowDraggingState::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)
//{
//	return pSheet->m_spRowDragger->OnDragMouseLeave(pSheet, e);
//}
//ISheetState* CRowDraggingState::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
//{
//	return pSheet->m_spRowDragger->OnDragSetCursor(pSheet, e);
//}
//
//ISheetState* CRowDraggingState::State()
//{
//	static CRowDraggingState state;
//	return &state;
//}
//
//
//
//
////CSheetState* CDraggingState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();};
////CSheetState* CDraggingState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
////{
////	return pSheet->m_spDragger->OnColumnHeaderEndDrag(pSheet, e);		
////}
////CSheetState* CDraggingState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();};
////CSheetState* CDraggingState::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
////CSheetState* CDraggingState::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
////{
////	return pSheet->m_spDragger->OnColumnHeaderDrag(pSheet, e);
////}
////CSheetState* CDraggingState::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)
////{
////	return pSheet->m_spDragger->OnColumnHeaderDrag(pSheet, e);
////}
////CSheetState* CDraggingState::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
////{
////	return CSheetState::Dragging();
////}
