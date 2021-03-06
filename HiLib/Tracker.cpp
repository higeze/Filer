#include "Tracker.h"
#include "Sheet.h"
#include "Row.h"
#include "Column.h"
#include "SheetEventArgs.h"
#include "GridView.h"




//CSheetState* CTracker::OnColumnHeaderBeginTrack(CSheet* pSheet, MouseEventArgs& e, int col)
//{
//	::SetCursor(::LoadCursor(NULL,IDC_SIZEWE));
//	m_coTrackLeftVisib=col;	
//	return CSheetState::Tracking();
//}	

//CSheetState* CTracker::OnColumnHeaderTrack(CSheet* pSheet, MouseEventArgs& e)
//{
//	::SetCursor(::LoadCursor(NULL,IDC_SIZEWE));
//	auto pCol=pSheet->Index2Pointer<ColTag, VisTag>(m_coTrackLeftVisib);
//	pCol->SetWidthWithoutSignal(max(e.Point.x-pCol->GetLeft(),CColumn::kMinWidth));
//	pSheet->ColumnHeaderTrack(CColumnEventArgs(pCol.get()));
//	return CSheetState::Tracking();
//}
//CSheetState* CTracker::OnColumnHeaderEndTrack(CSheet* pSheet, MouseEventArgs& e)
//{
//	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
//	auto pCol=pSheet->Index2Pointer<ColTag, VisTag>(m_coTrackLeftVisib);
//	pCol->SetWidthWithoutSignal(max(e.Point.x-pCol->GetLeft(), CColumn::kMinWidth));
//	pSheet->ColumnHeaderEndTrack(CColumnEventArgs(pCol.get()));
//	return CSheetState::Normal();
//}
//
//CSheetState* CTracker::OnColumnHeaderDividerDblClk(CSheet* pSheet, MouseEventArgs& e, int col)
//{
//	auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();
//	auto pColumn=pSheet->Index2Pointer<ColTag, VisTag>(col);
//	pSheet->ColumnHeaderFitWidth(CColumnEventArgs(pColumn.get()));	
//	return CSheetState::Normal();
//}
//
//
//CSheetState* CTracker::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	if(!pSheet->Visible())return CSheetState::Normal();
//
//	auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();
//	auto visibleIndexes = pSheet->Point2VisibleIndexes(e.Point);
//	//If Header except Filter
//	if(IsTrackable(pSheet, visibleIndexes)){
//		auto maxColumn = boost::prior(colDictionary.end())->DataPtr->GetIndex<VisTag>();
//		auto minColumn = colDictionary.begin()->DataPtr->GetIndex<VisTag>();
//		if(visibleIndexes.second < minColumn){
//			//Out of Left
//			//Do Nothing	
//		}else if(visibleIndexes.second > maxColumn){
//			//Out of Right
//			if(e.Point.x < boost::prior(colDictionary.end())->DataPtr->GetRight() + CBand::kResizeAreaHarfWidth){
//				return OnColumnHeaderBeginTrack(pSheet, e, visibleIndexes.second -1);
//			}
//		}else if(e.Point.x < (pSheet->Index2Pointer<ColTag, VisTag>(visibleIndexes.second)->GetLeft() + CBand::kResizeAreaHarfWidth)){
//			return OnColumnHeaderBeginTrack(pSheet, e, max(visibleIndexes.second - 1, minColumn));
//		}else if((pSheet->Index2Pointer<ColTag, VisTag>(visibleIndexes.second)->GetRight() - CBand::kResizeAreaHarfWidth) < e.Point.x){
//			return OnColumnHeaderBeginTrack(pSheet, e, min(visibleIndexes.second, maxColumn));
//		}else{
//			//Do Nothing
//		}
//	}
//	return CSheetState::Normal();
//}
//
//CSheetState* CTracker::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
//
//CSheetState* CTracker::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
//
//CSheetState* CTracker::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
//{
//	if(!pSheet->Visible())return CSheetState::Normal();
//
//	auto& colDictionary=pSheet->m_columnVisibleDictionary.get<IndexTag>();
//
//	//If ColumnmHeader
//	auto visibleIndexes = pSheet->Point2VisibleIndexes(e.Point);
//	//If Header except Filter
//	if(IsTrackable(pSheet, visibleIndexes)){
//		auto maxColumn = boost::prior(colDictionary.end())->DataPtr->GetIndex<VisTag>();
//		auto minColumn = colDictionary.begin()->DataPtr->GetIndex<VisTag>();
//		if(visibleIndexes.second < minColumn){
//			//Out of Left
//			//Do Nothing	
//		}else if(visibleIndexes.second > maxColumn){
//			//Out of Right
//			if(e.Point.x < boost::prior(colDictionary.end())->DataPtr->GetRight() + CBand::kResizeAreaHarfWidth){
//				return OnColumnHeaderDividerDblClk(pSheet, e, visibleIndexes.second -1);
//			}
//		}else if(e.Point.x < (pSheet->Index2Pointer<ColTag, VisTag>(visibleIndexes.second)->GetLeft() + CBand::kResizeAreaHarfWidth)){
//			return OnColumnHeaderDividerDblClk(pSheet, e, max(visibleIndexes.second - 1, minColumn));
//		}else if((pSheet->Index2Pointer<ColTag, VisTag>(visibleIndexes.second)->GetRight() - CBand::kResizeAreaHarfWidth) < e.Point.x){
//			return OnColumnHeaderDividerDblClk(pSheet, e, min(visibleIndexes.second, maxColumn));
//		}else{
//			//Do Nothing
//		}
//	}
//	return CSheetState::Normal();
//}
//
//CSheetState* CTracker::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
//{
//	return CSheetState::Normal();
//}
//
//CSheetState* CTracker::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
//{
//	std::function<CSheetState*()> arrowFunction = [&]()->CSheetState*{
//		//e.Handled = FALSE;
//		//::SetCursor(::LoadCursor(NULL,IDC_ARROW));
//		return CSheetState::Normal();
//	};
//
//	std::function<CSheetState*()> resizeFunction = [&]()->CSheetState*{
//		e.Handled = TRUE;
//		::SetCursor(::LoadCursor(NULL,IDC_SIZEWE));
//		return CSheetState::Normal();
//	};
//
//	std::function<CSheetState*()> defaultFunction = [&]()->CSheetState*{
//		//e.Handled = FALSE;
//		return CSheetState::Normal();
//	};
//
//	if(!pSheet->Visible()){
//		return arrowFunction();
//	}
//
//	//Cursor must be in Inplace edit
//	if(e.HWnd!=pSheet->GetGridPtr()->m_hWnd){
//		return defaultFunction();
//	}
//
//	CPoint pt;
//	::GetCursorPos(&pt);
//	::ScreenToClient(pSheet->GetGridPtr()->m_hWnd, &pt);
//	auto visibleIndexes = pSheet->Point2VisibleIndexes(pt);
//	//If Header except Filter
//	if(IsTrackable(pSheet, visibleIndexes)){
//		auto& colDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();
//		auto maxColumn = boost::prior(colDictionary.end())->DataPtr->GetIndex<VisTag>();
//		auto minColumn = colDictionary.begin()->DataPtr->GetIndex<VisTag>();
//		if(visibleIndexes.second < minColumn){
//			//Out of Left
//			return arrowFunction();
//		}else if(visibleIndexes.second > maxColumn){
//			//Out of Right
//			if(pt.x < boost::prior(colDictionary.end())->DataPtr->GetRight() + CBand::kResizeAreaHarfWidth){
//				return resizeFunction();				
//			}else{
//				return arrowFunction();
//			}
//		}else if(pt.x < (pSheet->Index2Pointer<ColTag, VisTag>(visibleIndexes.second)->GetLeft() + CBand::kResizeAreaHarfWidth)){
//			return resizeFunction();					
//		}else if((pSheet->Index2Pointer<ColTag, VisTag>(visibleIndexes.second)->GetRight() - CBand::kResizeAreaHarfWidth) < pt.x){
//			return resizeFunction();						
//		}else{
//			return arrowFunction();
//		}
//	}else{
//		return arrowFunction();
//	}
//}
//
//CSheetState* CTracker::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e){return CSheetState::Normal();}
//
//bool CTracker::IsTrackable(CSheet* pSheet, std::pair<int, int> visibleIndexes)
//{
//	auto& rowDictionary = pSheet->m_rowVisibleDictionary.get<IndexTag>();
//	auto& colDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();
//
//	auto maxRow = boost::prior(rowDictionary.end())->DataPtr->GetIndex<VisTag>();
//	auto minRow = rowDictionary.begin()->DataPtr->GetIndex<VisTag>();
//	auto maxCol = boost::prior(colDictionary.end())->DataPtr->GetIndex<VisTag>();
//	auto minCol = colDictionary.begin()->DataPtr->GetIndex<VisTag>();
//	CPoint pt;
//	::GetCursorPos(&pt);
//	::ScreenToClient(pSheet->GetGridPtr()->m_hWnd, &pt);
//
//	auto spRow = pSheet->Index2Pointer<RowTag, VisTag>(visibleIndexes.first);
//	if( visibleIndexes.first >= 0 || 
//		visibleIndexes.first < minRow || visibleIndexes.first > maxRow || 
//		visibleIndexes.second < minCol || (visibleIndexes.second > maxCol && pt.x > boost::prior(colDictionary.end())->DataPtr->GetRight() + CBand::kResizeAreaHarfWidth )){
//		return false;
//	}else if(spRow && !spRow->IsDragTrackable()){
//		return false;
//	}
//	return true;
//}
//
//CSheetState* CTracker::OnTrackLButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	return CSheetState::Normal();
//}
//CSheetState* CTracker::OnTrackLButtonUp(CSheet* pSheet, MouseEventArgs& e)
//{
//	return OnColumnHeaderEndTrack(pSheet, e);		
//}
//CSheetState* CTracker::OnTrackLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
//{
//	return CSheetState::Normal();
//}
//CSheetState* CTracker::OnTrackRButtonDown(CSheet* pSheet, MouseEventArgs& e)
//{
//	return CSheetState::Normal();
//}
//CSheetState* CTracker::OnTrackMouseMove(CSheet* pSheet, MouseEventArgs& e)
//{
//	return OnColumnHeaderTrack(pSheet, e);
//}
//CSheetState* CTracker::OnTrackMouseLeave(CSheet* pSheet, MouseEventArgs& e)
//{
//	return OnColumnHeaderTrack(pSheet, e);
//}
//CSheetState* CTracker::OnTrackSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
//{
//	return CSheetState::Tracking();
//}