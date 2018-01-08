#pragma once
#include "UIElement.h"
#include "MyMenu.h"
#include "RowColumn.h"
#include "Operation.h"
#include "SheetEnums.h"
#include "SheetDictionary.h"
#include "SheetEventArgs.h"
#include "Row.h"


struct CSheetStateMachine;
class IDragger;
class ITracker;
class CCell;
//class CRow;
class CColumn;
class CCellProperty;
class CRect;
class CPoint;
struct XTag;
struct YTag;
class CDC;
template <typename TRC, typename TRCYou> class CDragger;
class CCursorer;
class IMouseObserver;
class CSerializeData;

struct ColTag;

struct RowTag
{
	typedef std::shared_ptr<CRow> SharedPtr;
	typedef const CRow* Ptr;
	typedef RowDictionary Dictionary;
	typedef YTag Axis;
	typedef ColTag Other;
};
struct ColTag
{
	typedef std::shared_ptr<CColumn> SharedPtr;
	typedef const CColumn* Ptr;
	typedef ColumnDictionary Dictionary;
	typedef XTag Axis;
	typedef RowTag Other;
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

const UINT WM_FILTER = RegisterWindowMessage(L"WM_FILTER");
const UINT WM_EDITCELL = RegisterWindowMessage(L"WM_EDITCELL");
const UINT WM_LBUTTONDBLCLKTIMEXCEED = RegisterWindowMessage(L"WM_LBUTTONDBLCLKTIMEXCEED");
/**
 *  CSheet
 *  Base class for GridView and SheetCell
 */
class CSheet:public CUIElement
{
//Friend classes
friend class CDragger<ColTag, RowTag>;
friend class CFileDragger;//TODO
friend class CCursorer;
friend class CSerializeData;
protected:
	//typedef
	typedef int size_type;
	typedef int coordinates_type;
	typedef std::shared_ptr<CCell> cell_type;
	typedef std::shared_ptr<CColumn> column_type;
	typedef std::shared_ptr<CRow> row_type;
	typedef std::wstring string_type;
public:
	static CMenu ContextMenu; // ContextMenu
public:
	//Simple cell Accessor
	static std::shared_ptr<CCell>& Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn);
	static std::shared_ptr<CCell>& Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow);
	static std::shared_ptr<CCell>& Cell( CRow* pRow,  CColumn* pColumn);

	std::shared_ptr<ITracker> m_spRowTracker; /**< Tracker */
	std::shared_ptr<ITracker> m_spColTracker; /**< Tracker */
	std::shared_ptr<IDragger> m_spRowDragger; /**< Dragger */
	std::shared_ptr<IDragger> m_spColDragger; /**< Dragger */
	std::shared_ptr<IDragger> m_spItemDragger; /**< Dragger */
	std::shared_ptr<CCursorer> m_spCursorer; /**< Cursor */

protected:

	std::shared_ptr<CSheetStateMachine> m_spStateMachine;
	std::vector<std::shared_ptr<IMouseObserver>> m_mouseObservers; /**< Mouse Observers */

	std::set<Updates> m_setUpdate; /**< Set posted update */

	RowDictionary m_rowAllDictionary; /**< Index-Pointer All Row Dictionary */
	RowDictionary m_rowVisibleDictionary; /**< Index-Pointer Visible Row Dictionary */
	RowDictionary m_rowPaintDictionary; /**< Index-Pointer Paint Row Dictionary */

	ColumnDictionary m_columnAllDictionary; /**< Index-Pointer All Column Dictionary */
	ColumnDictionary m_columnVisibleDictionary; /**< Index-Pointer Visible Column Dictionary */
	ColumnDictionary m_columnPaintDictionary; /**< Index-Pointer Paint Column Dictionary */

	std::shared_ptr<CCellProperty> m_spHeaderProperty; /**< HeaderProperty */
	std::shared_ptr<CCellProperty> m_spFilterProperty; /**< FilterProperty */
	std::shared_ptr<CCellProperty> m_spCellProperty; /**< CellProperty */

	column_type m_pHeaderColumn; /**< Header column */

	row_type m_rowHeader; /**< Header row */

	bool m_bSelected; /**< Selected or not */
	bool m_bFocused; /**< Focused or not */

	CRowColumn m_rocoContextMenu; /**< Store RowColumn of Caller */
	CRowColumn m_rocoMouse; /** RowColumn where mouse is */
public:
	//Signals
	boost::signals2::signal<void(CellEventArgs&)> CellLButtonDown;
	boost::signals2::signal<void(CellEventArgs&)> CellLButtonClk;
	boost::signals2::signal<void(CellEventArgs&)> CellLButtonDblClk;
	boost::signals2::signal<void(CellContextMenuEventArgs&)> CellContextMenu;

	boost::signals2::signal<void(CCell*,LPCTSTR)> CellPropertyChanged;

	boost::signals2::signal<void(LPCTSTR)> ColumnPropertyChanged;
	boost::signals2::signal<void(LPCTSTR)> RowPropertyChanged;

	//Constructor
	CSheet(std::shared_ptr<CCellProperty> spHeaderProperty,
		std::shared_ptr<CCellProperty> spFilterProperty,
		std::shared_ptr<CCellProperty> spCellProperty,
		CMenu* pContextMenu= &CSheet::ContextMenu);
	//Destructor
	virtual ~CSheet(){}
	//Getter Setter
	std::shared_ptr<CSheetStateMachine> GetSheetStateMachine() { return m_spStateMachine; }
	std::shared_ptr<CCursorer> GetCursorerPtr(){return m_spCursorer;} /**< Cursor */
	CRowColumn GetMouseRowColumn(){return m_rocoMouse;} /** Getter for Mouse CRowColumn */
	void SetMouseRowColumn(CRowColumn roco){m_rocoMouse = roco;} /** Setter for Mouse CRowColumn */

	void SetContextMenuRowColumn(const CRowColumn& roco){m_rocoContextMenu = roco;}

	virtual std::shared_ptr<CCellProperty> GetHeaderProperty(){return m_spHeaderProperty;} /** Getter for Header Cell Property */
	virtual std::shared_ptr<CCellProperty> GetFilterProperty(){return m_spFilterProperty;} /** Getter for Filter Cell Property */
	virtual std::shared_ptr<CCellProperty> GetCellProperty(){return m_spCellProperty;} /** Getter for Cell Property */

	ColumnDictionary* GetColumnVisibleDictionaryPtr(){return &m_columnVisibleDictionary;} /** Getter for Visible Column Dictionary */
	RowDictionary& RowAllDictionary(){return m_rowAllDictionary;} /** Getter for All Row Dictionary  */

	virtual column_type GetHeaderColumnPtr()const{return m_pHeaderColumn;} /** Getter for Header Column */
	virtual void SetHeaderColumnPtr(column_type column){m_pHeaderColumn=column;} /** Setter for Header Column */

	virtual row_type GetHeaderRowPtr()const{return m_rowHeader;} /** Getter for Header Row */
	virtual void SetHeaderRowPtr(row_type row){m_rowHeader=row;} /** Setter for Header Row */

	virtual bool GetSelected()const{return m_bSelected;};
	virtual void SetSelected(const bool& bSelected){m_bSelected=bSelected;};
	virtual bool GetFocused()const{return m_bFocused;};
	//virtual void SetFocused(const bool& bFocused){m_bFocused=bFocused;};
	//Function
	void SetAllRowMeasureValid(bool valid);
	void SetAllColumnMeasureValid(bool valid);
	void SetColumnAllCellMeasureValid(CColumn* pColumn, bool valid);

	//Observer
	virtual void CellSizeChanged(CellEventArgs& e);
	virtual void CellValueChanged(CellEventArgs& e);

	virtual void ColumnVisibleChanged(CColumnEventArgs& e){}
	virtual void ColumnInserted(CColumnEventArgs& e);
	virtual void ColumnErased(CColumnEventArgs& e);
	virtual void ColumnMoved(CMovedEventArgs<ColTag>& e) {}

	virtual void ColumnHeaderTrack(CColumnEventArgs& e);
	virtual void ColumnHeaderEndTrack(CColumnEventArgs& e);
	virtual void ColumnHeaderFitWidth(CColumnEventArgs& e);

	virtual void RowVisibleChanged(CRowEventArgs& e){}
	virtual void RowInserted(CRowEventArgs& e);
	virtual void RowErased(CRowEventArgs& e);
	virtual void RowsErased(CRowsEventArgs& e);
	virtual void RowMoved(CMovedEventArgs<RowTag>& e) {}

	virtual void Sorted();
	virtual void SizeChanged();
	virtual void Scroll();

	//Update
	virtual void UpdateRow();
	virtual void UpdateColumn();
	virtual void UpdateScrolls(){}
	virtual void Invalidate(){}
	virtual void UpdateAll();
	virtual void PostUpdate(Updates type);
	virtual void SubmitUpdate();

	//Sort
	virtual void ResetColumnSort();

	virtual std::shared_ptr<CDC> GetClientDCPtr()const=0;
	virtual CPoint GetScrollPos()const=0;
	virtual CGridView* GetGridPtr()=0;
	virtual bool Empty()const;
	virtual bool Visible()const;

	//Index-Pointer-Cell Accessor
	virtual void UpdateRowVisibleDictionary();
	virtual void UpdateColumnVisibleDictionary();
	virtual void UpdateRowPaintDictionary();
	virtual void UpdateColumnPaintDictionary();
	virtual size_type GetRowVisibleSize();
	virtual size_type GetColumnVisibleSize();
	virtual size_type GetAllRowSize()const;
	virtual size_type GetAllColumnSize()const;

	virtual size_type GetPlusRowSize()const;
	virtual size_type GetPlusColumnSize()const;

	virtual std::shared_ptr<CColumn> GetVisibleZeroColumnPointer()const;
	virtual std::shared_ptr<CColumn> GetVisibleLastColumnPointer()const;

	std::pair<size_type,size_type> GetMinMaxAllRowIndex()const;

	std::pair<size_type,size_type> GetMinMaxAllColumnIndex()const;

	std::pair<size_type,size_type> GetMinMaxVisibleRowIndex()const;

	std::pair<size_type,size_type> GetMinMaxVisibleColumnIndex()const;

	virtual void Sort(CColumn* pCol, Sorts sort);

	virtual void Filter(size_type colDisp,std::function<bool(const cell_type&)> predicate);

	virtual void ResetFilter();

	virtual void EraseColumn(column_type spColumn){EraseColumnImpl(spColumn);}
	virtual void EraseColumnImpl(column_type spColumn);
	virtual void EraseRow(CRow* pRow);
	virtual void EraseRows(const std::vector<CRow*>& vpRow);

	virtual void MoveColumn(size_type colTo, column_type spFromColumn){MoveColumnImpl(colTo, spFromColumn);}
	virtual void MoveColumnImpl(size_type colTo, column_type spFromColumn);

	virtual void InsertColumn(size_type colTo, column_type pColumn){InsertColumnImpl(colTo, pColumn);}
	virtual void InsertColumnImpl(size_type colTo, column_type pColumn, bool notify = true);
	virtual void InsertColumnImplLeft(size_type colTo, column_type pColumn);
	virtual void InsertRow(size_type rowVisib, row_type pRow, bool notify = true);



	virtual coordinates_type GetColumnInitWidth(CColumn* pColumn);
	virtual coordinates_type GetColumnFitWidth(CColumn* pColumn);

	virtual coordinates_type GetRowHeight(CRow* pRow);
	

	virtual string_type GetSheetString()const;

	virtual CPoint GetOriginPoint();
	virtual CSize MeasureSize()const;

	virtual CRect GetRect()const;
	virtual CRect GetCellsRect();
	virtual CRect GetPaintRect()=0;
	//Event handler
	virtual void OnPaint(PaintEventArgs& e);
	virtual void OnPaintAll(PaintEventArgs& e);
	virtual void OnRButtonDown(MouseEventArgs& e);

	virtual void OnLButtonDown(MouseEventArgs& e);
	virtual void OnLButtonUp(MouseEventArgs& e);
	virtual void OnLButtonSnglClk(MouseEventArgs& e);
	virtual void OnLButtonDblClk(MouseEventArgs& e);
	virtual void OnLButtonBeginDrag(MouseEventArgs& e);
	virtual void OnLButtonEndDrag(MouseEventArgs& e);


	virtual void OnContextMenu(ContextMenuEventArgs& e);
	virtual void OnMouseMove(MouseEventArgs& e);
	virtual void OnMouseLeave(MouseEventArgs& e);
	virtual void OnSetCursor(SetCursorEventArgs& e);
	virtual void OnSetFocus(EventArgs& e);
	virtual void OnKillFocus(EventArgs& e);
	virtual void OnKeyDown(KeyEventArgs& e){};
	
	//Tag dispatch
	template<typename TRC, typename TAV> TRC::template Dictionary& GetDictionary() { return nullptr; }
	template<typename TAV> std::shared_ptr<CCell> Cell(size_type row, size_type col)
	{
		auto pRow = Index2Pointer<RowTag, TAV>(row);
		auto pCol = Index2Pointer<ColTag, TAV>(col);
		if (pRow && pCol) {
			return CSheet::Cell(pRow, pCol);
		}
		else {
			return cell_type();
		}
	}

	template<typename TRC> TRC::template SharedPtr Coordinate2Pointer(coordinates_type coordinate) { return nullptr; }
	template<typename TRC> int Point2Coordinate(CPoint pt) { return CBand::kInvalidIndex; }

	template<typename TRC, typename TAV> int Coordinate2Index(coordinates_type coordinate) { return CBand::kInvalidIndex; }
	template<typename TAV> RC Point2Indexes(CPoint pt)
	{
		return RC(Coordinate2Index<RowTag, TAV>(pt.y), Coordinate2Index<ColTag, TAV>(pt.x));
	}
	template<typename TAV> std::pair<int, int> Coordinates2Indexes(CPoint pt) 
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
	//template<typename TRC, typename TAV> int Pointer2Index(TRC::template SharedPtr pointer)
	//{ 
	//	auto& dic = GetDictionary<TRC, TAV>().get<PointerTag>();

	//	auto iter = dic.find(pointer.get());
	//	if (iter != dic.end()) {
	//		return iter->Index;
	//	}
	//	else {
	//		throw std::exception("Sheet::Pointer2Index");
	//	}
	//}

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
		}
		else {
			return nullptr;
		}
	}
	template<typename TRC, typename TAV> int GetMaxIndex() 
	{
		auto& indexDictionary = GetDictionary<TRC, TAV>().get<IndexTag>();
		auto i = boost::prior(indexDictionary.end())->Index;
		return i;
	}
	template<typename TRC, typename TAV> int GetMinIndex()
	{
		auto& indexDictionary = GetDictionary<TRC, TAV>().get<IndexTag>();
		auto i = indexDictionary.begin()->Index;
		return i;
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
		//auto& otherDic = GetDictionary<TRC::Other, AllTag>();
		//for (const auto& other : otherDic) {
		//	CSheet::Cell(ptr, other.DataPtr)->SetActMeasureValid(false);
		//}
		//for (const auto& other : otherDic) {
		//	other.DataPtr->SetMeasureValid(false);
		//}

		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
	}
	template<typename TRC> void FitWidth(TRC::template SharedPtr& e) {  }


	template<typename TRC>
	void MoveImpl(size_type indexTo, TRC::template SharedPtr spFrom)
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
		InsertLeftImpl<TRC>(to, spFrom);

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

	template<typename TRC>
	void InsertLeftImpl(size_type allIndex, TRC::template SharedPtr pInsert)
	{
		auto& dictionary = GetDictionary<TRC, AllTag>();
		auto& ptrDictionary = dictionary.get<PointerTag>();
		auto& idxDictionary = dictionary.get<IndexTag>();

		if (allIndex >= 0) {//Plus
			size_type size = dictionary.size() - MinusSize(dictionary);
			if (allIndex >= size) {
				allIndex = size;
			}
			//Slide right
			auto iterTo = idxDictionary.find(allIndex);
			if (iterTo != idxDictionary.end()) {
				auto iter = idxDictionary.end(); iter--;
				auto end = iterTo; end--;
				for (; iter != end; --iter) {
					auto newdata = *iter;
					newdata.Index++;
					idxDictionary.replace(iter, newdata);
				}
			}

		}
		else {//Minus
			size_type size = MinusSize(dictionary);
			if (allIndex <= (-size - 1)) {
				allIndex = (-size - 1);
			}
			//Slide left
			auto iterTo = idxDictionary.find(allIndex);
			if (iterTo != idxDictionary.end()) {
				auto iter = idxDictionary.begin();
				auto end = iterTo;
				for (; iter != end; ++iter) {
					auto newdata = *iter;
					newdata.Index--;
					idxDictionary.replace(iter, newdata);
				}
			}
			allIndex--;
		}
		//Insert
		dictionary.insert(TRC::Dictionary::value_type(allIndex, pInsert));
	}
	cell_type Point2Cell(const CPoint& ptClient);
	CRowColumn Point2RowColumn(const CPoint& ptClient);

	//size_type Y2AllRowIndex(coordinates_type y);
	//size_type X2AllColumnIndex(coordinates_type x);
	//size_type Y2VisibleRowIndex(coordinates_type y);
	//size_type X2VisibleColumnIndex(coordinates_type x);

	//std::pair<size_type, size_type> Point2AllIndexes(const CPoint& ptClient);
	//std::pair<size_type, size_type> Point2VisibleIndexes(const CPoint& ptClient);


	
	virtual void SelectRange(CRowColumn pCell1, CRowColumn pCell2);
	virtual void DeselectRange(CRowColumn pCell1, CRowColumn pCell2);
	//virtual void DeselectRowRange(CRow* pRow1, CRow* pRow2);
	//virtual void SelectRowRange(CRow* pRow1, CRow* pRow2);
	
	virtual void SelectAll();
	virtual void DeselectAll();
	virtual void UnhotAll();
	virtual bool IsFocusedCell(CRowColumn roco)const;
	virtual bool IsDoubleFocusedCell(CRowColumn roco)const;
	//virtual bool IsSelectedCell(CRowColumn roco)const;

	//virtual RowDictionary::index<IndexTag>::type::iterator RowBegin()
	//{
	//	auto& dictionary=m_rowAllDictionary.get<IndexTag>();
	//	return dictionary.find(0);
	//}

	//virtual RowDictionary::index<IndexTag>::type::iterator RowEnd()
	//{
	//	auto& dictionary=m_rowAllDictionary.get<IndexTag>();
	//	return dictionary.end();
	//}
	RowDictionary::index<IndexTag>::type::iterator RowAllBegin()
	{
		auto& dictionary=m_rowAllDictionary.get<IndexTag>();
		return dictionary.begin();
	}

	RowDictionary::index<IndexTag>::type::iterator RowAllZero()
	{
		auto& dictionary=m_rowAllDictionary.get<IndexTag>();
		return dictionary.find(0);
	}

	RowDictionary::index<IndexTag>::type::iterator RowAllEnd()
	{
		auto& dictionary=m_rowAllDictionary.get<IndexTag>();
		return dictionary.end();
	}

	ColumnDictionary::index<IndexTag>::type::iterator ColumnAllBegin()
	{
		auto& dictionary=m_columnAllDictionary.get<IndexTag>();
		return dictionary.begin();
	}

	ColumnDictionary::index<IndexTag>::type::iterator ColumnAllZero()
	{
		auto& dictionary=m_columnAllDictionary.get<IndexTag>();
		return dictionary.find(0);
	}

	ColumnDictionary::index<IndexTag>::type::iterator ColumnAllEnd()
	{
		auto& dictionary=m_columnAllDictionary.get<IndexTag>();
		return dictionary.end();
	}

	ColumnDictionary::index<IndexTag>::type::iterator ColumnVisibleBegin()
	{
		if(m_columnVisibleDictionary.empty()){
			UpdateColumnVisibleDictionary();
		}
		auto& dictionary=m_columnVisibleDictionary.get<IndexTag>();
		return dictionary.begin();
	}

	ColumnDictionary::index<IndexTag>::type::iterator ColumnVisibleZero()
	{
		if(m_columnVisibleDictionary.empty()){
			UpdateColumnVisibleDictionary();
		}
		auto& dictionary=m_columnVisibleDictionary.get<IndexTag>();
		return dictionary.find(0);
	}

	ColumnDictionary::index<IndexTag>::type::iterator ColumnVisibleEnd()
	{
		if(m_columnVisibleDictionary.empty()){
			UpdateColumnVisibleDictionary();
		}
		auto& dictionary=m_columnVisibleDictionary.get<IndexTag>();
		return dictionary.end();
	}

	Compares CheckEqualRange(RowDictionary::iterator rowBegin, RowDictionary::iterator rowEnd,ColumnDictionary::iterator colBegin, ColumnDictionary::iterator colEnd, std::function<void(CCell*, Compares)> action);
	Compares CheckEqualRow(CRow* pRow, ColumnDictionary::iterator colBegin, ColumnDictionary::iterator colEnd, std::function<void(CCell*, Compares)> action);
	
	virtual void Clear();

	virtual CColumn* GetParentColumnPtr(CCell* pCell) = 0;


	//Menu
	CMenu* const m_pContextMenu;
	virtual CMenu* GetContextMenuPtr(){return m_pContextMenu;}

/*
 *  Template function
 */
protected:
	template<class T>
	size_type VisibleIndex2AllIndex(size_type visibleIndex, T visibleDictionary, T allDictionary)
	{
		auto pointer = Index2Pointer(visibleDictionary, visibleIndex);
		return Pointer2Index(allDictionary, pointer);
	}

	template<class T>
	size_type Y2RowIndex(coordinates_type y, T& rowDictionary)
	{
		auto ptOrigin=GetOriginPoint();

		auto& dictionary=rowDictionary.get<IndexTag>();
		auto rowIter=std::lower_bound(dictionary.begin(),dictionary.end(),y,
			[ptOrigin](const RowData& rowData, const int& rhs)->bool{
				if(rowData.Index>=0){
					return max(ptOrigin.y,rowData.DataPtr->GetBottom()) < rhs;
				}else{
					return rowData.DataPtr->GetBottom() < rhs;
				}
		});
		size_type row=0;
		if(rowIter==dictionary.end()){
			row = boost::prior(rowIter)->DataPtr->GetIndex<VisTag>() + 1;
		}else if(rowIter==dictionary.begin() && rowIter->DataPtr->GetTop()>y){
			row = rowIter->DataPtr->GetIndex<VisTag>() - 1;
		}else{
			row = rowIter->DataPtr->GetIndex<VisTag>();
		}
		return row;
	}

	template<class T>
	size_type X2ColumnIndex(coordinates_type x, T& colDictionary)
	{
		auto ptOrigin=GetOriginPoint();

		auto& dictionary=colDictionary.get<IndexTag>();
		auto colIter=std::lower_bound(dictionary.begin(),dictionary.end(),x,
		[ptOrigin](const ColumnData& colData,const int& rhs)->bool{
			if(colData.Index>=0){
				return max(ptOrigin.x,colData.DataPtr->GetRight()) < rhs;
			}else{
				return colData.DataPtr->GetRight() < rhs;
			}
		});

		size_type col=0;
		if(colIter==dictionary.end()){
			col = boost::prior(colIter)->DataPtr->GetIndex<VisTag>() + 1;
		}else if(colIter==dictionary.begin() && colIter->DataPtr->GetLeft()>x){
			col = colIter->DataPtr->GetIndex<VisTag>() - 1;
		}else{
			col = colIter->DataPtr->GetIndex<VisTag>();
		}
		return col;
	}

	
	template<class T,class U>
	void Insert(T& dictionary,size_type allIndex, U& pInsert)
	{
		auto& ptrDictionary = dictionary.get<PointerTag>();	
		auto& idxDictionary = dictionary.get<IndexTag>();


		if(allIndex>=0){//Plus
			size_type size=dictionary.size()-MinusSize(dictionary);
			if(allIndex>=size){
				allIndex=size;
			}
			//Slide right
			auto iterTo=idxDictionary.find(allIndex);
			if(iterTo!=idxDictionary.end()){
				auto iter=idxDictionary.end();iter--;
				auto end=iterTo;end--;
				for(;iter!=end;--iter){
					auto newdata=*iter;
					newdata.Index++;
					idxDictionary.replace(iter,newdata);
				}
			}

		}else{//Minus
			size_type size=MinusSize(dictionary);
			if(allIndex<=(-size-1)){
				allIndex=(-size-1);
			}
			//Slide left
			auto iterTo=idxDictionary.find(allIndex);
			if(iterTo!=idxDictionary.end()){
				auto iter=idxDictionary.begin();
				auto end=iterTo;end++;
				for(;iter!=end;++iter){
					auto newdata=*iter;
					newdata.Index--;
					idxDictionary.replace(iter,newdata);
				}
			}
		}
		//Insert
		dictionary.insert(T::value_type(allIndex,pInsert));
	}

	template<class T,class U>
	void InsertLeft(T& dictionary,size_type allIndex, U& pInsert)
	{
		auto& ptrDictionary = dictionary.get<PointerTag>();	
		auto& idxDictionary = dictionary.get<IndexTag>();


		if(allIndex>=0){//Plus
			size_type size=dictionary.size()-MinusSize(dictionary);
			if(allIndex>=size){
				allIndex=size;
			}
			//Slide right
			auto iterTo=idxDictionary.find(allIndex);
			if(iterTo!=idxDictionary.end()){
				auto iter=idxDictionary.end();iter--;
				auto end=iterTo;end--;
				for(;iter!=end;--iter){
					auto newdata=*iter;
					newdata.Index++;
					idxDictionary.replace(iter,newdata);
				}
			}

		}else{//Minus
			size_type size=MinusSize(dictionary);
			if(allIndex<=(-size-1)){
				allIndex=(-size-1);
			}
			//Slide left
			auto iterTo=idxDictionary.find(allIndex);
			if(iterTo!=idxDictionary.end()){
				auto iter=idxDictionary.begin();
				auto end=iterTo;
				for(;iter!=end;++iter){
					auto newdata=*iter;
					newdata.Index--;
					idxDictionary.replace(iter,newdata);
				}
			}
			allIndex--;
		}
		//Insert
		dictionary.insert(T::value_type(allIndex,pInsert));
	}

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

	template<class T,class U>
	typename T::template index<PointerTag>::type::value_type::index_type Pointer2Index(const T& dictionary,const U& pointer)const
	{
		auto& m = dictionary.get<PointerTag>();

		auto iter = m.find(pointer);  
		if (iter != m.end()) {
			return iter->Index;
		}else{
			throw std::exception("Sheet::Pointer2Index");
		}
	}

	template<class T,class U>
//	typename T::index<IndexTag>::type::value_type::pointer_type Index2Pointer(const T& dictionary,const U& index)const
	typename T::template index<IndexTag>::type::value_type::pointer_type Index2Pointer(const T& dictionary,const U& index)const
	{
		auto& m = dictionary.get<IndexTag>();

		auto iter = m.find(index);
		if (iter != m.end()) {
			return iter->DataPtr.get();
		}else{
			return nullptr;
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
	void UpdatePaintDictionary(T& visibleDictionary, T& paintDictionary, coordinates_type pageFirst, coordinates_type pageLast, U predFirst, V predLast)
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

	template<class T, class U,class V, class W>
	void UpdateDisplayVector(T& visibleDictionary, U& displayVector, coordinates_type pageFirst, coordinates_type pageLast, V predFirst, W predLast)
	{
		displayVector.clear();
		auto& dispDictionary = visibleDictionary.get<IndexTag>();
		//Copy Minus Elements
		std::copy(dispDictionary.begin(),dispDictionary.find(0),std::back_inserter(displayVector));
		//Find Displayed Plus Elements
		auto beginIter=std::upper_bound(dispDictionary.find(0),dispDictionary.end(),pageFirst,predFirst);
		if(beginIter!=dispDictionary.find(0)){
			--beginIter;
		}
		auto endIter=std::lower_bound(dispDictionary.find(0),dispDictionary.end(),pageLast,predLast);
		if(endIter!=dispDictionary.end()){
			++endIter;
		}
		std::copy(beginIter,endIter,std::back_inserter(displayVector));
	}

	template<class T>
	size_type MinusSize(T& dictionary)const
	{
		auto minusSize=0;
		auto& dispDictionary=dictionary.get<IndexTag>();
		for(auto iter=dispDictionary.begin(),end=dispDictionary.find(0);iter!=end;++iter){
			minusSize++;
		}
		return minusSize;
	}

	template<class T>
	size_type PlusSize(T& dictionary)const
	{
		auto plusSize=dictionary.size();

		auto& dispDictionary=dictionary.get<IndexTag>();

		for(auto iter=dispDictionary.begin(),end=dispDictionary.find(0);iter!=end;++iter){
			plusSize--;
		}
		return plusSize;
	}

	template<class T>
	std::pair<size_type, size_type> GetMinMaxAllIndex(T& dictionary)const
	{
		auto& idxDictionary = dictionary.get<IndexTag>();

		auto min = idxDictionary.begin()->DataPtr->GetIndex<AllTag>();
		auto max = boost::prior(idxDictionary.end())->DataPtr->GetIndex<AllTag>();

		return std::make_pair(min, max);
	}

	template<class T>
	std::pair<size_type, size_type> GetMinMaxVisibleIndex(T& dictionary)const
	{
		auto& idxDictionary = dictionary.get<IndexTag>();

		auto min = idxDictionary.begin()->DataPtr->GetIndex<VisTag>();
		auto max = boost::prior(idxDictionary.end())->DataPtr->GetIndex<VisTag>();

		return std::make_pair(min, max);
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

template<> inline RowTag::SharedPtr CSheet::Coordinate2Pointer<RowTag>(int coordinate)
{
	auto ptOrigin = GetOriginPoint();

	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
	auto rowIter = std::upper_bound(rowDictionary.begin(), rowDictionary.end(), coordinate,
		[ptOrigin](const int& y, const RowData& rowData)->bool {
		if (rowData.Index >= 0) {
			return y<max(ptOrigin.y, rowData.DataPtr->GetTop());
		}
		else {
			return y<rowData.DataPtr->GetTop();
		}
	});

	auto prevRowIter = boost::prior(rowIter);

	if (rowIter == rowDictionary.begin() || (rowIter == rowDictionary.end() && coordinate>prevRowIter->DataPtr->GetBottom())) {
		return nullptr;
	}
	else {
		--rowIter;
	}
	return rowIter->DataPtr;
}

template<> inline int CSheet::Point2Coordinate<RowTag>(CPoint pt) { return pt.y; }
template<> inline int CSheet::Point2Coordinate<ColTag>(CPoint pt) { return pt.x; }



template<> inline int CSheet::Coordinate2Index<RowTag, AllTag>(int coordinate)
{
//	return Coordinate2Pointer<RowTag>(coordinate)->GetIndex<AllTag>();
	auto ptOrigin = GetOriginPoint();

	auto& dictionary = GetDictionary<RowTag, AllTag>().get<IndexTag>();
	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
		[ptOrigin](const RowData& rowData, const int& rhs)->bool {
		if (rowData.Index >= 0) {
			return max(ptOrigin.y, rowData.DataPtr->GetBottom()) < rhs;
		}
		else {
			return rowData.DataPtr->GetBottom() < rhs;
		}
	});
	size_type index = 0;
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

template<> inline int CSheet::Coordinate2Index<RowTag, VisTag>(int coordinate)
{
	auto ptOrigin = GetOriginPoint();

	auto& dictionary = GetDictionary<RowTag, VisTag>().get<IndexTag>();
	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
		[ptOrigin](const RowData& rowData, const int& rhs)->bool {
		if (rowData.Index >= 0) {
			return max(ptOrigin.y, rowData.DataPtr->GetBottom()) < rhs;
		}
		else {
			return rowData.DataPtr->GetBottom() < rhs;
		}
	});
	size_type index = 0;
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



template<> inline int CSheet::Coordinate2Index<ColTag, VisTag>(int coordinate)
{
	auto ptOrigin = GetOriginPoint();

	auto& dictionary = GetDictionary<ColTag, VisTag>().get<IndexTag>();
	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
		[ptOrigin](const ColumnData& rowData, const int& rhs)->bool {
		if (rowData.Index >= 0) {
			return max(ptOrigin.x, rowData.DataPtr->GetRight()) < rhs;
		}
		else {
			return rowData.DataPtr->GetRight() < rhs;
		}
	});
	size_type index = 0;
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

template<> inline int CSheet::Coordinate2Index<ColTag, AllTag>(int coordinate)
{
	auto ptOrigin = GetOriginPoint();

	auto& dictionary = GetDictionary<ColTag, AllTag>().get<IndexTag>();
	auto rowIter = std::lower_bound(dictionary.begin(), dictionary.end(), coordinate,
		[ptOrigin](const ColumnData& rowData, const int& rhs)->bool {
		if (rowData.Index >= 0) {
			return max(ptOrigin.x, rowData.DataPtr->GetRight()) < rhs;
		}
		else {
			return rowData.DataPtr->GetRight() < rhs;
		}
	});
	size_type index = 0;
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