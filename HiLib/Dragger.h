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
	int m_dragFromVisIndex = CBand::kInvalidIndex;
	int m_dragToVisIndex = CBand::kInvalidIndex;
	//int m_dragFromIndex;
	//int m_dragToIndex;
public:
	CDragger() {}// :m_dragFromIndex(CBand::kInvalidIndex), m_dragToIndex(CBand::kInvalidIndex) {}
	virtual ~CDragger() {}

	//int GetDragToAllIndex() { return m_dragToIndex; }
	//int GetDragFromAllIndex() { return m_dragFromIndex; }

	virtual bool IsTarget(CSheet* pSheet, const MouseEvent& e) override
	{
		auto visIndexes = pSheet->Point2Indexes(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
		auto v = visIndexes.Get<TRCYou::IndexesTag>();
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
		m_dragFromVisIndex = pSheet->Point2Index<TRC>(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
		if (static_cast<size_t>(m_dragFromVisIndex) >= pSheet->GetContainer<TRC, VisTag>().size()) {
			m_dragFromVisIndex = CBand::kInvalidIndex;
		}
		m_dragToVisIndex = CBand::kInvalidIndex;
		//m_dragFromIndex = pSheet->Vis2AllIndex<TRC>(pSheet->Point2Index<TRC>(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.Point)));
		//m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		auto curVisIndex = pSheet->Point2Index<TRC>(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));

		if (curVisIndex == CBand::kInvalidIndex) {
			m_dragToVisIndex = CBand::kInvalidIndex;
		} else {
			int dragToVisIndex = std::clamp(curVisIndex,
				(int)(pSheet->GetFrozenCount<TRC>()),
				(int)(pSheet->GetContainer<TRC, VisTag>().size() - 1));
			//int allDragToIndex = pSheet->Vis2AllIndex<TRC>(visDragToIndex);
			if (dragToVisIndex < m_dragFromVisIndex) {
				m_dragToVisIndex = dragToVisIndex;
			} else if (dragToVisIndex > m_dragFromVisIndex) {
				m_dragToVisIndex = dragToVisIndex + 1;
			} else {//m_dragToIndex == m_dragFromIndex
				m_dragToVisIndex = dragToVisIndex;
			}
		}



		//int visIndex = pSheet->Point2Index<TRC>(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.Point));

		//if (visIndex == CBand::kInvalidIndex) {
		//	m_dragToIndex = pSheet->Vis2AllIndex<TRC>(visIndex);
		//} else {
		//	int visDragToIndex = std::clamp(visIndex,
		//		(int)(pSheet->GetFrozenCount<TRC>()),
		//		(int)(pSheet->GetContainer<TRC, VisTag>().size() - 1));
		//	int allDragToIndex = pSheet->Vis2AllIndex<TRC>(visDragToIndex);
		//	if (allDragToIndex < m_dragFromIndex) {
		//		m_dragToIndex = allDragToIndex;
		//	} else if (allDragToIndex > m_dragFromIndex) {
		//		m_dragToIndex = allDragToIndex + 1;
		//	} else {//m_dragToIndex == m_dragFromIndex
		//		m_dragToIndex = allDragToIndex;
		//	}
		//}
	}

	void OnEndDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		//Drag to from check
		if (m_dragToVisIndex == CBand::kInvalidIndex || m_dragFromVisIndex == CBand::kInvalidIndex || m_dragToVisIndex == m_dragFromVisIndex) {
			//Do nothing
		} else {
			auto sp = pSheet->Index2Pointer<TRC, VisTag>(m_dragFromVisIndex);
			pSheet->Move<TRC>(m_dragToVisIndex, sp);
		}
		m_dragFromVisIndex = CBand::kInvalidIndex;
		m_dragToVisIndex = CBand::kInvalidIndex;

		//if (m_dragToIndex == CBand::kInvalidIndex) {

		//}
		//else {
		//	if (m_dragToIndex >= 0 && m_dragToIndex != m_dragFromIndex) {//TODO Only when Row
		//		auto sp = pSheet->Index2Pointer<TRC, AllTag>(m_dragFromIndex);
		//		pSheet->Move<TRC>(m_dragToIndex, sp);
		//	}
		//}
		//m_dragFromIndex = CBand::kInvalidIndex;
		//m_dragToIndex = CBand::kInvalidIndex;
	}

	void OnLeaveDrag(CSheet* pSheet, const MouseEvent& e) override
	{
		m_dragFromVisIndex = CBand::kInvalidIndex;
		m_dragToVisIndex = CBand::kInvalidIndex;

		//m_dragFromIndex = CBand::kInvalidIndex;
		//m_dragToIndex = CBand::kInvalidIndex;
	}

	virtual void OnPaintDragLine(CSheet* pSheet, const PaintEvent& e)
	{
		//Drag to from check
		if (m_dragFromVisIndex == CBand::kInvalidIndex ||
			m_dragToVisIndex == CBand::kInvalidIndex ||
			m_dragFromVisIndex == m_dragToVisIndex) {
			return;
		} else {

			//Get Left/Top Line
			//o/max : Right/Bottom
			//u/min : Left/Top
			//other : Left
			FLOAT coome = 0;
			if (m_dragToVisIndex >= (int)pSheet->GetContainer<TRC, VisTag>().size()) {
				coome = pSheet->GetContainer<TRC, VisTag>().back()->GetEnd();
			} else if (m_dragToVisIndex <= pSheet->GetFrozenCount<TRC>()) {
				coome = pSheet->GetContainer<TRC, VisTag>()[pSheet->GetFrozenCount<TRC>()]->GetStart();
			} else {
				coome = pSheet->Index2Pointer<TRC, VisTag>(m_dragToVisIndex)->GetStart();
			}

			//Get Right/Bottom Line
			FLOAT cooyou0 = pSheet->GetContainer<TRCYou, VisTag>().front()->GetStart();
			FLOAT cooyou1 = pSheet->GetContainer<TRCYou, VisTag>().back()->GetEnd();

			//Paint DragLine
			PaintLine(pSheet->GetWndPtr()->GetDirectPtr(), *(pSheet->GetHeaderProperty()->DragLine), coome, cooyou0, cooyou1);
		}


		////Drag to from check
		//if (m_dragFromIndex == CBand::kInvalidIndex ||
		//	m_dragToIndex == CBand::kInvalidIndex ||
		//	m_dragFromIndex == m_dragToIndex) {
		//	return;
		//} else {

		//	//Get Left/Top Line
		//	//o/max : Right/Bottom
		//	//u/min : Left/Top
		//	//other : Left
		//	FLOAT coome = 0;
		//	if (m_dragToIndex >= (int)pSheet->GetContainer<TRC, AllTag>().size()) {
		//		coome = pSheet->GetContainer<TRC, AllTag>().back()->GetEnd();
		//	} else if (m_dragToIndex <= 0) {
		//		coome = pSheet->GetContainer<TRC, AllTag>()[pSheet->GetFrozenCount<TRC>()]->GetStart();
		//	} else {
		//		coome = pSheet->Index2Pointer<TRC, AllTag>(m_dragToIndex)->GetStart();
		//	}

		//	//Get Right/Bottom Line
		//	FLOAT cooyou0 = pSheet->GetContainer<TRCYou, AllTag>().front()->GetStart();
		//	FLOAT cooyou1 = pSheet->GetContainer<TRCYou, AllTag>().back()->GetEnd();

		//	//Paint DragLine
		//	PaintLine(pSheet->GetWndPtr()->GetDirectPtr(), *(pSheet->GetHeaderProperty()->DragLine), coome, cooyou0, cooyou1);
		//}
	}

	void PaintLine(CDirect2DWrite* pDirect, SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1) {}

};

template <>
inline void CDragger<RowTag, ColTag>::PaintLine(CDirect2DWrite* pDirect, SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1)
{
	pDirect->DrawSolidLine(line, CPointF(cooyou0, coome), CPointF(cooyou1, coome));
}

template <>
inline void CDragger<ColTag, RowTag>::PaintLine(CDirect2DWrite* pDirect, SolidLine& line, FLOAT coome, FLOAT cooyou0, FLOAT cooyou1)
{
	pDirect->DrawSolidLine(line, CPointF(coome, cooyou0), CPointF(coome, cooyou1));
}


typedef CDragger<RowTag, ColTag> CRowDragger;
typedef CDragger<ColTag, RowTag> CColDragger;

template<typename TRC, typename TRCYou>
class CSheetCellDragger:public CDragger<TRC, TRCYou>
{
	using base = CDragger<TRC, TRCYou>;
public:
    /*! Constructor*/
	CSheetCellDragger():base(){}
	virtual ~CSheetCellDragger(){}
	virtual bool IsTarget(CSheet* pSheet, const MouseEvent& e) override { return false; }
};

typedef CSheetCellDragger<RowTag, ColTag> CSheetCellRowDragger;
typedef CSheetCellDragger<ColTag, RowTag> CSheetCellColDragger;
