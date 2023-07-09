#include "Sheet.h"
#include <numeric>
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
#include "Debug.h"
#include "ApplicationProperty.h"
#include "Tracker.h"
#include "Dragger.h"
#include "Cursorer.h"
#include "Celler.h"
#include "SheetStateMachine.h"
#include "GridView.h"
#include "D2DWWindow.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CMenu CSheet::ContextMenu;

CSheet::CSheet(
	CD2DWControl* pParentControl,
	std::shared_ptr<SheetProperty> spSheetProperty,
	CMenu* pContextMenu)
	:CD2DWControl(pParentControl),
	m_spSheetProperty(spSheetProperty),
	m_bSelected(false),
	//m_bFocused(false),
	m_pContextMenu(pContextMenu?pContextMenu:&CSheet::ContextMenu),
	m_pMachine(new CSheetStateMachine(this)),
	m_allRows([](std::shared_ptr<CRow>& sp, size_t idx) { sp->SetIndex<AllTag>(idx); }),
	m_visRows([](std::shared_ptr<CRow>& sp, size_t idx) { sp->SetIndex<VisTag>(idx); }),
	m_pntRows([](std::shared_ptr<CRow>& sp, size_t idx) { /* Do Nothing */ }),
	m_allCols([](std::shared_ptr<CColumn>& sp, size_t idx) { sp->SetIndex<AllTag>(idx); }),
	m_visCols([](std::shared_ptr<CColumn>& sp, size_t idx) { sp->SetIndex<VisTag>(idx); }),
	m_pntCols([](std::shared_ptr<CColumn>& sp, size_t idx) { /* Do Nothing */ }),
	m_bindType(BindType::Row)
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

CSheet::~CSheet() = default;

//std::shared_ptr<CCell>& CSheet::Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn)
//{
//	return spColumn->Cell(spRow.get());
//}
//
//std::shared_ptr<CCell>& CSheet::Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow)
//{
//	return spColumn->Cell(spRow.get());
//}
//
//std::shared_ptr<CCell>& CSheet::Cell( CRow* pRow,  CColumn* pColumn)
//{
//	return pColumn->Cell(pRow);
//}

const std::shared_ptr<CCell>& CSheet::Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn) 
{
	return Cell(spRow.get(), spColumn.get());
}
const std::shared_ptr<CCell>& CSheet::Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow) 
{
	return Cell(spRow, spColumn);
}
const std::shared_ptr<CCell>& CSheet::Cell(CRow* const pRow, CColumn* const pColumn) 
{
	auto& container = m_allCells.get<rowcol_tag>();
	auto iter = container.find(MAKELONGLONG(pRow, pColumn));
	if (iter == container.end()) {
		if (m_bindType == BindType::Row) {
			if (pRow == GetHeaderRowPtr().get()) {
				iter = container.insert(pColumn->HeaderCellTemplate(pRow, pColumn)).first;
			} else if (pRow == GetNameHeaderRowPtr().get()) {
				iter = container.insert(pColumn->NameHeaderCellTemplate(pRow, pColumn)).first;
			} else if (pRow == GetFilterRowPtr().get()) {
				iter = container.insert(pColumn->FilterCellTemplate(pRow, pColumn)).first;
			} else {
				iter = container.insert(pColumn->CellTemplate(pRow, pColumn)).first;
			}
		} else if (m_bindType == BindType::Column) {
			if (pRow == GetHeaderRowPtr().get()) {
				iter = container.insert(pRow->HeaderCellTemplate(pRow, pColumn)).first;
			} else if (pRow == GetNameHeaderRowPtr().get()) {
				iter = container.insert(pRow->NameHeaderCellTemplate(pRow, pColumn)).first;
			} else if (pRow == GetFilterRowPtr().get()) {
				iter = container.insert(pRow->FilterCellTemplate(pRow, pColumn)).first;
			} else {
				iter = container.insert(pRow->CellTemplate(pRow, pColumn)).first;
			}
		}



	}
	return const_cast<std::shared_ptr<CCell>&>(*iter);

}


std::shared_ptr<CCell> CSheet::Cell(const CPointF& pt)
{
	auto rowPtr = Coordinate2Pointer<RowTag>(pt.y);
	auto colPtr = Coordinate2Pointer<ColTag>(pt.x);
	if (rowPtr.get() != nullptr && colPtr.get() != nullptr){
		return Cell(rowPtr, colPtr);
	}
	else {
		return nullptr;
	}
}

void CSheet::SetAllRowMeasureValid(bool valid)
{
	for (const auto& ptr : m_allRows) {
		ptr->SetIsMeasureValid(valid);
	}
}

void CSheet::SetAllRowFitMeasureValid(bool valid)
{
	for (const auto& ptr : m_allRows) {
		ptr->SetIsFitMeasureValid(valid);
	}
}

void CSheet::SetAllColumnMeasureValid(bool valid)
{
	for (const auto& ptr : m_allCols) {
		ptr->SetIsMeasureValid(valid);
	}
}

void CSheet::SetAllColumnFitMeasureValid(bool valid)
{
	for (const auto& ptr : m_allCols) {
		ptr->SetIsFitMeasureValid(valid);
	}
}


void CSheet::SetColumnAllCellMeasureValid(CColumn* pColumn, bool valid)
{
	for (const auto& ptr : m_allRows) {
		Cell(ptr.get(), pColumn)->SetActMeasureValid(false);
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
	if (!_tcsicmp(name, L"selected")) {
		////0506PostUpdate(Updates::RowVisible);
		////0506PostUpdate(Updates::Column);
		////0506PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	} else if (_tcsicmp(name, L"visible") == 0) {
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);//
	}
}

void CSheet::OnColumnPropertyChanged(CColumn* pCol, const wchar_t* name)
{
	if (!_tcsicmp(name, L"visible") || !_tcsicmp(name, L"size")) {
		for (const auto& ptr : m_allRows) {
			ptr->SetIsMeasureValid(false);
		}
		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(name, L"selected")) {
		PostUpdate(Updates::Invalidate);
	} else if (!_tcsicmp(name, L"sort")) {
		PostUpdate(Updates::Sort);
	}
}

void CSheet::ColumnInserted(const CColumnEventArgs& e)
{
	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CSheet::ColumnErased(const CColumnEventArgs& e)
{
	this->SetAllRowMeasureValid(false);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CSheet::ColumnMoved(const CMovedEventArgs<ColTag>& e)
{
	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}


void CSheet::ColumnHeaderFitWidth(const CColumnEventArgs& e)
{
	e.m_pColumn->SetIsMeasureValid(false);
	for(const auto& ptr : m_allRows){
		Cell(ptr, e.m_pColumn)->SetActMeasureValid(false);
	}
	this->SetAllColumnMeasureValid(false);
	
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
}

void CSheet::RowInserted(const CRowEventArgs& e)
{
	for(auto& spCol : m_allCols){
		spCol->SetIsFitMeasureValid(false);
	};
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::RowErased(const CRowEventArgs& e)
{
	this->SetAllColumnFitMeasureValid(false);

	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::RowsErased(const CRowsEventArgs& e)
{
	//Remeasure column width irritate user, therefore Column measure doesn't run.
	//boost::for_each(m_columnAllDictionary,[&](const ColumnData& colData){
	//	colData.DataPtr->SetMeasureValid(false);
	//});
	this->SetAllColumnFitMeasureValid(false);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);//
}

void CSheet::RowMoved(const CMovedEventArgs<RowTag>& e)
{
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
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
	for(const auto& type : m_setUpdate){
		switch(type){
		case Updates::RowVisible:
			{
				LOG_SCOPED_TIMER_THIS_1("UpdateRowVisibleDictionary");
				UpdateRowVisibleDictionary();
			}
			break;
		case Updates::ColumnVisible:
			{
				LOG_SCOPED_TIMER_THIS_1("UpdateColumnVisibleDictionary");
				UpdateColumnVisibleDictionary();
				break;
			}
		case Updates::Column:
			{
				LOG_SCOPED_TIMER_THIS_1("UpdateColumn");
				UpdateColumn();
				break;
			}
		case Updates::Row:
			{
				LOG_SCOPED_TIMER_THIS_1("UpdateRow");
				UpdateRow();
				break;
			}
		case Updates::Scrolls:
			{
				LOG_SCOPED_TIMER_THIS_1("UpdateScrolls");
				UpdateScrolls();
				break;
			}
		case Updates::Invalidate:
			{
				LOG_SCOPED_TIMER_THIS_1("Invalidate");
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
		m_setUpdate.insert(Updates::Sort);
		m_setUpdate.insert(Updates::Filter);
		m_setUpdate.insert(Updates::Invalidate);
		break;
	case Updates::EnsureVisibleFocusedCell:
		////0506m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::Row);
		////0506m_setUpdate.insert(Updates::ColumnVisible);
		m_setUpdate.insert(Updates::Column);
		m_setUpdate.insert(Updates::Scrolls);
		////0506m_setUpdate.insert(Updates::Sort);
		m_setUpdate.insert(type);
		m_setUpdate.insert(Updates::Invalidate);
		break;
	case Updates::Sort:
		////0506m_setUpdate.insert(Updates::RowVisible);
		m_setUpdate.insert(Updates::Row);
		m_setUpdate.insert(type);
		m_setUpdate.insert(Updates::Invalidate);
		PostUpdate(Updates::EnsureVisibleFocusedCell);
	case Updates::Filter:
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
		m_setUpdate.insert(type);
	case Updates::RowVisible:
		m_setUpdate.insert(Updates::Sort);
	default:
		m_setUpdate.insert(type);
		break;
	}

}

std::wstring CSheet::GetSheetString()
{
	std::wstring str;

	for(const auto& rowPtr : m_visRows){
		bool bFirstLine=true;
		for(const auto& colPtr : m_visCols){
			const auto& pCell = Cell(rowPtr, colPtr);
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
	UpdateVisibleContainer<RowTag>();
}

void CSheet::UpdateColumnVisibleDictionary()
{
	UpdateVisibleContainer<ColTag>();
}

void CSheet::UpdateRowPaintDictionary()
{
	CRectF rcClip(GetPaintRect());
	CPointF ptOrigin(GetFrozenPoint());
	rcClip.left = ptOrigin.x;
	rcClip.top = ptOrigin.y;
	UpdatePaintContainer<RowTag>(
		rcClip.top,
		rcClip.bottom);
}

void CSheet::UpdateColumnPaintDictionary()
{
	CRectF rcClip(GetPaintRect());
	CPointF ptOrigin(GetFrozenPoint());
	rcClip.left = ptOrigin.x;
	rcClip.top = ptOrigin.y;
	UpdatePaintContainer<ColTag>(
		rcClip.left,
		rcClip.right);
}

void CSheet::ResetColumnSort()
{
	//Reset Sort Mark
	for(auto& ptr : m_allCols){
		ptr->SetSort(Sorts::None);
	}
}

void CSheet::Sort(CColumn* pCol, Sorts sort, bool postUpdate)
{
	//Sort
	switch(sort){
	case Sorts::Down:
		m_visRows.idx_stable_sort(m_visRows.begin() + m_frozenRowCount, m_visRows.end(),
			[pCol, this](const auto& lhs, const auto& rhs)->bool{
				auto cmp = _tcsicmp(Cell(lhs.get(), pCol)->GetSortString().c_str(), Cell(rhs.get(), pCol)->GetSortString().c_str());
				if (cmp > 0) {
					return true;
				} else if (cmp < 0) {
					return false;
				} else {
					if (lhs->GetIndex<AllTag>() == rhs->GetIndex<AllTag>()) {
						::OutputDebugString(std::format(L"{}=={}", lhs->GetIndex<AllTag>(), rhs->GetIndex<AllTag>()).c_str());
					}
					return lhs->GetIndex<AllTag>() > rhs->GetIndex<AllTag>();
				}
			});

		break;
	case Sorts::Up:
		m_visRows.idx_stable_sort(m_visRows.begin() + m_frozenRowCount, m_visRows.end(),
			[pCol, this](const auto& lhs, const auto& rhs)->bool {
				auto cmp = _tcsicmp(Cell(lhs.get(), pCol)->GetSortString().c_str(), Cell(rhs.get(), pCol)->GetSortString().c_str());
				if (cmp < 0) {
					return true;
				} else if (cmp == 0) {
					return lhs->GetIndex<AllTag>() < rhs->GetIndex<AllTag>();
				} else {
					return false;
				}
			});
		break;
	default:
		break;
	}
}

void CSheet::Filter(int colDisp,std::function<bool(const std::shared_ptr<CCell>&)> predicate)
{
	for(auto rowIter= m_allRows.begin() + m_frozenRowCount;rowIter!=m_allRows.end(); ++rowIter){
		if(predicate(Cell(*rowIter, m_allCols[colDisp]))){
			(*rowIter)->SetIsVisible(true);
		}else{
			(*rowIter)->SetIsVisible(false);		
		}
	}
}

void CSheet::ResetFilter()
{
	auto setVisible = [](const auto& ptr) {ptr->SetIsVisible(true); };
	std::for_each(m_allRows.begin(), m_allRows.end(), setVisible);
	std::for_each(m_allCols.begin(), m_allCols.end(), setVisible);
}

void CSheet::EraseColumn(const std::shared_ptr<CColumn>& spCol, bool notify)
{
	m_allCols.idx_erase(std::find(m_allCols.begin(), m_allCols.end(), spCol));
	m_spCursorer->OnCursorClear(this);
	if (notify) {
		ColumnErased(CColumnEventArgs(spCol));
	}
}

void CSheet::EraseRow(const std::shared_ptr<CRow>& spRow, bool notify)
{
	m_allRows.idx_erase(std::find(m_allRows.begin(), m_allRows.end(), spRow));
	m_spCursorer->OnCursorClear(this);
	if (notify) {
		RowErased(CRowEventArgs(spRow));
	}
}

void CSheet::EraseRows(const std::vector<std::shared_ptr<CRow>>& vspRow, bool notify)
{
	for (auto rowPtr : vspRow) {
		EraseRow(rowPtr);
	}
}

void CSheet::InsertColumn(int pos, const std::shared_ptr<CColumn>& spColumn, bool notify)
{
	m_allCols.idx_insert(m_allCols.cbegin() + pos, spColumn);
	if (notify) {
		ColumnInserted(CColumnEventArgs(spColumn));
	}
}


void CSheet::PushColumn(const std::shared_ptr<CColumn>& spCol, bool notify)
{
	//spCol->InsertNecessaryRows();
	m_allCols.idx_push_back(spCol);
	if (notify) {
		ColumnInserted(CColumnEventArgs(spCol));
	}
}

void CSheet::InsertRow(int row, const std::shared_ptr<CRow>& spRow, bool notify)
{
	m_allRows.idx_insert(m_allRows.cbegin() + row, spRow);
	if (notify) {
		RowInserted(CRowEventArgs(spRow));
	}
}


void CSheet::PushRow(const std::shared_ptr<CRow>& spRow, bool notify)
{
	m_allRows.idx_push_back(spRow);
	if(notify){
		RowInserted(CRowEventArgs(spRow));
	}
}

FLOAT CSheet::GetColumnInitWidth(CColumn* pColumn)
{
	return std::accumulate(m_visRows.begin(), m_visRows.end(), 0.f,
		[this, pColumn](const FLOAT val, const std::shared_ptr<CRow>& rowPtr)->FLOAT {
			return (std::max)(Cell(rowPtr.get(), pColumn)->GetInitSize(GetWndPtr()->GetDirectPtr()).width, val);
		});
}

FLOAT CSheet::GetColumnFitWidth(CColumn* pColumn)
{
	return std::accumulate(m_visRows.begin(), m_visRows.end(), 0.f,
		[this, pColumn](const FLOAT val, const std::shared_ptr<CRow>& rowPtr)->FLOAT {
			return (std::max)(Cell(rowPtr.get(), pColumn)->GetFitSize(GetWndPtr()->GetDirectPtr()).width, val);
		});
}

FLOAT CSheet::GetRowHeight(CRow* pRow)
{
	return std::accumulate(m_visCols.begin(), m_visCols.end(), 0.f,
		[this, pRow](const FLOAT val, const std::shared_ptr<CColumn>& colPtr)->FLOAT {
			return (std::max)(Cell(pRow, colPtr.get())->GetActSize(GetWndPtr()->GetDirectPtr()).height, val);
		});
}

FLOAT CSheet::GetRowFitHeight(CRow* pRow)
{
	return std::accumulate(m_visCols.begin(), m_visCols.end(), 0.f,
						   [this, pRow](const FLOAT val, const std::shared_ptr<CColumn>& colPtr)->FLOAT {
							   return (std::max)(Cell(pRow, colPtr.get())->GetFitSize(GetWndPtr()->GetDirectPtr()).height, val);
						   });
}

bool CSheet::Empty()const
{
	return m_allRows.empty() || m_allCols.empty();
}
bool CSheet::Visible()const
{
	return (!m_visRows.empty()) && (!m_visCols.empty()) && GetRectInWnd().Width() > 0 && GetRectInWnd().Height() > 0;
}

CPointF CSheet::GetFrozenPoint()
{
	if(!Visible()){
		return GetRectInWnd().LeftTop();
	}

	FLOAT x = 0;
	FLOAT y = 0;
	if (m_frozenRowCount == 0) {
		y = GetRectInWnd().top + GetCellProperty()->Line->Width * 0.5f;
	} else {
		y = m_visRows[m_frozenRowCount - 1]->GetBottom();
	}

	if (m_frozenColumnCount == 0) {
		x = GetRectInWnd().left + GetCellProperty()->Line->Width * 0.5f;
	} else {
		x = m_visCols[m_frozenColumnCount - 1]->GetRight();
	}

	return CPointF(x,y);
}


CSizeF CSheet::MeasureSize()const
{
	//CRectF rc(CSheet::GetRectInWnd());
	//return CSizeF(rc.Width(), rc.Height());
	if(!Visible()){return CSizeF();}
	FLOAT left=m_visCols.front()->GetLeft();
	FLOAT top=m_visRows.front()->GetTop();
	FLOAT right=m_visCols.back()->GetRight();
	FLOAT bottom=m_visRows.back()->GetBottom();

	CRectF rc(left, top, right, bottom);

	auto outerPenWidth = m_spSheetProperty->CellPropPtr->Line->Width/2.0f;
	rc.InflateRect(outerPenWidth, outerPenWidth);
	return rc.Size();
}

//GetRect return Sheet(All) Rect in Window Client
//GetCellsRect return Cells(=Scrollable Cells) Rect in Window Client
//GetCellsClipRect return Cells(=Scrollable Cells) Clip(=Paint area) Rect in Window Client
//CRectF CSheet::GetRectInWnd()const
//{
//	if(!Visible()){return CRectF();}
//	FLOAT left=m_visCols.front()->GetLeft();
//	FLOAT top=m_visRows.front()->GetTop();
//	FLOAT right=m_visCols.back()->GetRight();
//	FLOAT bottom=m_visRows.back()->GetBottom();
//
//	CRectF rc(left, top, right, bottom);
//
//	auto outerPenWidth = m_spSheetProperty->CellPropPtr->Line->Width/2.0f;
//	rc.InflateRect(outerPenWidth, outerPenWidth);
//	return rc;
//}

FLOAT CSheet::GetCellsHeight()
{
	if (GetContainer<RowTag, VisTag>().size() == 0 || GetContainer<RowTag, VisTag>().size() <= m_frozenRowCount) {
		return 0.0f;
	}

	FLOAT top = m_visRows[m_frozenRowCount]->GetTop();
	FLOAT bottom = m_visRows.back()->GetBottom();

	return bottom - top;
}

FLOAT CSheet::GetCellsWidth()
{
	if (GetContainer<ColTag, VisTag>().size() == 0 || GetContainer<ColTag, VisTag>().size() <= m_frozenColumnCount) {
		return 0.0f;
	}

	FLOAT left = m_visCols[m_frozenColumnCount]->GetLeft();
	FLOAT right = m_visCols.back()->GetRight();

	return right - left;
}

CRectF CSheet::GetCellsRect()
{
	if(GetContainer<RowTag, VisTag>().size() == 0 || GetContainer<RowTag, VisTag>().size() <= m_frozenRowCount || GetContainer<ColTag, VisTag>().size() <= m_frozenColumnCount){
		return CRectF(0,0,0,0);
	}

	FLOAT left=m_visCols[m_frozenColumnCount]->GetLeft();
	FLOAT top= m_visRows[m_frozenRowCount]->GetTop();
	FLOAT right=m_visCols.back()->GetRight();
	FLOAT bottom=m_visRows.back()->GetBottom();

	return CRectF(left,top,right,bottom);
}

void CSheet::UpdateAll()
{
	UpdateRowVisibleDictionary();//TODO today
	UpdateColumnVisibleDictionary();
	UpdateColumn();
	UpdateRow();
}

/**********/
/* Normal */
/**********/
void CSheet::Normal_Paint(const PaintEvent& e)
{
	if (!Visible())return;

	//Update Paint Dictioanary
	UpdateRowPaintDictionary();
	UpdateColumnPaintDictionary();

	//Paint
	for (auto rowIter = m_pntRows.rbegin(), rowEnd = m_pntRows.rend(); rowIter != rowEnd; ++rowIter) {
		(*rowIter)->OnPaint(e);
		for (auto colIter = m_pntCols.rbegin(), colEnd = m_pntCols.rend(); colIter != colEnd; ++colIter) {
			(*colIter)->OnPaint(e);
		}
	}
	for (auto rowIter = m_pntRows.rbegin(), rowEnd = m_pntRows.rend(); rowIter != rowEnd; ++rowIter) {
		for (auto colIter = m_pntCols.rbegin(), colEnd = m_pntCols.rend(); colIter != colEnd; ++colIter) {
			Cell(*rowIter, *colIter)->OnPaint(e);
		}
	}

	//Paint Drag Target Line
	m_spRowDragger->OnPaintDragLine(this, e);
	m_spColDragger->OnPaintDragLine(this, e);
	if (m_spItemDragger) { m_spItemDragger->OnPaintDragLine(this, e); }
}
void CSheet::Normal_LButtonDown(const LButtonDownEvent& e)
{
	m_spCeller->OnLButtonDown(this, e);
	m_spCursorer->OnLButtonDown(this, e);
}
void CSheet::Normal_LButtonUp(const LButtonUpEvent& e)
{
	m_spCursorer->OnLButtonUp(this, e);
	m_spCeller->OnLButtonUp(this, e);
}
void CSheet::Normal_LButtonClk(const LButtonClkEvent& e)
{
	m_spCeller->OnLButtonClk(this, e);
}
void CSheet::Normal_LButtonSnglClk(const LButtonSnglClkEvent& e)
{
	m_spCeller->OnLButtonSnglClk(this, e);
}
void CSheet::Normal_LButtonDblClk(const LButtonDblClkEvent& e)
{
	if (m_spColTracker->IsTarget(this, e)) {
		m_spColTracker->OnDividerDblClk(this, e);
	}
	else if (m_spRowTracker->IsTarget(this, e)) {
		m_spRowTracker->OnDividerDblClk(this, e);
	}
	else {
		m_spCursorer->OnLButtonDblClk(this, e);
		m_spCeller->OnLButtonDblClk(this, e);
	}
}
void CSheet::Normal_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	m_spCeller->OnLButtonBeginDrag(this, e);
}

void CSheet::Normal_RButtonDown(const RButtonDownEvent& e)
{
	m_spCursorer->OnRButtonDown(this, e);
}
void CSheet::Normal_MouseMove(const MouseMoveEvent& e)
{
	m_spCeller->OnMouseMove(this, e);
}
void CSheet::Normal_MouseLeave(const MouseLeaveEvent& e)
{
	m_spCeller->OnMouseLeave(this, e);
}
bool CSheet::Normal_Guard_SetCursor(const SetCursorEvent& e)
{
	return e.HitTest == HTCLIENT;
}

void CSheet::Normal_SetCursor(const SetCursorEvent& e)
{
	if (!*e.HandledPtr) { 
		m_spRowTracker->OnSetCursor(this, e);
	}
	if (!*e.HandledPtr) {
		m_spColTracker->OnSetCursor(this, e);
	}
	if (!*e.HandledPtr) {
		m_spCeller->OnSetCursor(this, e);
	}
}
void CSheet::Normal_ContextMenu(const ContextMenuEvent& e)
{
	m_spCeller->OnContextMenu(this, e);
}
void CSheet::Normal_KeyDown(const KeyDownEvent& e)
{
	m_spCursorer->OnKeyDown(this, e);
	m_spCeller->OnKeyDown(this, e);
}
void CSheet::Normal_Char(const CharEvent& e)
{
	m_spCeller->OnChar(this, e);
}

void CSheet::Normal_SetFocus(const SetFocusEvent& e)
{
	if (e.HandledPtr) { *e.HandledPtr = FALSE; }
	//	m_spCursorer->OnCursorClear(this);
	//m_bFocused = true;
}

void CSheet::Normal_KillFocus(const KillFocusEvent& e)
{
	if (e.HandledPtr) { *e.HandledPtr = FALSE; }
	//m_spCursorer->OnCursorClear(this);
	//m_bFocused = false;
}


/***********/
/* RowTrack*/
/***********/
void CSheet::RowTrack_LButtonDown(const LButtonDownEvent& e)
{

	m_spRowTracker->OnBeginTrack(this, e);
}

bool CSheet::RowTrack_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_spRowTracker->IsTarget(this, e);
}

void CSheet::RowTrack_MouseMove(const MouseMoveEvent& e)
{
	m_spRowTracker->OnTrack(this, e);
}

void CSheet::RowTrack_LButtonUp(const LButtonUpEvent& e)
{
	m_spRowTracker->OnEndTrack(this, e);
}

void CSheet::RowTrack_MouseLeave(const MouseLeaveEvent& e)
{
	m_spRowTracker->OnLeaveTrack(this, e);
}

/***********/
/* ColTrack*/
/***********/
void CSheet::ColTrack_LButtonDown(const LButtonDownEvent& e)
{
	m_spColTracker->OnBeginTrack(this, e);
}

bool CSheet::ColTrack_Guard_LButtonDown(const LButtonDownEvent& e)
{
	return m_spColTracker->IsTarget(this, e);
}

void CSheet::ColTrack_MouseMove(const MouseMoveEvent& e)
{
	m_spColTracker->OnTrack(this, e);
}

void CSheet::ColTrack_LButtonUp(const LButtonUpEvent& e)
{
	m_spColTracker->OnEndTrack(this, e);
}

void CSheet::ColTrack_MouseLeave(const MouseLeaveEvent& e)
{
	m_spColTracker->OnLeaveTrack(this, e);
}
/***********/
/* RowDrag */
/***********/
void CSheet::RowDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spRowDragger->OnBeginDrag(this, e);
}

void CSheet::RowDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spRowDragger->OnEndDrag(this, e);
}

bool CSheet::RowDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spRowDragger->IsTarget(this, e);
}

void CSheet::RowDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spRowDragger->OnDrag(this, e);
}

void CSheet::RowDrag_MouseLeave(const MouseLeaveEvent& e)
{
	m_spRowDragger->OnLeaveDrag(this, e);
}
/***********/
/* ColDrag */
/***********/
void CSheet::ColDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	m_spColDragger->OnBeginDrag(this, e);
}

void CSheet::ColDrag_OnExit(const LButtonEndDragEvent& e)
{
	m_spColDragger->OnEndDrag(this, e);
}

bool CSheet::ColDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spColDragger->IsTarget(this, e);
}

void CSheet::ColDrag_MouseMove(const MouseMoveEvent& e)
{
	m_spColDragger->OnDrag(this, e);
}

void CSheet::ColDrag_MouseLeave(const MouseLeaveEvent& e)
{
	m_spColDragger->OnLeaveDrag(this, e);
}
/************/
/* ItemDrag */
/************/
void CSheet::ItemDrag_OnEntry(const LButtonBeginDragEvent& e)
{
	::OutputDebugString(L"ItemDrag_BeginDrag\r\n");
	m_spItemDragger->OnBeginDrag(this, e);
}

void CSheet::ItemDrag_OnExit(const LButtonEndDragEvent& e)
{
	::OutputDebugString(L"ItemDrag_LButtonUp\r\n");
	//m_spCursorer->OnLButtonEndDrag(this, e);
	//m_spCeller->OnLButtonEndDrag(this, e);

	m_spItemDragger->OnEndDrag(this, e);
}

bool CSheet::ItemDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e)
{
	return m_spItemDragger->IsTarget(this, e);
}

void CSheet::ItemDrag_MouseMove(const MouseMoveEvent& e)
{
	::OutputDebugString(L"ItemDrag_MouseMove\r\n");
	m_spItemDragger->OnDrag(this, e);
}

void CSheet::ItemDrag_MouseLeave(const MouseLeaveEvent& e)
{
	::OutputDebugString(L"ItemDrag_Leave\r\n");
	m_spItemDragger->OnLeaveDrag(this, e);
}

/*********/
/* Error */
/*********/
void CSheet::Error_StdException(const std::exception& e)
{
	::OutputDebugStringA(e.what());

	std::string msg = fmt::format(
		"What:{}\r\n"
		"Last Error:{}\r\n",
		e.what(), GetLastErrorString());

	MessageBoxA(GetWndPtr()->m_hWnd, msg.c_str(), "Exception in StateMachine", MB_ICONWARNING);
	Clear();
}

/**************/
/* UI Message */
/**************/
void CSheet::OnPaint(const PaintEvent& e) { m_pMachine->process_event(e);  SubmitUpdate(); }
void CSheet::OnContextMenu(const ContextMenuEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnRButtonDown(const RButtonDownEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnLButtonDown(const LButtonDownEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnLButtonUp(const LButtonUpEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnLButtonClk(const LButtonClkEvent& e) { m_pMachine->process_event(e); SubmitUpdate();  }
void CSheet::OnLButtonSnglClk(const LButtonSnglClkEvent& e) { m_pMachine->process_event(e); PostUpdate(Updates::Invalidate); SubmitUpdate(); }
void CSheet::OnLButtonDblClk(const LButtonDblClkEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnLButtonBeginDrag(const LButtonBeginDragEvent& e) 
{
	e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
	m_pMachine->process_event(e); SubmitUpdate();
}
void CSheet::OnLButtonEndDrag(const LButtonEndDragEvent& e)
{
	e.WndPtr->ReleaseCapturedControlPtr();
	m_pMachine->process_event(e); SubmitUpdate();
}
void CSheet::OnMouseMove(const MouseMoveEvent& e) { m_pMachine->process_event(e); PostUpdate(Updates::Invalidate); SubmitUpdate(); }
void CSheet::OnSetCursor(const SetCursorEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnMouseLeave(const MouseLeaveEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnKeyDown(const KeyDownEvent& e) { m_pMachine->process_event(e); PostUpdate(Updates::Invalidate); SubmitUpdate(); }
void CSheet::OnSetFocus(const SetFocusEvent& e) { m_pMachine->process_event(e);  SubmitUpdate(); }
void CSheet::OnKillFocus(const KillFocusEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnChar(const CharEvent& e) { m_pMachine->process_event(e);  SubmitUpdate(); }
void CSheet::OnBeginEdit(const BeginEditEvent& e) { m_pMachine->process_event(e); SubmitUpdate(); }
void CSheet::OnEndEdit(const EndEditEvent& e){ m_pMachine->process_event(e);  SubmitUpdate();}


void CSheet::SelectRange(std::shared_ptr<CCell>& cell1, std::shared_ptr<CCell>& cell2, bool doSelect)
{
	if (cell1 || cell2) { return; }

	auto	rowBeg= (std::min)(cell1->GetRowPtr()->GetIndex<VisTag>(), cell2->GetRowPtr()->GetIndex<VisTag>());
	auto	rowLast= (std::max)(cell1->GetRowPtr()->GetIndex<VisTag>(), cell2->GetRowPtr()->GetIndex<VisTag>());
	auto	colBeg= (std::min)(cell1->GetColumnPtr()->GetIndex<VisTag>(), cell2->GetColumnPtr()->GetIndex<VisTag>());
	auto	colLast= (std::max)(cell1->GetColumnPtr()->GetIndex<VisTag>(), cell2->GetColumnPtr()->GetIndex<VisTag>());

	for(auto col = colBeg; col<=colLast; col++){
		for(auto row = rowBeg; row<=rowLast; row++){
			Cell(m_visRows[row], m_visCols[col])->SetIsSelected(doSelect);
		}
	}	
}

void CSheet::SelectAll()
{
	for (auto iter = std::next(m_visRows.begin(), m_frozenRowCount); iter!=m_visRows.end(); ++iter) {
		(*iter)->SetIsSelected(true);
	}
}

void CSheet::DeselectAll()
{
	auto setSelected = [](auto& ptr) {ptr->SetIsSelected(false); };

	for(const auto& colPtr : m_allCols){
		for(const auto& rowPtr : m_allRows){
			setSelected(Cell(rowPtr, colPtr));
		}
	}
	std::for_each(m_allCols.begin(), m_allCols.end(), setSelected);
	std::for_each(m_allRows.begin(), m_allRows.end(), setSelected);
}

void CSheet::UnhotAll()
{
	for (const auto& colPtr : m_allCols) {
		for (const auto& rowPtr : m_allRows) {
			Cell(rowPtr, colPtr)->SetState(UIElementState::Normal);
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


void CSheet::Clear()
{
	m_setUpdate.clear();

	m_allRows.clear();
	m_visRows.clear();
	m_pntRows.clear();

	m_allCols.clear();
	m_visCols.clear();
	m_pntCols.clear();

	m_spCursorer->Clear();
	m_spCeller->Clear();
	//m_spColDragger->Clear();
	//m_spRowDragger->Clear();
	//m_spColTracker->Clear();
	//m_spRowTracker->Clear();

	m_rocoContextMenu=CRowColumn();

	//m_pHeaderColumn=std::shared_ptr<CColumn>();
	//m_rowHeader=std::shared_ptr<CRow>();

	m_bSelected = false;
	//m_bFocused = false;
	m_keepEnsureVisibleFocusedCell = false;
}




