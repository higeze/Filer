#pragma once
#include "IMouseObserver.h"
#include "Sheet.h"
#include "SheetState.h"
//Pre-Declaration
//class CSheet;
struct MouseEventArgs;
struct SetCursorEventArgs;
//struct RowTag;
//struct ColTag;

//class CColumnDragger:public IMouseObserver
//{
//private:
//	typedef int size_type;
//	typedef int coordinates_type;
//	static const size_type COLUMN_INDEX_INVALID = 9999;
//	static const size_type COLUMN_INDEX_MAX = 1000;
//	static const size_type COLUMN_INDEX_MIN = -1000;
//	static const coordinates_type MIN_COLUMN_WIDTH=2;
//	static const coordinates_type RESIZE_AREA_HARF_WIDTH=4;
//
//private:
//	size_type m_dragFromIndex;
//	size_type m_dragToIndex;
//public:
//	CColumnDragger():m_dragFromIndex(COLUMN_INDEX_INVALID), m_dragToIndex(COLUMN_INDEX_INVALID){}
//	virtual ~CColumnDragger(){}
//	size_type GetDragToAllIndex(){return m_dragToIndex;}
//	size_type GetDragFromAllIndex(){return m_dragFromIndex;}
//	
//	virtual CSheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
//
//	virtual CSheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e);
//
//	virtual CSheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e);
//
//	virtual CSheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e);
//
//
//	virtual CSheetState* OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e);
//	virtual CSheetState* OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e);
//
//
//private:
//
//	CSheetState* OnColumnHeaderBeginDrag(CSheet* pSheet, MouseEventArgs& e);
//	CSheetState* OnColumnHeaderDrag(CSheet* pSheet, MouseEventArgs& e);
//	CSheetState* OnColumnHeaderEndDrag(CSheet* pSheet, MouseEventArgs& e);
//private:
//	bool IsDragable(CSheet* pSheet, std::pair<size_type, size_type> visibleIndexes);
//
//};

template<typename TRCMe, typename TRCYou>
class CDragger :public IMouseObserver
{
private:
	typedef int size_type;
	typedef int coordinates_type;
	static const size_type INDEX_INVALID = 9999;
	static const size_type INDEX_MAX = 1000;
	static const size_type INDEX_MIN = -1000;
	static const coordinates_type MIN_ROW_HIGHT = 2;
	static const coordinates_type RESIZE_AREA_HARF_HIGHT = 4;

private:
	size_type m_dragFromIndex;
	size_type m_dragToIndex;
public:
	CDragger() :m_dragFromIndex(INDEX_INVALID), m_dragToIndex(INDEX_INVALID) {}
	virtual ~CDragger() {}

	size_type GetDragToAllIndex() { return m_dragToIndex; }
	size_type GetDragFromAllIndex() { return m_dragFromIndex; }

	CSheetState* OnLButtonDown(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
	CSheetState* OnLButtonUp(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
	CSheetState* OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
	CSheetState* OnRButtonDown(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
	CSheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
	{
		if (!pSheet->Visible())return CSheetState::Normal();

		auto visIndex = pSheet->Coordinate2Index<TRCYou, VisTag>(e.Point.x);
		//If Header except Filter
		if (IsDragable(pSheet, visIndex)) {
			if (e.Flags == MK_LBUTTON) {
				return OnHeaderBeginDrag(pSheet, e);
			}
		}
		return CSheetState::Normal();
	}
	CSheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
	CSheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e) { return CSheetState::Normal(); }
	CSheetState* OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e)
	{
		return CSheetState::Normal();
	}
	CSheetState* OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e)
	{
		return OnColumnHeaderEndDrag(pSheet, e);
	}
	CSheetState* OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
	{
		return CSheetState::Normal();
	}
	CSheetState* OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e)
	{
		return CSheetState::Normal();
	}
	CSheetState* OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e)
	{
		return OnColumnHeaderDrag(pSheet, e);
	}
	CSheetState* OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e)
	{
		return OnColumnHeaderDrag(pSheet, e);
	}
	CSheetState* OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
	{
		return DraggingState();
	}

private:
	CSheetState* DraggingState() { return nullptr; }

	CSheetState* OnHeaderBeginDrag(CSheet* pSheet, MouseEventArgs& e)
	{
		m_dragFromIndex = pSheet->Coordinate2Index<TRCMe, AllTag>(e.Point.y);
		m_dragToIndex = INDEX_INVALID;
		return DraggingState();
	}

	CSheetState* OnHeaderDrag(CSheet* pSheet, MouseEventArgs& e)
	{
		if (e.Flags == MK_LBUTTON) {

			auto visibleIndex = pSheet->Coordinate2Index<TRCMe, VisTag>(e.Point.y);

			auto visMinMax = pSheet->GetMinMaxIndexes<TRCMe, VisTag>();
			auto allMinMax = pSheet->GetMinMaxIndexes<TRCMe, AllTag>();

			if (visibleIndex == INDEX_INVALID) {
				m_dragToIndex = pSheet->Vis2AllIndex<TRCMe>(visibleIndex);
			}
			else if (visibleIndex < visMinMax.first) {
				m_dragToIndex = pSheet->Vis2AllIndex<TRCMe>(visMinMax.first);
			}
			else if (visibleIndex > visMinMax.second) {
				m_dragToIndex = pSheet->Vis2AllIndex<TRCMe>(visMinMax.second) + 1;
			}
			else if (visibleIndex == visMinMax.first) {
				m_dragToIndex = pSheet->Vis2AllIndex<TRCMe>(visMinMax.first);
			}
			else if (visibleIndex == visMinMax.second) {
				m_dragToIndex = pSheet->Vis2AllIndex<TRCMe>(visMinMax.second) + 1;
			}
			else {
				auto allIndex = pSheet->Vis2AllIndex<TRCMe>(visibleIndex);
				if (allIndex < m_dragFromIndex) {
					m_dragToIndex = allIndex;
				}
				else if (allIndex > m_dragFromIndex) {
					m_dragToIndex = pSheet->Vis2AllIndex<TRCMe>(visibleIndex + 1);
				}
				else {
					m_dragToIndex = allIndex;
				}
			}
			return DraggingState();

		}
		else {
			return CSheetState::Normal();
		}
	}

	CSheetState* OnHeaderEndDrag(CSheet* pSheet, MouseEventArgs& e)
	{
		if (m_dragToIndex == INDEX_INVALID) {

		}
		else {
			if (m_dragToIndex > 0 && m_dragToIndex != m_dragFromIndex) {//TODO Only when Row
				auto sp = pSheet->Index2Pointer < TRCMe, AllTag>(m_dragFromIndex);
				pSheet->MoveImpl<TRCMe>(m_dragToIndex, sp);
			}
		}
		m_dragFromIndex = INDEX_INVALID;
		m_dragToIndex = INDEX_INVALID;

		return CSheetState::Normal();
	}

	bool IsDragable(CSheet* pSheet, size_type visibleIndex)
	{
		if (visibleIndex <= pSheet->GetMaxIndex<TRCYou, VisTag>() &&
			visibleIndex >= pSheet->GetMinIndex<TRCYou, VisTag>() &&
			pSheet->Index2Pointer<TRCYou, VisTag>(visibleIndex)->IsDragTrackable()) {
			return true;
		}
		else {
			return false;
		}
	}

};

template<> CSheetState* CDragger<RowTag, ColTag>::DraggingState()
{
	return CRowDraggingState::State();
}

template<> CSheetState* CDragger<ColTag, RowTag>::DraggingState()
{
	return CColumnDraggingState::State();
}

//template class CDragger<RowTag, ColTag>;
//
//typedef CDragger<RowTag, ColTag> CRowDragger;
//
//template class CDragger<ColTag, RowTag>;
//
//typedef CDragger<ColTag, RowTag> CColumnDragger;

/*! @class CSheetCellDragger
    @brief Dragger behavior for CSheetCell
*/
class CSheetCellDragger:public CDragger<ColTag, RowTag>
{
public:
    /*! Constructor*/
	CSheetCellDragger():CDragger<ColTag, RowTag>(){}
	virtual ~CSheetCellDragger(){}
	/*! Do not drag Header for CSheetCell*/
	virtual CSheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
};
