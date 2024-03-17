#pragma once
#include "D2DWControl.h"
#include "DeadlineTimer.h"

#include "GridViewProperty.h"
#include "MyMenu.h"
#include "SheetEnums.h"
#include "SheetEventArgs.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "RowColumn.h"
#include "index_vector.h"

#include <queue>
#include <boost/sml.hpp>
#include <sigslot/signal.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace sml = boost::sml;

class CDirect2DWrite;
struct BackgroundProperty;
struct GridViewProperty;
class CMouseStateMachine;

class CVScroll;
class CHScroll;
class CCellTextBox;

struct GridViewStateMachine;

struct rowcol_tag {};
struct row_tag {};
struct col_tag {};

using cell_container = boost::multi_index_container <
	std::shared_ptr<CCell>,
	boost::multi_index::indexed_by <

	boost::multi_index::hashed_unique <
	boost::multi_index::tag<rowcol_tag>,
	boost::multi_index::const_mem_fun<CCell, LONGLONG, &CCell::GetRowColumnPtr>>,

	boost::multi_index::hashed_non_unique <
	boost::multi_index::tag<row_tag>,
	boost::multi_index::const_mem_fun<CCell, CRow*, &CCell::GetRowPtr>>,

	boost::multi_index::hashed_non_unique<
	boost::multi_index::tag<col_tag>,
	boost::multi_index::const_mem_fun<CCell, CColumn*, &CCell::GetColumnPtr>>>>;

class IStateMachine;
class IDragger;
class ITracker;
class ICeller;
class CCursorer;
struct HeaderProperty;
struct CellProperty;
class CRect;
class CPoint;
class IMouseObserver;
struct SheetStateMachine;
class CGridView;

class CVScroll;
class CHScroll;


struct Indexes
{
	struct RowIdxTag {};
	struct ColIdxTag {};
	Indexes(const int row, const int col) :Row(row), Col(col) {}
	int Row;
	int Col;
	template<typename TIdx> int Get() const
	{
		if constexpr (std::is_same_v<TIdx, RowIdxTag>) {
			return Row;
		} else if constexpr (std::is_same_v<TIdx, ColIdxTag>) {
			return Col;
		} else {
			static_assert(false_v<TIdx>);
		}

	}
};
struct ColTag;
struct RowTag
{
	using PointTag = CPointF::YTag;
	using IndexesTag = Indexes::RowIdxTag;
	using SharedPtr = std::shared_ptr<CRow>;
	typedef const CRow* Ptr;
	using Container = index_vector<std::shared_ptr<CRow>>;
	using Other = ColTag ;
};

struct ColTag
{
	using PointTag = CPointF::XTag;
	using IndexesTag = Indexes::ColIdxTag;
	using SharedPtr = std::shared_ptr<CColumn>;
	typedef const CColumn* Ptr;
	using Container = index_vector<std::shared_ptr<CColumn>>;
	typedef RowTag Other;
};

enum class GridViewMode
{
	None,
	ExcelLike,
};

/***********/
/* GridView*/
/***********/

class CGridView: public CD2DWControl
{

/**********/
/* Friend */
/**********/
	friend class CMouseStateMachine;
	friend class CFileDragger;
	friend class CSerializeData;
	friend class CCeller;

public:
	std::unique_ptr<CVScroll> m_pVScroll;
	std::unique_ptr<CHScroll> m_pHScroll;
	std::unique_ptr<IStateMachine> m_pMachine;

	std::shared_ptr<ITracker> m_spRowTracker; // Tracker for Row
	std::shared_ptr<ITracker> m_spColTracker; // Tracker for Column
	std::shared_ptr<IDragger> m_spRowDragger; // Dragger for Row
	std::shared_ptr<IDragger> m_spColDragger; // Dragger for Column
	std::shared_ptr<IDragger> m_spItemDragger; // Dragger for Item
	std::shared_ptr<ICeller> m_spCeller; //Celler
	std::shared_ptr<CCursorer> m_spCursorer; // Cursor



	std::set<Updates> m_setUpdate; // Set posted update

	//cell_container m_allCells;

	index_vector<std::shared_ptr<CRow>> m_allRows;
	index_vector<std::shared_ptr<CRow>> m_visRows;
	index_vector<std::shared_ptr<CRow>> m_pntRows;
	size_t m_frozenRowCount = 0;

	index_vector<std::shared_ptr<CColumn>> m_allCols;
	index_vector<std::shared_ptr<CColumn>> m_visCols;
	index_vector<std::shared_ptr<CColumn>> m_pntCols;
	size_t m_frozenColumnCount = 0;

	cell_container m_allCells;


	std::shared_ptr<GridViewProperty> m_spSheetProperty; // SheetProperty

	std::shared_ptr<CColumn> m_pHeaderColumn; // Header column
	std::shared_ptr<CColumn> m_pNameColumn;
	//std::shared_ptr<CRow> m_pHeaderRow; // Header row
	std::shared_ptr<CRow> m_pNameHeaderRow; /**< Name Header row */
	std::shared_ptr<CRow> m_pFilterRow; /**< Filter row */


	bool m_bSelected; // Selected or not
	//bool m_bFocused; // Focused or not


	CRowColumn m_rocoContextMenu; // Store RowColumn of Caller

	//For open mode flag to keep ensurevisiblecell after open
	bool m_keepEnsureVisibleFocusedCell = false;

public:
	//Signals
	sigslot::signal<const CellEventArgs&> CellLButtonDown;
	sigslot::signal<const CellEventArgs&> CellLButtonClk;
	sigslot::signal<const CellEventArgs&> CellLButtonDblClk;
	sigslot::signal<const CellContextMenuEventArgs&> CellContextMenu;

protected:
	std::shared_ptr<CCellTextBox> m_pEdit = nullptr;
	std::shared_ptr<CCell> m_pJumpCell = nullptr;
	CRectF m_rcUpdateRect;
	bool m_isUpdating = false;
	bool m_isEditExiting = false;
	GridViewMode m_gridViewMode = GridViewMode::None;
	std::vector<std::pair<size_t, size_t>> m_frozenTabStops = { std::make_pair(0, 1), std::make_pair(2, 1), std::make_pair(3, 1)};
	//std::vector<std::pair<size_t, size_t>> m_tabStops = { std::make_pair(3, 1) };

protected:
	CDeadlineTimer m_invalidateTimer;

	std::shared_ptr<GridViewProperty> m_spGridViewProp;
public:

	sigslot::signal<> SignalPreDelayUpdate;
	void DelayUpdate();


public:
	/***********/
	/* Default */
	/***********/
	CGridView(
		CD2DWControl* pParentControl,
		const std::shared_ptr<GridViewProperty>& spGridViewProp);
	//Destructor
	virtual ~CGridView();

	/**********/
	/* Update */
	/**********/
	//virtual bool HasSheetCell(){ return false; }
	virtual bool IsVirtualPage() { return false; }
	virtual FLOAT UpdateHeadersRow(FLOAT top);
	virtual FLOAT UpdateCellsRow(FLOAT top, FLOAT pageTop, FLOAT pageBottom);
	virtual void UpdateRow();
	virtual void UpdateColumn();
	virtual void UpdateScrolls();
	virtual void Invalidate();
	virtual void UpdateAll();
	virtual void PostUpdate(Updates type);
	virtual void SubmitUpdate();

	/********/
	/* Cell */
	/********/
	virtual std::shared_ptr<CCell> Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn);
	virtual std::shared_ptr<CCell> Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow);
	virtual std::shared_ptr<CCell> Cell(CRow* pRow, CColumn* pColumn) = 0;
	virtual std::shared_ptr<CCell> Cell(const CPointF& pt);

	/*****************/
	/* Getter Setter */
	/*****************/
	std::shared_ptr<GridViewProperty>& GetGridViewPropPtr() { return m_spGridViewProp; }
	std::shared_ptr<CCellTextBox> GetEditPtr() { return m_pEdit; }
	void SetEditPtr(std::shared_ptr<CCellTextBox> pEdit) { m_pEdit = pEdit; }
	std::shared_ptr<CCursorer> GetCursorerPtr(){return m_spCursorer;}
	void SetContextMenuRowColumn(const CRowColumn& roco){m_rocoContextMenu = roco;}
	virtual std::shared_ptr<SheetProperty> GetSheetProperty() { return m_spSheetProperty; }
	virtual std::shared_ptr<HeaderProperty> GetHeaderProperty(){return m_spSheetProperty->HeaderPropPtr;}
	virtual std::shared_ptr<CellProperty> GetFilterProperty(){return m_spSheetProperty->CellPropPtr;}
	virtual std::shared_ptr<CellProperty> GetCellProperty(){return m_spSheetProperty->CellPropPtr;}
	virtual std::shared_ptr<CColumn> GetHeaderColumnPtr()const{return m_pHeaderColumn;}
	virtual void SetHeaderColumnPtr(std::shared_ptr<CColumn> column){m_pHeaderColumn=column;}
	virtual std::shared_ptr<CColumn> GetNameColumnPtr() { return m_pNameColumn; }
	virtual void SetNameColumnPtr(const std::shared_ptr<CColumn>& spCol) { m_pNameColumn = spCol; }
	virtual std::shared_ptr<CRow> GetNameHeaderRowPtr()const { return m_pNameHeaderRow; }
	virtual void SetNameHeaderRowPtr(std::shared_ptr<CRow> row) { m_pNameHeaderRow = row; }
	virtual std::shared_ptr<CRow> GetFilterRowPtr()const { return m_pFilterRow; }
	virtual void SetFilterRowPtr(std::shared_ptr<CRow> row) { m_pFilterRow = row; }
	virtual bool GetIsSelected()const{return m_bSelected;};
	virtual void SetIsSelected(const bool& bSelected){m_bSelected=bSelected;};

	//Function
	void SetAllRowMeasureValid(bool valid);
	void SetAllRowFitMeasureValid(bool valid);
	void SetAllColumnMeasureValid(bool valid);
	void SetAllColumnFitMeasureValid(bool valid);
	void SetColumnAllCellMeasureValid(CColumn* pColumn, bool valid);

	//Observer
	virtual void ColumnInserted(const CColumnEventArgs& e);
	virtual void ColumnErased(const CColumnEventArgs& e);
	virtual void ColumnMoved(const CMovedEventArgs<ColTag>& e);
	virtual void ColumnHeaderFitWidth(const CColumnEventArgs& e);

	virtual void RowInserted(const CRowEventArgs& e);
	virtual void RowErased(const CRowEventArgs& e);
	virtual void RowsErased(const CRowsEventArgs& e);
	virtual void RowMoved(const CMovedEventArgs<RowTag>& e);

	virtual void SizeChanged();
	virtual void Scroll();

	//Sort
	virtual void ResetColumnSort();

	virtual CPointF GetScrollPos()const;
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

	virtual void InsertRow(int row, const std::shared_ptr<CRow>& spRow, bool notify = true);
	virtual void PushRow(const std::shared_ptr<CRow>& pRow, bool notify = true);
	
	void PushRows(){}
	template<class Head, class... Tail>
	void PushRows(Head&& head, Tail&&... tail)
	{
		PushRow(head, true);
		PushRows(std::forward<Tail>(tail)...);
	}
	
	virtual void InsertColumn(int pos, const std::shared_ptr<CColumn>& spColumn, bool notify = true);
	virtual void PushColumn(const std::shared_ptr<CColumn>& pColumn, bool notify = true);
	void PushColumns() {}
	template<class Head, class... Tail>
	void PushColumns(Head&& head, Tail&&... tail)
	{
		PushColumn(head, true);
		PushColumns(std::forward<Tail>(tail)...);
	}

	virtual FLOAT GetColumnInitWidth(CColumn* pColumn);
	virtual FLOAT GetColumnFitWidth(CColumn* pColumn);

	virtual FLOAT GetRowFitHeight(CRow* pRow);
	virtual FLOAT GetRowHeight(CRow* pRow);

	virtual std::wstring GetSheetString();

	virtual CPointF GetFrozenPoint();
	virtual CSizeF MeasureSize()const;

	virtual FLOAT GetCellsHeight();
	virtual FLOAT GetCellsWidth();
	virtual CRectF GetCellsRect();
	virtual CRectF GetPaintRect();

	/******************/
	/* Window Message */
	/******************/
public:

	virtual void OnFilter();
	virtual void OnDelayUpdate();

	virtual void OnCommandEditHeader(const CommandEvent& e);
	virtual void OnCommandDeleteColumn(const CommandEvent& e);
	//virtual void OnCommandResizeSheetCell(const CommandEvent& e);
	virtual void OnCommandSelectAll(const CommandEvent& e);
	virtual void OnCommandDelete(const CommandEvent& e) { }
	virtual void OnCommandCopy(const CommandEvent& e);
	virtual void OnCommandCut(const CommandEvent& e) { }
	virtual void OnCommandPaste(const CommandEvent& e) { }
	virtual void OnCommandFind(const CommandEvent& e);

	/**************/
	/* UI Message */
	/**************/
	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnRect(const RectEvent& e) override;
	virtual void OnRectWoSubmit(const RectEvent& e);
	virtual	void OnPaint(const PaintEvent& e) override;
	virtual void OnMouseWheel(const MouseWheelEvent& e) override;
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnKillFocus(const KillFocusEvent& e);
	virtual void OnSetCursor(const SetCursorEvent& e);
	virtual void OnContextMenu(const ContextMenuEvent& e);

	virtual void OnRButtonDown(const RButtonDownEvent& e) override;
	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonClk(const LButtonClkEvent& e);
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e);
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e);
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e);
	virtual void OnKeyTraceDown(const KeyTraceDownEvent& e);
	virtual void OnKeyTraceUp(const KeyTraceUpEvent& e) {}
	virtual void OnChar(const CharEvent& e);
	virtual void OnImeStartComposition(const ImeStartCompositionEvent& e);
	virtual void OnBeginEdit(const BeginEditEvent& e);
	virtual void OnEndEdit(const EndEditEvent& e);


	//virtual void OnPaintAll(const PaintEvent& e);
	virtual void OnBkGndLButtondDblClk(const LButtonDblClkEvent& e) {}



	//virtual void OnCommand(const CommandEvent& e);

public:
	std::shared_ptr<CCell> TabNext(const std::shared_ptr<CCell>& spCurCell);
	std::shared_ptr<CCell> TabPrev(const std::shared_ptr<CCell>& spCurCell);
	void BeginEdit(CCell* pCell);
	void EndEdit();
	virtual void ClearFilter();
	virtual void FilterAll();

	virtual void EnsureVisibleCell(const std::shared_ptr<CCell>& pCell);
	void Jump(const std::shared_ptr<CCell>& spCell);
	virtual void Clear();

	virtual std::wstring FullXMLSave(){return std::wstring();}
	virtual void FullXMLLoad(const std::wstring& str){}

public:

	void FindNext(const std::wstring& findWord, bool matchCase, bool matchWholeWord);
	void FindPrev(const std::wstring& findWord, bool matchCase, bool matchWholeWord);

	virtual void SetScrollPos(const CPoint& ptScroll);

	virtual FLOAT GetVerticalScrollPos()const;
	virtual FLOAT GetHorizontalScrollPos()const;
	virtual CRectF GetPageRect();
	std::pair<bool, bool> GetHorizontalVerticalScrollNecessity();
	

	virtual void SortAllInSubmitUpdate();



	//virtual void ColumnErased(CColumnEventArgs& e);
	virtual CColumn* GetParentColumnPtr(CCell* pCell);	

	virtual void Arrange(const CRectF& rc) override;


	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name);
	virtual void OnRowPropertyChanged(CRow* pRow, const wchar_t* name);
	virtual void OnColumnPropertyChanged(CColumn* pCol, const wchar_t* name);

	virtual void SelectAll();
	virtual void DeselectAll();
	virtual void UnhotAll();
	virtual bool IsFocusedCell(const CCell* pCell)const;
	virtual bool IsDoubleFocusedCell(const CCell* pCell)const;
	virtual void Sort(CColumn* pCol, Sorts sort, bool postUpdate = true);
	virtual void Filter(int colDisp, std::function<bool(const std::shared_ptr<CCell>&)> predicate);
	virtual void ResetFilter();


/****************/
/* StateMachine */
/****************/
public:
	//Normal
	virtual void Normal_Paint(const PaintEvent& e);
	virtual void Normal_LButtonDown(const LButtonDownEvent& e);
	virtual void Normal_LButtonUp(const LButtonUpEvent& e);
	virtual void Normal_LButtonClk(const LButtonClkEvent& e) ;
	virtual void Normal_LButtonSnglClk(const LButtonSnglClkEvent& e);
	virtual void Normal_LButtonDblClk(const LButtonDblClkEvent& e);
	virtual void Normal_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void Normal_RButtonDown(const RButtonDownEvent& e);
	virtual void Normal_MouseMove(const MouseMoveEvent& e);
	virtual void Normal_MouseLeave(const MouseLeaveEvent& e);
	virtual bool Normal_Guard_SetCursor(const SetCursorEvent& e);
	virtual void Normal_SetCursor(const SetCursorEvent& e);
	virtual void Normal_ContextMenu(const ContextMenuEvent& e);
	virtual void Normal_KeyDown(const KeyDownEvent& e);
	virtual void Normal_KeyTraceDown(const KeyTraceDownEvent& e);
	virtual void Normal_KeyTraceUp(const KeyTraceUpEvent& e);
	virtual void Normal_Char(const CharEvent& e);
	virtual void Normal_ImeStartComposition(const ImeStartCompositionEvent& e);
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
	virtual void RowDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual bool RowDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void RowDrag_MouseMove(const MouseMoveEvent& e);
	virtual void RowDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void RowDrag_MouseLeave(const MouseLeaveEvent& e);
	//ColDrag
	virtual void ColDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual bool ColDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void ColDrag_MouseMove(const MouseMoveEvent& e);
	virtual void ColDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void ColDrag_MouseLeave(const MouseLeaveEvent& e);
	//ItemDrag
	virtual void ItemDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual bool ItemDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void ItemDrag_MouseMove(const MouseMoveEvent& e);
	virtual void ItemDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void ItemDrag_MouseLeave(const MouseLeaveEvent& e);
	//VScrollDrag
	virtual void VScrlDrag_OnEntry();
	virtual void VScrlDrag_OnExit();
	virtual void VScrlDrag_LButtonDown(const LButtonDownEvent& e);
	virtual bool VScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void VScrlDrag_MouseMove(const MouseMoveEvent& e);
	//HScrollDrag
	virtual void HScrlDrag_OnEntry();
	virtual void HScrlDrag_OnExit();
	virtual void HScrlDrag_LButtonDown(const LButtonDownEvent& e);
	virtual bool HScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void HScrlDrag_MouseMove(const MouseMoveEvent& e);
	//Edit
	//virtual void Edit_BeginEdit(const BeginEditEvent& e);
	//virtual void Edit_EndEdit(const EndEditEvent& e);
	virtual void Edit_OnEntry(const BeginEditEvent& e);
	virtual void Edit_OnExit();
	virtual void Edit_MouseMove(const MouseMoveEvent& e);
	virtual bool Edit_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void Edit_LButtonDown(const LButtonDownEvent& e);
	virtual void Edit_LButtonUp(const LButtonUpEvent& e);
	virtual void Edit_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void Edit_LButtonEndDrag(const LButtonEndDragEvent& e);
	//virtual bool Edit_Guard_KeyDown(const KeyDownEvent& e);
	virtual bool Edit_Guard_KeyDown_ToNormal_Tab(const KeyDownEvent& e, boost::sml::back::process<BeginEditEvent> process);
	virtual bool Edit_Guard_KeyDown_ToNormal(const KeyDownEvent& e);
	virtual void Edit_KeyDown_Tab(const KeyDownEvent& e, boost::sml::back::process<BeginEditEvent> process);
	virtual void Edit_KeyDown(const KeyDownEvent& e);
	virtual void Edit_Char(const CharEvent& e);
	//Error
	virtual void Error_StdException(const std::exception& e);

/************/
/* Template */
/************/
	//Tag dispatch
	template<typename TRC, typename TAV> typename TRC::Container& GetContainer() { return m_allRows; }
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

	template<typename TRC> int GetFrozenCount() 
	{ 
		if constexpr (std::is_same_v<TRC, RowTag>) {
			return m_frozenRowCount;
		} else if constexpr (std::is_same_v<TRC, ColTag>) {
			return m_frozenColumnCount;
		} else {
			static_assert(false_v<TRC>);
		}
	}

	template<typename TRC> void SetFrozenCount(int count)
	{
		if constexpr (std::is_same_v<TRC, RowTag>) {
			m_frozenRowCount = count;
		} else if constexpr (std::is_same_v<TRC, ColTag>) {
			m_frozenColumnCount = count;
		} else {
			static_assert(false_v<TRC>);
		}
	}

	template<typename TAV> std::shared_ptr<CCell> Cell(int row, int col)
	{
		auto pRow = Index2Pointer<RowTag, TAV>(row);
		auto pCol = Index2Pointer<ColTag, TAV>(col);
		if (pRow && pCol) {
			return Cell(pRow, pCol);
		}
		else {
			return std::shared_ptr<CCell>();
		}
	}

	template<typename TRC> typename TRC::SharedPtr Coordinate2Pointer(FLOAT coordinate)
	{
		auto container = GetContainer<TRC, VisTag>();
		int idx = Coordinate2Index<TRC>(coordinate);
		if (0 <= idx && idx < (int)container.size()) {
			return container[idx];
		} else {
			return nullptr;
		}
	}

	template<typename TRC> int Coordinate2Index(FLOAT coordinate)
	{
		auto ptOrigin = GetFrozenPoint();

		auto& visContainer = GetContainer<TRC, VisTag>();
		auto iter = std::lower_bound(visContainer.begin(), visContainer.end(), coordinate,
			[this, ptOrigin](const typename TRC::SharedPtr& ptr, const FLOAT& rhs)->bool {
				if (ptr->GetIndex<VisTag>() >= GetFrozenCount<TRC>()) {
					return (std::max)(ptOrigin.Get<TRC::PointTag>(), ptr->GetEnd()) < rhs;
				} else {
					return ptr->GetEnd() < rhs;
				}
			});

		if (iter == visContainer.end()) {
			return visContainer.size();
		} else if (iter == visContainer.begin() && (*iter)->GetStart() > coordinate) {
			return (*iter)->GetIndex<VisTag>() - 1;
		} else {
			return (*iter)->GetIndex<VisTag>();
		}
	}
	template<typename TRC> int Point2Index(const CPointF& pt)
	{
		return Coordinate2Index<TRC>(pt.Get<TRC::PointTag>());
	}

	Indexes Point2Indexes(CPointF pt)
	{
		return Indexes(Coordinate2Index<RowTag>(pt.y), Coordinate2Index<ColTag>(pt.x));
	}

	template<typename TRC, typename TAV> typename TRC::SharedPtr Index2Pointer(int index) 
	{ 
		auto& container = GetContainer<TRC, TAV>();

		if (0 <= index && index < (int)container.size()) {
			return container[index];
		} else {
			return nullptr;
		}
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
		auto& otherContainer = GetContainer<TRC::Other, AllTag>();
		for(const auto& otherPtr : otherContainer) {
			otherPtr->SetIsMeasureValid(false);
			otherPtr->SetIsFitMeasureValid(false);
			Cell(ptr, otherPtr)->SetActMeasureValid(false);
		};

		ptr->FitLength();
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
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
			otherPtr->SetIsMeasureValid(false);
			Cell(ptr, otherPtr)->OnPropertyChanged(L"size");
		}

		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);

	}

	virtual void MoveRow(int indexTo, typename RowTag::SharedPtr spFrom)
	{
		int from = spFrom->GetIndex<VisTag>();
		int to = indexTo > from ? indexTo - 1 : indexTo;

		auto& container = GetContainer<RowTag, VisTag>();
		auto iter = std::find(container.begin(), container.end(), spFrom);
		if (iter != container.end()) {
			container.idx_erase(iter);
		}
		container.idx_insert(container.cbegin() + to, spFrom);

		RowMoved(CMovedEventArgs<RowTag>(spFrom, from, indexTo));
	}

	virtual void MoveColumn(int indexTo, typename ColTag::SharedPtr spFrom)
	{
		int from = spFrom->GetIndex<VisTag>();
		int to = indexTo > from ? indexTo - 1 : indexTo;
		from = Vis2AllIndex<ColTag>(from);
		to = Vis2AllIndex<ColTag>(to);

		auto& container = GetContainer<ColTag, AllTag>();
		auto iter = std::find(container.begin(), container.end(), spFrom);
		if (iter != container.end()) {
			container.idx_erase(iter);
		}
		container.idx_insert(container.cbegin() + to, spFrom);

		ColumnMoved(CMovedEventArgs<ColTag>(spFrom, from, to));
	}

	template<typename TRC>
	void Move(int indexTo, typename TRC::SharedPtr spFrom)
	{
		MoveRow(indexTo, spFrom);
	}

	template<> inline
	void Move<ColTag>(int indexTo, typename ColTag::SharedPtr spFrom)
	{
		MoveColumn(indexTo, spFrom);
	}


	virtual void SelectRange(std::shared_ptr<CCell>& cell1, std::shared_ptr<CCell>& cell2, bool doSelect);
	template<class TRect>
	void SelectBandRange(TRect* pBand1, TRect* pBand2, bool doSelect)
	{
		if (!pBand1 || !pBand2)return;
		auto& container = GetContainer<TRect::Tag, VisTag>();
		auto idx1 = pBand1->GetIndex<VisTag>();
		auto idx2 = pBand2->GetIndex<VisTag>();
		auto beg = (std::min)(idx1, idx2);
		auto last = (std::max)(idx1, idx2);
		for (auto iter = std::next(container.begin(), beg), end = std::next(container.begin(), last + 1); iter != end; ++iter) {
			(*iter)->SetIsSelected(doSelect);
		}
	}


	template<class TRC>
	void UpdateVisibleContainer()
	{
		auto& allContainer = GetContainer<TRC, AllTag>();
		auto& visContainer = GetContainer<TRC, VisTag>();
		visContainer.clear();
		for (auto ptr : allContainer) {
			if (ptr->GetIsVisible()) {
				visContainer.idx_push_back(ptr);
			}
		}
	}

	template<class TRC>
	void UpdatePaintContainer(FLOAT pageFirst, FLOAT pageLast)
	{
		auto& pntContainer = GetContainer<TRC, PntTag>();
		auto& visContainer = GetContainer<TRC, VisTag>();
		pntContainer.clear();

		//Copy FrozenColumns
		auto cellBegin = std::next(visContainer.begin(), GetFrozenCount<TRC>());
		for (auto iter = visContainer.begin(); iter != cellBegin ; ++iter) {
			pntContainer.idx_push_back(*iter);
		}
		//Find Displayed Plus Elements
		auto beginIter=std::upper_bound(cellBegin, visContainer.end(), pageFirst,
			[this](const FLOAT& x, const auto& ptr)->bool { return x < ptr->GetStart(); });
		if(beginIter != cellBegin){
			--beginIter;
		}
		auto endIter = std::lower_bound(cellBegin, visContainer.end(), pageLast,
			[this](const auto& ptr, const FLOAT& x)->bool { return x > ptr->GetEnd(); });
		if(endIter != visContainer.end()){
			++endIter;
		}
		for (auto iter = beginIter; iter != endIter; ++iter) {
			pntContainer.idx_push_back(*iter);
		}
	}

};

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
