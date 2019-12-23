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


struct CSheetStateMachine;
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

struct RowTag
{
	typedef std::shared_ptr<CRow> SharedPtr;
	typedef const CRow* Ptr;
	typedef RowDictionary Dictionary;
	typedef YTag Axis;
	typedef XTag OtherAxis;
	typedef ColTag Other;
	typedef RowData Data;
};

struct ColTag
{
	typedef std::shared_ptr<CColumn> SharedPtr;
	typedef const CColumn* Ptr;
	typedef ColumnDictionary Dictionary;
	typedef XTag Axis;
	typedef YTag OtherAxis;
	typedef RowTag Other;
	typedef ColumnData Data;
};

struct AllTag
{};

struct VisTag
{};

struct RC
{
	RC(int row, int col) :Row(row), Col(col) {}
	int Row;
	int Col;
	template<typename TRC> int Get() const { return CBand::kInvalidIndex; }
};
template<> inline int RC::Get<RowTag>() const { return Row; }
template<> inline int RC::Get<ColTag>() const { return Col; }

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
	friend class CFileDragger;
	friend class CSerializeData;
public:
	//static cell Accessor
	static std::shared_ptr<CCell>& Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn);
	static std::shared_ptr<CCell>& Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow);
	static std::shared_ptr<CCell>& Cell( CRow* pRow,  CColumn* pColumn);

public:
	std::shared_ptr<ITracker> m_spRowTracker; // Tracker for Row
	std::shared_ptr<ITracker> m_spColTracker; // Tracker for Column
	std::shared_ptr<IDragger> m_spRowDragger; // Dragger for Row
	std::shared_ptr<IDragger> m_spColDragger; // Dragger for Column
	std::shared_ptr<IDragger> m_spItemDragger; // Dragger for Item
	std::shared_ptr<ICeller> m_spCeller; //Celler
	std::shared_ptr<CCursorer> m_spCursorer; // Cursor
private:
	std::shared_ptr<CSheetStateMachine> m_spStateMachine;

protected:
	std::set<Updates> m_setUpdate; // Set posted update

	RowDictionary m_rowAllDictionary; // Index-Pointer All Row Dictionary
	RowDictionary m_rowVisibleDictionary; // Index-Pointer Visible Row Dictionary
	RowDictionary m_rowPaintDictionary; // Index-Pointer Paint Row Dictionary

	ColumnDictionary m_columnAllDictionary; // Index-Pointer All Column Dictionary
	ColumnDictionary m_columnVisibleDictionary; // Index-Pointer Visible Column Dictionary
	ColumnDictionary m_columnPaintDictionary; // Index-Pointer Paint Column Dictionary

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
	virtual ~CSheet(){}

	//Getter Setter
	std::shared_ptr<CSheetStateMachine> GetSheetStateMachine() { return m_spStateMachine; }
	void SetSheetStateMachine(std::shared_ptr<CSheetStateMachine>& machine) { m_spStateMachine = machine; }
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

	virtual void EraseColumn(std::shared_ptr<CColumn> spColumn){EraseColumnImpl(spColumn);}
	virtual void EraseColumnImpl(std::shared_ptr<CColumn> spColumn);
	virtual void EraseRow(CRow* pRow);
	virtual void EraseRowNotify(CRow* pRow, bool notify = true);	virtual void EraseRows(const std::vector<CRow*>& vpRow);

	virtual void MoveColumn(int colTo, std::shared_ptr<CColumn> spFromColumn){MoveImpl<ColTag>(colTo, spFromColumn);}

	virtual void InsertRow(int rowVisib, std::shared_ptr<CRow> pRow);
	virtual void InsertRowNotify(int rowVisib, std::shared_ptr<CRow> pRow, bool notify = true);
	virtual void InsertColumn(int colTo, std::shared_ptr<CColumn> pColumn);
	virtual void InsertColumnNotify(int colTo, std::shared_ptr<CColumn> pColumn, bool notify = true);

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
	//Event handler
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnPaintAll(const PaintEvent& e);
	virtual void OnRButtonDown(const RButtonDownEvent& e);

	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonClk(const LButtonClkEvent& e);
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e);
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e);
	virtual void OnBkGndLButtondDblClk(const LButtonDblClkEvent& e) {}
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e);
	//virtual void OnLButtonEndDrag(MouseEventArgs& e);
	virtual void OnContextMenu(const ContextMenuEvent& e);
	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseLeave(const MouseLeaveEvent& e);
	virtual void OnSetCursor(const SetCursorEvent& e);
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnKillFocus(const KillFocusEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e);

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

	Compares CheckEqualRange(RowDictionary::iterator rowBegin, RowDictionary::iterator rowEnd, ColumnDictionary::iterator colBegin, ColumnDictionary::iterator colEnd, std::function<void(CCell*, Compares)> action);
	Compares CheckEqualRow(CRow* pRow, ColumnDictionary::iterator colBegin, ColumnDictionary::iterator colEnd, std::function<void(CCell*, Compares)> action);

	//Menu
	CMenu* const m_pContextMenu;
	virtual CMenu* GetContextMenuPtr() { return m_pContextMenu; }

	//Tag dispatch
	template<typename TRC, typename TAV> TRC::template Dictionary& GetDictionary() { return nullptr; }

	template<typename TRC, typename TAV, typename TIP>
	auto Begin()->decltype(GetDictionary <TRC, TAV>().get<TIP>().begin())
	{
		return GetDictionary<TRC, TAV>().get<TIP>().begin();
	}

	template<typename TRC, typename TAV, typename TIP>
	auto End()->decltype(GetDictionary <TRC, TAV>().get<TIP>().end())
	{
		return GetDictionary<TRC, TAV>().get<TIP>().end();
	}

	template<typename TRC, typename TAV>
	auto Zero()->decltype(GetDictionary <TRC, TAV>().get<IndexTag>().begin())
	{
		return GetDictionary<TRC, TAV>().get<IndexTag>().find(0);
	}

	//RowDictionary::index<IndexTag>::type::iterator RowAllBegin()
	//{
	//	auto& dictionary = m_rowAllDictionary.get<IndexTag>();
	//	return dictionary.begin();
	//}

	//RowDictionary::index<IndexTag>::type::iterator RowAllZero()
	//{
	//	auto& dictionary = m_rowAllDictionary.get<IndexTag>();
	//	return dictionary.find(0);
	//}

	//RowDictionary::index<IndexTag>::type::iterator RowAllEnd()
	//{
	//	auto& dictionary = m_rowAllDictionary.get<IndexTag>();
	//	return dictionary.end();
	//}

	//ColumnDictionary::index<IndexTag>::type::iterator ColumnAllBegin()
	//{
	//	auto& dictionary = m_columnAllDictionary.get<IndexTag>();
	//	return dictionary.begin();
	//}

	//ColumnDictionary::index<IndexTag>::type::iterator ColumnAllZero()
	//{
	//	auto& dictionary = m_columnAllDictionary.get<IndexTag>();
	//	return dictionary.find(0);
	//}

	//ColumnDictionary::index<IndexTag>::type::iterator ColumnAllEnd()
	//{
	//	auto& dictionary = m_columnAllDictionary.get<IndexTag>();
	//	return dictionary.end();
	//}

	//ColumnDictionary::index<IndexTag>::type::iterator ColumnVisibleBegin()
	//{
	//	if (m_columnVisibleDictionary.empty()) {
	//		UpdateColumnVisibleDictionary();
	//	}
	//	auto& dictionary = m_columnVisibleDictionary.get<IndexTag>();
	//	return dictionary.begin();
	//}

	//ColumnDictionary::index<IndexTag>::type::iterator ColumnVisibleZero()
	//{
	//	if (m_columnVisibleDictionary.empty()) {
	//		UpdateColumnVisibleDictionary();
	//	}
	//	auto& dictionary = m_columnVisibleDictionary.get<IndexTag>();
	//	return dictionary.find(0);
	//}

	//ColumnDictionary::index<IndexTag>::type::iterator ColumnVisibleEnd()
	//{
	//	if (m_columnVisibleDictionary.empty()) {
	//		UpdateColumnVisibleDictionary();
	//	}
	//	auto& dictionary = m_columnVisibleDictionary.get<IndexTag>();
	//	return dictionary.end();
	//}

	template<typename TRC, typename TAV> int Size()
	{
		return GetDictionary<TRC, TAV>().size();
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

		auto& visDic = GetDictionary<TRC, VisTag>().get<IndexTag>();
		auto iter = std::upper_bound(visDic.begin(), visDic.end(), coordinate,
			[ptOrigin](const FLOAT& c, const TRC::Data & data)->bool {
			if (data.Index >= 0) {
				return c<(std::max)(ptOrigin.Get<TRC::Axis>(), data.DataPtr->GetLeftTop());
			}
			else {
				return c<data.DataPtr->GetLeftTop();
			}
		});

		auto prevIter = boost::prior(iter);

		if (iter == visDic.begin() || (iter == visDic.end() && coordinate > prevIter->DataPtr->GetRightBottom())) {
			return nullptr;
		}
		else {
			--iter;
		}
		return iter->DataPtr;
	}
	template<typename TRC> FLOAT Point2Coordinate(d2dw::CPointF pt) { return CBand::kInvalidIndex; }

	template<typename TRC, typename TAV> int Coordinate2Index(FLOAT coordinate) { return CBand::kInvalidIndex; }
	template<typename TAV> RC Point2Indexes(d2dw::CPointF pt)
	{
		return RC(Coordinate2Index<RowTag, TAV>(pt.y), Coordinate2Index<ColTag, TAV>(pt.x));
	}
	template<typename TAV> std::pair<int, int> Coordinates2Indexes(d2dw::CPointF pt)
	{ 
		return std::make_pair(Coordinate2Index<RowTag, TAV>(pt.y), Coordinate2Index<ColTag, TAV>(pt.x));
	}
	template<typename TRC, typename TAV> TRC::template SharedPtr LastPointer()
	{
		return boost::prior(GetDictionary<TRC, TAV>().end())->DataPtr;
	}
	template<typename TRC, typename TAV> TRC::template SharedPtr ZeroPointer()
	{
		return GetDictionary<TRC, TAV>().find(0)->DataPtr;
	}
	template<typename TRC, typename TAV> TRC::template SharedPtr FirstPointer()
	{
		return GetDictionary<TRC, TAV>().begin()->DataPtr;
	}

	template<typename TRC, typename TAV> int Pointer2Index(TRC::template Ptr pointer)
	{
		auto& dic = GetDictionary<TRC, TAV>().get<PointerTag>();

		auto iter = dic.find(pointer);
		if (iter != dic.end()) {
			return iter->Index;
		}
		else {
			throw std::exception("Sheet::Pointer2Index");
		}
	}

	template<typename TRC, typename TAV> TRC::template SharedPtr Index2Pointer(int index) 
	{ 
		auto& dic = GetDictionary<TRC, TAV>().get<IndexTag>();

		auto iter = dic.find(index);
		if (iter != dic.end()) {
			return iter->DataPtr;
			return iter->DataPtr;
		}
		else {
			return nullptr;
		}
	}

	template<typename TRC, typename TAV> int GetMaxIndex()
	{
		auto& indexDictionary = GetDictionary<TRC, TAV>().get<IndexTag>();
		if (!indexDictionary.empty()) {
			return boost::prior(indexDictionary.end())->Index;
		}
		else {
			return CBand::kInvalidIndex;
		}
	}

	template<typename TRC, typename TAV> int GetMinIndex()
	{
		auto& indexDictionary = GetDictionary<TRC, TAV>().get<IndexTag>();
		if (!indexDictionary.empty()) {
			return indexDictionary.begin()->Index;
		}
		else {
			return CBand::kInvalidIndex;
		}
	}

	template<typename TRC, typename TAV> std::pair<int, int> GetMinMaxIndexes()
	{ 
		auto& indexDictionary = GetDictionary<TRC, TAV>().get<IndexTag>();
		return std::make_pair(indexDictionary.begin()->Index, boost::prior(indexDictionary.end())->Index);
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
		ptr->SetWidthHeightWithoutSignal(0.0f);
		auto& otherDic = GetDictionary<TRC::Other, AllTag>();
		for(const auto& other : otherDic) {
			other.DataPtr->SetMeasureValid(false);
			CSheet::Cell(ptr, other.DataPtr)->SetActMeasureValid(false);
		};

		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	}

	template<typename TRC> void Moved(CMovedEventArgs<TRC>& e) {  }
	template<typename TRC> void Track(TRC::template SharedPtr& ptr)
	{
		PostUpdate(Updates::Column);//TODO
		PostUpdate(Updates::Row);//TODO
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	}
	template<typename TRC> void EndTrack(TRC::template SharedPtr& ptr)
	{
		auto& otherDic = GetDictionary<TRC::Other, AllTag>();
		for (const auto& other : otherDic) {
			other.DataPtr->SetMeasureValid(false);
			CSheet::Cell(ptr, other.DataPtr)->OnPropertyChanged(L"size");
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

	template<typename TRC>
	void EraseImpl(TRC::template SharedPtr erasePtr)
	{
		auto& ptrDictionary = GetDictionary<TRC, AllTag>().get<PointerTag>();
		auto& idxDictionary = GetDictionary<TRC, AllTag>().get<IndexTag>();
		auto iterPtr = ptrDictionary.find(erasePtr.get());
		auto iterIdx = idxDictionary.find(iterPtr->Index);
		auto eraseIdx = iterPtr->Index;
		if (eraseIdx >= 0) {
			//slide left
			auto iter = boost::next(iterIdx);
			auto end = idxDictionary.end();
			idxDictionary.erase(iterIdx);
			for (; iter != end; ++iter) {
				auto newdata = *iter;
				newdata.Index--;
				idxDictionary.replace(iter, newdata);
			}
		}
		else {
			//slide right
			auto iter = boost::prior(iterIdx);
			auto end = boost::prior(idxDictionary.begin());
			idxDictionary.erase(iterIdx);
			for (; iter != end; --iter) {
				auto newdata = *iter;
				newdata.Index++;
				idxDictionary.replace(iter, newdata);
			}
		}
	}

	template<typename TRC, typename TAV>
	void InsertImpl(int index, TRC::template SharedPtr& pInsert)
	{
		auto& dict = GetDictionary<TRC, TAV>();
		auto& ptrDict = dict.get<PointerTag>();
		auto& idxDict = dict.get<IndexTag>();

		if (index >= 0) {//Plus
			auto max = GetMaxIndex<TRC, AllTag>();
			if (max == CBand::kInvalidIndex) {
				index = 0;
			}else if (index > max) {
				index = max + 1;
			}
			//Slide right
			auto iterTo = idxDict.find(index);
			if (iterTo != idxDict.end()) {
				auto iter = idxDict.end(); iter--;
				auto end = iterTo; end--;
				for (; iter != end; --iter) {
					auto newdata = *iter;
					newdata.Index++;
					idxDict.replace(iter, newdata);
				}
			}
		}
		else {//Minus
			int min = (std::min)(GetMinIndex<TRC, TAV>(), -1);
			if (min == CBand::kInvalidIndex) {
				index = -1;
			}else if (index < min) {
				index = min -1;
			}
			//Slide left
			auto iterTo = idxDict.find(index);
			if (iterTo != idxDict.end()) {
				auto iter = idxDict.begin();
				auto end = iterTo; end++;
				for (; iter != end; ++iter) {
					auto newdata = *iter;
					newdata.Index--;
					idxDict.replace(iter, newdata);
				}
			}
		}
		//Insert
		dict.insert(TRC::Data(index, pInsert));
	}
	
	virtual void SelectRange(std::shared_ptr<CCell>& cell1, std::shared_ptr<CCell>& cell2, bool doSelect);
	template<class T>
	void SelectBandRange(T* pBand1, T* pBand2, bool doSelect)
	{
		if (!pBand1 || !pBand2)return;
		auto& dict = GetDictionary<T::Tag, VisTag>().get<IndexTag>();
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
	void Erase(T& dictionary, U erasePtr)
	{
		auto& ptrDictionary = dictionary.get<PointerTag>();
		auto& idxDictionary = dictionary.get<IndexTag>();
		auto iterPtr=ptrDictionary.find(erasePtr);
		auto iterIdx=idxDictionary.find(iterPtr->Index);
		auto eraseIdx=iterPtr->Index;
		if(eraseIdx>=0){
			//slide left
			auto iter=boost::next(iterIdx);
			auto end=idxDictionary.end();
			idxDictionary.erase(iterIdx);
			for(;iter!=end;++iter){
				auto newdata=*iter;
				newdata.Index--;
				idxDictionary.replace(iter,newdata);
			}	
		}else{
			//slide right
			auto iter=boost::prior(iterIdx);
			auto end=boost::prior(idxDictionary.begin());
			idxDictionary.erase(iterIdx);
			for(;iter!=end;--iter){
				auto newdata=*iter;
				newdata.Index++;
				idxDictionary.replace(iter,newdata);
			}	

		}
	}

	template<class T, class U>
	void EraseSome(T& dictionary, std::vector<U> erasePtrs)
	{
		auto& ptrDictionary = dictionary.get<PointerTag>();
		auto& idxDictionary = dictionary.get<IndexTag>();

		for(auto& erasePtr : erasePtrs){
			auto iterPtr=ptrDictionary.find(erasePtr);
			auto iterIdx=idxDictionary.find(iterPtr->Index);
			auto eraseIdx=iterPtr->Index;
			if(eraseIdx>=0){
				//slide left
				auto iter=boost::next(iterIdx);
				auto end=idxDictionary.end();
				idxDictionary.erase(iterIdx);
				for(;iter!=end;++iter){
					auto newdata=*iter;
					newdata.Index--;
					idxDictionary.replace(iter,newdata);
				}	
			}else{
				//slide right
				auto iter=boost::prior(iterIdx);
				auto end=boost::prior(idxDictionary.begin());
				idxDictionary.erase(iterIdx);
				for(;iter!=end;--iter){
					auto newdata=*iter;
					newdata.Index++;
					idxDictionary.replace(iter,newdata);
				}	

			}
		}
	}

	template<class T>
	void UpdateVisibleDictionary(T& orderDictionary, T& visibleDictionary)
	{
		visibleDictionary.clear();

		auto& dispDictionary = orderDictionary.get<IndexTag>();

		//Insert Minus Elements
		{
		if(dispDictionary.find(-1)!=dispDictionary.end()){
			int newdisp=-1;
			//In case of reverse iterator one iterator plused. therefore it is necessary to minul.
			for(auto iter=boost::prior(boost::make_reverse_iterator(dispDictionary.find(-1))),end=dispDictionary.rend();iter!=end;++iter){
				if(iter->DataPtr->GetVisible()){
					visibleDictionary.insert(T::value_type(newdisp,iter->DataPtr));
					newdisp--;
				}
			}
		}
		}

		//Insert Plus Elements
		{
		int newdisp=0;
		for(auto iter=dispDictionary.find(0),end=dispDictionary.end();iter!=end;++iter){
			if(iter->DataPtr->GetVisible()){
				visibleDictionary.insert(T::value_type(newdisp,iter->DataPtr));
				newdisp++;
			}
		}
		}
	}

	template<class T, class U, class V>
	void UpdatePaintDictionary(T& visibleDictionary, T& paintDictionary, FLOAT pageFirst, FLOAT pageLast, U predFirst, V predLast)
	{
		paintDictionary.clear();

		auto& dispDictionary = visibleDictionary.get<IndexTag>();
		//Copy Minus Elements
		std::copy(dispDictionary.begin(),dispDictionary.find(0),std::inserter(paintDictionary,paintDictionary.end()));
		//Find Displayed Plus Elements
		auto beginIter=std::upper_bound(dispDictionary.find(0),dispDictionary.end(),pageFirst,predFirst);
		if(beginIter!=dispDictionary.find(0)){
			--beginIter;
		}
		auto endIter=std::lower_bound(dispDictionary.find(0),dispDictionary.end(),pageLast,predLast);
		if(endIter!=dispDictionary.end()){
			++endIter;
		}
		std::copy(beginIter,endIter,std::inserter(paintDictionary,paintDictionary.end()));
	}
};

template<> inline RowTag::Dictionary& CSheet::GetDictionary<RowTag, AllTag>()
{
	return m_rowAllDictionary;
}
template<> inline RowTag::Dictionary& CSheet::GetDictionary<RowTag, VisTag>()
{
	return m_rowVisibleDictionary;
}
template<> inline ColTag::Dictionary& CSheet::GetDictionary<ColTag, AllTag>()
{
	return m_columnAllDictionary;
}
template<> inline ColTag::Dictionary& CSheet::GetDictionary<ColTag, VisTag>()
{
	return m_columnVisibleDictionary;
}

template<> inline FLOAT CSheet::Point2Coordinate<RowTag>(d2dw::CPointF pt) { return pt.y; }
template<> inline FLOAT CSheet::Point2Coordinate<ColTag>(d2dw::CPointF pt) { return pt.x; }

template<> inline int CSheet::Coordinate2Index<RowTag, AllTag>(FLOAT coordinate)
{
	auto ptOrigin = GetOriginPoint();

	auto& dictionary = GetDictionary<RowTag, AllTag>().get<IndexTag>();
	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
		[ptOrigin](const RowData& rowData, const FLOAT& rhs)->bool {
		if (rowData.Index >= 0) {
			return (std::max)(ptOrigin.y, rowData.DataPtr->GetBottom()) < rhs;
		}
		else {
			return rowData.DataPtr->GetBottom() < rhs;
		}
	});
	int index = 0;
	if (rowIter == dictionary.end()) {
		index = boost::prior(rowIter)->Index + 1;
	}
	else if (rowIter == dictionary.begin() && rowIter->DataPtr->GetTop()>coordinate) {
		index = rowIter->Index - 1;
	}
	else {
		index = rowIter->Index;
	}
	return index;
}

template<> inline int CSheet::Coordinate2Index<RowTag, VisTag>(FLOAT coordinate)
{
	auto ptOrigin = GetOriginPoint();

	auto& dictionary = GetDictionary<RowTag, VisTag>().get<IndexTag>();
	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
		[ptOrigin](const RowData& rowData, const FLOAT& rhs)->bool {
		if (rowData.Index >= 0) {
			return (std::max)(ptOrigin.y, rowData.DataPtr->GetBottom()) < rhs;
		}
		else {
			return rowData.DataPtr->GetBottom() < rhs;
		}
	});
	int index = 0;
	if (rowIter == dictionary.end()) {
		index = boost::prior(rowIter)->Index + 1;
	}
	else if (rowIter == dictionary.begin() && rowIter->DataPtr->GetTop()>coordinate) {
		index = rowIter->Index - 1;
	}
	else {
		index = rowIter->Index;
	}
	return index;
}

template<> inline int CSheet::Coordinate2Index<ColTag, VisTag>(FLOAT coordinate)
{
	auto ptOrigin = GetOriginPoint();

	auto& dictionary = GetDictionary<ColTag, VisTag>().get<IndexTag>();
	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
		[ptOrigin](const ColumnData& rowData, const FLOAT& rhs)->bool {
		if (rowData.Index >= 0) {
			return (std::max)(ptOrigin.x, rowData.DataPtr->GetRight()) < rhs;
		}
		else {
			return rowData.DataPtr->GetRight() < rhs;
		}
	});
	int index = 0;
	if (rowIter == dictionary.end()) {
		index = boost::prior(rowIter)->Index + 1;
	}
	else if (rowIter == dictionary.begin() && rowIter->DataPtr->GetLeft()>coordinate) {
		index = rowIter->Index - 1;
	}
	else {
		index = rowIter->Index;
	}
	return index;
}

template<> inline int CSheet::Coordinate2Index<ColTag, AllTag>(FLOAT coordinate)
{
	auto ptOrigin = GetOriginPoint();

	auto& dictionary = GetDictionary<ColTag, AllTag>().get<IndexTag>();
	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
		[ptOrigin](const ColumnData& rowData, const FLOAT& rhs)->bool {
		if (rowData.Index >= 0) {
			return (std::max)(ptOrigin.x, rowData.DataPtr->GetRight()) < rhs;
		}
		else {
			return rowData.DataPtr->GetRight() < rhs;
		}
	});
	int index = 0;
	if (rowIter == dictionary.end()) {
		index = boost::prior(rowIter)->Index + 1;
	}
	else if (rowIter == dictionary.begin() && rowIter->DataPtr->GetLeft()>coordinate) {
		index = rowIter->Index - 1;
	}
	else {
		index = rowIter->Index;
	}
	return index;
}


template<> inline void CSheet::Moved<RowTag>(CMovedEventArgs<RowTag>& e)
{
	RowMoved(e);
}

template<> inline void CSheet::Moved<ColTag>(CMovedEventArgs<ColTag>& e)
{
	ColumnMoved(e);
}



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