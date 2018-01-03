#pragma once
#include "IMouseObserver.h"
#include "Sheet.h"
#include "SheetState.h"

//Pre-Declaration
struct MouseEventArgs;
struct SetCursorEventArgs;

class IDragger
{
public:
	virtual bool IsTarget() = 0;
	virtual void OnBeginDrag() = 0;
	virtual void OnDrag() = 0;
	virtual void OnEndDrag() = 0;
};

template<typename TRCMe, typename TRCYou>
class CDragger :public IDragger
{
private:
	typedef int size_type;
	typedef int coordinates_type;
	//static const coordinates_type MIN_ROW_HIGHT = 2;
	//static const coordinates_type RESIZE_AREA_HARF_HIGHT = 4;

private:
	size_type m_dragFromIndex;
	size_type m_dragToIndex;
public:
	CDragger() :m_dragFromIndex(CBand::kInvalidIndex), m_dragToIndex(CBand::kInvalidIndex) {}
	virtual ~CDragger() {}

	size_type GetDragToAllIndex() { return m_dragToIndex; }
	size_type GetDragFromAllIndex() { return m_dragFromIndex; }

	//CSheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
	//{
	//	if (!pSheet->Visible())return CSheetState::Normal();

	//	auto visIndex = pSheet->Coordinate2Index<TRCYou, VisTag>(e.Point.x);
	//	//If Header except Filter
	//	if (IsDragable(pSheet, visIndex)) {
	//		if (e.Flags == MK_LBUTTON) {
	//			return OnHeaderBeginDrag(pSheet, e);
	//		}
	//	}
	//	return CSheetState::Normal();
	//}
	//CSheetState* OnMouseLeave(CSheet* pSheet, MouseEventArgs& e) { return CSheetState::Normal(); }
	//CSheetState* OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e) { return CSheetState::Normal(); }
//	CSheetState* OnDragLButtonDown(CSheet* pSheet, MouseEventArgs& e)
//	{
//		return CSheetState::Normal();
//	}
//	CSheetState* OnDragLButtonUp(CSheet* pSheet, MouseEventArgs& e)
//	{
//		return OnHeaderEndDrag(pSheet, e);
//	}
//	CSheetState* OnDragLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
//	{
//		return CSheetState::Normal();
//	}
//	CSheetState* OnDragRButtonDown(CSheet* pSheet, MouseEventArgs& e)
//	{
//		return CSheetState::Normal();
//	}
//	CSheetState* OnDragMouseMove(CSheet* pSheet, MouseEventArgs& e)
//	{
//		return OnHeaderDrag(pSheet, e);
//	}
//	CSheetState* OnDragMouseLeave(CSheet* pSheet, MouseEventArgs& e)
//	{
//		return OnHeaderDrag(pSheet, e);
//	}
//	CSheetState* OnDragSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
//	{
//		return DraggingState();
//	}
//
//private:
	//CSheetState* DraggingState() { return nullptr; }

	void OnBeginDrag(CSheet* pSheet, MouseEventArgs& e) override
	{
		m_dragFromIndex = pSheet->Coordinate2Index<TRCMe, AllTag>(e.Point.y);
		m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnDrag(CSheet* pSheet, MouseEventArgs& e) override
	{
		auto visibleIndex = pSheet->Coordinate2Index<TRCMe, VisTag>(e.Point.y);

		auto visMinMax = pSheet->GetMinMaxIndexes<TRCMe, VisTag>();
		auto allMinMax = pSheet->GetMinMaxIndexes<TRCMe, AllTag>();

		if (visibleIndex == CBand::kInvalidIndex) {
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
	}

	void OnEndDrag(CSheet* pSheet, MouseEventArgs& e) override
	{
		if (m_dragToIndex == CBand::kInvalidIndex) {

		}
		else {
			if (m_dragToIndex > 0 && m_dragToIndex != m_dragFromIndex) {//TODO Only when Row
				auto sp = pSheet->Index2Pointer < TRCMe, AllTag>(m_dragFromIndex);
				pSheet->MoveImpl<TRCMe>(m_dragToIndex, sp);
			}
		}
		m_dragFromIndex = CBand::kInvalidIndex;
		m_dragToIndex = CBand::kInvalidIndex;

		return ISheetState::Normal();
	}

	//bool IsDragable(CSheet* pSheet, size_type visibleIndex)
	//{
	//	if (visibleIndex <= pSheet->GetMaxIndex<TRCYou, VisTag>() &&
	//		visibleIndex >= pSheet->GetMinIndex<TRCYou, VisTag>() &&
	//		pSheet->Index2Pointer<TRCYou, VisTag>(visibleIndex)->IsDragTrackable()) {
	//		return true;
	//	}
	//	else {
	//		return false;
	//	}
	//}

};

template<> ISheetState* CDragger<RowTag, ColTag>::DraggingState()
{
	return CRowDraggingState::State();
}

template<> ISheetState* CDragger<ColTag, RowTag>::DraggingState()
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
	virtual ISheetState* OnMouseMove(CSheet* pSheet, MouseEventArgs& e);
};
