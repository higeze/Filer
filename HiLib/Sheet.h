#pragma once
#include "UIElement.h"
#include "MyMenu.h"
#include "RowColumn.h"
#include "Operation.h"
#include "SheetEnums.h"
#include "SheetDictionary.h"
#include "SheetEventArgs.h"
#include "Row.h"
#include "Cell.h"
#include "observable.h"

class IStateMachine;
class IDragger;
class ITracker;
class ICeller;
//class CCell;
//class CRow;
class CColumn;
struct HeaderProperty;
class CellProperty;
class CRect;
class CPoint;
struct XTag;
struct YTag;
class CDC;
template <typename TRC, typename TRCYou> class CDragger;
class CCursorer;
class IMouseObserver;
class CSerializeData;
struct SheetStateMachine;

struct RowTag
{
	using SharedPtr = std::shared_ptr<CRow>;
	typedef const CRow* Ptr;
	typedef observable_vector<std::shared_ptr<CRow>> Container;
	typedef YTag Axis;
	typedef XTag OtherAxis;
	typedef ColTag Other;
	typedef RowData Data;
};

struct ColTag
{
	using SharedPtr = std::shared_ptr<CColumn>;
	typedef const CColumn* Ptr;
	typedef observable_vector<std::shared_ptr<CColumn>> Container;
	typedef XTag Axis;
	typedef YTag OtherAxis;
	typedef RowTag Other;
	typedef ColumnData Data;
};

template<typename TRC>
FLOAT Point2Coordinate(d2dw::CPointF pt)
{ 
	if constexpr (std::is_same_v<TRC, RowTag>) {
		return pt.y;
	} else if constexpr (std::is_same_v<TRC, ColTag>) {
		return pt.x;
	} else {
		throw std::exception("Point2Coordinate");
	}
}

FLOAT GetLeftTop(const std::shared_ptr<CRow>& ptr)
{
	return ptr->GetTop();
}

FLOAT GetLeftTop(const std::shared_ptr<CColumn>& ptr)
{
	return ptr->GetLeft();
}

FLOAT GetRightBottom(const std::shared_ptr<CRow>& ptr)
{
	return ptr->GetBottom();
}

FLOAT GetRightBottom(const std::shared_ptr<CColumn>& ptr)
{
	return ptr->GetRight();
}

FLOAT GetWidthHeight(const std::shared_ptr<CRow>& ptr)
{
	return ptr->GetHeight();
}

FLOAT GetWidthHeight(const std::shared_ptr<CColumn>& ptr)
{
	return ptr->GetWidth();
}

void SetWidthHeight(const std::shared_ptr<CRow>& ptr, const FLOAT height)
{
	return ptr->SetHeight(height);
}

void SetWidthHeight(const std::shared_ptr<CColumn>& ptr, const FLOAT width)
{
	return ptr->SetWidth(width);
}

void SetWidthHeightWithoutSignal(const std::shared_ptr<CRow>& ptr, const FLOAT height)
{
	return ptr->SetHeightWithoutSignal(height);
}

void SetWidthHeightWithoutSignal(const std::shared_ptr<CColumn>& ptr, const FLOAT width)
{
	return ptr->SetWidthWithoutSignal(width);
}


template<typename TRC>
int Indexes2Index(const std::pair<int, int>& pair)
{
	if constexpr (std::is_same_v<TRC, RowTag>) {
		return pair.first;
	} else if constexpr (std::is_same_v<TRC, ColTag>) {
		return pair.second;
	} else {
		throw std::exception("Indexes2Index");
	}
}

//struct RC
//{
//	RC(int row, int col) :Row(row), Col(col) {}
//	int Row;
//	int Col;
//	template<typename TRC> int Get() const { return CBand::kInvalidIndex; }
//};
//template<> inline int RC::Get<RowTag>() const { return Row; }
//template<> inline int RC::Get<ColTag>() const { return Col; }

//Self defined message
const UINT WM_FILTER = RegisterWindowMessage(L"WM_FILTER");
const UINT WM_EDITCELL = RegisterWindowMessage(L"WM_EDITCELL");
const UINT WM_LBUTTONDBLCLKTIMEXCEED = RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED");

/********/
/*CSheet*/
/********/
class CSheet:virtual public CUIElement
{
	//Friend classes
	friend class CMouseStateMachine;
	friend class CFileDragger;
	friend class CSerializeData;
	friend class CCeller;
public:
	//static cell Accessor
	static std::shared_ptr<CCell>& Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn);
	static std::shared_ptr<CCell>& Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow);
	static std::shared_ptr<CCell>& Cell( CRow* pRow,  CColumn* pColumn);
protected:
	//State machine
	std::unique_ptr<IStateMachine> m_pMachine;
public:
	std::shared_ptr<ITracker> m_spRowTracker; // Tracker for Row
	std::shared_ptr<ITracker> m_spColTracker; // Tracker for Column
	std::shared_ptr<IDragger> m_spRowDragger; // Dragger for Row
	std::shared_ptr<IDragger> m_spColDragger; // Dragger for Column
	std::shared_ptr<IDragger> m_spItemDragger; // Dragger for Item
	std::shared_ptr<ICeller> m_spCeller; //Celler
	std::shared_ptr<CCursorer> m_spCursorer; // Cursor
private:

protected:
	std::set<Updates> m_setUpdate; // Set posted update

	observable_vector<std::shared_ptr<CRow>> m_allRows;
	observable_vector<std::shared_ptr<CRow>> m_visRows;
	observable_vector<std::shared_ptr<CRow>> m_pntRows;
	size_t m_frozenRowCount = 0;

	observable_vector<std::shared_ptr<CColumn>> m_allCols;
	observable_vector<std::shared_ptr<CColumn>> m_visCols;
	observable_vector<std::shared_ptr<CColumn>> m_pntCols;
	size_t m_frozenColumnCount = 0;

	//RowDictionary m_rowAllDictionary; // Index-Pointer All Row Dictionary
	//RowDictionary m_rowVisibleDictionary; // Index-Pointer Visible Row Dictionary
	//RowDictionary m_rowPaintDictionary; // Index-Pointer Paint Row Dictionary

	//ColumnDictionary m_columnAllDictionary; // Index-Pointer All Column Dictionary
	//ColumnDictionary m_columnVisibleDictionary; // Index-Pointer Visible Column Dictionary
	//ColumnDictionary m_columnPaintDictionary; // Index-Pointer Paint Column Dictionary

	std::shared_ptr<HeaderProperty> m_spHeaderProperty; // HeaderProperty
	std::shared_ptr<CellProperty> m_spFilterProperty; // FilterProperty
	std::shared_ptr<CellProperty> m_spCellProperty; // CellProperty

	std::shared_ptr<CColumn> m_pHeaderColumn; // Header column
	std::shared_ptr<CRow> m_rowHeader; // Header row

	bool m_bSelected; // Selected or not
	bool m_bFocused; // Focused or not

	CRowColumn m_rocoContextMenu; // Store RowColumn of Caller

	//For open mode flag to keep ensurevisiblecell after open
	bool m_keepEnsureVisibleFocusedCell = false;

public:
	static CMenu ContextMenu; // ContextMenu

public:
	//Signals
	boost::signals2::signal<void(CellEventArgs&)> CellLButtonDown;
	boost::signals2::signal<void(CellEventArgs&)> CellLButtonClk;
	boost::signals2::signal<void(CellEventArgs&)> CellLButtonDblClk;
	boost::signals2::signal<void(CellContextMenuEventArgs&)> CellContextMenu;

	//Constructor
	CSheet(std::shared_ptr<HeaderProperty> spHeaderProperty,
		std::shared_ptr<CellProperty> spFilterProperty,
		std::shared_ptr<CellProperty> spCellProperty,
		CMenu* pContextMenu= &CSheet::ContextMenu);

	//Destructor
	virtual ~CSheet();

	//Getter Setter
//	std::shared_ptr<CSheetStateMachine> GetSheetStateMachine() { return m_spStateMachine; }
//	void SetSheetStateMachine(std::shared_ptr<CSheetStateMachine>& machine) { m_spStateMachine = machine; }
	std::shared_ptr<CCursorer> GetCursorerPtr(){return m_spCursorer;} /**< Cursor */
	void SetContextMenuRowColumn(const CRowColumn& roco){m_rocoContextMenu = roco;}
	virtual std::shared_ptr<HeaderProperty> GetHeaderProperty(){return m_spHeaderProperty;} /** Getter for Header Cell Property */
	virtual std::shared_ptr<CellProperty> GetFilterProperty(){return m_spFilterProperty;} /** Getter for Filter Cell Property */
	virtual std::shared_ptr<CellProperty> GetCellProperty(){return m_spCellProperty;} /** Getter for Cell Property */
	virtual std::shared_ptr<CColumn> GetHeaderColumnPtr()const{return m_pHeaderColumn;} /** Getter for Header Column */
	virtual void SetHeaderColumnPtr(std::shared_ptr<CColumn> column){m_pHeaderColumn=column;} /** Setter for Header Column */
	virtual std::shared_ptr<CRow> GetHeaderRowPtr()const{return m_rowHeader;} /** Getter for Header Row */
	virtual void SetHeaderRowPtr(std::shared_ptr<CRow> row){m_rowHeader=row;} /** Setter for Header Row */
	virtual bool GetSelected()const{return m_bSelected;};
	virtual void SetSelected(const bool& bSelected){m_bSelected=bSelected;};
	virtual bool GetFocused()const{return m_bFocused;};
	//virtual void SetFocused(const bool& bFocused){m_bFocused=bFocused;};

	//Function
	void SetAllRowMeasureValid(bool valid);
	void SetAllColumnMeasureValid(bool valid);
	void SetColumnAllCellMeasureValid(CColumn* pColumn, bool valid);

	//Observer
	virtual void ColumnInserted(CColumnEventArgs& e);
	virtual void ColumnErased(CColumnEventArgs& e);
	virtual void ColumnMoved(CMovedEventArgs<ColTag>& e) {}
	virtual void ColumnHeaderFitWidth(CColumnEventArgs& e);

	virtual void RowInserted(CRowEventArgs& e);
	virtual void RowErased(CRowEventArgs& e);
	virtual void RowsErased(CRowsEventArgs& e);
	virtual void RowMoved(CMovedEventArgs<RowTag>& e) {}

	virtual void SizeChanged();
	virtual void Scroll();

	//Update
	virtual bool HasSheetCell(){ return false; }
	virtual bool IsVirtualPage() { return false; }
	virtual void UpdateRow() = 0;
	virtual void UpdateColumn();
	virtual void UpdateScrolls(){}
	virtual void Invalidate(){}
	virtual void UpdateAll();
	virtual void PostUpdate(Updates type);
	virtual void SubmitUpdate();

	//Sort
	virtual void ResetColumnSort();

	virtual std::shared_ptr<CDC> GetClientDCPtr()const=0;
	virtual d2dw::CPointF GetScrollPos()const=0;
	virtual CGridView* GetGridPtr()=0;
	virtual bool Empty()const;
	virtual bool Visible()const;

	//Index-Pointer-Cell Accessor
	virtual void UpdateRowVisibleDictionary();
	virtual void UpdateColumnVisibleDictionary();
	virtual void UpdateRowPaintDictionary();
	virtual void UpdateColumnPaintDictionary();

	virtual void EraseColumn(const std::shared_ptr<CColumn>& spColumn, bool notify = true);
	virtual void EraseRow(const std::shared_ptr<CRow>& spColumn, bool notify = true);
	virtual void EraseRows(const std::vector<std::shared_ptr<CRow>>& vpRow, bool notify = true);

	virtual void MoveColumn(int colTo, std::shared_ptr<CColumn> spFromColumn){MoveImpl<ColTag>(colTo, spFromColumn);}

	virtual void InsertRow(int rowVisib, const std::shared_ptr<CRow>& pRow, bool notify = true);
	virtual void InsertColumn(int colTo, const std::shared_ptr<CColumn>& pColumn, bool notify = true);

	virtual FLOAT GetColumnInitWidth(CColumn* pColumn);
	virtual FLOAT GetColumnFitWidth(CColumn* pColumn);

	virtual FLOAT GetRowHeight(CRow* pRow);

	virtual std::wstring GetSheetString()const;

	virtual d2dw::CPointF GetOriginPoint();
	virtual d2dw::CSizeF MeasureSize()const;

	virtual d2dw::CRectF GetRect()const;
	virtual FLOAT GetCellsHeight();
	virtual FLOAT GetCellsWidth();
	virtual d2dw::CRectF GetCellsRect();
	virtual d2dw::CRectF GetPaintRect()=0;

	/**************/
	/* UI Message */
	/**************/
protected:
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnRButtonDown(const RButtonDownEvent& e);
	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonClk(const LButtonClkEvent& e);
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e);
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e);
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void OnContextMenu(const ContextMenuEvent& e);
	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnSetCursor(const SetCursorEvent& e);
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnKillFocus(const KillFocusEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e);
	virtual void OnChar(const CharEvent& e);
	virtual void OnBeginEdit(const BeginEditEvent& e);


	virtual void OnPaintAll(const PaintEvent& e);
	virtual void OnBkGndLButtondDblClk(const LButtonDblClkEvent& e) {}

public:

	std::shared_ptr<CCell> Cell(const d2dw::CPointF& pt);

	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name);
	virtual void OnRowPropertyChanged(CRow* pRow, const wchar_t* name);
	virtual void OnColumnPropertyChanged(CColumn* pCol, const wchar_t* name);

	virtual void SelectAll();
	virtual void DeselectAll();
	virtual void UnhotAll();
	virtual bool IsFocusedCell(const CCell* pCell)const;
	virtual bool IsDoubleFocusedCell(const CCell* pCell)const;
	virtual void Clear();
	virtual CColumn* GetParentColumnPtr(CCell* pCell) = 0;
	virtual void Sort(CColumn* pCol, Sorts sort, bool postUpdate = true);
	virtual void Filter(int colDisp, std::function<bool(const std::shared_ptr<CCell>&)> predicate);
	virtual void ResetFilter();

	//Compares CheckEqualRange(RowDictionary::iterator rowBegin, RowDictionary::iterator rowEnd, ColumnDictionary::iterator colBegin, ColumnDictionary::iterator colEnd, std::function<void(CCell*, Compares)> action);
	//Compares CheckEqualRow(CRow* pRow, ColumnDictionary::iterator colBegin, ColumnDictionary::iterator colEnd, std::function<void(CCell*, Compares)> action);

	//Normal
	virtual void Normal_Paint(const PaintEvent& e);
	virtual void Normal_LButtonDown(const LButtonDownEvent& e);
	virtual void Normal_LButtonUp(const LButtonUpEvent& e);
	virtual void Normal_LButtonClk(const LButtonClkEvent& e);
	virtual void Normal_LButtonSnglClk(const LButtonSnglClkEvent& e);
	virtual void Normal_LButtonDblClk(const LButtonDblClkEvent& e);
	virtual void Normal_RButtonDown(const RButtonDownEvent& e);
	virtual void Normal_MouseMove(const MouseMoveEvent& e);
	virtual void Normal_MouseLeave(const MouseLeaveEvent& e);
	virtual bool Normal_Guard_SetCursor(const SetCursorEvent& e);
	virtual void Normal_SetCursor(const SetCursorEvent& e);
	virtual void Normal_ContextMenu(const ContextMenuEvent& e);
	virtual void Normal_KeyDown(const KeyDownEvent& e);
	virtual void Normal_SetFocus(const SetFocusEvent& e);
	virtual void Normal_KillFocus(const KillFocusEvent& e);
	//RowTrack
	virtual void RowTrack_LButtonDown(const LButtonDownEvent& e);
	virtual bool RowTrack_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void RowTrack_MouseMove(const MouseMoveEvent& e);
	virtual void RowTrack_LButtonUp(const LButtonUpEvent& e);
	virtual void RowTrack_MouseLeave(const MouseLeaveEvent& e);
	//ColTrack
	virtual void ColTrack_LButtonDown(const LButtonDownEvent& e);
	virtual bool ColTrack_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void ColTrack_MouseMove(const MouseMoveEvent& e);
	virtual void ColTrack_LButtonUp(const LButtonUpEvent& e);
	virtual void ColTrack_MouseLeave(const MouseLeaveEvent& e);
	//RowDrag
	void RowDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	bool RowDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void RowDrag_MouseMove(const MouseMoveEvent& e);
	void RowDrag_LButtonUp(const LButtonUpEvent& e);
	void RowDrag_MouseLeave(const MouseLeaveEvent& e);
	//ColDrag
	void ColDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	bool ColDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void ColDrag_MouseMove(const MouseMoveEvent& e);
	void ColDrag_LButtonUp(const LButtonUpEvent& e);
	void ColDrag_MouseLeave(const MouseLeaveEvent& e);
	//ItemDrag
	void ItemDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	bool ItemDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void ItemDrag_MouseMove(const MouseMoveEvent& e);
	void ItemDrag_LButtonUp(const LButtonUpEvent& e);
	void ItemDrag_MouseLeave(const MouseLeaveEvent& e);

	//Menu
	CMenu* const m_pContextMenu;
	virtual CMenu* GetContextMenuPtr() { return m_pContextMenu; }

	//Tag dispatch
	template<typename TRC, typename TAV> TRC::template Container& GetContainer() { return m_allRows; }
	template<> inline RowTag::Container& GetContainer<RowTag, AllTag>()
	{
		return m_allRows;
	}
	template<> inline RowTag::Container& GetContainer<RowTag, VisTag>()
	{
		return m_visRows;
	}
	template<> inline RowTag::Container& GetContainer<RowTag, PntTag>()
	{
		return m_pntRows;
	}
	template<> inline ColTag::Container& GetContainer<ColTag, AllTag>()
	{
		return m_allCols;
	}
	template<> inline ColTag::Container& GetContainer<ColTag, VisTag>()
	{
		return m_visCols;
	}
	template<> inline ColTag::Container& GetContainer<ColTag, PntTag>()
	{
		return m_pntCols;
	}

	template<typename TRC> int GetFrozenCount() { return CBand::kInvalidIndex; }
	template<> int GetFrozenCount<RowTag>() { return m_frozenRowCount; }
	template<> int GetFrozenCount<ColTag>() { return m_frozenColumnCount; }

	template<typename TRC, typename TAV, typename TIP>
	auto Begin()->decltype(GetContainer<TRC, TAV>().begin())
	{
		return GetContainer<TRC, TAV>().begin();
	}

	template<typename TRC, typename TAV, typename TIP>
	auto End()->decltype(GetContainer<TRC, TAV>().end())
	{
		return GetContainer<TRC, TAV>().end();
	}

	template<typename TRC, typename TAV>
	auto CellBegin()->decltype(GetContainer <TRC, TAV>().end())
	{
		return std::next(GetContainer<TRC, TAV>().begin(), GetForzenCount<TRC>());
	}

	template<typename TRC, typename TAV> int Size()
	{
		return GetContainer<TRC, TAV>().size();
	}

	template<typename TAV> std::shared_ptr<CCell> Cell(int row, int col)
	{
		auto pRow = Index2Pointer<RowTag, TAV>(row);
		auto pCol = Index2Pointer<ColTag, TAV>(col);
		if (pRow && pCol) {
			return CSheet::Cell(pRow, pCol);
		}
		else {
			return std::shared_ptr<CCell>();
		}
	}


	template<typename TRC> TRC::template SharedPtr Coordinate2Pointer(FLOAT coordinate)
	{
		auto ptOrigin = GetOriginPoint();

		auto& visContainer = GetContainer<TRC, VisTag>();
		auto iter = std::upper_bound(visContainer.begin(), visContainer.end(), coordinate,
			[this, ptOrigin](const FLOAT& c, const TRC::SharedPtr& ptr)->bool {
				if (ptr->GetIndex<VisTag>() >= GetFrozenCount<TRC>()) {
					return c < (std::max)(Point2Coordinate<TRC>(ptOrigin), GetLeftTop(ptr));
				} else {
					return c < GetLeftTop(ptr);
				}
			});

		auto prevIter = boost::prior(iter);

		if (iter == visContainer.begin() || (iter == visContainer.end() && coordinate > GetRightBottom(*prevIter))) {
			return nullptr;
		} else {
			--iter;
		}
		return *iter;
	}

	template<typename TRC> int Coordinate2Index(FLOAT coordinate)
	{
		auto ptOrigin = GetOriginPoint();

		auto& visContainer = GetContainer<TRC, VisTag>();
		auto iter = std::lower_bound(visContainer.begin(), visContainer.end(), coordinate,
			[this, ptOrigin](const typename TRC::SharedPtr& ptr, const FLOAT& rhs)->bool {
				if (ptr->GetIndex<VisTag>() >= 0) {
					return (std::max)(Point2Coordinate<TRC>(ptOrigin), GetRightBottom(ptr)) < rhs;
				} else {
					return GetRightBottom(ptr) < rhs;
				}
			});
		int index = 0;
		if (iter == visContainer.end()) {
			index = visContainer.size();
		} else if (iter == visContainer.begin() && GetLeftTop(*iter) > coordinate) {
			index = (*iter)->GetIndex<VisTag>() - 1;
		} else {
			index = (*iter)->GetIndex<VisTag>();
		}
		return index;
	}
	template<typename TRC> int Point2Index(const d2dw::CPointF& pt)
	{
		return Coordinate2Index<TRC>(Point2Coordinate<TRC>(pt));
	}

	std::pair<int, int> Point2Indexes(d2dw::CPointF pt)
	{
		return std::make_pair(Coordinate2Index<RowTag>(pt.y), Coordinate2Index<ColTag>(pt.x));
	}
	template<typename TRC, typename TAV> TRC::template SharedPtr LastPointer()
	{
		return GetContainer<TRC, TAV>().back();
	}
	template<typename TRC, typename TAV> TRC::template SharedPtr CellFirstPointer()
	{
		return GetContainer<TRC, TAV>()[GetFrozenCount<TRC>()];
	}
	template<typename TRC, typename TAV> TRC::template SharedPtr FirstPointer()
	{
		return GetContainer<TRC, TAV>().front();
	}

	template<typename TRC, typename TAV> int Pointer2Index(TRC::template Ptr pointer)
	{
		auto& container = GetContainer<TRC, TAV>();

		auto iter = container.find(pointer);
		if (iter != container.end()) {
			return iter->GetIndex<TAV>();
		}
		else {
			throw std::exception("Sheet::Pointer2Index");
		}
	}

	template<typename TRC, typename TAV> TRC::template SharedPtr Index2Pointer(int index) 
	{ 
		auto& container = GetContainer<TRC, TAV>();

		if (0 <= index && index < container.size()) {
			return container[index];
		} else {
			return nullptr;
		}
	}

	template<typename TRC, typename TAV> int GetMaxIndex()
	{
		return GetContainer<TRC, TAV>().size();
	}

	template<typename TRC, typename TAV> int GetMinIndex()
	{
		return 0;
	}

	template<typename TRC, typename TAV> std::pair<int, int> GetMinMaxIndexes()
	{ 
		return std::make_pair(GetMinIndex<TRC, TAV>(), GetMaxIndex<TRC, TAV>());
	}

	template<typename TRC> int Vis2AllIndex(int index)
	{
		auto p = Index2Pointer<TRC, VisTag>(index);
		return p->GetIndex<AllTag>();
	}

	template<typename TRC> int All2VisIndex(int index)
	{
		auto p = Index2Pointer<TRC, AllTag>(index);
		return Pointer2Index<TRC, VisTag>();
	}

	template<typename TRC> void FitBandWidth(TRC::template SharedPtr& ptr)
	{
		ptr->SetMeasureValid(false);
		SetWidthHeightWithoutSignal(ptr, 0.0f);
		auto& otherContainer = GetContainer<TRC::Other, AllTag>();
		for(const auto& otherPtr : otherContainer) {
			otherPtr->SetMeasureValid(false);
			CSheet::Cell(ptr, otherPtr)->SetActMeasureValid(false);
		};

		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	}

	template<typename TRC> void Moved(CMovedEventArgs<TRC>& e) {  }
	template<> inline void CSheet::Moved<RowTag>(CMovedEventArgs<RowTag>& e)
	{
		RowMoved(e);
	}

	template<> inline void CSheet::Moved<ColTag>(CMovedEventArgs<ColTag>& e)
	{
		ColumnMoved(e);
	}

	template<typename TRC> void Track(TRC::template SharedPtr& ptr)
	{
		PostUpdate(Updates::Column);//TODO
		PostUpdate(Updates::Row);//TODO
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	}
	template<typename TRC> void EndTrack(TRC::template SharedPtr& ptr)
	{
		auto& otherContainer = GetContainer<TRC::Other, AllTag>();
		for (const auto& otherPtr : otherContainer) {
			otherPtr->SetMeasureValid(false);
			CSheet::Cell(ptr, otherPtr)->OnPropertyChanged(L"size");
		}

		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	}
	template<typename TRC> void FitWidth(TRC::template SharedPtr& e) {  }

	template<typename TRC>
	void MoveImpl(int indexTo, TRC::template SharedPtr spFrom)
	{
		int from = spFrom->GetIndex<AllTag>();
		int to = indexTo;
		if (from >= 0 && to >= 0 && to>from) {
			to--;
		}
		if (from<0 && to<0 && to<from) {
			to++;
		}

		EraseImpl<TRC>(spFrom);
		InsertImpl<TRC, AllTag>(to, spFrom);

		TRC::Ptr p = nullptr;
		Moved<TRC>(CMovedEventArgs<TRC>(p, from, to));

		//ColumnMoved(CColumnMovedEventArgs(nullptr, from, to));//TODO change to tag dispatch
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Row);

		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::Column);
		//PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);//
	}

	template<typename TRC, typename TAV = AllTag>
	void EraseImpl(TRC::template SharedPtr erasePtr)
	{
		auto& container = GetContainer<TRC, TAV>();
		auto iter = std::find(container.begin(), container.end(), erasePtr);
		if (iter != container.end()) {
			container.notify_erase(iter);
		}
		//auto& ptrDictionary = GetContainer<TRC, AllTag>().get<PointerTag>();
		//auto& idxDictionary = GetContainer<TRC, AllTag>().get<IndexTag>();
		//auto iterPtr = ptrDictionary.find(erasePtr.get());
		//auto iterIdx = idxDictionary.find(iterPtr->Index);
		//auto eraseIdx = iterPtr->Index;
		//if (eraseIdx >= 0) {
		//	//slide left
		//	auto iter = boost::next(iterIdx);
		//	auto end = idxDictionary.end();
		//	idxDictionary.erase(iterIdx);
		//	for (; iter != end; ++iter) {
		//		auto newdata = *iter;
		//		newdata.Index--;
		//		idxDictionary.replace(iter, newdata);
		//	}
		//}
		//else {
		//	//slide right
		//	auto iter = boost::prior(iterIdx);
		//	auto end = boost::prior(idxDictionary.begin());
		//	idxDictionary.erase(iterIdx);
		//	for (; iter != end; --iter) {
		//		auto newdata = *iter;
		//		newdata.Index++;
		//		idxDictionary.replace(iter, newdata);
		//	}
		//}
	}

	template<typename TRC, typename TAV = AllTag>
	void InsertImpl(int index, const TRC::template SharedPtr& pInsert)
	{
		auto& container = GetContainer<TRC, TAV>();
		container.notify_insert(std::next(container.begin(), index), pInsert);
		//auto& dict = GetContainer<TRC, TAV>();
		//auto& ptrDict = dict.get<PointerTag>();
		//auto& idxDict = dict.get<IndexTag>();

		//if (index >= 0) {//Plus
		//	auto max = GetMaxIndex<TRC, AllTag>();
		//	if (max == CBand::kInvalidIndex) {
		//		index = 0;
		//	}else if (index > max) {
		//		index = max + 1;
		//	}
		//	//Slide right
		//	auto iterTo = idxDict.find(index);
		//	if (iterTo != idxDict.end()) {
		//		auto iter = idxDict.end(); iter--;
		//		auto end = iterTo; end--;
		//		for (; iter != end; --iter) {
		//			auto newdata = *iter;
		//			newdata.Index++;
		//			idxDict.replace(iter, newdata);
		//		}
		//	}
		//}
		//else {//Minus
		//	int min = (std::min)(GetMinIndex<TRC, TAV>(), -1);
		//	if (min == CBand::kInvalidIndex) {
		//		index = -1;
		//	}else if (index < min) {
		//		index = min -1;
		//	}
		//	//Slide left
		//	auto iterTo = idxDict.find(index);
		//	if (iterTo != idxDict.end()) {
		//		auto iter = idxDict.begin();
		//		auto end = iterTo; end++;
		//		for (; iter != end; ++iter) {
		//			auto newdata = *iter;
		//			newdata.Index--;
		//			idxDict.replace(iter, newdata);
		//		}
		//	}
		//}
		////Insert
		//dict.insert(TRC::Data(index, pInsert));
	}
	
	virtual void SelectRange(std::shared_ptr<CCell>& cell1, std::shared_ptr<CCell>& cell2, bool doSelect);
	template<class T>
	void SelectBandRange(T* pBand1, T* pBand2, bool doSelect)
	{
		if (!pBand1 || !pBand2)return;
		auto& dict = GetContainer<T::Tag, VisTag>()();
		auto idx1 = pBand1->GetIndex<VisTag>();
		auto idx2 = pBand2->GetIndex<VisTag>();
		auto beg = (std::min)(idx1, idx2);
		auto last = (std::max)(idx1, idx2);
		for (auto iter = dict.find(beg), end = dict.find(last + 1); iter != end; ++iter) {
			iter->DataPtr->SetSelected(doSelect);
		}
	}
	


/*
 *  Template function
 */
protected:
	template<class T, class U>
	void Erase(T& container, U erasePtr)
	{
		auto iter = std::find(container.begin(), container.end(), erasePtr);
		if (iter != container.end()) {
			container.notify_erase(iter);
		}

		//auto& ptrDictionary = dictionary.get<PointerTag>();
		//auto& idxDictionary = dictionary.get<IndexTag>();
		//auto iterPtr=ptrDictionary.find(erasePtr);
		//auto iterIdx=idxDictionary.find(iterPtr->Index);
		//auto eraseIdx=iterPtr->Index;
		//if(eraseIdx>=0){
		//	//slide left
		//	auto iter=boost::next(iterIdx);
		//	auto end=idxDictionary.end();
		//	idxDictionary.erase(iterIdx);
		//	for(;iter!=end;++iter){
		//		auto newdata=*iter;
		//		newdata.Index--;
		//		idxDictionary.replace(iter,newdata);
		//	}	
		//}else{
		//	//slide right
		//	auto iter=boost::prior(iterIdx);
		//	auto end=boost::prior(idxDictionary.begin());
		//	idxDictionary.erase(iterIdx);
		//	for(;iter!=end;--iter){
		//		auto newdata=*iter;
		//		newdata.Index++;
		//		idxDictionary.replace(iter,newdata);
		//	}	

		//}
	}

	template<class T, class U>
	void EraseSome(T& container, std::vector<U> erasePtrs)
	{
		//auto& ptrDictionary = dictionary.get<PointerTag>();
		//auto& idxDictionary = dictionary.get<IndexTag>();

		for(auto& erasePtr : erasePtrs){
			Erase<T, U>(container, erasePtr);
			//auto iterPtr=ptrDictionary.find(erasePtr);
			//auto iterIdx=idxDictionary.find(iterPtr->Index);
			//auto eraseIdx=iterPtr->Index;
			//if(eraseIdx>=0){
			//	//slide left
			//	auto iter=boost::next(iterIdx);
			//	auto end=idxDictionary.end();
			//	idxDictionary.erase(iterIdx);
			//	for(;iter!=end;++iter){
			//		auto newdata=*iter;
			//		newdata.Index--;
			//		idxDictionary.replace(iter,newdata);
			//	}	
			//}else{
			//	//slide right
			//	auto iter=boost::prior(iterIdx);
			//	auto end=boost::prior(idxDictionary.begin());
			//	idxDictionary.erase(iterIdx);
			//	for(;iter!=end;--iter){
			//		auto newdata=*iter;
			//		newdata.Index++;
			//		idxDictionary.replace(iter,newdata);
			//	}	

			//}
		}
	}

	template<class T>
	void UpdateVisibleDictionary(T& allContainer, T& visContainer)
	{
		visContainer.notify_clear();
		for (auto ptr : allContainer) {
			if (ptr->GetVisible()) {
				visContainer.notify_push_back(ptr);
			}
		}
		//auto& dispDictionary = orderDictionary.get<IndexTag>();

		////Insert Minus Elements
		//{
		//if(dispDictionary.find(-1)!=dispDictionary.end()){
		//	int newdisp=-1;
		//	//In case of reverse iterator one iterator plused. therefore it is necessary to minul.
		//	for(auto iter=boost::prior(boost::make_reverse_iterator(dispDictionary.find(-1))),end=dispDictionary.rend();iter!=end;++iter){
		//		if(iter->DataPtr->GetVisible()){
		//			visibleDictionary.insert(T::value_type(newdisp,iter->DataPtr));
		//			newdisp--;
		//		}
		//	}
		//}
		//}

		////Insert Plus Elements
		//{
		//int newdisp=0;
		//for(auto iter=dispDictionary.find(0),end=dispDictionary.end();iter!=end;++iter){
		//	if(iter->DataPtr->GetVisible()){
		//		visibleDictionary.insert(T::value_type(newdisp,iter->DataPtr));
		//		newdisp++;
		//	}
		//}
		//}
	}

	template<class TRC>
	void UpdatePaintContainer(FLOAT pageFirst, FLOAT pageLast)
	{
		auto& pntContainer = GetContainer<TRC, PntTag>();
		auto& visContainer = GetContainer<TRC, VisTag>();
		pntContainer.notify_clear();

		//Copy FrozenColumns
		auto cellBegin = std::next(visContainer.begin(), GetFrozenCount<TRC>());
		for (auto iter = visContainer.begin(); iter != cellBegin ; ++iter) {
			pntContainer.notify_push_back(*iter);
		}
		//Find Displayed Plus Elements
		auto beginIter=std::upper_bound(cellBegin, visContainer.end(), pageFirst,
			[this](const FLOAT& x, const auto& ptr)->bool { return x < GetLeftTop(ptr); });
		if(beginIter != cellBegin){
			--beginIter;
		}
		auto endIter = std::lower_bound(cellBegin, visContainer.end(), pageLast,
			[this](const auto& ptr, const FLOAT& x)->bool { return x > GetRightBottom(ptr); });
		if(endIter != visContainer.end()){
			++endIter;
		}
		for (auto iter = beginIter; iter != endIter; ++iter) {
			pntContainer.notify_push_back(*iter);
		}
	}
};

//
//template<> inline int CSheet::Coordinate2Index<RowTag, AllTag>(FLOAT coordinate)
//{
//	auto ptOrigin = GetOriginPoint();
//
//	auto& dictionary = GetContainer<RowTag, AllTag>().get<IndexTag>();
//	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
//		[ptOrigin](const RowData& rowData, const FLOAT& rhs)->bool {
//		if (rowData.Index >= 0) {
//			return (std::max)(ptOrigin.y, rowData.DataPtr->GetBottom()) < rhs;
//		}
//		else {
//			return rowData.DataPtr->GetBottom() < rhs;
//		}
//	});
//	int index = 0;
//	if (rowIter == dictionary.end()) {
//		index = boost::prior(rowIter)->Index + 1;
//	}
//	else if (rowIter == dictionary.begin() && rowIter->DataPtr->GetTop()>coordinate) {
//		index = rowIter->Index - 1;
//	}
//	else {
//		index = rowIter->Index;
//	}
//	return index;
//}
//
//template<> inline int CSheet::Coordinate2Index<RowTag, VisTag>(FLOAT coordinate)
//{
//	auto ptOrigin = GetOriginPoint();
//
//	auto& dictionary = GetContainer<RowTag, VisTag>().get<IndexTag>();
//	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
//		[ptOrigin](const RowData& rowData, const FLOAT& rhs)->bool {
//		if (rowData.Index >= 0) {
//			return (std::max)(ptOrigin.y, rowData.DataPtr->GetBottom()) < rhs;
//		}
//		else {
//			return rowData.DataPtr->GetBottom() < rhs;
//		}
//	});
//	int index = 0;
//	if (rowIter == dictionary.end()) {
//		index = boost::prior(rowIter)->Index + 1;
//	}
//	else if (rowIter == dictionary.begin() && rowIter->DataPtr->GetTop()>coordinate) {
//		index = rowIter->Index - 1;
//	}
//	else {
//		index = rowIter->Index;
//	}
//	return index;
//}
//
//template<> inline int CSheet::Coordinate2Index<ColTag, VisTag>(FLOAT coordinate)
//{
//	auto ptOrigin = GetOriginPoint();
//
//	auto& dictionary = GetContainer<ColTag, VisTag>().get<IndexTag>();
//	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
//		[ptOrigin](const ColumnData& rowData, const FLOAT& rhs)->bool {
//		if (rowData.Index >= 0) {
//			return (std::max)(ptOrigin.x, rowData.DataPtr->GetRight()) < rhs;
//		}
//		else {
//			return rowData.DataPtr->GetRight() < rhs;
//		}
//	});
//	int index = 0;
//	if (rowIter == dictionary.end()) {
//		index = boost::prior(rowIter)->Index + 1;
//	}
//	else if (rowIter == dictionary.begin() && rowIter->DataPtr->GetLeft()>coordinate) {
//		index = rowIter->Index - 1;
//	}
//	else {
//		index = rowIter->Index;
//	}
//	return index;
//}
//
//template<> inline int CSheet::Coordinate2Index<ColTag, AllTag>(FLOAT coordinate)
//{
//	auto ptOrigin = GetOriginPoint();
//
//	auto& dictionary = GetContainer<ColTag, AllTag>().get<IndexTag>();
//	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
//		[ptOrigin](const ColumnData& rowData, const FLOAT& rhs)->bool {
//		if (rowData.Index >= 0) {
//			return (std::max)(ptOrigin.x, rowData.DataPtr->GetRight()) < rhs;
//		}
//		else {
//			return rowData.DataPtr->GetRight() < rhs;
//		}
//	});
//	int index = 0;
//	if (rowIter == dictionary.end()) {
//		index = boost::prior(rowIter)->Index + 1;
//	}
//	else if (rowIter == dictionary.begin() && rowIter->DataPtr->GetLeft()>coordinate) {
//		index = rowIter->Index - 1;
//	}
//	else {
//		index = rowIter->Index;
//	}
//	return index;
//}





#ifdef _DEBUG
//#define _DEBUG_SHEET
#endif

#ifdef _DEBUG_SHEET
#define	DEBUG_OUTPUT_COLUMN_ALL_DICTIONARY \
		::OutputDebugStringA("m_columnAllDictionary\r\n");\
		boost::range::for_each(m_columnAllDictionary,[](const ColumnData& data){\
			::OutputDebugStringA((boost::format("Display:%1%, Pointer:%2%\r\n")%data.Index%data.DataPtr.get()).str().c_str());\
		});
#define	DEBUG_OUTPUT_ROW_ALL_DICTIONARY \
	::OutputDebugStringA("m_rowAllDictionary\r\n");\
	boost::range::for_each(m_rowAllDictionary,[](const RowData& data){\
	::OutputDebugStringA((boost::format("Display:%1%, Pointer:%2%\r\n")%data.Index%data.DataPtr.get()).str().c_str());\
	});
#define	DEBUG_OUTPUT_COLUMN_VISIBLE_DICTIONARY \
	std::cout<<"m_columnVisibleDictionary\r\n";\
	boost::range::for_each(m_columnVisibleDictionary,[](const ColumnData& data){\
		std::cout<<(boost::format("Display:%1%, Pointer:%2%\r\n")%data.Index%data.DataPtr.get()).str();\
	});
#define	DEBUG_OUTPUT_ROW_VISIBLE_DICTIONARY \
	::OutputDebugStringA("m_rowVisibleDictionary\r\n");\
	boost::range::for_each(m_rowVisibleDictionary,[](const RowData& data){\
	::OutputDebugStringA((boost::format("Display:%1%, Pointer:%2%\r\n")%data.Index%data.DataPtr.get()).str().c_str());\
	});
#define	DEBUG_OUTPUT_COLUMN_PAINT_DICTIONARY \
	std::cout<<"m_columnPaintDictionary\r\n";\
	boost::range::for_each(m_columnPaintDictionary,[](const ColumnData& data){\
		std::cout<<(boost::format("Index:%1%, Pointer:%2%\r\n")%data.Index%data.DataPtr.get()).str();\
	});
#define	DEBUG_OUTPUT_ROW_PAINT_DICTIONARY \
	std::cout<<"m_rowPaintDictionary\r\n";\
	boost::range::for_each(m_rowPaintDictionary,[](const RowData& data){\
		std::cout<<(boost::format("Display:%1%, Pointer:%2%\r\n")%data.Index%data.DataPtr.get()).str();\
	});
#define DEBUG_OUTPUT_ROWCOLUMN(roco) \
	::OutputDebugStringA((boost::format("Row:%1%, Column:%2%\r\n")%roco.GetRowPtr()->GetIndex<VisTag>()%roco.GetColumnPtr()->GetIndex<VisTag>()).str().c_str());

#else
#define DEBUG_OUTPUT_COLUMN_ALL_DICTIONARY
#define	DEBUG_OUTPUT_ROW_ALL_DICTIONARY
#define DEBUG_OUTPUT_COLUMN_VISIBLE_DICTIONARY
#define	DEBUG_OUTPUT_ROW_VISIBLE_DICTIONARY
#define DEBUG_OUTPUT_COLUMN_PAINT_DICTIONARY
#define	DEBUG_OUTPUT_ROW_PAINR_DICTIONARY
#define DEBUG_OUTPUT_ROWCOLUMN(roco)
#endif