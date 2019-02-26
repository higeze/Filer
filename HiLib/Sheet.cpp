#include "Sheet.h"
#include "SheetEventArgs.h"
#include "CellProperty.h"
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
#include "GridView.h"

#include <iterator>

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CMenu CSheet::ContextMenu;

CSheet::CSheet(
	std::shared_ptr<HeaderProperty> spHeaderProperty,
	std::shared_ptr<CellProperty> spFilterProperty,
	std::shared_ptr<CellProperty> spCellProperty,
	CMenu* pContextMenu)
	:m_spHeaderProperty(spHeaderProperty),
	m_spFilterProperty(spFilterProperty),
	m_spCellProperty(spCellProperty),
	m_spStateMachine(std::make_shared<CSheetStateMachine>(this)),
	m_bSelected(false),
	m_bFocused(false),
	m_pContextMenu(pContextMenu?pContextMenu:&CSheet::ContextMenu)
{

	if(!m_spRowTracker){
		m_spRowTracker = std::make_shared<CTracker<RowTag>>();
	}
	if (!m_spColTracker) {
		m_spColTracker = std::make_shared<CTracker<ColTag>>();
	}
	if(!m_spRowDragger){
		m_spRowDragger = std::make_shared<CDragger<RowTag, ColTag>>();
	}
	if (!m_spColDragger) {
		m_spColDragger = std::make_shared<CDragger<ColTag, RowTag>>();
	}
	if (!m_spItemDragger) {
		m_spItemDragger = std::make_shared<CNullDragger>();
	}
	if(!m_spCursorer){
		m_spCursorer = std::make_shared<CCursorer>();
	}
	if (!m_spCeller) {
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

std::shared_ptr<CCell> CSheet::Cell(const d2dw::CPointF& pt)
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

void CSheet::OnCellPropertyChanged(CCell* pCell, const wchar_t* name)
{
	if (!_tcsicmp(L"value", name)) {
		PostUpdate(Updates::Sort);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		//PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(L"size", name)) {
		//PostUpdate(Updates::Sort);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	}else if (!_tcsicmp(L"state", name)) {
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(L"focus", name)) {
		PostUpdate(Updates::Invalidate);		
	} else if (!_tcsicmp(L"selected", name)) {
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(L"checked", name)) {
		PostUpdate(Updates::Invalidate);
	}
}

void CSheet::OnRowPropertyChanged(CRow* pRow, const wchar_t* name)
{
	if (_tcsicmp(name, L"selected") == 0) {
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	} else if (_tcsicmp(name, L"selected") == 0) {
		PostUpdate(Updates::Invalidate);//
	}
}

void CSheet::OnColumnPropertyChanged(CColumn* pCol, const wchar_t* name)
{
	if (_tcsicmp(name, L"visible") == 0) {
		for (const auto& rowData : m_rowAllDictionary) {
			rowData.DataPtr->SetMeasureValid(false);
		}
		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	} else if (_tcsicmp(name, L"selected") == 0) {
		PostUpdate(Updates::Invalidate);
	} else if (_tcsicmp(name, L"sort") == 0) {
		PostUpdate(Updates::Sort);
	}
}


void CSheet::ColumnInserted(CColumnEventArgs& e)
{
	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CSheet::ColumnErased(CColumnEventArgs& e)
{
	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CSheet::ColumnHeaderFitWidth(CColumnEventArgs& e)
{
	e.m_pColumn->SetMeasureValid(false);
	for(const auto& rowData : m_rowAllDictionary){
		e.m_pColumn->Cell(rowData.DataPtr.get())->SetActMeasureValid(false);
	}
	this->SetAllColumnMeasureValid(false);
	
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
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
	for(const auto& type : m_setUpdate){
		switch(type){
		case Updates::RowVisible:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"UpdateRowVisibleDictionary")
				UpdateRowVisibleDictionary();
			}
			break;
		case Updates::ColumnVisible:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"UpdateColumnVisibleDictionary")
				UpdateColumnVisibleDictionary();
				break;
			}
		case Updates::Column:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"UpdateColumn")
				UpdateColumn();
				break;
			}
		case Updates::Row:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"UpdateRow")	
				UpdateRow();
				break;
			}
		case Updates::Scrolls:
			{
				CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"UpdateScrolls")
				UpdateScrolls();
				break;
			}
		case Updates::Invalidate:
		{
			CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"UpdateScrolls")
				Invalidate();
		}
		default:
			break;
		}
	}
	m_setUpdate.clear();
}

void CSheet::PostUpdate(Updates type)
{
	switch (type) {
	case Updates::All:
		m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::Row);
		m_setUpdate.insert(Updates::ColumnVisible);
		m_setUpdate.insert(Updates::Column);
		m_setUpdate.insert(Updates::Scrolls);
		m_setUpdate.insert(Updates::Invalidate);
		break;
	case Updates::EnsureVisibleFocusedCell:
		m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::Row);
		m_setUpdate.insert(Updates::ColumnVisible);
		m_setUpdate.insert(Updates::Column);
		m_setUpdate.insert(Updates::Scrolls);
		m_setUpdate.insert(type);
		m_setUpdate.insert(Updates::Invalidate);
		break;
	case Updates::Sort:
		m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::Row);
		m_setUpdate.insert(type);
		m_setUpdate.insert(Updates::Invalidate);
		PostUpdate(Updates::EnsureVisibleFocusedCell);
	case Updates::Rect:
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
		m_setUpdate.insert(type);
	default:
		m_setUpdate.insert(type);
		break;
	}

}

std::wstring CSheet::GetSheetString()const
{
	std::wstring str;

	for(const auto& rowData : m_rowVisibleDictionary){
		bool bFirstLine=true;
		for(const ColumnData& colData : this->m_columnVisibleDictionary){
			auto pCell=colData.DataPtr->Cell(rowData.DataPtr.get());
			if(bFirstLine){
				bFirstLine=false;
			}else{
				str.append(L"\t");
			}
			str.append(pCell->GetString());
		}
		str.append(L"\r\n");
	}
	return str;
}

void CSheet::UpdateRowVisibleDictionary()
{
	UpdateVisibleDictionary(m_rowAllDictionary,m_rowVisibleDictionary);
}

void CSheet::UpdateColumnVisibleDictionary()
{
	UpdateVisibleDictionary(m_columnAllDictionary,m_columnVisibleDictionary);
}

void CSheet::UpdateRowPaintDictionary()
{
	d2dw::CRectF rcClip(GetPaintRect());
	d2dw::CPointF ptOrigin(GetOriginPoint());
	rcClip.left = ptOrigin.x;
	rcClip.top = ptOrigin.y;
	UpdatePaintDictionary(m_rowVisibleDictionary,m_rowPaintDictionary,rcClip.top,rcClip.bottom,
		[](const FLOAT& x,const RowData& data)->bool{return x<data.DataPtr->GetTop();},
		[](const RowData& data,const FLOAT& x)->bool{return x>data.DataPtr->GetBottom();});
}

void CSheet::UpdateColumnPaintDictionary()
{
	d2dw::CRectF rcClip(GetPaintRect());
	d2dw::CPointF ptOrigin(GetOriginPoint());
	rcClip.left = ptOrigin.x;
	rcClip.top = ptOrigin.y;
	UpdatePaintDictionary(m_columnVisibleDictionary,m_columnPaintDictionary,rcClip.left,rcClip.right,
		[](const FLOAT& x,const ColumnData& data)->bool{return x<data.DataPtr->GetLeft();},
		[](const ColumnData& data,const FLOAT& x)->bool{return x>data.DataPtr->GetRight();});
}

void CSheet::ResetColumnSort()
{
	//Reset Sort Mark
	for(auto iter=m_columnAllDictionary.begin(),end=m_columnAllDictionary.end();iter!=end;++iter){
		iter->DataPtr->SetSort(Sorts::None);
	}
}

void CSheet::Sort(CColumn* pCol, Sorts sort, bool postUpdate)
{
	auto& rowDictionary=m_rowAllDictionary.get<IndexTag>();
	std::vector<std::pair<RowData, std::wstring>> vRowMinusData,vRowPlusData;
	//Copy all to vector
	for (auto iter = rowDictionary.begin(); iter != rowDictionary.find(0); ++iter) {
		vRowMinusData.emplace_back(*iter, CSheet::Cell(iter->DataPtr.get(), pCol)->GetSortString());
	}
	for (auto iter = rowDictionary.find(0); iter != rowDictionary.end(); ++iter) {
		vRowPlusData.emplace_back(*iter, CSheet::Cell(iter->DataPtr.get(), pCol)->GetSortString());
	}

	//Sort
	switch(sort){
	case Sorts::Down:
		std::stable_sort(vRowPlusData.begin(),vRowPlusData.end(),[pCol](const auto& lhs,const auto& rhs)->bool{
			return _tcsicmp(lhs.second.c_str(), rhs.second.c_str())>0;
		});
		break;
	case Sorts::Up:
		std::stable_sort(vRowPlusData.begin(), vRowPlusData.end(), [pCol](const auto& lhs, const auto& rhs)->bool {
			return _tcsicmp(lhs.second.c_str(), rhs.second.c_str())<0;
		});
		break;
	default:
		break;
	}

	//Copy again
	m_rowAllDictionary.clear();
	for (auto begin = vRowMinusData.begin(), iter = vRowMinusData.begin(), end = vRowMinusData.end(); iter != end; ++iter) {
		m_rowAllDictionary.insert(iter->first);
	}

	for(auto begin=vRowPlusData.begin(),iter=vRowPlusData.begin(),end=vRowPlusData.end();iter!=end;++iter){
		iter->first.Index=std::distance(begin, iter);
		m_rowAllDictionary.insert(iter->first);
	}
}

void CSheet::Filter(int colDisp,std::function<bool(const std::shared_ptr<CCell>&)> predicate)
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

void CSheet::EraseColumnImpl(std::shared_ptr<CColumn> spColumn)
{
	Erase(m_columnAllDictionary, spColumn.get());
	m_spCursorer->OnCursorClear(this);
	ColumnErased(CColumnEventArgs(spColumn.get()));
	DEBUG_OUTPUT_COLUMN_ALL_DICTIONARY
}

void CSheet::EraseRow(CRow* pRow)
{
	EraseRowNotify(pRow, true);
}

void CSheet::EraseRowNotify(CRow* pRow, bool notify)
{
	Erase(m_rowAllDictionary, pRow);
	m_spCursorer->OnCursorClear(this);
	if (notify) {
		RowErased(CRowEventArgs(pRow));
	}
}

void CSheet::EraseRows(const std::vector<CRow*>& vpRow)
{
	EraseSome(m_rowAllDictionary, vpRow);
	m_spCursorer->OnCursorClear(this);
	RowsErased(CRowsEventArgs(vpRow));
}

void CSheet::InsertColumn(int allIndex, std::shared_ptr<CColumn> pColumn)
{
	InsertColumnNotify(allIndex, pColumn);
}

void CSheet::InsertColumnNotify(int allIndex, std::shared_ptr<CColumn> pColumn, bool notify)
{
	pColumn->InsertNecessaryRows();
	InsertImpl<ColTag, AllTag>(allIndex, pColumn);
	if (notify) {
		ColumnInserted(CColumnEventArgs(pColumn.get()));
	}
}

void CSheet::InsertRow(int allIndex, std::shared_ptr<CRow> pRow)
{
	InsertRowNotify(allIndex, pRow);
}

void CSheet::InsertRowNotify(int allIndex, std::shared_ptr<CRow> pRow, bool notify)
{
	InsertImpl<RowTag, AllTag>(allIndex, pRow);
	if(notify){
		RowInserted(CRowEventArgs(pRow.get()));
	}
}

FLOAT CSheet::GetColumnInitWidth(CColumn* pColumn)
{
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
	FLOAT maxWidth=0;
	for(const auto& rowData : rowDictionary){
		auto pCell=pColumn->Cell(rowData.DataPtr.get());
		maxWidth= (std::max)(pCell->GetInitSize(*(GetGridPtr()->GetDirect())).width, maxWidth);
	};
	return maxWidth;
}

FLOAT CSheet::GetColumnFitWidth(CColumn* pColumn)
{
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
	FLOAT maxWidth=0;
	//CDC dc(::CreateCompatibleDC(*GetClientDCPtr().get()));	
	for(const auto& rowData : rowDictionary){
		auto pCell=pColumn->Cell(rowData.DataPtr.get());
		maxWidth= (std::max)(pCell->GetFitSize(*(GetGridPtr()->GetDirect())).width, maxWidth);
	};
	return maxWidth;
}

FLOAT CSheet::GetRowHeight(CRow* pRow)
{
	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	FLOAT maxHeight=0;
	for(const auto& colData : colDictionary){
		auto pCell=colData.DataPtr->Cell(pRow);
		maxHeight= (std::max)(pCell->GetActSize(*(GetGridPtr()->GetDirect())).height, maxHeight);
	};
	return maxHeight;
}

void CSheet::UpdateColumn()
{
	if(!Visible()){return;}

	auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();
	FLOAT left= m_spCellProperty->Line->Width/2.0f;
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

d2dw::CPointF CSheet::GetOriginPoint()
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
	FLOAT x=0;
	FLOAT y=0;
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

	return d2dw::CPointF(x,y);
}


d2dw::CSizeF CSheet::MeasureSize()const
{
	d2dw::CRectF rc(CSheet::GetRect());
	return d2dw::CSizeF(rc.Width(), rc.Height());
}

//GetRect return Sheet(All) Rect in Window Client
//GetCellsRect return Cells(=Scrollable Cells) Rect in Window Client
//GetCellsClipRect return Cells(=Scrollable Cells) Clip(=Paint area) Rect in Window Client
d2dw::CRectF CSheet::GetRect()const
{
	if(!Visible()){return d2dw::CRectF();}

	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();

	FLOAT left=colDictionary.begin()->DataPtr->GetLeft();
	FLOAT top=rowDictionary.begin()->DataPtr->GetTop();
	FLOAT right=boost::prior(colDictionary.end())->DataPtr->GetRight();
	FLOAT bottom=boost::prior(rowDictionary.end())->DataPtr->GetBottom();

	d2dw::CRectF rc(left, top, right, bottom);

	auto outerPenWidth = m_spCellProperty->Line->Width/2.0f;
	rc.InflateRect(outerPenWidth, outerPenWidth);
	return rc;
}

FLOAT CSheet::GetCellsHeight()
{
	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();

	if (Size<RowTag, VisTag>() == 0 || GetMaxIndex<RowTag, VisTag>() < 0) {
		return 0.0f;
	}

	FLOAT top = rowDictionary.find(0)->DataPtr->GetTop();
	FLOAT bottom = boost::prior(rowDictionary.end())->DataPtr->GetBottom();

	return bottom - top;

}

FLOAT CSheet::GetCellsWidth()
{
	auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();

	if (Size<ColTag, VisTag>() == 0 || GetMaxIndex<ColTag, VisTag>() < 0) {
		return 0.0f;
	}

	FLOAT left = colDictionary.find(0)->DataPtr->GetLeft();
	FLOAT right = boost::prior(colDictionary.end())->DataPtr->GetRight();

	return right - left;
}



d2dw::CRectF CSheet::GetCellsRect()
{
	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();

	if(Size<RowTag, VisTag>()==0 ||Size<ColTag, VisTag>()==0 || GetMaxIndex<RowTag, VisTag>()<0 || GetMaxIndex<ColTag, VisTag>()<0 ){
		return d2dw::CRectF(0,0,0,0);
	}

	FLOAT left=colDictionary.find(0)->DataPtr->GetLeft();
	FLOAT top=rowDictionary.find(0)->DataPtr->GetTop();
	FLOAT right=boost::prior(colDictionary.end())->DataPtr->GetRight();
	FLOAT bottom=boost::prior(rowDictionary.end())->DataPtr->GetBottom();

	return d2dw::CRectF(left,top,right,bottom);
}

void CSheet::OnPaintAll(const PaintEvent& e)
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
void CSheet::OnPaint(const PaintEvent& e)
{

	DEBUG_OUTPUT_COLUMN_VISIBLE_DICTIONARY

	if(!Visible())return;
	//Update Paint Dictioanary
	UpdateRowPaintDictionary();
	UpdateColumnPaintDictionary();

	DEBUG_OUTPUT_COLUMN_PAINT_DICTIONARY


	//Paint

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
	UpdateColumn();
	UpdateRow();
}

void CSheet::OnContextMenu(const ContextMenuEvent& e)
{
	m_spStateMachine->ContextMenu(e);
}

void CSheet::OnRButtonDown(const RButtonDownEvent& e)
{
	m_spStateMachine->RButtonDown(e);
}

void CSheet::OnLButtonDown(const LButtonDownEvent& e)
{
	m_spStateMachine->LButtonDown(e);
}

void CSheet::OnLButtonUp(const LButtonUpEvent& e)
{
	m_spStateMachine->LButtonUp(e);
}

void CSheet::OnLButtonClk(const LButtonClkEvent& e)
{
	m_spStateMachine->LButtonClk(e);
}


void CSheet::OnLButtonSnglClk(const LButtonSnglClkEvent& e)
{
	m_spStateMachine->LButtonSnglClk(e);
}

void CSheet::OnLButtonDblClk(const LButtonDblClkEvent& e)
{
	m_spStateMachine->LButtonDblClk(e);
}

void CSheet::OnLButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	m_spStateMachine->LButtonBeginDrag(e);
}

void CSheet::OnMouseMove(const MouseMoveEvent& e)
{
	m_spStateMachine->MouseMove(e);
}
void CSheet::OnSetCursor(const SetCursorEvent& e)
{
	m_spStateMachine->SetCursor(e);
}

void CSheet::OnMouseLeave(const MouseLeaveEvent& e)
{
	m_spStateMachine->MouseLeave(e);
}

void CSheet::OnSetFocus(const SetFocusEvent& e)
{
	if(!Visible())return;
//	m_spCursorer->OnCursorClear(this);
	m_bFocused = true;
}

void CSheet::OnKillFocus(const KillFocusEvent& e)
{
	if(!Visible())return;
	m_spCursorer->OnCursorClear(this);
	m_bFocused = false;
}

void CSheet::OnKeyDown(const KeyDownEvent& e)
{
	m_spStateMachine->KeyDown(e);
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
	for (auto iter = m_rowAllDictionary.find(0); iter != m_rowAllDictionary.end(); ++iter) {
		iter->DataPtr->SetSelected(true);
	}
}

void CSheet::DeselectAll()
{
	for(const auto& colData : m_columnAllDictionary){
		for(const auto rowData : m_rowAllDictionary){
			colData.DataPtr->Cell(rowData.DataPtr.get())->SetSelected(false);
		}
	}

	for (const auto& colData : m_columnAllDictionary) {
		colData.DataPtr->SetSelected(false);
	}

	for (const auto rowData : m_rowAllDictionary) {
		rowData.DataPtr->SetSelected(false);
	}
}

void CSheet::UnhotAll()
{
	for (const auto& colData : m_columnAllDictionary) {
		for (const auto rowData : m_rowAllDictionary) {
			colData.DataPtr->Cell(rowData.DataPtr.get())->SetState(UIElementState::Normal);
		}
	}
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
	//m_rocoMouse = CRowColumn();

	//m_coTrackLeftVisib=COLUMN_INDEX_INVALID;

	//m_operation=Operation::None;

	m_pHeaderColumn=std::shared_ptr<CColumn>();


	m_rowHeader=std::shared_ptr<CRow>();


	m_bSelected = false;
	m_bFocused = false;
}




