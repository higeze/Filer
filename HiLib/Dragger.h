#pragma once
#include "IDragger.h"
#include "Sheet.h"
#include "Band.h"
#include "CellProperty.h"
#include "MyPen.h"

//Pre-Declaration
struct MouseEvent;
struct SetCursorEvent;

template<typename TRC, typename TRCYou>
class CDragger :public IDragger
{
private:
	int m_dragFromIndex;
	int m_dragToIndex;
public:
	CDragger() :m_dragFromIndex(CBand::kInvalidIndex), m_dragToIndex(CBand::kInvalidIndex) {}
	virtual ~CDragger() {}

	int GetDragToAllIndex() { return m_dragToIndex; }
	int GetDragFromAllIndex() { return m_dragFromIndex; }

	virtual bool IsTarget(CSheet* pSheet, const MouseEvent& e) override
	{
		auto visIndexes = pSheet->Point2Indexes(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.Point));
		if (visIndexes.Row < (int)pSheet->GetContainer<RowTag, VisTag>().size() &&
			visIndexes.Row >= 0 &&
			visIndexes.Col < (int)pSheet->GetContainer<ColTag, VisTag>().size() &&
			visIndexes.Col >= 0 &&
			visIndexes.Get<TRCYou::IndexesTag>() < pSheet->GetFrozenCount<TRCYou>()) {
			return true;
		}else{
			return false;
		}
	}

	void OnBeginDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		m_dragFromIndex = pSheet->Point2Index<TRC>(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.Point));
		m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		auto visibleIndex = pSheet->Point2Index<TRC>(e.WndPtr->GetDirectPtr()->Pixels2Dips(e.Point));

		int visMax = pSheet->GetContainer<TRC, VisTag>().size() - 1;
		int allMax = pSheet->GetContainer<TRC, AllTag>().size() - 1;

		if (visibleIndex == CBand::kInvalidIndex) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visibleIndex);
		}
		else if (visibleIndex < 0) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(0);
		}
		else if (visibleIndex > visMax) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visMax) + 1;
		}
		else if (visibleIndex == 0) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(0);
		}
		else if (visibleIndex == visMax) {
			m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visMax) + 1;
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

		m_dragToIndex = (std::max)(m_dragToIndex, 0);
	}

	void OnEndDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		if (m_dragToIndex == CBand::kInvalidIndex) {

		}
		else {
			if (m_dragToIndex >= 0 && m_dragToIndex != m_dragFromIndex) {//TODO Only when Row
				auto sp = pSheet->Index2Pointer<TRC, AllTag>(m_dragFromIndex);
				pSheet->Move<TRC>(m_dragToIndex, sp);
			}
		}
		m_dragFromIndex = CBand::kInvalidIndex;
		m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnLeaveDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		m_dragFromIndex = CBand::kInvalidIndex;
		m_dragToIndex = CBand::kInvalidIndex;
	}

	virtual void OnPaintDragLine(CSheet* pSheet, const PaintEvent& e)
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
		FLOAT coome = 0;
		if (m_dragToIndex >= (int)pSheet->GetContainer<TRC, AllTag>().size()) {
			coome = pSheet->GetContainer<TRC, AllTag>().back()->GetRightBottom();
		}
		else if(m_dragToIndex <= 0) {
			coome = pSheet->GetContainer<TRC, AllTag>()[pSheet->GetFrozenCount<TRC>()]->GetLeftTop();
		}
		else {
			coome = pSheet->Index2Pointer<TRC, AllTag>(m_dragToIndex)->GetLeftTop();
		}

		//Get Right/Bottom Line
		FLOAT cooyou0 = pSheet->GetContainer<TRCYou, AllTag>().front()->GetLeftTop();
		FLOAT cooyou1 = pSheet->GetContainer<TRCYou, AllTag>().back()->GetRightBottom();

		//Paint DragLine
		PaintLine(e.WndPtr->GetDirectPtr(), *(pSheet->GetHeaderProperty()->DragLine), coome, cooyou0, cooyou1);
	}

	void PaintLine(d2dw::CDirect2DWrite* pDirect, d2dw::SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1) {}

};

template <>
void CDragger<RowTag, ColTag>::PaintLine(d2dw::CDirect2DWrite* pDirect, d2dw::SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1)
{
	pDirect->DrawSolidLine(line, d2dw::CPointF(cooyou0, coome), d2dw::CPointF(cooyou1, coome));
}

template <>
void CDragger<ColTag, RowTag>::PaintLine(d2dw::CDirect2DWrite* pDirect, d2dw::SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1)
{
	pDirect->DrawSolidLine(line, d2dw::CPointF(coome, cooyou0), d2dw::CPointF(coome, cooyou1));
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
	virtual bool IsTarget(CSheet* pSheet, const MouseEvent& e) override { return false; }
};

typedef CSheetCellDragger<RowTag, ColTag> CSheetCellRowDragger;
typedef CSheetCellDragger<ColTag, RowTag> CSheetCellColDragger;
