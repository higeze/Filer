#include "Sheet.h"
#include "SheetEventArgs.h"
#include "CellProperty.h"
//
#include "MyRect.h"
#include "MyDC.h"
#include "MyFont.h"
#include "MyPen.h"
#include "Cell.h"
#include "Row.h"
#include "Column.h"
#include "RowColumn.h"
#include "ConsoleTimer.h"
#include "ApplicationProperty.h"
#include "Tracker.h"
#include "Dragger.h"
#include "Cursorer.h"
#include "Celler.h"
#include "SheetStateMachine.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CMenu CSheet::ContextMenu;

CSheet::CSheet(
	std::shared_ptr<CCellProperty> spHeaderProperty,
	std::shared_ptr<CCellProperty> spFilterProperty,
	std::shared_ptr<CCellProperty> spCellProperty,
	CMenu* pContextMenu)
	:m_spHeaderProperty(spHeaderProperty),
	m_spFilterProperty(spFilterProperty),
	m_spCellProperty(spCellProperty),
	m_spStateMachine(std::make_shared<CSheetStateMachine>()),
	m_bSelected(false),
	m_bFocused(false),
	m_pContextMenu(pContextMenu?pContextMenu:&CSheet::ContextMenu)
{

	if(m_spRowTracker.get()==nullptr){
		m_spRowTracker = std::make_shared<CTracker<RowTag>>();
	}
	if (m_spColTracker.get() == nullptr) {
		m_spColTracker = std::make_shared<CTracker<ColTag>>();
	}
	if(m_spRowDragger.get()==nullptr){
		m_spRowDragger = std::make_shared<CDragger<RowTag, ColTag>>();
	}
	if (m_spColDragger.get() == nullptr) {
		m_spColDragger = std::make_shared<CDragger<ColTag, RowTag>>();
	}
	if(m_spCursorer.get()==nullptr){
		m_spCursorer = std::make_shared<CCursorer>();
	}
	if (m_spCeller.get() == nullptr) {
		m_spCeller = std::make_shared<CCeller>();
	}


}

std::shared_ptr<CCell>& CSheet::Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn)
{
	return spColumn->Cell(spRow.get());
}

std::shared_ptr<CCell>& CSheet::Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow)
{
	return spColumn->Cell(spRow.get());
}

std::shared_ptr<CCell>& CSheet::Cell( CRow* pRow,  CColumn* pColumn)
{
	return pColumn->Cell(pRow);
}

std::shared_ptr<CCell> CSheet::Cell(const CPoint& pt)
{
	auto rowPtr = Coordinate2Pointer<RowTag>(pt.y);
	auto colPtr = Coordinate2Pointer<ColTag>(pt.x);
	if (rowPtr.get() != nullptr && colPtr.get() != nullptr){
		return CSheet::Cell(rowPtr, colPtr);
	}
	else {
		return nullptr;
	}
}


void CSheet::SetAllRowMeasureValid(bool valid)
{
	for (auto const & rowData : m_rowAllDictionary) {
		rowData.DataPtr->SetMeasureValid(valid);
	}
}

void CSheet::SetAllColumnMeasureValid(bool valid)
{
	for (auto const & colData : m_columnAllDictionary) {
		colData.DataPtr->SetMeasureValid(valid);
	}
}

void CSheet::SetColumnAllCellMeasureValid(CColumn* pColumn, bool valid)
{
	for (auto const & rowData : m_rowAllDictionary) {
		CSheet::Cell(rowData.DataPtr.get(), pColumn)->SetActMeasureValid(false);
	}
}

void CSheet::CellSizeChanged(CellEventArgs& e)
{
	e.CellPtr->GetColumnPtr()->SetMeasureValid(false);
	e.CellPtr->GetRowPtr()->SetMeasureValid(false);
	e.CellPtr->SetFitMeasureValid(false);
	e.CellPtr->SetActMeasureValid(false);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}
void CSheet::CellValueChanged(CellEventArgs& e)
{
//	e.CellPtr->GetColumnPtr()->SetMeasureValid(false);//TODO value change doesnt change width
	e.CellPtr->GetRowPtr()->SetMeasureValid(false);
	e.CellPtr->SetFitMeasureValid(false);
	e.CellPtr->SetActMeasureValid(false);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CSheet::ColumnInserted(CColumnEventArgs& e)
{
	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::ColumnErased(CColumnEventArgs& e)
{
	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}
void CSheet::ColumnHeaderTrack(CColumnEventArgs& e)
{
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//	
}
void CSheet::ColumnHeaderEndTrack(CColumnEventArgs& e)
{
	boost::for_each(m_rowAllDictionary,[&](const RowData& rowData){
		e.m_pColumn->Cell(rowData.DataPtr.get())->SetActMeasureValid(false);
	});

	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::ColumnHeaderFitWidth(CColumnEventArgs& e)
{
	e.m_pColumn->SetMeasureValid(false);
	boost::for_each(m_rowAllDictionary,[&](const RowData& rowData){
		e.m_pColumn->Cell(rowData.DataPtr.get())->SetActMeasureValid(false);
	});
	this->SetAllColumnMeasureValid(false);
	
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::RowInserted(CRowEventArgs& e)
{
	//boost::for_each(m_columnAllDictionary,[&](const ColumnData& colData){
	//	colData.DataPtr->SetMeasureValid(false);
	//});
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::RowErased(CRowEventArgs& e)
{
	this->SetAllColumnMeasureValid(false);

	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::RowsErased(CRowsEventArgs& e)
{
	//Remeasure column width irritate user, therefore Column measure doesn't run.
	//boost::for_each(m_columnAllDictionary,[&](const ColumnData& colData){
	//	colData.DataPtr->SetMeasureValid(false);
	//});
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::Sorted()
{
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Invalidate);
}
void CSheet::SizeChanged()
{
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}
void CSheet::Scroll()
{
	PostUpdate(Updates::Invalidate);
}

void CSheet::SubmitUpdate()
{
	boost::for_each(m_setUpdate,[&](const Updates type){
		switch(type){
		case Updates::RowVisible:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, "UpdateRowVisibleDictionary")
					UpdateRowVisibleDictionary();
			}
			break;
		case Updates::ColumnVisible:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, "UpdateColumnVisibleDictionary")
					UpdateColumnVisibleDictionary();
				break;
			}
		case Updates::Column:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, "UpdateColumn")
					UpdateColumn();
				break;
			}
		case Updates::Row:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, "UpdateRow")	
					UpdateRow();
				break;
			}
		case Updates::Scrolls:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, "UpdateScrolls")
					UpdateScrolls();
				break;
			}
		case Updates::Invalidate:
			Invalidate();
		default:
			break;
		}
	});
	m_setUpdate.clear();
}

void CSheet::PostUpdate(Updates type)
{
	if(type==Updates::All){
		m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::ColumnVisible);
		m_setUpdate.insert(Updates::Column);
		m_setUpdate.insert(Updates::Row);
		m_setUpdate.insert(Updates::Scrolls);
		m_setUpdate.insert(Updates::Invalidate);
	}else{
		m_setUpdate.insert(type);
	}
}

CSheet::string_type CSheet::GetSheetString()const
{
	string_type str;

	boost::range::for_each(m_rowVisibleDictionary,[&](const RowData& rowData){
		bool bFirstLine=true;
		boost::range::for_each(this->m_columnVisibleDictionary,[&](const ColumnData& colData){
			auto pCell=colData.DataPtr->Cell(rowData.DataPtr.get());
			if(bFirstLine){
				bFirstLine=false;
			}else{
				str.append(L"\t");
			}
			str.append(pCell->GetString());
		});
		str.append(L"\r\n");
	});

	return str;
}

void CSheet::UpdateRowVisibleDictionary()
{
	//OnCursorClear(EventArgs(nullptr));
	UpdateVisibleDictionary(m_rowAllDictionary,m_rowVisibleDictionary);
}

void CSheet::UpdateColumnVisibleDictionary()
{
	//OnCursorClear(EventArgs(nullptr));
	UpdateVisibleDictionary(m_columnAllDictionary,m_columnVisibleDictionary);
}

void CSheet::UpdateRowPaintDictionary()
{
	CRect rcClip(GetPaintRect());
	CPoint ptOrigin(GetOriginPoint());
	rcClip.left = ptOrigin.x;
	rcClip.top = ptOrigin.y;
	UpdatePaintDictionary(m_rowVisibleDictionary,m_rowPaintDictionary,rcClip.top,rcClip.bottom,
		[](const coordinates_type& x,const RowData& data)->bool{return x<data.DataPtr->GetTop();},
		[](const RowData& data,const coordinates_type& x)->bool{return x>data.DataPtr->GetBottom();});
}

void CSheet::UpdateColumnPaintDictionary()
{
	CRect rcClip(GetPaintRect());
	CPoint ptOrigin(GetOriginPoint());
	rcClip.left = ptOrigin.x;
	rcClip.top = ptOrigin.y;
	UpdatePaintDictionary(m_columnVisibleDictionary,m_columnPaintDictionary,rcClip.left,rcClip.right,
		[](const coordinates_type& x,const ColumnData& data)->bool{return x<data.DataPtr->GetLeft();},
		[](const ColumnData& data,const coordinates_type& x)->bool{return x>data.DataPtr->GetRight();});
}

void CSheet::ResetColumnSort()
{
	//Reset Sort Mark
	for(auto iter=m_columnAllDictionary.begin(),end=m_columnAllDictionary.end();iter!=end;++iter){
		iter->DataPtr->SetSort(Sorts::None);
	}
}

void CSheet::Sort(CColumn* pCol, Sorts sort)
{
	auto& rowDictionary=m_rowAllDictionary.get<IndexTag>();
	std::vector<RowData> vRowMinusData,vRowPlusData;
	//Copy all to vector
	std::copy(rowDictionary.begin(),rowDictionary.find(0),std::back_inserter(vRowMinusData));
	std::copy(rowDictionary.find(0),rowDictionary.end(),std::back_inserter(vRowPlusData));
	//Sort
	switch(sort){
	case Sorts::Down:
		std::stable_sort(vRowPlusData.begin(),vRowPlusData.end(),[pCol](const RowData& lhs,const RowData& rhs)->bool{
			return pCol->Cell(lhs.DataPtr.get())->operator<(*(pCol->Cell(rhs.DataPtr.get())));
		});
		break;
	case Sorts::Up:
		std::stable_sort(vRowPlusData.begin(),vRowPlusData.end(),[pCol](const RowData& lhs,const RowData& rhs)->bool{
			return pCol->Cell(lhs.DataPtr.get())->operator>(*(pCol->Cell(rhs.DataPtr.get())));
		});
		break;
	default:
		break;
	}

	//Copy again
	m_rowAllDictionary.clear();
	std::copy(vRowMinusData.begin(),vRowMinusData.end(),std::inserter(m_rowAllDictionary,m_rowAllDictionary.begin()));
	for(auto begin=vRowPlusData.begin(),iter=vRowPlusData.begin(),end=vRowPlusData.end();iter!=end;++iter){
		iter->Index=iter-begin;
		m_rowAllDictionary.insert(*iter);
	}
	//Update
	Sorted();
}


void CSheet::Filter(size_type colDisp,std::function<bool(const cell_type&)> predicate)
{
	auto& colDictionary=m_columnAllDictionary.get<IndexTag>();
	auto colIter=colDictionary.find(colDisp);
	auto& rowDictionary=m_rowAllDictionary.get<IndexTag>();

	for(auto rowIter=rowDictionary.find(0),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
		if(predicate(colIter->DataPtr->Cell(rowIter->DataPtr.get()))){
			rowIter->DataPtr->SetVisible(true);
		}else{
			rowIter->DataPtr->SetVisible(false);		
		}
	}
}

void CSheet::ResetFilter()
{
	auto& rowDictionary=m_rowAllDictionary.get<IndexTag>();
	for(auto rowIter=rowDictionary.find(0),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
		rowIter->DataPtr->SetVisible(true);
	}

	auto& colDictionary=m_columnAllDictionary.get<IndexTag>();
	for(auto colIter=colDictionary.find(0),colEnd=colDictionary.end();colIter!=colEnd;++colIter){
		colIter->DataPtr->SetVisible(true);		
	}
}

//void CSheet::MoveColumnImpl(size_type colTo, column_type spFromColumn)
//{ 
//	int from = spFromColumn->GetIndex<AllTag>();
//	int to = colTo;
//	if(from>=0 && to>=0  && to>from){
//		to--;
//	}
//	if(from<0 && to<0 && to<from){
//		to++;
//	}
//
//	Erase(m_columnAllDictionary, spFromColumn.get());
//	InsertLeft(m_columnAllDictionary,to,spFromColumn);
//	ColumnMoved(CMovedEventArgs<ColTag>(spFromColumn.get(), from, to));
//}

void CSheet::EraseColumnImpl(column_type spColumn)
{
	Erase(m_columnAllDictionary, spColumn.get());
	m_spCursorer->OnCursorClear(this);
	ColumnErased(CColumnEventArgs(spColumn.get()));
	DEBUG_OUTPUT_COLUMN_ALL_DICTIONARY
}

void CSheet::EraseRow(CRow* pRow)
{
	Erase(m_rowAllDictionary, pRow);
	m_spCursorer->OnCursorClear(this);
	RowErased(CRowEventArgs(pRow));
}

void CSheet::EraseRows(const std::vector<CRow*>& vpRow)
{
	EraseSome(m_rowAllDictionary, vpRow);
	m_spCursorer->OnCursorClear(this);
	RowsErased(CRowsEventArgs(vpRow));
}

void CSheet::InsertColumn(size_type allIndex, column_type pColumn, bool notify)
{
	pColumn->InsertNecessaryRows();
	InsertImpl<ColTag, AllTag>(allIndex,pColumn);
	if(notify){
		ColumnInserted(CColumnEventArgs(pColumn.get()));
	}
}

void CSheet::InsertRow(size_type allIndex, row_type pRow, bool notify)
{
	InsertImpl<RowTag, AllTag>(allIndex, pRow);
	if(notify){
		RowInserted(CRowEventArgs(pRow.get()));
	}
}

CSheet::coordinates_type CSheet::GetColumnInitWidth(CColumn* pColumn)
{
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
	coordinates_type maxWidth=0;
	CDC dc(::CreateCompatibleDC(*GetClientDCPtr().get()));	
	boost::for_each(rowDictionary, [&](const RowData& rowData){
		auto pCell=pColumn->Cell(rowData.DataPtr.get());
		maxWidth= (std::max)(pCell->GetInitSize(&dc).cx, (LONG)maxWidth);
	});
	return maxWidth;
}

CSheet::coordinates_type CSheet::GetColumnFitWidth(CColumn* pColumn)
{
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
	coordinates_type maxWidth=0;
	CDC dc(::CreateCompatibleDC(*GetClientDCPtr().get()));	
	boost::for_each(rowDictionary, [&](const RowData& rowData){
		auto pCell=pColumn->Cell(rowData.DataPtr.get());
		maxWidth= (std::max)(pCell->GetFitSize(&dc).cx, (LONG)maxWidth);
	});
	return maxWidth;
}

CSheet::coordinates_type CSheet::GetRowHeight(CRow* pRow)
{
	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	coordinates_type maxHeight=0;
	CDC dc(::CreateCompatibleDC(*GetClientDCPtr().get()));	
	boost::for_each(colDictionary,[&](const ColumnData& colData){
		auto pCell=colData.DataPtr->Cell(pRow);
		maxHeight= (std::max)(pCell->GetActSize(&dc).cy, (LONG)maxHeight);
	});
	return maxHeight;
}


void CSheet::UpdateRow()
{
	if(!Visible()){return;}

	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
	CPoint ptScroll=GetScrollPos();
	coordinates_type top=m_spCellProperty->GetPenPtr()->GetLeftTopPenWidth();
	for(auto iter = rowDictionary.begin(),end=rowDictionary.end();iter!=end;++iter){
		iter->DataPtr->SetTopWithoutSignal(top);
		top+=iter->DataPtr->GetHeight();
	}
}

void CSheet::UpdateColumn()
{
	if(!Visible()){return;}

	auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();
	coordinates_type left= m_spCellProperty->GetPenPtr()->GetLeftTopPenWidth();
	for(auto iter = colDictionary.begin(),end=colDictionary.end();iter!=end;++iter){
		iter->DataPtr->SetSheetLeftWithoutSignal(left);
		left+=iter->DataPtr->GetWidth();
	}
}

bool CSheet::Empty()const
{
	return m_rowAllDictionary.empty() || m_columnAllDictionary.empty();
}
bool CSheet::Visible()const
{
	return (!m_rowVisibleDictionary.empty()) && (!m_columnVisibleDictionary.empty());
}

CPoint CSheet::GetOriginPoint()
{
	if(!Visible()){
		MessageBox(NULL,L"CSheet::GetOriginPoint",NULL,0);
		throw std::exception("CSheet::GetOriginPoint");	
	}

	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();

	auto colMinusIter=colDictionary.find(-1);
	auto colPlusIter=colDictionary.find(0);
	auto rowMinusIter=rowDictionary.find(-1);
	auto rowPlusIter=rowDictionary.find(0);
	coordinates_type x=0;
	coordinates_type y=0;
	if(colMinusIter!=colDictionary.end()){
		x=colMinusIter->DataPtr->GetRight();
	}else{
		x=colPlusIter->DataPtr->GetLeft();
	}
	if(rowMinusIter!=rowDictionary.end()){
		y=rowMinusIter->DataPtr->GetBottom();
	}else{
		y=rowPlusIter->DataPtr->GetTop();
	}

	return CPoint(x,y);
}


CSize CSheet::MeasureSize()const
{
	CRect rc(CSheet::GetRect());
	return CSize(rc.Width(), rc.Height());
}

//GetRect return Sheet(All) Rect in Window Client
//GetCellsRect return Cells(=Scrollable Cells) Rect in Window Client
//GetCellsClipRect return Cells(=Scrollable Cells) Clip(=Paint area) Rect in Window Client
CRect CSheet::GetRect()const
{
	if(!Visible()){return CRect();}

	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();

	coordinates_type left=colDictionary.begin()->DataPtr->GetLeft();
	coordinates_type top=rowDictionary.begin()->DataPtr->GetTop();
	coordinates_type right=boost::prior(colDictionary.end())->DataPtr->GetRight();
	coordinates_type bottom=boost::prior(rowDictionary.end())->DataPtr->GetBottom();

	CRect rc(left, top, right, bottom);

	auto outerPenWidth = m_spCellProperty->GetPenPtr()->GetLeftTopPenWidth();
	rc.InflateRect(outerPenWidth, outerPenWidth);
	return rc;
}


CRect CSheet::GetCellsRect()
{
	CPoint ptScroll(GetScrollPos());
	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();

	if(Size<RowTag, VisTag>()==0 ||Size<ColTag, VisTag>()==0 || GetMaxIndex<RowTag, VisTag>()<0 || GetMaxIndex<ColTag, VisTag>()<0 ){
		return CRect(0,0,0,0);
	}

	coordinates_type left=colDictionary.find(0)->DataPtr->GetLeft();
	coordinates_type top=rowDictionary.find(0)->DataPtr->GetTop();
	coordinates_type right=boost::prior(colDictionary.end())->DataPtr->GetRight();
	coordinates_type bottom=boost::prior(rowDictionary.end())->DataPtr->GetBottom();

	return CRect(left,top,right,bottom);
}

//CRect CSheet::GetCellsClipRect()
//{
//	//Calculate Origin
//	CPoint ptOrigin(GetOriginPoint());
//	CRect rcClip(GetRect());
//
//	rcClip.left=ptOrigin.x;
//	rcClip.top=ptOrigin.y;
//	//CRect rcCells(GetCellsRect());
//	//CRect rcClip;
//	//
//	//rcClip.IntersectRect(rcCells,rcClient);
//
//	return rcClip;
//}
void CSheet::OnPaintAll(PaintEventArgs& e)
{
	DEBUG_OUTPUT_COLUMN_VISIBLE_DICTIONARY

	if(!Visible())return;

	//Paint
	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
	for(auto colIter=colDictionary.rbegin(),colEnd=colDictionary.rend();colIter!=colEnd;++colIter){
		for(auto rowIter=rowDictionary.rbegin(),rowEnd=rowDictionary.rend();rowIter!=rowEnd;++rowIter){
			colIter->DataPtr->Cell(rowIter->DataPtr.get())->OnPaint(e);
		}
	}
}
void CSheet::OnPaint(PaintEventArgs& e)
{

	DEBUG_OUTPUT_COLUMN_VISIBLE_DICTIONARY

	if(!Visible())return;
	//Update Paint Dictioanary
	UpdateRowPaintDictionary();
	UpdateColumnPaintDictionary();

	DEBUG_OUTPUT_COLUMN_PAINT_DICTIONARY


		//Paint
		//e.DCPtr->FillRect(GetCellsClipRect(),*(m_spCellProperty->GetBackgroundBrushPtr()));

	auto& colDictionary=m_columnPaintDictionary.get<IndexTag>();
	auto& rowDictionary=m_rowPaintDictionary.get<IndexTag>();
	for(auto colIter=colDictionary.rbegin(),colEnd=colDictionary.rend();colIter!=colEnd;++colIter){
		for(auto rowIter=rowDictionary.rbegin(),rowEnd=rowDictionary.rend();rowIter!=rowEnd;++rowIter){
			colIter->DataPtr->Cell(rowIter->DataPtr.get())->OnPaint(e);
		}
	}
}

void CSheet::UpdateAll()
{
	UpdateRowVisibleDictionary();//TODO today
	UpdateColumnVisibleDictionary();
	//	UpdateFitHeight();
	//	UpdateFitWidth();
	UpdateColumn();
	UpdateRow();

}

void CSheet::OnContextMenu(ContextMenuEventArgs& e)
{
	m_spStateMachine->ContextMenu(this, e);
}

void CSheet::OnRButtonDown(MouseEventArgs& e)
{
	m_spStateMachine->RButtonDown(this, e);
}

void CSheet::OnLButtonDown(MouseEventArgs& e)
{
	m_spStateMachine->LButtonDown(this, e);
}

void CSheet::OnLButtonUp(MouseEventArgs& e)
{
	m_spStateMachine->LButtonUp(this, e);
}

void CSheet::OnLButtonSnglClk(MouseEventArgs& e)
{
	m_spStateMachine->LButtonSnglClk(this, e);
}

void CSheet::OnLButtonDblClk(MouseEventArgs& e)
{
	m_spStateMachine->LButtonDblClk(this, e);
}

void CSheet::OnLButtonBeginDrag(MouseEventArgs& e)
{
	m_spStateMachine->LButtonBeginDrag(this, e);
}

void CSheet::OnLButtonEndDrag(MouseEventArgs& e)
{
	m_spStateMachine->LButtonEndDrag(this, e);
}

void CSheet::OnMouseMove(MouseEventArgs& e)
{
	m_spStateMachine->MouseMove(this, e);
}
void CSheet::OnSetCursor(SetCursorEventArgs& e)
{
	m_spStateMachine->SetCursor(this, e);
}

void CSheet::OnMouseLeave(MouseEventArgs& e)
{
	m_spStateMachine->MouseLeave(this, e);
}

void CSheet::OnSetFocus(EventArgs& e)
{
	if(!Visible())return;
	m_spCursorer->OnCursorClear(this);
	m_bFocused = true;
}

void CSheet::OnKillFocus(EventArgs& e)
{
	if(!Visible())return;
	m_spCursorer->OnCursorClear(this);
	m_bFocused = false;
}

void CSheet::OnKeyDown(KeyEventArgs& e)
{
	m_spStateMachine->KeyDown(this, e);
}


void CSheet::SelectRange(std::shared_ptr<CCell>& cell1, std::shared_ptr<CCell>& cell2, bool doSelect)
{
	if (cell1 || cell2) { return; }

	auto& rowDict=m_rowVisibleDictionary.get<IndexTag>();
	auto& colDict=m_columnVisibleDictionary.get<IndexTag>();

	auto	rowBeg= (std::min)(cell1->GetRowPtr()->GetIndex<VisTag>(), cell2->GetRowPtr()->GetIndex<VisTag>());
	auto	rowLast= (std::max)(cell1->GetRowPtr()->GetIndex<VisTag>(), cell2->GetRowPtr()->GetIndex<VisTag>());
	auto	colBeg= (std::min)(cell1->GetColumnPtr()->GetIndex<VisTag>(), cell2->GetColumnPtr()->GetIndex<VisTag>());
	auto	colLast= (std::max)(cell1->GetColumnPtr()->GetIndex<VisTag>(), cell2->GetColumnPtr()->GetIndex<VisTag>());

	for(auto colIter=colDict.find(colBeg),colEnd=colDict.find(colLast+1);colIter!=colEnd;++colIter){
		for(auto rowIter=rowDict.find(rowBeg),rowEnd=rowDict.find(rowLast+1);rowIter!=rowEnd;++rowIter){
			colIter->DataPtr->Cell(rowIter->DataPtr.get())->SetSelected(doSelect);
		}
	}	
}

void CSheet::SelectAll()
{
	boost::range::for_each(m_columnAllDictionary,[&](const ColumnData& colData){
		colData.DataPtr->SetSelected(true);
	});
	boost::range::for_each(m_rowAllDictionary,[&](const RowData& rowData){
		rowData.DataPtr->SetSelected(true);
	});
}

void CSheet::DeselectAll()
{
	boost::range::for_each(m_columnAllDictionary,[&](const ColumnData& colData){
		boost::range::for_each(m_rowAllDictionary,[&](const RowData& rowData){
			colData.DataPtr->Cell(rowData.DataPtr.get())->SetSelected(false);
		});
	});

	boost::range::for_each(m_columnAllDictionary,[&](const ColumnData& colData){
		colData.DataPtr->SetSelected(false);
	});

	boost::range::for_each(m_rowAllDictionary,[&](const RowData& rowData){
		rowData.DataPtr->SetSelected(false);
	});
}

void CSheet::UnhotAll()
{
	boost::range::for_each(m_columnAllDictionary,[&](const ColumnData& colData){
		boost::range::for_each(m_rowAllDictionary,[&](const RowData& rowData){
			colData.DataPtr->Cell(rowData.DataPtr.get())->SetState(UIElementState::Normal);
		});
	});
}

bool CSheet::IsFocusedCell(const CCell* pCell)const
{
	return m_spCursorer->GetFocusedCell().get() == pCell;
}

bool CSheet::IsDoubleFocusedCell(const CCell* pCell)const
{
	return m_spCursorer->GetDoubleFocusedCell().get() == pCell;
}

Compares CSheet::CheckEqualRange(RowDictionary::iterator rowBegin, RowDictionary::iterator rowEnd,ColumnDictionary::iterator colBegin, ColumnDictionary::iterator colEnd, std::function<void(CCell*, Compares)> action)
{
	Compares comp = Compares::Same;
	//if(m_columnAllDictionary.size()-MinusSize(m_columnAllDictionary)>=2){
		//auto& colDictionary=m_columnAllDictionary.get<IndexTag>();
		for(auto rowIter=rowBegin;rowIter!=rowEnd;++rowIter){
			auto cm = CheckEqualRow(rowIter->DataPtr.get(), colBegin, colEnd, action);
			if(comp == Compares::DiffNE || cm == Compares::DiffNE){
				comp = Compares::DiffNE;
			}else if(comp == Compares::Diff || cm == Compares::Diff){
				comp = Compares::Diff;
			}else{
				comp = Compares::Same;
			}
		}
	//}
	return comp;
}

Compares CSheet::CheckEqualRow(CRow* pRow, ColumnDictionary::iterator colBegin, ColumnDictionary::iterator colEnd, std::function<void(CCell*, Compares)> action)
{
	if(colBegin==colEnd || colBegin->Index>=boost::prior(colEnd)->Index){
		return Compares::Same;
	}
	Compares comp = Compares::Same;
	//auto& colDictionary=m_columnAllDictionary.get<IndexTag>();
	//auto colIter=colDictionary.find(0);
	//auto colEnd=boost::prior(colDictionary.end());
	std::shared_ptr<CCell> pCellComp;
	std::shared_ptr<CCell> pCell, pCellNext;
	for(auto colIter = colBegin;colIter!=boost::prior(colEnd);++colIter){

		pCell=colIter->DataPtr->Cell(pRow);
		pCellNext=boost::next(colIter)->DataPtr->Cell(pRow);

		if(pCell->IsComparable()){
			if(!pCellComp){
				//First Comparable Cell is Not Checked.
				pCell->EqualCell(pCell.get(), action);
				//pCell->SetChecked(false);
			}
			pCellComp=pCell;
		}

		if(pCellNext->IsComparable() && pCellComp){
			//Check Equal		
			auto cm = pCellComp->EqualCell(pCellNext.get(), action);

			/////
			//if(cm==Compares::Same){
			//	pCellNext->SetChecked(false);
			//}else{
			//	pCellNext->SetChecked(true);
			//}


			if(comp == Compares::DiffNE || cm == Compares::DiffNE){
				comp = Compares::DiffNE;
			}else if(comp == Compares::Diff || cm == Compares::Diff){
				comp = Compares::Diff;
			}else{
				comp = Compares::Same;
			}
		}
	}	

	if(pCellNext->IsComparable() && !pCellComp){
		//Final lonely Comparable Cell is Not Checked
		pCellNext->EqualCell(pCellNext.get(), action);
		//pCellNext->SetChecked(false);
	}
	return comp;
}

void CSheet::Clear()
{
	m_setUpdate.clear();

	m_rowAllDictionary.clear();
	m_rowVisibleDictionary.clear();
	m_rowPaintDictionary.clear();

	m_columnAllDictionary.clear();
	m_columnVisibleDictionary.clear();
	m_columnPaintDictionary.clear();

	m_spCursorer->Clear();

	m_rocoContextMenu=CRowColumn();
	m_rocoMouse = CRowColumn();

	//m_coTrackLeftVisib=COLUMN_INDEX_INVALID;

	//m_operation=Operation::None;

	m_pHeaderColumn=column_type();


	m_rowHeader=row_type();


	m_bSelected = false;
	m_bFocused = false;
}




