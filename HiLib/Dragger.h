#pragma once
#include "IDragger.h"
#include "Sheet.h"
#include "CellProperty.h"
#include "MyPen.h"

//Pre-Declaration
struct MouseEventArgs;
struct SetCursorEventArgs;



template<typename TRC, typename TRCYou>
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
		auto visIndexes = pSheet->Point2Indexes<VisTag>(e.Point);
		if (visIndexes.Row <= pSheet->GetMaxIndex<RowTag, VisTag>() &&
			visIndexes.Row >= pSheet->GetMinIndex<RowTag, VisTag>() &&
			visIndexes.Col <= pSheet->GetMaxIndex<ColTag, VisTag>() &&
			visIndexes.Col >= pSheet->GetMinIndex<ColTag, VisTag>() &&
			visIndexes.Get<TRCYou>()<0) {
			return true;
		}else{
			return false;
		}
	}

	void OnBeginDrag(CSheet* pSheet, MouseEventArgs const & e) override
	{
		m_dragFromIndex = pSheet->Point2Indexes<AllTag>(e.Point).Get<TRC>();
		m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnDrag(CSheet* pSheet, MouseEventArgs const & e) override
	{
		auto visibleIndex = pSheet->Point2Indexes<VisTag>(e.Point).Get<TRC>();

		auto visMinMax = pSheet->GetMinMaxIndexes<TRC, VisTag>();
		auto allMinMax = pSheet->GetMinMaxIndexes<TRC, AllTag>();

		if (visibleIndex == CBand::kInvalidIndex) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visibleIndex);
		}
		else if (visibleIndex < visMinMax.first) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visMinMax.first);
		}
		else if (visibleIndex > visMinMax.second) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visMinMax.second) + 1;
		}
		else if (visibleIndex == visMinMax.first) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visMinMax.first);
		}
		else if (visibleIndex == visMinMax.second) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visMinMax.second) + 1;
		}
		else {
			auto allIndex = pSheet->Vis2AllIndex<TRC>(visibleIndex);
			if (allIndex < m_dragFromIndex) {
				m_dragToIndex = allIndex;
			}
			else if (allIndex > m_dragFromIndex) {
				m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visibleIndex + 1);
			}
			else {
				m_dragToIndex = allIndex;
			}
		}

		m_dragToIndex = max(m_dragToIndex, 0);
	}

	void OnEndDrag(CSheet* pSheet, MouseEventArgs const & e) override
	{
		if (m_dragToIndex == CBand::kInvalidIndex) {

		}
		else {
			if (m_dragToIndex >= 0 && m_dragToIndex != m_dragFromIndex) {//TODO Only when Row
				auto sp = pSheet->Index2Pointer<TRC, AllTag>(m_dragFromIndex);
				pSheet->MoveImpl<TRC>(m_dragToIndex, sp);
			}
		}
		m_dragFromIndex = CBand::kInvalidIndex;
		m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnLeaveDrag(CSheet* pSheet, MouseEventArgs const & e) override
	{
		m_dragFromIndex = CBand::kInvalidIndex;
		m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnPaintDragLine(CSheet* pSheet, PaintEventArgs const & e)
	{
		//auto dragTo = m_spColDragger->GetDragToAllIndex();
		//auto dragFrom = m_spColDragger->GetDragFromAllIndex();
		//Drag to from check
		if (m_dragFromIndex == CBand::kInvalidIndex ||
			m_dragToIndex == CBand::kInvalidIndex ||
			m_dragFromIndex == m_dragToIndex) {
			return;
		}

		//Get Left/Top Line
		//o/max : Right/Bottom
		//u/min : Left/Top
		//other : Left
		coordinates_type coome = 0;
		if (m_dragToIndex > pSheet->GetMaxIndex<TRC, AllTag>()) {
			coome = pSheet->LastPointer<TRC, AllTag>()->GetRightBottom();
		}
		else if(m_dragToIndex <= 0) {
			coome = pSheet->ZeroPointer<TRC, AllTag>()->GetLeftTop();
		}
		else {
			coome = pSheet->Index2Pointer<TRC, AllTag>(m_dragToIndex)->GetLeftTop();
		}

		//Get Right/Bottom Line
		coordinates_type cooyou0 = pSheet->FirstPointer<TRCYou, AllTag>()->GetLeftTop();
		coordinates_type cooyou1 = pSheet->ZeroPointer<TRCYou, AllTag>()->GetRightBottom();

		//Paint
		CPen pen(pSheet->GetHeaderProperty()->GetPenPtr()->GetPenStyle(),
				 pSheet->GetHeaderProperty()->GetPenPtr()->GetWidth(),
				 CColor(RGB(255, 0, 0)));
		HPEN hPen = e.DCPtr->SelectPen(pen);
		PaintLine(e.DCPtr, coome, cooyou0, cooyou1);
		e.DCPtr->SelectPen(hPen);
	}

	void PaintLine(CDC* pDC, coordinates_type coome, coordinates_type cooyou0, coordinates_type cooyou1)
	{

	}

};

template <>
void CDragger<RowTag, ColTag>::PaintLine(CDC* pDC, coordinates_type coome, coordinates_type cooyou0, coordinates_type cooyou1)
{
	pDC->MoveToEx(cooyou0, coome);
	pDC->LineTo(cooyou1, coome);
}

template <>
void CDragger<ColTag, RowTag>::PaintLine(CDC* pDC, coordinates_type coome, coordinates_type cooyou0, coordinates_type cooyou1)
{
	pDC->MoveToEx(coome, cooyou0);
	pDC->LineTo(coome, cooyou1);
}


typedef CDragger<RowTag, ColTag> CRowDragger;
typedef CDragger<ColTag, RowTag> CColDragger;

template<typename TRC, typename TRCYou>
class CSheetCellDragger:public CDragger<TRC, TRCYou>
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
