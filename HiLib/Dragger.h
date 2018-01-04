#pragma once
#include "IDragger.h"
#include "Sheet.h"

//Pre-Declaration
struct MouseEventArgs;
struct SetCursorEventArgs;



template<typename TRCMe, typename TRCYou>
class CDragger :public IDragger
{
private:
	size_type m_dragFromIndex;
	size_type m_dragToIndex;
public:
	CDragger() :m_dragFromIndex(CBand::kInvalidIndex), m_dragToIndex(CBand::kInvalidIndex) {}
	virtual ~CDragger() {}

	size_type GetDragToAllIndex() { return m_dragToIndex; }
	size_type GetDragFromAllIndex() { return m_dragFromIndex; }

	virtual bool IsTarget(CSheet* pSheet, MouseEventArgs const & e) override
	{
		auto visIndexes = pSheet->Coordinates2Indexes<VisTag>(e.Point);
		if (visIndexes.first <= pSheet->GetMaxIndex<RowTag, VisTag>() &&
			visIndexes.first >= pSheet->GetMinIndex<RowTag, VisTag>() &&
			visIndexes.second <= pSheet->GetMaxIndex<ColTag, VisTag>() &&
			visIndexes.second >= pSheet->GetMinIndex<ColTag, VisTag>() &&
			pSheet->Index2Pointer<TRCMe, VisTag>(visIndexes.first)->IsDragTrackable()) {
			return true;
		}else{
			return false;
		}
	}

	void OnBeginDrag(CSheet* pSheet, MouseEventArgs const & e) override
	{
		m_dragFromIndex = pSheet->Coordinate2Index<TRCMe, AllTag>(e.Point.y);
		m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnDrag(CSheet* pSheet, MouseEventArgs const & e) override
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

	void OnEndDrag(CSheet* pSheet, MouseEventArgs const & e) override
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
	}


};

typedef CDragger<RowTag, ColTag> CRowDragger;
typedef CDragger<ColTag, RowTag> CColDragger;

template<typename TRCMe, typename TRCYou>
class CSheetCellDragger:public CDragger<TRCMe, TRCYou>
{
public:
    /*! Constructor*/
	CSheetCellDragger():CDragger(){}
	virtual ~CSheetCellDragger(){}
	/*! Do not drag Header for CSheetCell*/
	virtual void OnMouseMove(CSheet* pSheet, MouseEventArgs const & e) override {}
};

typedef CSheetCellDragger<RowTag, ColTag> CSheetCellRowDragger;
typedef CSheetCellDragger<ColTag, RowTag> CSheetCellColDragger;
