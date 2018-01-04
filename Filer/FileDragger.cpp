#include "FileDragger.h"
#include "Sheet.h"
#include "SheetState.h"
#include "Row.h"
#include "Column.h"
#include "FilerGridView.h"
#include "FileDraggingState.h"

void CFileDragger::OnBeginDrag(CSheet* pSheet, MouseEventArgs const & e)
{
	m_ptDragStart = e.Point;
}

void CFileDragger::OnDrag(CSheet* pSheet, MouseEventArgs const & e)
{
	if(e.Flags==MK_LBUTTON){
		auto distance = std::pow(m_ptDragStart.x-e.Point.x , 2) + std::pow(m_ptDragStart.y-e.Point.y, 2);
		if(distance >100){
			if(auto p = dynamic_cast<CFilerGridView*>(pSheet)){
				p->Drag();
			}
		}

	}else{
	}
}

void CFileDragger::OnEndDrag(CSheet* pSheet, MouseEventArgs const & e)
{
	//TODO
}

bool CFileDragger::IsTarget(CSheet* pSheet, MouseEventArgs const & e)
{
	auto visIndexes = pSheet->Coordinates2Indexes<VisTag>(e.Point);
	auto& rowDictionary = pSheet->m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary = pSheet->m_columnVisibleDictionary.get<IndexTag>();

	auto maxRow = pSheet->GetMaxIndex<RowTag, VisTag>();
	auto minRow = pSheet->GetMinIndex<RowTag, VisTag>();
	auto maxCol = pSheet->GetMaxIndex<ColTag, VisTag>();
	auto minCol = pSheet->GetMaxIndex<ColTag, VisTag>();

	auto spRow = pSheet->Index2Pointer<RowTag, VisTag>(visIndexes.first);
	if( visIndexes.first < 0 || 
		visIndexes.first < minRow || visIndexes.first > maxRow || 
		visIndexes.second < minCol || visIndexes.second > maxCol){
		return false;
	}else if(spRow && !spRow->IsDragTrackable()){
		return true;
	}else{
		return false;
	}
}

CFileDragger::size_type CFileDragger::GetDragToAllIndex() { return CBand::kInvalidIndex; }
CFileDragger::size_type CFileDragger::GetDragFromAllIndex() { return CBand::kInvalidIndex; }