#include "FileDragger.h"
#include "GridView.h"
#include "SheetState.h"
#include "Row.h"
#include "Column.h"
#include "FilerGridView.h"
#include "CellProperty.h"
//#include "FileDraggingState.h"

void CFileDragger::OnBeginDrag(CGridView* pSheet, MouseEvent const & e)
{
	m_ptDragStart = e.PointInClient;
	if (auto p = dynamic_cast<CFilerGridView*>(pSheet)) {
		p->Drag();
	}
}

void CFileDragger::OnDrag(CGridView* pSheet, MouseEvent const & e)
{
	//auto distance = std::pow(m_ptDragStart.x-e.Point.x , 2) + std::pow(m_ptDragStart.y-e.Point.y, 2);
	//if(distance > 100){
		//if(auto p = dynamic_cast<CFilerGridView*>(pSheet)){
		//	p->Drag();
		//}
	//}
}

void CFileDragger::OnEndDrag(CGridView* pSheet, MouseEvent const & e)
{
	//Cancel
}

void CFileDragger::OnLeaveDrag(CGridView* pSheet, MouseEvent const & e)
{
	//Reset
	m_ptDragStart.SetPoint(-1, -1);
}

bool CFileDragger::IsTarget(CGridView* pSheet, MouseEvent const & e)
{
	auto visIndexes = pSheet->Point2Indexes(pSheet->GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	int maxRow = pSheet->GetContainer<RowTag, VisTag>().size() - 1;
	int maxCol = pSheet->GetContainer<ColTag, VisTag>().size() - 1;

	if (visIndexes.Row >= 0 && visIndexes.Row <= maxRow &&
		visIndexes.Col >= 0 && visIndexes.Col <= maxCol) {
		return true;
	}
	else{
		return false;
	}
}
