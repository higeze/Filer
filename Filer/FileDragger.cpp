#include "FileDragger.h"
#include "Sheet.h"
#include "SheetState.h"
#include "Row.h"
#include "Column.h"
#include "FilerGridView.h"
//#include "FileDraggingState.h"

void CFileDragger::OnBeginDrag(CSheet* pSheet, MouseEventArgs const & e)
{
	m_ptDragStart = e.Point;
	if (auto p = dynamic_cast<CFilerGridView*>(pSheet)) {
		p->Drag();
	}
}

void CFileDragger::OnDrag(CSheet* pSheet, MouseEventArgs const & e)
{
	//auto distance = std::pow(m_ptDragStart.x-e.Point.x , 2) + std::pow(m_ptDragStart.y-e.Point.y, 2);
	//if(distance > 100){
		//if(auto p = dynamic_cast<CFilerGridView*>(pSheet)){
		//	p->Drag();
		//}
	//}
}

void CFileDragger::OnEndDrag(CSheet* pSheet, MouseEventArgs const & e)
{
	//Cancel
}

void CFileDragger::OnLeaveDrag(CSheet* pSheet, MouseEventArgs const & e)
{
	//Reset
	m_ptDragStart.SetPoint(-1, -1);
}

bool CFileDragger::IsTarget(CSheet* pSheet, MouseEventArgs const & e)
{
	auto visIndexes = pSheet->Point2Indexes<VisTag>(e.Point);
	auto maxRow = pSheet->GetMaxIndex<RowTag, VisTag>();
	auto maxCol = pSheet->GetMaxIndex<ColTag, VisTag>();

	if (visIndexes.Row >= 0 && visIndexes.Row <= maxRow &&
		visIndexes.Col >= 0 && visIndexes.Col <= maxCol) {
		return true;
	}
	else{
		return false;
	}
}
