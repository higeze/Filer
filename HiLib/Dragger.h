#pragma once
#include "IDragger.h"
#include "Sheet.h"
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
		auto visIndexes = pSheet->Point2Indexes<VisTag>(e.Direct.Pixels2Dips(e.Point));
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

	void OnBeginDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		m_dragFromIndex = pSheet->Point2Indexes<AllTag>(e.Direct.Pixels2Dips(e.Point)).Get<TRC>();
		m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		auto visibleIndex = pSheet->Point2Indexes<VisTag>(e.Direct.Pixels2Dips(e.Point)).Get<TRC>();

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

		m_dragToIndex = (std::max)(m_dragToIndex, 0);
	}

	void OnEndDrag(CSheet* pSheet, const MouseEvent& e) override
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
		FLOAT cooyou0 = GetLineLeftTop(pSheet);
		FLOAT cooyou1 = GetLineRightBottom(pSheet);

		//Paint DragLine
		PaintLine(e.Direct, *(pSheet->GetHeaderProperty()->DragLine), coome, cooyou0, cooyou1);
	}

	virtual int GetLineLeftTop(CSheet* pSheet) { return pSheet->FirstPointer<TRCYou, AllTag>()->GetLeftTop(); }
	virtual int GetLineRightBottom(CSheet* pSheet) { return pSheet->ZeroPointer<TRCYou, AllTag>()->GetRightBottom(); }

	void PaintLine(d2dw::CDirect2DWrite& direct, d2dw::SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1) {}

};

template <>
void CDragger<RowTag, ColTag>::PaintLine(d2dw::CDirect2DWrite& direct, d2dw::SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1)
{
	direct.DrawSolidLine(line, d2dw::CPointF(cooyou0, coome), d2dw::CPointF(cooyou1, coome));
}

template <>
void CDragger<ColTag, RowTag>::PaintLine(d2dw::CDirect2DWrite& direct, d2dw::SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1)
{
	direct.DrawSolidLine(line, d2dw::CPointF(coome, cooyou0), d2dw::CPointF(coome, cooyou1));
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
